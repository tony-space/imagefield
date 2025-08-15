#pragma once

#include <imf/core/Image.hpp>

#include <any>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace imf::core
{

struct IRuntime : public std::enable_shared_from_this<IRuntime>
{
	using init_config_t = std::map<std::string, std::any>;

	virtual ~IRuntime() = default;
	virtual std::string_view platform() const noexcept = 0;
	[[nodiscard]] virtual core::Image loadImage(const std::filesystem::path& path) = 0;

};

}
