#include <imf/core/core.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(basics)

BOOST_AUTO_TEST_CASE(GraphNodeFactoryRegistration)
{
	using namespace imf::core;

	register_graph_node("FakeGraphNode", []() { return std::shared_ptr<GraphNode>{}; });

	BOOST_REQUIRE_THROW(register_graph_node("FakeGraphNode", []() { return std::shared_ptr<GraphNode>{}; }), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(RuntimeFactoryRegistration)
{
	using namespace imf::core;

	BOOST_REQUIRE_THROW(make_runtime("FakeRuntime")->platform(), std::out_of_range);

	register_runtime("FakeRuntime", [](const IRuntime::init_config_t&) { return std::shared_ptr<IRuntime>{}; });

	BOOST_REQUIRE_THROW(register_runtime("FakeRuntime", [](const IRuntime::init_config_t&) { return std::shared_ptr<IRuntime>{}; }), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(SinkNodeType)
{
	using namespace imf::core;

	auto v1 = PlaceholderNode::make_constant(42);
	auto v2 = PlaceholderNode::make_constant(42.0f);

	auto s1 = SinkNode::make(v1->outputs().front());
	auto s2 = SinkNode::make(v2->outputs().front());

	BOOST_REQUIRE(s1->inputTypes().front() == TypeID::make<int>());
	BOOST_REQUIRE(s2->inputTypes().front() == TypeID::make<float>());

	s1->setInput("", v2->outputs().front());
	BOOST_REQUIRE(s1->inputTypes().front() == TypeID::make<float>());

	s2->setInput("test", v1->outputs().front());
	BOOST_REQUIRE(s2->inputTypes().front() == TypeID::make<int>());

}

BOOST_AUTO_TEST_SUITE_END()
