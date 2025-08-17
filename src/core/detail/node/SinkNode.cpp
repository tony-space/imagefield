#include <imf/core/node/SinkNode.hpp>
#include <imf/core/DataFlow.hpp>

#include <boost/container_hash/hash.hpp>

namespace imf::core
{

SinkNode::SinkNode(const DataFlow& flow) : m_input(flow.sharedPtr())
{
}

std::string_view SinkNode::operationName() const noexcept
{
	return operation_name;
}

iterator_range<const std::string_view*> SinkNode::inputNames() const noexcept
{
	return { nullptr, nullptr };
}

iterator_range<const TypeID*> SinkNode::inputTypes() const noexcept
{
	const auto& type = m_input->dataType();
	return { &type, &type + 1 };
}

iterator_range<const std::shared_ptr<const DataFlow>*> SinkNode::inputs() const noexcept
{
	return { &m_input, &m_input + 1 };
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
	GraphNode::hast_t result = 0;

	boost::hash_combine(result, std::hash<std::string_view>{}(operation_name));
	boost::hash_combine(result, m_input->producer()->hash());

	return result;
}

void SinkNode::setInput(const std::string_view&, const DataFlow& flow)
{
	m_input = flow.sharedPtr();
}

std::shared_ptr<SinkNode> SinkNode::make(const DataFlow& flow)
{
	return std::make_shared<SinkNode>(flow);
}

}
