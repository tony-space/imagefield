#pragma once

#include <imf/core/IRuntime.hpp>

#include <filesystem>

namespace imf::tests
{

const std::filesystem::path& project_root();

const std::filesystem::path& project_assets();

const std::filesystem::path& build_dir();

const std::filesystem::path& output_dir();

}
