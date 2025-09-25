#pragma once

#include <imf/core/GraphNode.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string_view>

namespace imf::core
{

using graph_node_instantiator_t = std::function<std::shared_ptr<GraphNode>()>;
void register_graph_node(std::string_view name, graph_node_instantiator_t instantiator);

std::map<std::string_view, graph_node_instantiator_t> register_standard_graph_nodes();

#define DeclareGraphNode(TYPE)\
std::shared_ptr<imf::core::GraphNode> make_##TYPE();\
std::shared_ptr<imf::core::GraphNode> make_##TYPE()\
{\
	return std::make_shared<imf::core::TYPE>();\
}\
\
std::string_view operation_name_##TYPE() noexcept;\
std::string_view operation_name_##TYPE() noexcept\
{\
	return imf::core::TYPE::operation_name;\
}

[[nodiscard]] std::shared_ptr<GraphNode> make_graph_node(std::string_view name);

template<typename T>
[[nodiscard]] std::shared_ptr<GraphNode> make_graph_node()
{
	return std::make_shared<T>();
}

template<typename ...Args>
[[nodiscard]] std::shared_ptr<GraphNode> make_graph_node(std::string_view name, Args&& ...args)
{
	auto result = make_graph_node(name);

	assert(sizeof...(Args) <= result->inputNames().size());

	auto rangeIt = result->inputNames().begin();

	(result->setInput(*(rangeIt++), std::forward<Args>(args)), ...);

	return result;
}

}
