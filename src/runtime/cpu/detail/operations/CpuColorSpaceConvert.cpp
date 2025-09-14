#include <imf/runtime/cpu/CpuOperationFactory.hpp>
#include <imf/runtime/cpu/CpuSampler.hpp>
#include <imf/runtime/cpu/Rasterizer.hpp>

#include <imf/core/ColorSpace.hpp>

namespace imf::runtime::cpu
{

class CpuColorSpaceConvert : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "ColorSpaceConvert";

	CpuColorSpaceConvert(CpuRuntime& runtime, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_runtime(runtime),
		m_dst(outputs[0]),
		m_image(inputs[0]),
		m_sourceColorSpace(inputs[1]),
		m_targetColorSpace(inputs[2])
	{
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		const auto& image = core::fetch_operand<core::Image>(context, m_image);
		const auto& sourceColorSpace = core::fetch_operand<core::color::ColorSpace>(context, m_sourceColorSpace);
		const auto& targetColorSpace = core::fetch_operand<core::color::ColorSpace>(context, m_targetColorSpace);

		const auto toXYZmat = core::color::XYZ::from_RGB_matrix(sourceColorSpace.r, sourceColorSpace.g, sourceColorSpace.b, sourceColorSpace.w);
		const auto toTargetMat = glm::inverse(core::color::XYZ::from_RGB_matrix(targetColorSpace.r, targetColorSpace.g, targetColorSpace.b, targetColorSpace.w));
		const auto conversionMat = toTargetMat * toXYZmat;

		const auto& targetBox = image.boundingBox();
		const auto targetDim = targetBox.textureSize<glm::uvec2>();
		auto targetTexture = std::make_shared<CpuTexture>(targetDim, m_runtime.workingFormat());

		const auto sampler = CpuSampler(m_runtime, image);

		Rasterizer::rasterize(m_runtime.threadPool(), *targetTexture, targetBox, image.localRegion()->triangles(), image.uvToWorldMat(),
		[&](const glm::mat4x2& pixelPosQuad)
		{
			auto pixelQuad = sampler.sample(pixelPosQuad);

			pixelQuad[0] = glm::vec4(targetColorSpace.oetf(conversionMat * sourceColorSpace.eotf(pixelQuad[0].xyz())), pixelQuad[0].a);
			pixelQuad[1] = glm::vec4(targetColorSpace.oetf(conversionMat * sourceColorSpace.eotf(pixelQuad[1].xyz())), pixelQuad[1].a);
			pixelQuad[2] = glm::vec4(targetColorSpace.oetf(conversionMat * sourceColorSpace.eotf(pixelQuad[2].xyz())), pixelQuad[2].a);
			pixelQuad[3] = glm::vec4(targetColorSpace.oetf(conversionMat * sourceColorSpace.eotf(pixelQuad[3].xyz())), pixelQuad[3].a);

			return pixelQuad;
		});

		targetTexture->msaaResolve(m_runtime.threadPool());

		context.set(m_dst.location, core::Image
		(
			std::move(targetTexture),
			targetBox
		));
	}
protected:
	CpuRuntime& m_runtime;
	core::destination_operand m_dst;
	core::source_operand m_image;
	core::source_operand m_sourceColorSpace;
	core::source_operand m_targetColorSpace;
};

}

DeclareCpuOperation(CpuColorSpaceConvert)
