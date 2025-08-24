#include <imf/core/node/PlaceholderNode.hpp>

namespace imf::core
{

PlaceholderNode::PlaceholderNode(TypeQualifier qualifier, TypeID typeId) :
	m_output(*this, std::move(typeId)),
	m_qualifier(qualifier)
{

}

std::string_view PlaceholderNode::operationName() const noexcept
{
	return operation_name;
}

iterator_range<const std::string_view*> PlaceholderNode::inputNames() const noexcept
{
	return { nullptr, nullptr };
}

iterator_range<const TypeID*> PlaceholderNode::inputTypes() const noexcept
{
	return { nullptr, nullptr };
}

iterator_range<const std::shared_ptr<const DataFlow>*> PlaceholderNode::inputs() const noexcept
{
	return { nullptr, nullptr };
}

iterator_range<const std::string_view*> PlaceholderNode::outputNames() const noexcept
{
	return { nullptr, nullptr };
}

iterator_range<const TypeID*> PlaceholderNode::outputTypes() const noexcept
{
	const TypeID* outputType = &m_output.dataType();

	return { outputType, outputType + 1 };
}

iterator_range<const DataFlow*> PlaceholderNode::outputs() const noexcept
{
	return { &m_output, &m_output + 1 };
}

void PlaceholderNode::setInput(const std::string_view&, const DataFlow&)
{
	throw std::runtime_error("Placeholders cannot contain inputs");
}

}
