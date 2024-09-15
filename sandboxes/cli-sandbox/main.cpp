#include <imf/core/GraphNode.hpp>
#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/log.hpp>

int main()
{
	try
	{
		using namespace imf::core;

		log::info("start") << "hello";

		register_graph_node("hello", []() -> std::shared_ptr<GraphNode>
		{
			return nullptr;
		});

		register_graph_node("hello", []() -> std::shared_ptr<GraphNode>
		{
			return nullptr;
		});
	}
	catch (const std::exception& ex)
	{
		imf::core::log::err("uncaught exception") << ex.what();
	}
	
}
