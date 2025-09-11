#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/log.hpp>

#include <map>
#include <stdexcept>

namespace imf::core
{

static std::map<std::string_view, graph_node_instantiator_t>& graph_node_instantiators()
{
	static std::map<std::string_view, graph_node_instantiator_t> map(register_standard_graph_nodes());

	return map;
}

std::shared_ptr<GraphNode> make_graph_node(std::string_view name)
{
	auto& map = graph_node_instantiators();
	auto it = map.find(name);
	assert(it != map.end());
	if (it == map.end())
	{
		log::err("graph") << "graph node '" << name << "' not registered";
		throw std::invalid_argument("graph node not registered");
	}
	return it->second();
}

void register_graph_node(std::string_view name, graph_node_instantiator_t instantiator)
{
	auto _pair = graph_node_instantiators().emplace(name, std::move(instantiator));
	if (!_pair.second)
	{
		log::err("graph") << "graph node '" << name << "' already registered";

		throw std::invalid_argument("graph node already registered");
	}
}

}
