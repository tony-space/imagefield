#pragma once

#include <imf/core/IRuntime.hpp>

namespace imf::runtime::cpu
{

class CpuRuntime : public core::IRuntime
{
public:
	CpuRuntime(const core::IRuntime::init_config_t&);
	
	std::vector<std::uint8_t> fetchContent(const std::filesystem::path& path);

	std::string_view platform() const noexcept override { return "cpu"; }
	std::shared_ptr<core::IGraphCompiler> compiler() override;
	core::Image loadImage(const std::filesystem::path& path) override;
};

}
