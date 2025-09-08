#include <imf/core/core.hpp>
#include <imf/runtime/cpu/CpuTexture.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(cpu_rasterizer)

//
// A special case of a 1x1 white opaque image rotated by 45 degrees.
// The result should be 2x2 half-transparent white image.
//
BOOST_AUTO_TEST_CASE(OnePixelImage)
{
	using namespace imf;
	using namespace imf::runtime;

	auto whiteOpaque = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	core::TextureData hostTextureData;
	hostTextureData.format = core::TextureFormat::RGBA32F;
	hostTextureData.data = &whiteOpaque;
	hostTextureData.dim = glm::uvec3(1, 1, 1);
	hostTextureData.rowAlignment = 1;
	hostTextureData.planeAlignment = 1;

	auto image = core::Image
	(
		std::make_shared<cpu::CpuTexture>(hostTextureData, core::TextureFormat::RGBA32F),
		core::BoundingBox(1.0f, 1.0f)
	);

	auto imageRotated = image.transformed(core::rotate_deg(45.0f));

	auto cpuRuntime = core::make_runtime("cpu");
	cpuRuntime->setWorkingFormat(core::TextureFormat::RGBA32F);
	auto bakedImage = cpuRuntime->blit(imageRotated);

	const auto boxSize = glm::round(bakedImage.boundingBox().size() * core::BoundingBox::kPrecision) * core::BoundingBox::kEpsilon;
	const auto expectedSize = glm::round(glm::sqrt(glm::vec2(2.0f)) * core::BoundingBox::kPrecision) * core::BoundingBox::kEpsilon;
	
	BOOST_REQUIRE(boxSize == expectedSize);

	std::dynamic_pointer_cast<const core::IReadMapTexture>(bakedImage.texture())->mapUnmap([](const core::TextureData& deviceData)
	{
		BOOST_REQUIRE(deviceData.dim.x == 2);
		BOOST_REQUIRE(deviceData.dim.y == 2);
		BOOST_REQUIRE(deviceData.dim.z == 1);
		BOOST_REQUIRE(deviceData.format == core::TextureFormat::RGBA32F);

		const auto imageSize = core::calc_image_size(deviceData.format, deviceData.dim, deviceData.rowAlignment, deviceData.planeAlignment);
		const auto row0 = static_cast<const glm::vec4*>(deviceData.data);
		const auto row1 = reinterpret_cast<const glm::vec4*>(reinterpret_cast<const std::uint8_t*>(deviceData.data) + imageSize.rowByteSize);
		
		BOOST_REQUIRE(row0[0] == glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		BOOST_REQUIRE(row0[1] == glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		BOOST_REQUIRE(row1[0] == glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		BOOST_REQUIRE(row1[1] == glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
	}, 0);

}

BOOST_AUTO_TEST_SUITE_END()
