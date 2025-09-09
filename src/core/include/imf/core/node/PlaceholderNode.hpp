#pragma once

#include <imf/core/DataFlow.hpp>
#include <imf/core/GraphNode.hpp>
#include <imf/core/TypeQualifier.hpp>
#include <imf/core/TypeID.hpp>

#include <any>
#include <optional>

namespace imf::core
{

class PlaceholderNode final : public GraphNode
{
public:
	constexpr static std::string_view operation_name = "Placeholder";

	template<typename T> PlaceholderNode(TypeQualifier qualifier, T&& value);
	PlaceholderNode(TypeQualifier qualifier, TypeID typeId);
	PlaceholderNode(const PlaceholderNode&) = delete;
	PlaceholderNode(PlaceholderNode&&) noexcept = delete;
	PlaceholderNode& operator=(const PlaceholderNode&) = delete;
	PlaceholderNode& operator=(PlaceholderNode&&) noexcept = delete;

	//
	// getters
	//
	std::string_view operationName() const noexcept override;
	iterator_range<const std::string_view*> inputNames() const noexcept override;
	iterator_range<const TypeID*> inputTypes() const noexcept override;
	iterator_range<const std::shared_ptr<const DataFlow>*> inputs() const noexcept override;

	iterator_range<const std::string_view*> outputNames() const noexcept override;
	iterator_range<const TypeID*> outputTypes() const noexcept override;
	iterator_range<const DataFlow*> outputs() const noexcept override;

	TypeQualifier typeQualifier() const noexcept { return m_qualifier; }
	
	//
	// setters
	//
	void setInput(const std::string_view& name, const DataFlow& flow) override;
	void setTypeQualifier(TypeQualifier qualifier) noexcept { m_qualifier = qualifier; }

	template<typename T> void setValue(T&& value);
	template<typename T> const T& value() const { return std::any_cast<const T&>(m_value); }
	template<typename T> T& value() { return std::any_cast<T&>(m_value); }
	const std::any& value() const noexcept { return m_value; }
	std::any& value() noexcept { return m_value; }
	void resetValue() noexcept { m_value.reset(); }

	template<typename... Args> static std::shared_ptr<PlaceholderNode> make_constant(Args&& ...args);
	template<typename... Args> static std::shared_ptr<PlaceholderNode> make_variable(Args&& ...args);
	template<typename... Args> static std::shared_ptr<PlaceholderNode> make(Args&& ...args);

private:
	DataFlow m_output;
	std::any m_value;
	TypeQualifier m_qualifier;
};

template<typename T>
PlaceholderNode::PlaceholderNode(TypeQualifier qualifier, T&& value) :
	m_output(*this, TypeID::make<std::decay_t<T>>()),
	m_value(std::forward<T>(value)),
	m_qualifier(qualifier)
{
}

template<typename T> void PlaceholderNode::setValue(T&& value)
{
	if (TypeID::make<std::decay_t<T>>() != m_output.dataType())
	{
		throw std::invalid_argument("value type does not match output type");
	}

	m_value = std::forward<T>(value);
}

template<typename... Args> std::shared_ptr<PlaceholderNode> PlaceholderNode::make_constant(Args&& ...args)
{
	return std::make_shared<PlaceholderNode>(TypeQualifier::Constant, std::forward<Args>(args)...);
}

template<typename... Args> std::shared_ptr<PlaceholderNode> PlaceholderNode::make_variable(Args&& ...args)
{
	return std::make_shared<PlaceholderNode>(TypeQualifier::Variable, std::forward<Args>(args)...);
}

template<typename... Args> std::shared_ptr<PlaceholderNode> PlaceholderNode::make(Args&& ...args)
{
	return std::make_shared<PlaceholderNode>(std::forward<Args>(args)...);
}

template<typename T>
void GraphNode::setInput(std::string_view name, TypeQualifier qualifier, T&& value)
{
	using value_type = std::decay_t<T>;
	static_assert(!std::is_same_v<value_type, std::shared_ptr<const DataFlow>>, "value argument is DataFlow");
	static_assert(!std::is_same_v<value_type, std::shared_ptr<DataFlow>>, "value argument is DataFlow");

	auto inNames = inputNames();
	auto it = std::find(std::begin(inNames), std::end(inNames), name);

	assert(it != std::end(inNames));
	if (it == std::end(inNames))
	{
		throw std::out_of_range("no such parameter");
	}

	auto idx = std::distance(std::begin(inNames), it);
	auto flow = inputs()[idx];
	if (flow == nullptr)
	{
		auto placeholder = PlaceholderNode::make(qualifier, std::forward<T>(value));
		setInput(name, placeholder->outputs().front());
		return;
	}
	auto& producer = const_cast<GraphNode&>(flow->producer());
	auto& placeholder = producer.is<PlaceholderNode>();
	placeholder.setValue(std::forward<T>(value));
	placeholder.setTypeQualifier(qualifier);
}


}
