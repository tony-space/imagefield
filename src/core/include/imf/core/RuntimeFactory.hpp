#pragma once

#include <imf/core/IRuntime.hpp>

#include <functional>
#include <memory>
#include <string_view>

namespace imf::core
{

using runtime_instantiator_t = std::function<std::shared_ptr<IRuntime>(const IRuntime::init_config_t& config)>;
void register_runtime(std::string_view name, runtime_instantiator_t instantiator);

[[nodiscard]] std::shared_ptr<IRuntime> make_runtime(std::string_view name, const IRuntime::init_config_t& config = {});

}
