#include <imf/core/core.hpp>
#include <imf/core/node/node.hpp>
#include <imf/tests/system.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(GaussainBlur)

BOOST_AUTO_TEST_CASE(CpuGaussianBlur)
{
	static constexpr std::string_view kTestImages[] =
	{
		"dice",
		"lenna",
		"uv1k"
	};

	using namespace imf;
	using namespace imf::core;

	auto cpuRuntime = make_runtime("cpu");

	auto imagePlaceholder = PlaceholderNode::make_variable(TypeID::make<Image>());
	auto referencePlaceholder = PlaceholderNode::make_variable(TypeID::make<Image>());
	auto radiusPlaceholder = PlaceholderNode::make_variable(TypeID::make<glm::uvec2>());
	auto executor = std::shared_ptr<GraphExecutor>();

	{
		auto toLinear = make_graph_node("ColorSpaceConvert");
		toLinear->setInput("image", imagePlaceholder->outputs().front());
		toLinear->setInput("sourceColorSpace", TypeQualifier::Constant, color::ColorSpace::sRGB);
		toLinear->setInput("targetColorSpace", TypeQualifier::Constant, color::ColorSpace::Linear_sRGB);

		auto blur = make_graph_node("GaussianBlur");
		blur->setInput("image", toLinear->outputs().front());
		blur->setInput("radius", radiusPlaceholder->outputs().front());

		auto toGamma = make_graph_node("ColorSpaceConvert");
		toGamma->setInput("image", blur->outputs().front());
		toGamma->setInput("sourceColorSpace", TypeQualifier::Constant, color::ColorSpace::Linear_sRGB);
		toGamma->setInput("targetColorSpace", TypeQualifier::Constant, color::ColorSpace::sRGB);

		auto quantized = make_graph_node("TextureFormatConvert");
		quantized->setInput("image", toGamma->outputs().front());
		quantized->setInput("targetTextureFormat", TypeQualifier::Constant, TextureFormat::RGBA8);
		auto sink1 = SinkNode::make(quantized->outputs().front());

		auto deltaImage = make_graph_node("DeltaImage", quantized->outputs().front(), referencePlaceholder->outputs().front());
		auto sink2 = SinkNode::make(deltaImage->outputs().front());

		auto psnr = make_graph_node("PeakSignalToNoiseRatio", deltaImage->outputs().front());
		auto sink3 = SinkNode::make(psnr->outputs().front());
		executor = GraphExecutor::make(cpuRuntime, sink1, sink2, sink3);
	}

	std::filesystem::create_directory(tests::output_dir() / "gaussian_blur");

	for (const auto& img : kTestImages)
	{
		auto imageFilename = std::filesystem::path(std::string(img) + ".png");
		executor->setPlaceholderValue(imagePlaceholder->instanceId(), cpuRuntime->loadImage(tests::project_root() / "assets/png" / imageFilename));

		for (unsigned radius = 20; radius <= 100; radius += 20u)
		{
			auto referenceFilename = std::filesystem::path(std::string(img) + '_' + std::to_string(radius) + ".png");

			executor->setPlaceholderValue(radiusPlaceholder->instanceId(), glm::uvec2(radius));
			executor->setPlaceholderValue(referencePlaceholder->instanceId(),
				cpuRuntime->loadImage(tests::project_root() / "assets/test-references/gaussian-blur" / referenceFilename)
				.transformed(translate(glm::vec2(-static_cast<float>(radius))))
			);

			auto executionResult = executor->run();

			cpuRuntime->saveImage(std::any_cast<Image&&>(std::move(executionResult[0].value)),
				tests::output_dir() / "gaussian_blur" / (std::string(img) + '_' + std::to_string(radius) + "_result.png")
			);

			cpuRuntime->saveImage(std::any_cast<Image&&>(std::move(executionResult[1].value)),
				tests::output_dir() / "gaussian_blur" / (std::string(img) + '_' + std::to_string(radius) + "_delta.png")
			);

			auto psnr = std::any_cast<float>(executionResult[2].value);

			imf::core::log::info("test") << "Testing [" << img << "] with radius [" << std::to_string(radius) << "] psnr [" << psnr << ']';

			BOOST_REQUIRE(psnr >= 46.0f);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
