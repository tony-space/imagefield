#include <imf/runtime/cpu/CpuRuntime.hpp>
#include <imf/core/Image.hpp>
#include <imf/core/RuntimeFactory.hpp>

namespace imf::core
{

runtime_instantiator_t make_cpu_runtime = [](const IRuntime::init_config_t& config)
{
	return std::make_shared<runtime::cpu::CpuRuntime>(config);
};

}

namespace imf::runtime::cpu
{

CpuRuntime::CpuRuntime(const core::IRuntime::init_config_t&)
{

}

core::Image CpuRuntime::loadImage(const std::filesystem::path& /*path*/)
{
	return core::Image(nullptr, {});
}

}
