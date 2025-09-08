#pragma once

#include <imf/core/IRuntime.hpp>
#include <imf/core/ITexture.hpp>
#include <imf/core/ThreadPool.hpp>

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
	core::Image blit(const core::Image& image) override;

	void setWorkingFormat(core::TextureFormat format) override { m_workingFormat = format; }
	core::TextureFormat workingFormat() const noexcept override { return m_workingFormat; }

	core::ThreadPool& threadPool() { return m_threadPool; }
private:
	core::ThreadPool m_threadPool;
	core::TextureFormat m_workingFormat{ core::TextureFormat::RGBA32F };
};

}
