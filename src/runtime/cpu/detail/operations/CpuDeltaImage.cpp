#include <imf/runtime/cpu/CpuOperationFactory.hpp>
#include <imf/runtime/cpu/CpuSampler.hpp>
#include <imf/runtime/cpu/CpuTexture.hpp>
#include <imf/runtime/cpu/Rasterizer.hpp>

#include <imf/core/Region.hpp>
#include <imf/core/log.hpp>

namespace imf::runtime::cpu
{

class CpuDeltaImage : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "DeltaImage";

	CpuDeltaImage(CpuRuntime& runtime, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_runtime(runtime),
		m_dst(outputs[0]),
		m_image1(inputs[0]),
		m_image2(inputs[1])
	{
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		core::log::info("cpu") << "Executing CpuDeltaImage";

		const auto& in1 = core::fetch_operand<core::Image>(context, m_image1);
		const auto& in2 = core::fetch_operand<core::Image>(context, m_image2);

		auto region1 = in1.worldRegion();
		auto region2 = in2.worldRegion();

		auto targetBox = in1.boundingBox().union_(in2.boundingBox());

		auto texture = CpuTexture::make(targetBox.textureSize(), m_runtime.workingFormat());

		core::SamplerDesc samplerDesc =
		{
			core::MinMagFilter::Nearest,
			core::MinMagFilter::Nearest,
			core::MipFilter::Disabled
		};
		CpuSampler sampler1(m_runtime, in1, samplerDesc);
		CpuSampler sampler2(m_runtime, in2, samplerDesc);
		if (const auto intersection = core::region_intersection(*region1, *region2))
		{
			Rasterizer::rasterizeMSAA(m_runtime.threadPool(), *texture, targetBox, intersection->triangles(), glm::mat3(1.0f), [&](const glm::mat4x2& worldQuad)
			{
				const auto quad1 = sampler1.sample(worldQuad);
				const auto quad2 = sampler2.sample(worldQuad);

				return glm::mat4
				{
					glm::abs(quad2[0] - quad1[0]),
					glm::abs(quad2[1] - quad1[1]),
					glm::abs(quad2[2] - quad1[2]),
					glm::abs(quad2[3] - quad1[3])
				};
			});
		}

		if (const auto diff1 = core::region_difference(*region1, *region2))
		{
			Rasterizer::rasterizeMSAA(m_runtime.threadPool(), *texture, targetBox, diff1->triangles(), glm::mat3(1.0f), [&](const glm::mat4x2& worldQuad)
			{
				return sampler1.sample(worldQuad);
			});
		}

		if (const auto diff2 = core::region_difference(*region2, *region1))
		{
			Rasterizer::rasterizeMSAA(m_runtime.threadPool(), *texture, targetBox, diff2->triangles(), glm::mat3(1.0f), [&](const glm::mat4x2& worldQuad)
			{
				return sampler2.sample(worldQuad);
			});
		}

		texture->msaaResolve(m_runtime.threadPool());
		context.set(m_dst.location, core::Image
		(
			std::move(texture),
			targetBox
		));
	}
protected:
	CpuRuntime& m_runtime;
	core::destination_operand m_dst;
	core::source_operand m_image1;
	core::source_operand m_image2;
};

}

DeclareCpuOperation(CpuDeltaImage)
