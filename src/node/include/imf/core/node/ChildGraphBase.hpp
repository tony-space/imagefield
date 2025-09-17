#pragma once

#include <imf/core/TypeID.hpp>
#include <imf/core/GraphNode.hpp>
#include <imf/core/DataFlow.hpp>

#include <imf/core/node/GraphNodeBase.hpp>
#include <imf/core/node/ReferenceNode.hpp>


namespace imf::core
{

struct IChildGraph
{
	virtual ~IChildGraph() = default;
	virtual iterator_range<const std::shared_ptr<ReferenceNode>*> outputReferences() const noexcept = 0;
};

template <typename Derived, std::size_t kInputs, std::size_t kOutputs = 1>
class ChildGraphBase : public GraphNodeBase<Derived, kInputs, kOutputs>, public IChildGraph
{
public:
	ChildGraphBase()
	{
		for (std::size_t input = 0; input != kInputs; ++input)
		{
			m_inputReferences[input] = ReferenceNode::make(Derived::input_types[input]);
		}

		for (std::size_t output = 0; output != kOutputs; ++output)
		{
			m_outputReferences[output] = ReferenceNode::make(Derived::output_types[output]);
		}
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

		GraphNodeBase<Derived, kInputs, kOutputs>::m_inputs[idx] = flow.sharedPtr();
		m_inputReferences[idx]->setInput("arg", flow);
	}

	iterator_range<const std::shared_ptr<ReferenceNode>*> outputReferences() const noexcept override
	{
		return { m_outputReferences.data(), m_outputReferences.data() + m_outputReferences.size() };
	}

protected:
	std::array<std::shared_ptr<ReferenceNode>, kInputs> m_inputReferences;
	std::array<std::shared_ptr<ReferenceNode>, kOutputs> m_outputReferences;
};

}
