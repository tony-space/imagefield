#include "../CpuOperationFactory.hpp"

#include <imf/core/DataFlow.hpp>
#include <imf/core/node/FunctorNode.hpp>

namespace imf::runtime::cpu
{

class CpuFunctor : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "Functor";

	CpuFunctor(CpuRuntime& runtime, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) : m_runtime(runtime)
	{
		m_inputs.reserve(inputs.size());
		std::copy(inputs.begin(), inputs.end(), std::back_inserter(m_inputs));

		m_outputs.reserve(outputs.size());
		std::copy(outputs.begin(), outputs.end(), std::back_inserter(m_outputs));
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		const auto& functor = core::fetch_operand<core::FunctorNode::functor_t>(context, m_inputs[0]);

		functor
		(
			context,
			core::destination_operands_range{ m_outputs.data(), m_outputs.data() + m_outputs.size() },
			core::source_operands_range{ m_inputs.data() + 1, m_inputs.data() + m_inputs.size() }
		);
	}
protected:
	CpuRuntime& m_runtime;

	core::FunctorNode::small_vector_t<core::source_operand> m_inputs;
	core::FunctorNode::small_vector_t<core::destination_operand> m_outputs;
};

}

DeclareCpuOperation(CpuFunctor)
