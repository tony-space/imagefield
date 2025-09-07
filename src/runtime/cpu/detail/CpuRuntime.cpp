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
	const auto& targetBox = image.boundingBox();
	const auto targetDim = targetBox.textureSize();
	auto targetTexture = std::make_shared<CpuTexture>(targetDim, m_workingFormat);
	
	auto desc = core::SamplerDesc{};
	//desc.magFilter = core::MinMagFilter::Nearest;

	const auto sampler = CpuSampler(*this, image, desc);
	Rasterizer::rasterize(threadPool(), *targetTexture, targetBox, image.localRegion()->triangles(), image.uvToWorldMat(),
	[&](const glm::mat4x2& pixelQuad)
	{
		return sampler.sample(pixelQuad);
	});

	targetTexture->msaaResolve(threadPool());
	return core::Image
	(
		std::move(targetTexture),
		//core::BoundingBox::fromOrigin(targetBox.min(), targetDim)
		targetBox
	);
}

}
