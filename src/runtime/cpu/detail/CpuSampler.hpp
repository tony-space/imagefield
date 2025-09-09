#pragma once

#include <imf/core/Image.hpp>
#include <imf/core/pixel_transform.hpp>
#include <imf/core/SamplerDesc.hpp>

namespace imf::runtime::cpu
{

class CpuRuntime;
class CpuTexture;

class CpuSampler
{
public:
	explicit CpuSampler(CpuRuntime& runtime, const core::Image& img, const core::SamplerDesc& desc = {});

	glm::mat4 sample(const glm::mat4x2& worldQuad) const noexcept;
private:
	using wrapFunc = unsigned (*)(long coord, unsigned size) noexcept;
	using MagSamplingFunc = glm::vec4(*) (const CpuSampler*, const glm::vec2& tc) noexcept;
	using SamplingFunc = glm::vec4(*) (const CpuSampler*, const glm::vec2& tc, float level) noexcept;
	struct ComponentSwizzleFunctor
	{
		core::ComponentMapping mapping;

		glm::mat4 operator() (const glm::mat4&) const noexcept;
		glm::vec4 operator() (const glm::vec4&) const noexcept;
	};

	std::shared_ptr<const CpuTexture> m_cpuTexture;
	glm::mat3 m_worldToUvMat;
	core::Image::LodSettings m_lodSettings;
	core::TransformRowFunc m_readFunc;
	wrapFunc m_wrapFuncS;
	wrapFunc m_wrapFuncT;
	MagSamplingFunc m_magSamplingFunc;
	SamplingFunc m_minSamplingFunc;
	ComponentSwizzleFunctor m_swizzleFunc;


	glm::mat4 sample(glm::mat4x2 normalizedTcQuad, float lod) const noexcept;
	static glm::vec4 sampleNearest(const CpuSampler*, const glm::vec2& tc, unsigned level) noexcept;
	static glm::vec4 sampleBilinear(const CpuSampler*, const glm::vec2& tc, unsigned level) noexcept;

	static wrapFunc getWrapFunc(core::AddressMode mode) noexcept;
	static MagSamplingFunc getMagSamplingFunc(core::MinMagFilter filter) noexcept;
	static SamplingFunc getMinSamplingFunc(core::MinMagFilter minFilter, core::MipFilter mipFilter) noexcept;
};

}
