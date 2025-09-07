#include <imf/core/core.hpp>

#include <stb/stb_image_write.h>

int main()
{
	try
	{
		using namespace imf::core;

		auto cpuRuntime = make_runtime("cpu");
		
		auto imgPlaceholder = PlaceholderNode::make_variable(cpuRuntime->loadImage("../assets/png/lenna.png"));
		auto transformPlaceholder = PlaceholderNode::make_variable(scale(0.15f, 0.15f) * rotate_deg(5.0f));

		auto affine = make_graph_node("Transform");
		affine->setInput("image", imgPlaceholder->outputs().front());
		affine->setInput("matrix", transformPlaceholder->outputs().front());

		auto sink = SinkNode::make(affine->outputs().front());
		
		auto executor = GraphExecutor::make(cpuRuntime, sink);

		auto executionResult = executor->run();
		auto result = std::any_cast<Image&&>(std::move(executionResult.front().value));
		result = cpuRuntime->blit(result);

		const auto texture = std::dynamic_pointer_cast<const IReadMapTexture>(result.texture());
		texture->mapUnmap([](const TextureData& deviceData)
		{
			constexpr static std::size_t kAlignment = 32;
			const auto dstSize = calc_image_size(TextureFormat::RGBA8, deviceData.dim, kAlignment, 1);
			auto result = std::vector<uint8_t>(dstSize.volumeByteSize);

			convert_pixels(deviceData, TextureFormat::RGBA8, kAlignment, 1, result.data(), dstSize.volumeByteSize);

			stbi_write_png("out.png", (int)deviceData.dim.x, (int)deviceData.dim.y, 4, result.data(), (int)dstSize.rowByteSize);
		}, 0);

	}
	catch (const std::exception& ex)
	{
		imf::core::log::err("uncaught exception") << ex.what();
	}

}
