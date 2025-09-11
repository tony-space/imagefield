#include "CpuSampler.hpp"
#include <imf/runtime/cpu/CpuRuntime.hpp>
#include <imf/runtime/cpu/CpuTexture.hpp>

namespace imf::runtime::cpu
{

namespace detail
{

static constexpr auto kLodEpsilon = 1.0f / 128.0f;

static glm::vec2 getDx(const glm::mat4x2& quad)
{
	return 0.5f * (quad[1] - quad[0] + quad[3] - quad[2]);
}

static glm::vec2 getDy(const glm::mat4x2& quad)
{
	return 0.5f * (quad[2] - quad[0] + quad[3] - quad[1]);
}

}

CpuSampler::CpuSampler(CpuRuntime& runtime, const core::Image& img, const core::SamplerDesc& desc)
{
	m_cpuTexture = std::dynamic_pointer_cast<const CpuTexture>(img.texture());
	assert(m_cpuTexture->dim().z == 1); // only 2D texture is supported for now

	if (desc.mipFilter != core::MipFilter::Disabled)
	{
		std::const_pointer_cast<CpuTexture>(m_cpuTexture)->generateMipMaps(runtime.threadPool());
	}

	m_worldToUvMat = img.worldToUvMat();
	m_lodSettings = img.lodSettings();

	m_readFunc = core::get_convert_func(m_cpuTexture->format(), core::TextureFormat::RGBA32F);
	m_wrapFuncS = getWrapFunc(desc.sAddressMode);
	m_wrapFuncT = getWrapFunc(desc.tAddressMode);
	m_magSamplingFunc = getMagSamplingFunc(desc.magFilter);
	m_minSamplingFunc = getMinSamplingFunc(desc.minFilter, desc.mipFilter);
	m_swizzleFunc = ComponentSwizzleFunctor{ img.componentMapping() };
}

glm::mat4x2 CpuSampler::textureCoords(const glm::mat4x2& worldQuad) const noexcept
{
	return core::projectToPlane(m_worldToUvMat, worldQuad);
}

glm::mat4 CpuSampler::sample(const glm::mat4x2& worldQuad) const noexcept
{
	return sampleByTextureCoords(textureCoords(worldQuad));
}

glm::mat4 CpuSampler::sampleByTextureCoords(const glm::mat4x2& normalizedTcQuad) const noexcept
{
	// world quad layout
	// 2 3
	// 0 1
	// see 3.8.10 Texture Minification
	// https://registry.khronos.org/OpenGL/specs/es/3.0/es_spec_3.0.pdf#subsection.3.8.10
	const auto originalDim = glm::vec2(m_cpuTexture->dim().xy());
	const auto baseLevelQuad = glm::mat4x2
	(
		normalizedTcQuad[0] * originalDim,
		normalizedTcQuad[1] * originalDim,
		normalizedTcQuad[2] * originalDim,
		normalizedTcQuad[3] * originalDim
	);

	const auto du = detail::getDx(baseLevelQuad);
	const auto dv = detail::getDy(baseLevelQuad);

	const auto jacobian = glm::mat2
	(
		glm::vec2(du.x, du.y),
		glm::vec2(dv.x, dv.y)
	);

	const auto dUlenSq = glm::dot(jacobian[0], jacobian[0]);
	const auto dVlenSq = glm::dot(jacobian[1], jacobian[1]);

	//0.5f * log2(x) == log2(sqrt(x))
	const auto isoLevelOfDetail = 0.5f * glm::log2(glm::vec2(dUlenSq, dVlenSq));
	const auto level = glm::clamp(glm::compMax(isoLevelOfDetail) + m_lodSettings.lodBias, m_lodSettings.lodMin, m_lodSettings.lodMax);

	return m_swizzleFunc(sample(normalizedTcQuad, level));
}

glm::mat4 CpuSampler::sample(glm::mat4x2 normalizedTcQuad, float lod) const noexcept
{
	if (lod < detail::kLodEpsilon)
	{
		return glm::mat4
		(
			m_magSamplingFunc(this, normalizedTcQuad[0]),
			m_magSamplingFunc(this, normalizedTcQuad[1]),
			m_magSamplingFunc(this, normalizedTcQuad[2]),
			m_magSamplingFunc(this, normalizedTcQuad[3])
		);
	}
	else
	{
		lod = glm::round(lod / detail::kLodEpsilon) * detail::kLodEpsilon;
		return glm::mat4
		(
			m_minSamplingFunc(this, normalizedTcQuad[0], lod),
			m_minSamplingFunc(this, normalizedTcQuad[1], lod),
			m_minSamplingFunc(this, normalizedTcQuad[2], lod),
			m_minSamplingFunc(this, normalizedTcQuad[3], lod)
		);
	}
}

glm::vec4 CpuSampler::sampleNearest(const CpuSampler* self, const glm::vec2& tc, unsigned level) noexcept
{
	const auto& mipLevel = (*self->m_cpuTexture)[level];
	const auto dim = mipLevel.dim.xy();
	const auto pixelCoord = glm::vec2(dim) * tc;

	const auto i = self->m_wrapFuncS(static_cast<unsigned>(glm::floor(pixelCoord.x)), dim.x);
	const auto j = self->m_wrapFuncT(static_cast<unsigned>(glm::floor(pixelCoord.y)), dim.y);

	const auto pixelOffset = mipLevel.size.rowByteSize * j + mipLevel.size.pixelByteSize * i;

	glm::vec4 color;

	self->m_readFunc(reinterpret_cast<const char*>(mipLevel.storage.get()) + pixelOffset, &color, 1);

	return color;
}

glm::vec4 CpuSampler::sampleBilinear(const CpuSampler* self, const glm::vec2& tc, unsigned level) noexcept
{
	const auto& mipLevel = (*self->m_cpuTexture)[level];
	const auto dim = mipLevel.dim.xy();
	const auto pixelCoord = glm::vec2(dim) * tc;

	const auto i0 = self->m_wrapFuncS(static_cast<unsigned>(glm::floor(pixelCoord.x - 0.5f)), dim.x);
	const auto j0 = self->m_wrapFuncT(static_cast<unsigned>(glm::floor(pixelCoord.y - 0.5f)), dim.y);
	const auto i1 = self->m_wrapFuncS(static_cast<unsigned>(glm::floor(pixelCoord.x - 0.5f)) + 1u, dim.x);
	const auto j1 = self->m_wrapFuncT(static_cast<unsigned>(glm::floor(pixelCoord.y - 0.5f)) + 1u, dim.y);

	const auto pixelOffsetSW = mipLevel.size.rowByteSize * j0 + mipLevel.size.pixelByteSize * i0;
	const auto pixelOffsetSE = mipLevel.size.rowByteSize * j0 + mipLevel.size.pixelByteSize * i1;
	const auto pixelOffsetNW = mipLevel.size.rowByteSize * j1 + mipLevel.size.pixelByteSize * i0;
	const auto pixelOffsetNE = mipLevel.size.rowByteSize * j1 + mipLevel.size.pixelByteSize * i1;

	glm::vec4 valueSW;
	glm::vec4 valueNE;
	glm::vec4 valueNW;
	glm::vec4 valueSE;

	self->m_readFunc(reinterpret_cast<const char*>(mipLevel.storage.get()) + pixelOffsetSW, &valueSW, 1);
	self->m_readFunc(reinterpret_cast<const char*>(mipLevel.storage.get()) + pixelOffsetSE, &valueSE, 1);
	self->m_readFunc(reinterpret_cast<const char*>(mipLevel.storage.get()) + pixelOffsetNW, &valueNW, 1);
	self->m_readFunc(reinterpret_cast<const char*>(mipLevel.storage.get()) + pixelOffsetNE, &valueNE, 1);

	const auto fraction = glm::fract(pixelCoord - glm::vec2(0.5f));

	return glm::mix
	(
		glm::mix(valueSW, valueSE, fraction.x),
		glm::mix(valueNW, valueNE, fraction.x),
		fraction.y
	);
}

CpuSampler::wrapFunc CpuSampler::getWrapFunc(core::AddressMode mode) noexcept
{
	switch (mode)
	{
	case core::AddressMode::Repeat:
	{
		return +[](long coord, unsigned size) noexcept
		{
			if (coord < 0)
			{
				const auto a = coord;
				const auto b = static_cast<long>(size);
				return static_cast<unsigned>(a + (b - a - 1) / b * b);
			}
			else
			{
				return static_cast<unsigned>(coord) % size;
			}
		};
	}
	case core::AddressMode::ClampToEdge:
	{
		return +[](long coord, unsigned size) noexcept
		{
			return static_cast<unsigned>(glm::clamp(coord, 0l, static_cast<long>(size) - 1));
		};
	}
	default:
		assert(false);
		std::terminate();
	}
}

CpuSampler::MagSamplingFunc CpuSampler::getMagSamplingFunc(core::MinMagFilter filter) noexcept
{
	switch (filter)
	{
	case core::MinMagFilter::Nearest:
		return +[](const CpuSampler* sampler, const glm::vec2& tc) noexcept
		{
			return sampler->sampleNearest(sampler, tc, 0u);
		};
	case core::MinMagFilter::Linear:
		return +[](const CpuSampler* sampler, const glm::vec2& tc) noexcept
		{
			return sampler->sampleBilinear(sampler, tc, 0u);
		};
	default:
		assert(false);
		std::terminate();
	}
}

CpuSampler::SamplingFunc CpuSampler::getMinSamplingFunc(core::MinMagFilter minFilter, core::MipFilter mipFilter) noexcept
{
	switch (minFilter)
	{
	case imf::core::MinMagFilter::Nearest:
		switch (mipFilter)
		{
		case imf::core::MipFilter::Disabled:
			return +[](const CpuSampler* sampler, const glm::vec2& tc, float) noexcept
			{
				return sampler->sampleNearest(sampler, tc, 0u);
			};
		case imf::core::MipFilter::Nearest:
			return +[](const CpuSampler* sampler, const glm::vec2& tc, float level) noexcept
			{
				const auto nearestLevel = static_cast<unsigned>(glm::round(level));
				return sampler->sampleNearest(sampler, tc, nearestLevel);
			};
		case imf::core::MipFilter::Linear:
			return +[](const CpuSampler* sampler, const glm::vec2& tc, float level) noexcept
			{
				const auto baseLevel = static_cast<unsigned>(glm::floor(level));
				const auto nextLevel = static_cast<unsigned>(glm::ceil(level));
				if (baseLevel == nextLevel)
				{
					return sampler->sampleNearest(sampler, tc, baseLevel);
				}
				const auto baseColor = sampler->sampleNearest(sampler, tc, baseLevel);
				const auto nextColor = sampler->sampleNearest(sampler, tc, nextLevel);
				return glm::mix(baseColor, nextColor, glm::fract(level));
			};
		}
		break;
	case imf::core::MinMagFilter::Linear:
		switch (mipFilter)
		{
		case imf::core::MipFilter::Disabled:
			return +[](const CpuSampler* sampler, const glm::vec2& tc, float) noexcept
			{
				return sampler->sampleBilinear(sampler, tc, 0u);
			};
		case imf::core::MipFilter::Nearest:
			return +[](const CpuSampler* sampler, const glm::vec2& tc, float level) noexcept
			{
				const auto nearestLevel = static_cast<unsigned>(glm::round(level));
				return sampler->sampleBilinear(sampler, tc, nearestLevel);
			};
		case imf::core::MipFilter::Linear:
			return +[](const CpuSampler* sampler, const glm::vec2& tc, float level) noexcept
			{
				const auto baseLevel = static_cast<unsigned>(glm::floor(level));
				const auto nextLevel = static_cast<unsigned>(glm::ceil(level));
				if (baseLevel == nextLevel)
				{
					return sampler->sampleBilinear(sampler, tc, baseLevel);
				}
				const auto baseColor = sampler->sampleBilinear(sampler, tc, baseLevel);
				const auto nextColor = sampler->sampleBilinear(sampler, tc, nextLevel);
				return glm::mix(baseColor, nextColor, glm::fract(level));
			};
		}
	}

	assert(false);
	std::terminate();
}

glm::mat4 CpuSampler::ComponentSwizzleFunctor::operator() (const glm::mat4& m) const noexcept
{
	return glm::mat4
	(
		(*this)(m[0]),
		(*this)(m[1]),
		(*this)(m[2]),
		(*this)(m[3])
	);
}

glm::vec4 CpuSampler::ComponentSwizzleFunctor::operator() (const glm::vec4& v) const noexcept
{
	if (mapping == core::ComponentMapping{})
	{
		return v;
	}

	glm::vec4 result(0.0f);

	if (mapping.red <= core::ComponentSwizzle::Alpha)
	{
		result[0] = v[static_cast<glm::length_t>(mapping.red)];
	}
	else if (mapping.red == core::ComponentSwizzle::One)
	{
		result[0] = 1.0f;
	}

	if (mapping.green <= core::ComponentSwizzle::Alpha)
	{
		result[1] = v[static_cast<glm::length_t>(mapping.green)];
	}
	else if (mapping.green == core::ComponentSwizzle::One)
	{
		result[1] = 1.0f;
	}

	if (mapping.blue <= core::ComponentSwizzle::Alpha)
	{
		result[2] = v[static_cast<glm::length_t>(mapping.blue)];
	}
	else if (mapping.blue == core::ComponentSwizzle::One)
	{
		result[2] = 1.0f;
	}

	if (mapping.alpha <= core::ComponentSwizzle::Alpha)
	{
		result[3] = v[static_cast<glm::length_t>(mapping.alpha)];
	}
	else if (mapping.alpha == core::ComponentSwizzle::One)
	{
		result[3] = 1.0f;
	}

	return result;
}

}
