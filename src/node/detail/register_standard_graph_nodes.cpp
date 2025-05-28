#include <imf/core/GraphNodeFactory.hpp>

#include <map>

namespace imf::core
{

std::map<std::string_view, graph_node_instantiator_t> register_standard_graph_nodes();
std::map<std::string_view, graph_node_instantiator_t> register_standard_graph_nodes()
{
	return {};
}

}
