#pragma once

#include <imf/runtime/cpu/CpuTexture.hpp>

#include <imf/core/Image.hpp>
#include <imf/core/SamplerDesc.hpp>

namespace imf::runtime::cpu
{

class CpuRuntime;

class CpuSampler
{
public:
	explicit CpuSampler(CpuRuntime& runtime, const core::Image& img, const core::SamplerDesc& desc);
private:
	
};

}
