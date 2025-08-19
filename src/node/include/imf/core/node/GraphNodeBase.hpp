#pragma once

#include <imf/core/TypeID.hpp>
#include <imf/core/Image.hpp>
#include <imf/core/GraphNode.hpp>
#include <imf/core/glm.hpp>
#include <imf/core/DataFlow.hpp>

#include <array>
#include <stdexcept>

namespace imf::core
{

namespace detail
{

template <typename Derived, std::size_t... Is>
constexpr std::array<DataFlow, sizeof...(Is)> make_outputs_impl(GraphNode* producer, std::index_sequence<Is...>)
{
	return { { DataFlow(producer, Derived::output_types[Is])... } };
}

template <typename Derived>
constexpr auto make_outputs(GraphNode* producer)
{
	constexpr std::size_t kOutputs = static_cast<std::size_t>(std::distance(std::begin(Derived::output_types), std::end(Derived::output_types)));
	return make_outputs_impl<Derived>(producer, std::make_index_sequence<kOutputs>{});
}

}

template <typename Derived, std::size_t kInputs, std::size_t kOutputs = 1>
class GraphNodeBase : public GraphNode
{
public:
	constexpr GraphNodeBase() : m_outputs(detail::make_outputs<Derived>(this))
	{

	}

	std::string_view operationName() const noexcept override final
	{
		return Derived::operation_name;
	}
	iterator_range<const std::string_view*> inputNames() const noexcept override final
	{
		return { std::begin(Derived::input_names), std::end(Derived::input_names) };
	}
	iterator_range<const TypeID*> inputTypes() const noexcept override final
	{
		return { std::begin(Derived::input_types), std::end(Derived::input_types) };
	}
	iterator_range<const std::shared_ptr<const DataFlow>*> inputs() const noexcept override final
	{
		return { m_inputs.data(), m_inputs.data() + m_inputs.size() };
	}

	iterator_range<const std::string_view*> outputNames() const noexcept override final
	{
		return { std::begin(Derived::output_names), std::end(Derived::output_names) };
	}
	iterator_range<const TypeID*> outputTypes() const noexcept override final
	{
		return { std::begin(Derived::output_types), std::end(Derived::output_types) };
	}
	iterator_range<const DataFlow*> outputs() const noexcept override final
	{
		return { m_outputs.data(), m_outputs.data() + m_outputs.size() };
	}

	void setInput(const std::string_view& name, const DataFlow& flow) override final
	{
		auto it = std::find(std::begin(Derived::input_names), std::end(Derived::input_names), name);
		if (it == std::end(Derived::input_names))
		{
			throw std::out_of_range("no such parameter");
		}
		auto idx = std::distance(std::begin(Derived::input_names), it);

		if (flow.dataType() != Derived::input_types[idx])
		{
			throw std::invalid_argument("invalid input parameter type");
		}

		m_inputs[idx] = flow.sharedPtr();
	}

protected:
	std::array<std::shared_ptr<const DataFlow>, kInputs> m_inputs;
	std::array<DataFlow, kOutputs> m_outputs;
};

}

