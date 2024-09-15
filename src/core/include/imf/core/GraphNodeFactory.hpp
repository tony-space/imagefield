#pragma once

#include <memory>
#include <functional>
#include <string_view>


namespace imf::core
{

class GraphNode;

using graph_node_instantiator_t = std::function<std::shared_ptr<GraphNode>()>;

std::shared_ptr<GraphNode> make_graph_node(std::string_view name);
void register_graph_node(std::string_view name, graph_node_instantiator_t instantiator);

}
