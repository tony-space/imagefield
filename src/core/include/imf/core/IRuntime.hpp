#pragma once

#include <imf/core/Image.hpp>
#include <imf/core/TextureFormat.hpp>

#include <any>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace imf::core
{

struct IGraphCompiler;

struct IRuntime : public std::enable_shared_from_this<IRuntime>
{
	using init_config_t = std::map<std::string, std::any>;

	virtual ~IRuntime() = default;
	virtual std::string_view platform() const noexcept = 0;
	virtual std::shared_ptr<IGraphCompiler> compiler() = 0;
	virtual core::Image loadImage(const std::filesystem::path& path) = 0;
	virtual void saveImage(core::Image image, const std::filesystem::path& path) = 0;
	virtual core::Image blit(const core::Image& image) = 0;

	virtual void setWorkingFormat(TextureFormat format) = 0;
	virtual TextureFormat workingFormat() const noexcept = 0;
};

}
