#include "NaiveGraphCompiler.hpp"

#include <imf/runtime/cpu/CpuOperationFactory.hpp>
#include <imf/runtime/cpu/CpuRuntime.hpp>
#include <imf/runtime/cpu/CpuSampler.hpp>
#include <imf/runtime/cpu/CpuTexture.hpp>
#include <imf/runtime/cpu/Rasterizer.hpp>

#include <imf/core/Image.hpp>
#include <imf/core/ITexture.hpp>
#include <imf/core/log.hpp>
#include <imf/core/RuntimeFactory.hpp>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

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
	core::log::info("runtime") << "cpu runtime initialized";
}

std::vector<std::uint8_t> CpuRuntime::fetchContent(const std::filesystem::path& path)
{
	core::log::info("runtime") << "reading binary content from: " << path.string();

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

void CpuRuntime::saveImage(core::Image image, const std::filesystem::path& path)
{
	core::log::info("runtime") << "attempting to save image to: " << path.string();

	image = blit(image);

	const auto texture = std::dynamic_pointer_cast<const core::IReadMapTexture>(image.texture());

	texture->mapUnmap([&](const core::TextureData& deviceData)
	{
		constexpr static std::size_t kAlignment = 32;
		const auto dstSize = core::calc_image_size(core::TextureFormat::RGBA8, deviceData.dim, kAlignment, 1);
		auto result = std::vector<uint8_t>(dstSize.volumeByteSize);

		convert_pixels(deviceData, core::TextureFormat::RGBA8, kAlignment, 1, result.data(), dstSize.volumeByteSize);

		stbi_write_png(path.string().c_str(), (int)deviceData.dim.x, (int)deviceData.dim.y, 4, result.data(), (int)dstSize.rowByteSize);
	}, 0);
}

core::Image CpuRuntime::blit(const core::Image& image, const core::SamplerDesc& desc)
{
	if (image.localRegion()->trivialRectangle() &&
		image.uvToWorldMat() == core::Image::calcUvToWorldMat(image.boundingBox()) &&
		desc == core::SamplerDesc{}
		)
	{
		return image;
	}

	const auto& targetBox = image.boundingBox();
	const auto targetDim = targetBox.textureSize<glm::uvec2>();
	auto targetTexture = std::make_shared<CpuTexture>(targetDim, m_workingFormat);

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
		targetBox
	);
}

}
