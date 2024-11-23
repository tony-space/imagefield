#include <imf/core/node/SinkNode.hpp>
#include <imf/core/DataFlow.hpp>

#include <boost/container_hash/hash.hpp>

namespace imf::core
{

SinkNode::SinkNode(std::shared_ptr<const DataFlow> flow) : m_input(std::move(flow))
{
	if (m_input == nullptr)
	{
		throw std::invalid_argument("sink requres non nullptr flow");
	}
}

std::string_view SinkNode::operationName() const noexcept
{
	return operation_type;
}

iterator_range<const std::string_view*> SinkNode::inputNames() const noexcept
{
	return { nullptr, nullptr };
}

iterator_range<const TypeID*> SinkNode::inputTypes() const noexcept
{
	return { nullptr, nullptr };
}

iterator_range<const std::string_view*> SinkNode::outputNames() const noexcept
{
	return iterator_range<const std::string_view*>();
}

iterator_range<const TypeID*> SinkNode::outputTypes() const noexcept
{
	return { nullptr, nullptr };
}

iterator_range<const DataFlow*> SinkNode::outputs() const noexcept
{
	return { nullptr, nullptr };
}

GraphNode::hast_t SinkNode::hash() const noexcept
{
	if (!m_hash)
	{
		GraphNode::hast_t result = 0;

		boost::hash_combine(result, std::hash<std::string_view>{}(operation_type));
		boost::hash_combine(result, m_input->producer()->hash());

		m_hash = result;
	}

	return *m_hash;
}

std::shared_ptr<SinkNode> SinkNode::make(std::shared_ptr<const DataFlow> flow)
{
	return std::make_shared<SinkNode>(std::move(flow));
}

}
