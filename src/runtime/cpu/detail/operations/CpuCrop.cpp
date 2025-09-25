#include <imf/runtime/cpu/CpuOperationFactory.hpp>
#include <imf/runtime/cpu/CpuSampler.hpp>
#include <imf/runtime/cpu/Rasterizer.hpp>

#include <imf/core/log.hpp>

namespace imf::runtime::cpu
{

class CpuCrop : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "Crop";

	CpuCrop(CpuRuntime& runtime, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_runtime(runtime),
		m_dst(outputs[0]),
		m_image(inputs[0]),
		m_box(inputs[1])
	{
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		core::log::info("cpu") << "Executing CpuCrop";

		const auto& in = core::fetch_operand<core::Image>(context, m_image);
		const auto& targetBox = core::fetch_operand<core::BoundingBox>(context, m_box);

		assert(targetBox.finite());
		if (!targetBox.finite())
		{
			throw std::invalid_argument("Crop: result box is infinite");
		}

		if (auto resultRegion = core::region_intersection(*in.worldRegion(), *core::Region::make(targetBox)))
		{
			auto sampler = CpuSampler(m_runtime, in);
			auto regionBox = resultRegion->boundingBox();

			auto texture = CpuTexture::make(regionBox.textureSize(), in.texture()->format());
			Rasterizer::rasterizeMSAA(m_runtime.threadPool(), *texture, targetBox, resultRegion->triangles(), glm::mat3(1.0f), [&](const glm::mat4x2& worldQuad)
			{
				return sampler.sample(worldQuad);
			});
			texture->msaaResolve(m_runtime.threadPool());

			auto uvToWorldMat = core::Image::calcUvToWorldMat(targetBox);
			resultRegion->transformPoints(glm::inverse(uvToWorldMat));

			context.set(m_dst.location, core::Image
			(
				std::move(texture),
				targetBox,
				std::move(resultRegion),
				in.componentMapping(),
				in.lodSettings(),
				uvToWorldMat
			));
		}
		else
		{
			//
			// TODO I'm not sure that output format should be `in.texture()->format()`. Need more data to choose.
			//
			auto texture = CpuTexture::make(glm::uvec2(1u), m_runtime.workingFormat());
			auto& mipLevel0 = texture->at(0);
			std::memset(mipLevel0.storage.get(), 0, mipLevel0.size.volumeByteSize);

			context.set(m_dst.location, core::Image
			(
				std::move(texture),
				targetBox
			));
		}

	}
protected:
	CpuRuntime& m_runtime;
	core::destination_operand m_dst;
	core::source_operand m_image;
	core::source_operand m_box;
};

}

DeclareCpuOperation(CpuCrop)
