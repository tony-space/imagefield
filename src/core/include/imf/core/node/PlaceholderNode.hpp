#pragma once

#include <imf/core/DataFlow.hpp>
#include <imf/core/GraphNode.hpp>
#include <imf/core/TypeQualifier.hpp>
#include <imf/core/TypeID.hpp>

#include <any>

namespace imf::core
{

class PlaceholderNode : public GraphNode
{
public:
	constexpr static std::string_view operation_type = "Placeholder";

	template<typename T> PlaceholderNode(TypeQualifier qualifier, T&& value);
	PlaceholderNode(TypeQualifier qualifier, TypeID typeId);
	PlaceholderNode(const PlaceholderNode&) = delete;
	PlaceholderNode(PlaceholderNode&&) noexcept = delete;
	PlaceholderNode& operator=(const PlaceholderNode&) = delete;
	PlaceholderNode& operator=(PlaceholderNode&&) noexcept = delete;

	std::string_view operationName() const noexcept override final;
	iterator_range<const std::string_view*> inputNames() const noexcept override final;
	iterator_range<const TypeID*> inputTypes() const noexcept override final;
	iterator_range<const std::string_view*> outputNames() const noexcept override final;
	iterator_range<const TypeID*> outputTypes() const noexcept override final;
	iterator_range<const DataFlow*> outputs() const noexcept override final;
	hast_t hash() const noexcept override final;


	template<typename... Args>
	static std::shared_ptr<PlaceholderNode> make_constant(Args&& ...args)
	{
		return std::make_shared<PlaceholderNode>(TypeQualifier::Constant, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static std::shared_ptr<PlaceholderNode> make_variable(Args&& ...args)
	{
		return std::make_shared<PlaceholderNode>(TypeQualifier::Variable, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static std::shared_ptr<PlaceholderNode> make(Args&& ...args)
	{
		return std::make_shared<PlaceholderNode>(std::forward<Args>(args)...);
	}
private:
	DataFlow m_output;
	std::any m_value;
	TypeQualifier m_qualifier;
};

template<typename T>
PlaceholderNode::PlaceholderNode(TypeQualifier qualifier, T&& value) :
	m_output(this, TypeID::make<std::decay_t<T>>()),
	m_value(std::forward<T>(value)),
	m_qualifier(qualifier)
{
}

}
