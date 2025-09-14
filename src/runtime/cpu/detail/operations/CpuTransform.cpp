#include <imf/runtime/cpu/CpuOperationFactory.hpp>

namespace imf::runtime::cpu
{

class CpuTransform : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "Transform";

	CpuTransform(CpuRuntime&, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_dst(outputs[0]),
		m_image(inputs[0]),
		m_matrix(inputs[1])
	{
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		const auto& in = core::fetch_operand<core::Image>(context, m_image);
		const auto& homogenousMat = core::fetch_operand<glm::mat3>(context, m_matrix);

		context.set(m_dst.location, in.transformed(homogenousMat));
	}
protected:
	core::destination_operand m_dst;
	core::source_operand m_image;
	core::source_operand m_matrix;
};

}

DeclareCpuOperation(CpuTransform)
