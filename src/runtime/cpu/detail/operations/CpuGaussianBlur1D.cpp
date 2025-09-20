#include <imf/runtime/cpu/CpuOperationFactory.hpp>
#include <imf/runtime/cpu/CpuSampler.hpp>
#include <imf/runtime/cpu/Rasterizer.hpp>

#include <boost/container/small_vector.hpp>

namespace imf::runtime::cpu
{

namespace detail
{

using weights_storage_t = boost::container::small_vector<float, 16>;
//using weights_storage_t =std::vector<float>;

static bool calculate_weights(weights_storage_t& weights, unsigned radius)
{
	const auto samples = radius * 2u + 1u;
	const auto sigma = radius / 3.0f;

	if (weights.size() == samples)
	{
		return false;
	}

	weights.resize(0);
	weights.reserve(samples);

	for (auto x = -int(radius); x <= int(radius); x++)
	{
		weights.emplace_back(glm::exp(-0.5f * (x / sigma) * (x / sigma)));
	}

	const auto invSum = 1.0f / std::accumulate(weights.begin(), weights.end(), 0.0f);
	std::for_each(weights.begin(), weights.end(), [&](auto& v) { v *= invSum; });

	return true;
}


}

class CpuGaussianBlur1D : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "GaussianBlur1D";

	CpuGaussianBlur1D(CpuRuntime& runtime, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_runtime(runtime),
		m_dst(outputs[0]),
		m_image(inputs[0]),
		m_kernelRadius(inputs[1]),
		m_horizontal(inputs[2])
	{
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		const auto image = core::fetch_operand<core::Image>(context, m_image);
		const auto kernelRadius = core::fetch_operand<unsigned>(context, m_kernelRadius);
		const auto horizontal = core::fetch_operand<bool>(context, m_horizontal);

		if (kernelRadius == 0u)
		{
			return context.set(m_dst.location, image);
		}

		detail::calculate_weights(m_weights, kernelRadius);

		auto samplerDesc = core::SamplerDesc{};
		samplerDesc.magFilter = core::MinMagFilter::Nearest;
		samplerDesc.minFilter = core::MinMagFilter::Nearest;
		samplerDesc.mipFilter = core::MipFilter::Disabled;
		// bake all the implicit transformations before doing further nearest sampling
		const auto bakedImage = m_runtime.blit(image);
		const auto sampler = CpuSampler(m_runtime, bakedImage, samplerDesc);

		const auto targetBox = bakedImage.boundingBox().expand(horizontal ? glm::vec2(kernelRadius, 0u) : glm::vec2(0u, kernelRadius));
		auto targetTexture = std::make_shared<CpuTexture>(targetBox.textureSize(), m_runtime.workingFormat());
		auto targetRegion = std::make_shared<core::Region>(core::BoundingBox(1.0f, 1.0f));
		auto targetMat = core::Image::calcUvToWorldMat(targetBox);

		const auto step = horizontal ? glm::ivec2(1, 0) : glm::ivec2(0, 1);

		Rasterizer::rasterize(m_runtime.threadPool(), *targetTexture, targetBox, targetRegion->triangles(), targetMat,
		[&](const glm::mat4x2& pixelPosQuad)
		{
			auto it = m_weights.begin();

			glm::mat4 sum(0.0f);

			for (auto i = -int(kernelRadius); i <= int(kernelRadius); i++)
			{
				const auto offset = glm::vec2(i * step);
				auto color = sampler.sampleForConvolution(pixelPosQuad + glm::mat4x2(offset, offset, offset, offset));
				const auto weight = *it;
				++it;

				for (int comp = 0; comp != 4; ++comp)
				{
					color[comp].r *= color[comp].a;
					color[comp].g *= color[comp].a;
					color[comp].b *= color[comp].a;
				}

				sum += color * weight;
			}

			for (int comp = 0; comp != 4; ++comp)
			{
				if (sum[comp].a > 0.0f)
				{
					sum[comp].r /= sum[comp].a;
					sum[comp].g /= sum[comp].a;
					sum[comp].b /= sum[comp].a;
				}
			}

			return sum;
		});

		targetTexture->msaaResolve(m_runtime.threadPool());

		context.set(m_dst.location, core::Image
		(
			std::move(targetTexture),
			targetBox,
			std::move(targetRegion),
			bakedImage.componentMapping(),
			bakedImage.lodSettings(),
			targetMat
		));
	}

protected:
	CpuRuntime& m_runtime;
	core::destination_operand m_dst;
	core::source_operand m_image;
	core::source_operand m_kernelRadius;
	core::source_operand m_horizontal;

	detail::weights_storage_t m_weights;
};

}

DeclareCpuOperation(CpuGaussianBlur1D)
