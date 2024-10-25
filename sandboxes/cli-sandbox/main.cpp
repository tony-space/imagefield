#include <imf/core/core.hpp>

int main()
{
	try
	{
		using namespace imf::core;

		log::info("info") << "hello info";
		log::err("err") << "hello err";

		register_graph_node("hello", []() -> std::shared_ptr<GraphNode>
		{
			return nullptr;
		});

		auto p1 = PlaceholderNode::make_variable(42.0f);
		auto p2 = PlaceholderNode::make_variable(42.0f);
		auto p3 = PlaceholderNode::make_constant(42.0f);
		auto p4 = PlaceholderNode::make_constant(42.0f);
		
		log::info("start") << p1->hash();
		log::info("start") << p2->hash();
		log::info("start") << p3->hash();
		log::info("start") << p4->hash();

		p1->setValue(1.0f);
		p2->setValue(1.0f);
		p3->setValue(1.0f);
		p4->setValue(1.0f);

		log::info("start") << p1->hash();
		log::info("start") << p2->hash();
		log::info("start") << p3->hash();
		log::info("start") << p4->hash();
	}
	catch (const std::exception& ex)
	{
		imf::core::log::err("uncaught exception") << ex.what();
	}
	
}
