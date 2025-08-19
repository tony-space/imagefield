#include <imf/core/node/SinkNode.hpp>
#include <imf/core/DataFlow.hpp>

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

void SinkNode::setInput(const std::string_view&, const DataFlow& flow)
{
	m_input = flow.sharedPtr();
}

std::shared_ptr<SinkNode> SinkNode::make(const DataFlow& flow)
{
	return std::make_shared<SinkNode>(flow);
}

}
