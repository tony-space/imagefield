#include "CpuSampler.hpp"
#include <imf/runtime/cpu/CpuRuntime.hpp>

namespace imf::runtime::cpu
{

CpuSampler::CpuSampler(CpuRuntime& runtime, const core::Image& img, const core::SamplerDesc& desc)
{
	if (desc.mipFilter != core::MipFilter::Disabled)
	{
		const auto cpuTex = std::static_pointer_cast<const CpuTexture>(img.texture());
		std::const_pointer_cast<CpuTexture>(cpuTex)->generateMipMaps(runtime.threadPool());
	}
}

}
