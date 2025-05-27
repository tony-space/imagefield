#include <imf/core/RuntimeFactory.hpp>
#include <imf/core/log.hpp>

#include <map>
#include <stdexcept>

namespace imf::core
{

using runtime_instantiators_map_t = std::map<std::string_view, runtime_instantiator_t>;

static runtime_instantiators_map_t register_standard_runtimes()
{
	runtime_instantiators_map_t map;

	return map;
}

static runtime_instantiators_map_t& runtime_instantiators()
{
	static runtime_instantiators_map_t map(register_standard_runtimes());

	return map;
}

std::shared_ptr<IRuntime> make_runtime(std::string_view name, const IRuntime::init_config_t& config)
{
	return runtime_instantiators().at(name)(config);
}

void register_runtime(std::string_view name, runtime_instantiator_t instantiator)
{
	auto _pair = runtime_instantiators().emplace(name, std::move(instantiator));
	if (!_pair.second)
	{
		log::err("runtime") << "runtime node '" << name << "' already registered";

		throw std::invalid_argument("runtime node already registered");
	}
}

}
