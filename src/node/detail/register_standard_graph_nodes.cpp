#include <imf/core/GraphNodeFactory.hpp>

#include <map>

#define RegisterType(TYPE)\
std::shared_ptr<imf::core::GraphNode> make_##TYPE(void);\
std::string_view operation_name_##TYPE(void) noexcept;\
map[operation_name_##TYPE()] = &(make_##TYPE)

static std::map<std::string_view, imf::core::graph_node_instantiator_t> register_standard_graph_nodes_impl()
{
	std::map<std::string_view, imf::core::graph_node_instantiator_t> map;

	RegisterType(ColorSpaceConvertNode);
	RegisterType(TransformNode);

	return map;
}

namespace imf::core
{

std::map<std::string_view, graph_node_instantiator_t> register_standard_graph_nodes()
{
	return register_standard_graph_nodes_impl();
}

}
