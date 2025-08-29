#include "CpuSampler.hpp"
#include "NaiveGraphCompiler.hpp"
#include "Rasterizer.hpp"

#include <imf/runtime/cpu/CpuRuntime.hpp>
#include <imf/runtime/cpu/CpuTexture.hpp>

#include <imf/core/Image.hpp>
#include <imf/core/ITexture.hpp>
#include <imf/core/RuntimeFactory.hpp>

#include <stb/stb_image.h>

#include <fstream>

namespace imf::core
{

runtime_instantiator_t make_cpu_runtime = [](const IRuntime::init_config_t& config)
{
	return std::make_shared<runtime::cpu::CpuRuntime>(config);
};

}

namespace imf::runtime::cpu
{

CpuRuntime::CpuRuntime(const core::IRuntime::init_config_t&)
{

}

std::vector<std::uint8_t> CpuRuntime::fetchContent(const std::filesystem::path& path)
{
	std::ifstream stream(path, std::ios_base::binary);
	assert(!stream.fail());
	if (stream.fail())
	{
		throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
	}

	return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
}

std::shared_ptr<core::IGraphCompiler> CpuRuntime::compiler()
{
	return std::make_shared<NaiveGraphCompiler>(*this);
}

core::Image CpuRuntime::loadImage(const std::filesystem::path& path)
{
	const auto rawData = fetchContent(path);

	int width;
	int height;
	int channels;
	auto data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(rawData.data()), static_cast<int>(rawData.size()), &width, &height, &channels, 0);

	if (data == nullptr)
	{
		throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory), path.string());
	}

	core::TextureData hostData;
	hostData.data = data;
	hostData.dim = { static_cast<unsigned>(width), static_cast<unsigned>(height), 1 };
	hostData.format = static_cast<core::TextureFormat>(channels - 1);
	hostData.rowAlignment = 1;
	hostData.planeAlignment = 1;

	try
	{
		auto texture = std::make_shared<CpuTexture>(hostData, m_workingFormat);
		stbi_image_free(data);

		return core::Image(std::move(texture), core::BoundingBox(glm::uvec2(width, height)));
	}
	catch (...)
	{
		stbi_image_free(data);
		throw;
	}

}

core::Image CpuRuntime::blit(const core::Image& image)
{
	const auto& srcBox = image.boundingBox();
	const auto targetDim = srcBox.intSize();
	auto targetTexture = std::make_shared<CpuTexture>(targetDim, m_workingFormat);
	
	const auto sampler = CpuSampler(*this, image, core::SamplerDesc{});
	(void)sampler;

	const auto& __mipLevel0 = std::dynamic_pointer_cast<const CpuTexture>(image.texture())->at(0);

	//Rasterizer::rasterize(threadPool(), *targetTexture, srcBox.min(), image.localRegion()->triangles(), image.uvToWorldMat(),
	//[&](const glm::vec2& pixelPos)
	//{
	//	const auto dim = glm::ivec2(__mipLevel0.dim);
	//	const auto uv = image.worldToUvMat()* glm::vec3(pixelPos, 1.0f);
	//	const auto pixelIdx = glm::ivec2(glm::vec2(dim) * uv.xy() / uv.z);
	//	if (pixelIdx.x < 0 || pixelIdx.y < 0 || pixelIdx.x >= dim.x || pixelIdx.y >= dim.y)
	//	{
	//		return glm::vec4(0.0f);
	//	}
	//	
	//	const auto pixelPtr = __mipLevel0.storage.get() + __mipLevel0.size.rowByteSize * pixelIdx.y + __mipLevel0.size.pixelByteSize * pixelIdx.x;

	//	return *reinterpret_cast<const glm::vec4*>(pixelPtr);
	//});

	Rasterizer::rasterize(threadPool(), *targetTexture, srcBox.min(), image.localRegion()->triangles(), image.uvToWorldMat(),
	[&](const glm::mat4x2& pixelQuad)
	{
		constexpr static auto get = +[](const CpuTexture::MipLevel& mip, glm::ivec2 pixelIdx)
		{
			const auto pixelPtr = mip.storage.get() + mip.size.rowByteSize * pixelIdx.y + mip.size.pixelByteSize * pixelIdx.x;
			return *reinterpret_cast<const glm::vec4*>(pixelPtr);
		};

		glm::mat4 result;
		for (int i = 0; i != 4; ++i)
		{
			const auto dim = glm::vec2(__mipLevel0.dim);
			const auto uv = core::projectToPlane(image.worldToUvMat(), pixelQuad[i]) * glm::vec2(dim);
			const auto SW = glm::clamp(glm::floor(uv - glm::vec2(0.5f)), glm::vec2(0.0f), dim - 1.0f);
			const auto NE = glm::clamp(glm::floor(uv - glm::vec2(0.5f)) + glm::vec2(1.0f), glm::vec2(0.0f), dim - 1.0f);
			const auto NW = glm::vec2{ SW.x, NE.y };
			const auto SE = glm::vec2{ NE.x, SW.y };

			const auto valueSW = get(__mipLevel0, glm::ivec2(SW));
			const auto valueNE = get(__mipLevel0, glm::ivec2(NE));
			const auto valueNW = get(__mipLevel0, glm::ivec2(NW));
			const auto valueSE = get(__mipLevel0, glm::ivec2(SE));

			const auto fraction = glm::fract(uv - glm::vec2(0.5f));

			result[i] = glm::mix
			(
				glm::mix(valueSW, valueSE, fraction.x),
				glm::mix(valueNW, valueNE, fraction.x),
				fraction.y
			);
		}
		return result;
	});

	targetTexture->msaaResolve(threadPool());
	return core::Image
	(
		std::move(targetTexture),
		core::BoundingBox::fromOrigin(srcBox.min(), targetDim)
	);
}

}
