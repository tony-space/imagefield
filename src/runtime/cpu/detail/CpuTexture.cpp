#include <imf/runtime/cpu/CpuTexture.hpp>

#include <imf/core/ThreadPool.hpp>

namespace imf::runtime::cpu
{

CpuTexture::CpuTexture(const core::TextureData& hostData, core::TextureFormat deviceFormat) :
	CpuTexture(hostData.dim, deviceFormat)
{
	m_mipMaps.front().storage = std::make_unique<std::uint8_t[]>(m_mipMaps.front().size.volumeByteSize);

	core::convert_pixels
	(
		hostData,
		deviceFormat,
		kRowAlignment,
		kPlaneAlignment,
		m_mipMaps.front().storage.get(),
		m_mipMaps.front().size.volumeByteSize
	);
}

CpuTexture::CpuTexture(glm::uvec3 dim, core::TextureFormat deviceFormat)
{
	const auto maxDim = (glm::max)(dim.x, dim.y, dim.z);
	const auto totalLevels = glm::log2(glm::ceilPowerOfTwo(maxDim + 1));
	m_mipMaps.reserve(totalLevels);
	m_format = deviceFormat;

	const auto imageSize = core::calc_image_size(deviceFormat, dim, kRowAlignment, kPlaneAlignment);

	m_mipMaps.emplace_back(MipLevel
	{
		nullptr,
		dim,
		imageSize
	});
}

CpuTexture::CpuTexture(glm::uvec2 dim, core::TextureFormat deviceFormat) :
	CpuTexture(glm::uvec3(dim, 1u), deviceFormat)
{
}

glm::uvec3 CpuTexture::dim() const noexcept
{
	return m_mipMaps.front().dim;
}

core::TextureFormat CpuTexture::format() const noexcept
{
	return m_format;
}

const core::TextureData CpuTexture::readMap(unsigned level) const
{
	assert(level < m_mipMaps.size());

	core::TextureData result;

	result.data = m_mipMaps.at(level).storage.get();
	result.dim = m_mipMaps.at(level).dim;
	result.format = m_format;
	result.rowAlignment = kRowAlignment;
	result.planeAlignment = kPlaneAlignment;

	return result;
}

void CpuTexture::unmap() const noexcept
{
}

void CpuTexture::generateMipMaps(core::ThreadPool& pool)
{
	for (std::size_t i = m_mipMaps.size(), len = m_mipMaps.capacity(); i < len; ++i)
	{
		m_mipMaps.emplace_back(downsample(pool, m_mipMaps.back()));
	}

	assert(m_mipMaps.back().dim.x == 1 && m_mipMaps.back().dim.y == 1 && m_mipMaps.back().dim.z == 1);
}

CpuTexture::MipLevel CpuTexture::downsample(core::ThreadPool& pool, const MipLevel& inLevel) const
{
	//3d volumes not supported yet
	assert(inLevel.dim.z == 1);

	const auto resultDim = glm::uvec3
	(
		(std::max)(inLevel.dim.x, 2u) / 2u,
		(std::max)(inLevel.dim.y, 2u) / 2u,
		(std::max)(inLevel.dim.z, 2u) / 2u
	);

	const auto readFunc = core::get_convert_func(m_format, core::TextureFormat::RGBA32F);
	const auto writeFunc = core::get_convert_func(core::TextureFormat::RGBA32F, m_format);

	const auto resultSize = core::calc_image_size(m_format, resultDim, kRowAlignment, kPlaneAlignment);
	auto resultStorage = std::make_unique<uint8_t[]>(resultSize.volumeByteSize);

	pool.forEachSync([&](unsigned from, unsigned to)
	{
		std::vector<glm::vec4> rowBuffer(inLevel.dim.x * 2);

		for (unsigned outY = from; outY < to; ++outY)
		{
			auto inY = outY * 2;
			auto inRowPtr1 = inLevel.storage.get() + inY * inLevel.size.rowByteSize;
			auto inRowPtr2 = inLevel.storage.get() + (inY + 1) * inLevel.size.rowByteSize;
			auto outRowPtr = resultStorage.get() + outY * resultSize.rowByteSize;

			readFunc(inRowPtr1, rowBuffer.data(), inLevel.dim.x);

			if (inY + 1 != inLevel.dim.y)
			{
				readFunc(inRowPtr2, rowBuffer.data() + inLevel.dim.x, inLevel.dim.x);
			}
			else
			{
				std::copy(rowBuffer.begin(), rowBuffer.begin() + inLevel.dim.x, rowBuffer.begin() + inLevel.dim.x);
			}

			for (unsigned x = 0; x < resultDim.x; ++x)
			{
				auto rightmost = x * 2;
				if (rightmost < inLevel.dim.x)
				{
					rightmost++;
				}

				// Regular box filter works fine for downsampling unless alpha is involved.
				// If there is a completely transparent pixel, it should not contribute to the average color.
				// For example if there is a black transparent pixel it will darken the average color,
				// which is not the desired effect.
				const auto a = rowBuffer[x * 2];
				const auto b = rowBuffer[rightmost];
				const auto c = rowBuffer[inLevel.dim.x + x * 2];
				const auto d = rowBuffer[inLevel.dim.x + rightmost];

				auto avg = glm::vec4(0.0f);

				const auto alphaSum = a.a + b.a + c.a + d.a;
				if (alphaSum > 0.0f)
				{
					avg.r = (a.r * a.a + b.r * b.a + c.r * c.a + d.r * d.a) / alphaSum;
					avg.g = (a.g * a.a + b.g * b.a + c.g * c.a + d.g * d.a) / alphaSum;
					avg.b = (a.b * a.a + b.b * b.a + c.b * c.a + d.b * d.a) / alphaSum;
					avg.a = alphaSum * 0.25f;
				}
				
				rowBuffer[x] = avg;
			}

			writeFunc(rowBuffer.data(), outRowPtr, resultDim.x);
		}
	}, resultDim.y);

	return
	{
		std::move(resultStorage),
		resultDim,
		resultSize
	};
}

const CpuTexture::MipLevel& CpuTexture::at(unsigned level) const
{
	const auto& l = m_mipMaps.at(level);
	if (!l.storage)
	{
		const_cast<MipLevel&>(l).storage = std::make_unique<std::uint8_t[]>(l.size.volumeByteSize);
	}
	return l;
}

CpuTexture::MipLevel& CpuTexture::getMsaaLevel()
{
	if (!m_msaa4x.storage)
	{
		m_msaa4x.dim = glm::uvec3(m_mipMaps.front().dim.xy() * 2u, 1u);
		m_msaa4x.size = core::calc_image_size(m_format, m_msaa4x.dim, kRowAlignment, kPlaneAlignment);
		m_msaa4x.storage = std::make_unique<std::uint8_t[]>(m_msaa4x.size.volumeByteSize);
	}
	return m_msaa4x;
}

void CpuTexture::msaaResolve(core::ThreadPool& pool)
{
	if (!m_msaa4x.storage)
	{
		return;
	}

	m_mipMaps.clear();
	m_mipMaps.emplace_back(downsample(pool, m_msaa4x));

	m_msaa4x = MipLevel();
}

}
