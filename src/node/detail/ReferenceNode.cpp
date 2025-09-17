#include <imf/core/node/ReferenceNode.hpp>

namespace imf::core
{

ReferenceNode::ReferenceNode(const DataFlow& flow) : m_input(flow.sharedPtr()), m_output(*this, flow.dataType())
{

}

ReferenceNode::ReferenceNode(const TypeID& type) : m_output(*this, type)
{

}

std::string_view ReferenceNode::operationName() const noexcept
{
	return operation_name;
}

iterator_range<const std::string_view*> ReferenceNode::inputNames() const noexcept
{
	return { std::begin(input_names), std::end(input_names) };
}

iterator_range<const TypeID*> ReferenceNode::inputTypes() const noexcept
{
	return iterator_range<const TypeID*>();
}

iterator_range<const std::shared_ptr<const DataFlow>*> ReferenceNode::inputs() const noexcept
{
	return { &m_input, &m_input + 1 };
}

iterator_range<const std::string_view*> ReferenceNode::outputNames() const noexcept
{
	return { std::begin(output_names), std::end(output_names) };
}

iterator_range<const TypeID*> ReferenceNode::outputTypes() const noexcept
{
	return iterator_range<const TypeID*>();
}

iterator_range<const DataFlow*> ReferenceNode::outputs() const noexcept
{
	return { &m_output, &m_output + 1 };
}

void ReferenceNode::setInput(const std::string_view& name, const DataFlow& flow)
{
	if (name != input_names[0])
	{
		throw std::invalid_argument("invalid input parameter key");
	}
	if (flow.dataType() != m_output.dataType())
	{
		throw std::runtime_error("incorret type");
	}

	m_input = flow.sharedPtr();
}

std::shared_ptr<ReferenceNode> ReferenceNode::make(const DataFlow& flow)
{
	return std::make_shared<ReferenceNode>(flow);
}

std::shared_ptr<ReferenceNode> ReferenceNode::make(const TypeID& id)
{
	return std::make_shared<ReferenceNode>(id);
}

}
