#include <imf/core/core.hpp>

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

		auto placeholder = PlaceholderNode::make(TypeQualifier::Constant, 3.14f);
		auto flow = placeholder->outputs().front().sharedPtr();
	}
	catch (const std::exception& ex)
	{
		imf::core::log::err("uncaught exception") << ex.what();
	}
	
}
