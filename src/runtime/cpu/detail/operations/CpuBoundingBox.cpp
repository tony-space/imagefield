#include <imf/runtime/cpu/CpuOperationFactory.hpp>

#include <imf/core/log.hpp>

namespace imf::runtime::cpu
{

class CpuBoundingBox : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "BoundingBox";

	CpuBoundingBox(CpuRuntime&, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_dst(outputs[0]),
		m_image(inputs[0])
	{
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		core::log::info("cpu") << "Executing CpuBoundingBox";

		const auto& in = core::fetch_operand<core::Image>(context, m_image);

		context.set(m_dst.location, in.boundingBox());
	}
protected:
	core::destination_operand m_dst;
	core::source_operand m_image;
};

}

DeclareCpuOperation(CpuBoundingBox)
