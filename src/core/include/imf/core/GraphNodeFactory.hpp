#pragma once

#include <functional>
#include <memory>
#include <string_view>

namespace imf::core
{

class GraphNode;

using graph_node_instantiator_t = std::function<std::shared_ptr<GraphNode>()>;
void register_graph_node(std::string_view name, graph_node_instantiator_t instantiator);

[[nodiscard]] std::shared_ptr<GraphNode> make_graph_node(std::string_view name);
}
