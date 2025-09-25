#include <imf/runtime/cpu/CpuOperationFactory.hpp>
#include <imf/runtime/cpu/CpuTexture.hpp>

#include <imf/core/log.hpp>

#include <atomic>
#include <numeric>

namespace imf::runtime::cpu
{

class CpuPeakSignalToNoiseRatio : public core::IBackendOperation
{
public:
	constexpr static std::string_view operation_name = "PeakSignalToNoiseRatio";

	CpuPeakSignalToNoiseRatio(CpuRuntime& runtime, const core::destination_operands_range& outputs, const core::source_operands_range& inputs) :
		m_runtime(runtime),
		m_dst(outputs[0]),
		m_image(inputs[0])
	{
	}

	virtual void execute(core::EvaluationContext& context) override
	{
		core::log::info("cpu") << "Executing CpuPeakSignalToNoiseRatio";

		const auto baked = m_runtime.blit(core::fetch_operand<core::Image>(context, m_image));
		const auto texture = std::dynamic_pointer_cast<const CpuTexture>(baked.texture());
		const auto& mip0 = texture->at(0u);
		
		//3D images not supported yet
		assert(mip0.dim.z == 1);

		const auto readFunc = core::get_convert_func(texture->format(), core::TextureFormat::RGBA32F);
		const auto invN = 1.0f / static_cast<float>(static_cast<unsigned long long>(mip0.dim.x) * mip0.dim.y * 4ull);

		std::atomic<float> totalError(0.0f);

		m_runtime.threadPool().forEachSync([&](unsigned begin, unsigned end)
		{
			float result = 0.0f;
			std::vector<glm::vec4> rowData(mip0.dim.x);

			for (unsigned i = begin; i != end; ++i)
			{
				const auto src = mip0.storage.get() + mip0.size.rowByteSize * i;
				readFunc(src, rowData.data(), mip0.dim.x);

				const auto meanSquaredError = std::transform_reduce(rowData.begin(), rowData.end(), 0.0f, [](float deltaSq1, float deltaSq2)
				{
					return deltaSq1 + deltaSq2;
				}, [&](const glm::vec4& diff)
				{
					return glm::dot(diff, diff) * invN;
				});

				result += meanSquaredError;
			}

			float oldVal = totalError.load(std::memory_order_relaxed);
			while (!totalError.compare_exchange_weak(oldVal, oldVal + result, std::memory_order_relaxed, std::memory_order_relaxed))
			{
				// nop
			}


		}, mip0.dim.y);

		// SDR content uses 1.0f as reference max luminance
		constexpr static float kPeakValue = 1.0f;

		float accumulatedError = totalError.load(std::memory_order_acquire);

		float psnr 
			= accumulatedError == 0.0f 
			? std::numeric_limits<float>::infinity() 
			: 10.0f * (glm::log(kPeakValue * kPeakValue / accumulatedError) / glm::log(10.0f));

		context.set(m_dst.location, psnr);
	}
protected:
	CpuRuntime& m_runtime;
	core::destination_operand m_dst;
	core::source_operand m_image;
};

}

DeclareCpuOperation(CpuPeakSignalToNoiseRatio)
