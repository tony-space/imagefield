#include "../CpuOperationFactory.hpp"
#include "../CpuSampler.hpp"
#include "../NaiveGraphCompiler.hpp"
#include "../Rasterizer.hpp"

namespace imf::runtime::cpu
{

class CpuBicubicUpscale : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "BicubicUpscale";

	CpuBicubicUpscale(CpuRuntime& runtime, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_runtime(runtime),
		m_dst(outputs[0]),
		m_image(inputs[0]),
		m_scale(inputs[1])
	{
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		auto image = core::fetch_operand<core::Image>(context, m_image);
		auto scale = core::fetch_operand<glm::vec2>(context, m_scale);

		if (glm::compMax(glm::abs(scale)) <= 1.0)
		{
			return context.set(m_dst.location, image.transformed(core::scale(scale)));
		}

		// bake all the implicit transformations before doing further nearest sampling
		image = m_runtime.blit(image);

		auto horizontalResult = pass(std::move(image), scale.x, glm::vec2(1.0f, 0.0f));
		auto verticalResult = pass(std::move(horizontalResult), scale.y, glm::vec2(0.0f, 1.0f));
			
		context.set(m_dst.location, std::move(verticalResult));
	}

	core::Image pass(core::Image image, float scale, const glm::vec2& dir)
	{
		auto transformed = image.transformed(core::scale(glm::vec2(1.0f) + dir * (scale - 1.0f)));
		if (scale <= 1.0f)
		{
			return m_runtime.blit(transformed);
		}

		auto samplerDesc = core::SamplerDesc{};
		samplerDesc.magFilter = core::MinMagFilter::Nearest;
		samplerDesc.minFilter = core::MinMagFilter::Nearest;
		samplerDesc.mipFilter = core::MipFilter::Disabled;

		const auto sampler = CpuSampler(m_runtime, transformed, samplerDesc);
		const auto textureSize = image.boundingBox().textureSize<glm::vec2>();
		const auto stepSize = dir / textureSize;

		auto targetBox = transformed.boundingBox();
		auto targetTexture = std::make_shared<CpuTexture>(targetBox.textureSize<glm::uvec2>(), m_runtime.workingFormat());

		Rasterizer::rasterize(m_runtime.threadPool(), *targetTexture, targetBox, transformed.localRegion()->triangles(), transformed.uvToWorldMat(),
		[&](glm::mat4x2 pixelPosQuad)
		{
			pixelPosQuad[0] += dir * glm::vec2(-1.0f, 1.0f) * (scale / 2.0f);
			pixelPosQuad[1] += dir * glm::vec2(-1.0f, 1.0f) * (scale / 2.0f);
			pixelPosQuad[2] += dir * glm::vec2(-1.0f, 1.0f) * (scale / 2.0f);
			pixelPosQuad[3] += dir * glm::vec2(-1.0f, 1.0f) * (scale / 2.0f);

			const auto tc = sampler.textureCoords(pixelPosQuad);

			const auto tc0 = tc - glm::mat4x2(stepSize, stepSize, stepSize, stepSize);
			const auto tc1 = tc;
			const auto tc2 = tc + glm::mat4x2(stepSize, stepSize, stepSize, stepSize);
			const auto tc3 = tc + glm::mat4x2(2.0f * stepSize, 2.0f * stepSize, 2.0f * stepSize, 2.0f * stepSize);

			const auto p0 = sampler.sampleByTextureCoords(tc0);
			const auto p1 = sampler.sampleByTextureCoords(tc1);
			const auto p2 = sampler.sampleByTextureCoords(tc2);
			const auto p3 = sampler.sampleByTextureCoords(tc3);

			glm::mat4 result;

			for (int i = 0; i < 4; ++i)
			{
				const auto texelIndexRaw = glm::dot(tc[i] * textureSize, dir);
				const auto interpolationParam = glm::fract(texelIndexRaw);
				//const auto __debugP0 = glm::pow(p0[i].xyz(), glm::vec3(1.0f / 2.2f)) * 255.0f;
				//const auto __debugP1 = glm::pow(p1[i].xyz(), glm::vec3(1.0f / 2.2f)) * 255.0f;
				//const auto __debugP2 = glm::pow(p2[i].xyz(), glm::vec3(1.0f / 2.2f)) * 255.0f;
				//const auto __debugP3 = glm::pow(p3[i].xyz(), glm::vec3(1.0f / 2.2f)) * 255.0f;

				result[i] = cubic(p0[i], p1[i], p2[i], p3[i], interpolationParam);

				//auto __debugResult = glm::pow(glm::clamp(result[i].xyz(), glm::vec3(0.0f), glm::vec3(1.0f)), glm::vec3(1.0f / 2.2f)) * 255.0f;
				//(void)__debugResult;
				//__debugResult = __debugResult;
			}
			
			return result;

		});

		targetTexture->msaaResolve(m_runtime.threadPool());

		return core::Image
		(
			std::move(targetTexture),
			targetBox,
			transformed.localRegion()
		);
	}


	// Derivation of 1D cubic
	// https://www.paulinternet.nl/?page=bicubic
	//
	static constexpr glm::mat4 kCubicCoeffs = glm::mat4
	(
		2.0f, -2.0f, 1.0f, 1.0f,
		-3.0f, 3.0f, -2.0f, -1.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f
	);

	static glm::vec4 cubic(const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2, const glm::vec4& p3, float t)
	{
		// values = (f(0), f(1), f'(0), f'(1))
		glm::mat4 values = glm::mat4(p1, p2, (p2 - p0) * 0.5f, (p3 - p1) * 0.5f);

		// a, b, c, d
		glm::mat4 polynomial = values * kCubicCoeffs;

		// x^3, x^2, x, 1
		glm::vec4 x = glm::vec4(t * t * t, t * t, t, 1.0f);

		// ax^3 + bx^2 + cx + d
		return polynomial * x;
	}


protected:
	CpuRuntime& m_runtime;
	core::destination_operand m_dst;
	core::source_operand m_image;
	core::source_operand m_scale;
};

}

DeclareCpuOperation(CpuBicubicUpscale)
