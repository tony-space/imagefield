#include <imf/core/core.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(basics)

BOOST_AUTO_TEST_CASE(GraphNodeFactoryRegistration)
{
	using namespace imf::core;

	BOOST_REQUIRE_THROW(make_graph_node("FakeGraphNode"), std::out_of_range);

	register_graph_node("FakeGraphNode", []() { return std::shared_ptr<GraphNode>{}; });

	BOOST_REQUIRE_THROW(register_graph_node("FakeGraphNode", []() { return std::shared_ptr<GraphNode>{}; }), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(PlaceholderHashing)
{
	using namespace imf::core;

	auto v1 = PlaceholderNode::make_variable(42.0f);
	auto v2 = PlaceholderNode::make_variable(42.0f);
	auto c1 = PlaceholderNode::make_constant(42.0f);
	auto c2 = PlaceholderNode::make_constant(42.0f);

	const auto v1_hash = v1->hash();
	const auto v2_hash = v2->hash();
	const auto c1_hash = c1->hash();
	const auto c2_hash = c2->hash();

	// variables must not have the same hash even if they have the same value
	BOOST_REQUIRE(v1->hash() != v2->hash());

	// constants must have the same hash when they hold the same value
	BOOST_REQUIRE(c1->hash() == c2->hash());

	v1->setValue(1.0f);
	v2->setValue(1.0f);

	c1->setValue(1.0f);
	// constants must have different hashes when they hold different values
	BOOST_REQUIRE(c1->hash() != c2->hash());
	c2->setValue(1.0f);

	// variables must not change their hash when their value changes
	BOOST_REQUIRE(v1_hash == v1->hash());
	BOOST_REQUIRE(v2_hash == v2->hash());

	// for constants hashes must change in the same way
	BOOST_REQUIRE(c1->hash() == c2->hash());
	BOOST_REQUIRE(c1_hash != c1->hash());
	BOOST_REQUIRE(c2_hash != c2->hash());
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
