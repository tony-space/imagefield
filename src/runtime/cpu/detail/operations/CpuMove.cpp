#include "../CpuOperationFactory.hpp"

namespace imf::runtime::cpu
{

class CpuMove : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "Move";

	CpuMove(CpuRuntime&, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_dst(outputs[0]),
		m_src(inputs[0])
	{
	}
	virtual void execute(core::EvaluationContext& evalCtx) override
	{
		evalCtx.set(m_dst.location, core::fetch_operand(evalCtx, m_src));
	}
protected:
	core::destination_operand m_dst;
	core::source_operand m_src;
};

}

DeclareCpuOperation(CpuMove)
