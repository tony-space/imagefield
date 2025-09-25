#include <imf/runtime/cpu/CpuOperationFactory.hpp>
#include <imf/runtime/cpu/CpuTexture.hpp>

#include <imf/core/TextureFormat.hpp>
#include <imf/core/log.hpp>

namespace imf::runtime::cpu
{

class CpuTextureFormatConvert : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "TextureFormatConvert";

	CpuTextureFormatConvert(CpuRuntime& runtime, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_runtime(runtime),
		m_dst(outputs[0]),
		m_image(inputs[0]),
		m_targetTextureFormat(inputs[1])
	{
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		core::log::info("cpu") << "Executing CpuTextureFormatConvert";

		const auto& image = core::fetch_operand<core::Image>(context, m_image);
		const auto& targetTextureFormat = core::fetch_operand<core::TextureFormat>(context, m_targetTextureFormat);

		if (image.texture()->format() == targetTextureFormat)
		{
			return context.set(m_dst.location, image);
		}

		auto baked = m_runtime.blit(image);
		auto srcTexture = std::dynamic_pointer_cast<const CpuTexture>(baked.texture());
		auto dstTexture = CpuTexture::make(srcTexture->dim(), targetTextureFormat);
		
		auto& srcMip0 = srcTexture->at(0u);
		auto& dstMip0 = dstTexture->at(0u);

		core::convert_pixels
		(
			m_runtime.threadPool(),
			core::TextureData
			{
				srcTexture->format(),
				srcTexture->dim(),
				CpuTexture::kRowAlignment,
				CpuTexture::kPlaneAlignment,
				srcMip0.storage.get()
			},
			core::TextureData
			{
				targetTextureFormat,
				srcTexture->dim(),
				CpuTexture::kRowAlignment,
				CpuTexture::kPlaneAlignment,
				dstMip0.storage.get(),
			}
		);

		context.set(m_dst.location, core::Image
		(
			std::move(dstTexture),
			baked.boundingBox()
		));
	}
protected:
	CpuRuntime& m_runtime;
	core::destination_operand m_dst;
	core::source_operand m_image;
	core::source_operand m_targetTextureFormat;
};

}

DeclareCpuOperation(CpuTextureFormatConvert)
