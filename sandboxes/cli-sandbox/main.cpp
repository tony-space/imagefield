#include <imf/core/core.hpp>

#include <stb/stb_image_write.h>

int main()
{
	try
	{
		using namespace imf::core;

		auto cpuRuntime = make_runtime("cpu");
		auto image = cpuRuntime->loadImage("../assets/png/lenna.png");

		const auto texture = std::dynamic_pointer_cast<const IReadMapTexture>(image.texture());
		texture->mapUnmap([](const TextureData& deviceData)
		{
			constexpr static std::size_t kAlignment = 32;
			const auto dstSize = calc_image_size(TextureFormat::RGBA8, deviceData.dim, kAlignment, 1);
			auto result = std::vector<uint8_t>(dstSize.volumeByteSize);

			convert_pixels(deviceData, TextureFormat::RGBA8, kAlignment, 1, result.data(), dstSize.volumeByteSize);

			stbi_write_png("out.png", (int)deviceData.dim.x, (int)deviceData.dim.y, 4, result.data(), (int)dstSize.rowByteSize);
		});

	}
	catch (const std::exception& ex)
	{
		imf::core::log::err("uncaught exception") << ex.what();
	}

}
