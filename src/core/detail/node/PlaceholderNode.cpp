#include <imf/core/node/PlaceholderNode.hpp>

#include <boost/container_hash/hash.hpp>

namespace imf::core
{

PlaceholderNode::PlaceholderNode(TypeQualifier qualifier, TypeID typeId) :
	m_output(this, std::move(typeId)),
	m_value_hash(std::hash<unique_id_t>{}(make_unique_id())),
	m_qualifier(qualifier)
{

}

std::string_view PlaceholderNode::operationName() const noexcept
{
	return operation_type;
}

iterator_range<const std::string_view*> PlaceholderNode::inputNames() const noexcept
{
	return { nullptr, nullptr };
}

iterator_range<const TypeID*> PlaceholderNode::inputTypes() const noexcept
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

GraphNode::hast_t PlaceholderNode::hash() const noexcept
{
	if (!m_hash)
	{
		GraphNode::hast_t result = 0;

		boost::hash_combine(result, std::hash<std::string_view>{}(operation_type));
		boost::hash_combine(result, std::hash<TypeID>{}(m_output.dataType()));
		boost::hash_combine(result, m_value_hash);

		m_hash = result;
	}

	return *m_hash;
}

void PlaceholderNode::setInput(const std::string_view&, const DataFlow&)
{
	throw std::runtime_error("Placeholders cannot contain inputs");
}

}
