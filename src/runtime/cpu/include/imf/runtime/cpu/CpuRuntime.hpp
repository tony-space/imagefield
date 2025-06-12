#pragma once

#include <imf/core/IRuntime.hpp>

namespace imf::runtime::cpu
{

class CpuRuntime : public core::IRuntime
{
public:
	CpuRuntime(const core::IRuntime::init_config_t&);
	std::string_view platform() const noexcept override { return "cpu"; }
	core::Image loadImage(const std::filesystem::path& path) override;

};

}
