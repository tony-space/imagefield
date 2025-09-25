#include <imf/core/core.hpp>
#include <imf/core/node/node.hpp>

static const std::filesystem::path& build_dir()
{
	static std::filesystem::path cached;
	if (!cached.empty())
	{
		return cached;
	}

	for (auto workingDirectory = std::filesystem::current_path(); !workingDirectory.empty(); workingDirectory = workingDirectory.parent_path())
	{
		auto candidate = workingDirectory.parent_path() / "assets/png";
		if (std::filesystem::is_directory(candidate))
		{
			cached = std::move(workingDirectory);
			return cached;
		}
	}

	throw std::runtime_error("cannot find project root location");
}

int main()
{
	try
	{
		using namespace imf::core;

		auto cpuRuntime = make_runtime("cpu");

		auto imagePlaceholder = PlaceholderNode::make_variable(cpuRuntime->loadImage(build_dir().parent_path() / "assets/png/uv1k.png"));
		auto referencePlaceholder = PlaceholderNode::make_variable
		(
			cpuRuntime->loadImage(build_dir().parent_path() / "assets/test-references/gaussian-blur/uv1k_20.png")
			.transformed(translate(glm::vec2(-20.0f)))
		);

		auto toLinear = make_graph_node("ColorSpaceConvert");
		toLinear->setInput("image", imagePlaceholder->outputs().front());
		toLinear->setInput("sourceColorSpace", TypeQualifier::Constant, color::ColorSpace::sRGB);
		toLinear->setInput("targetColorSpace", TypeQualifier::Constant, color::ColorSpace::Linear_sRGB);

		auto blur = make_graph_node("GaussianBlur");
		blur->setInput("image", toLinear->outputs().front());
		blur->setInput("radius", TypeQualifier::Constant, glm::uvec2(20u));

		auto toGamma = make_graph_node("ColorSpaceConvert");
		toGamma->setInput("image", blur->outputs().front());
		toGamma->setInput("sourceColorSpace", TypeQualifier::Constant, color::ColorSpace::Linear_sRGB);
		toGamma->setInput("targetColorSpace", TypeQualifier::Constant, color::ColorSpace::sRGB);

		auto quantized = make_graph_node("TextureFormatConvert");
		quantized->setInput("image", toGamma->outputs().front());
		quantized->setInput("targetTextureFormat", TypeQualifier::Constant, TextureFormat::RGBA8);

		auto deltaImage = make_graph_node("DeltaImage", quantized->outputs().front(), referencePlaceholder->outputs().front());

		auto sink = SinkNode::make(deltaImage->outputs().front());
		auto psnr = make_graph_node("PeakSignalToNoiseRatio", deltaImage->outputs().front());
		auto sink2 = SinkNode::make(psnr->outputs().front());
		
		auto executor = GraphExecutor::make(cpuRuntime, sink, sink2);

		auto executionResult = executor->run();
		auto result = std::any_cast<Image&&>(std::move(executionResult.front().value));
		cpuRuntime->saveImage(std::move(result), build_dir() / "out.png");
		
		imf::core::log::info("psnr") << std::any_cast<float>(executionResult.back().value);
	}
	catch (const std::exception& ex)
	{
		imf::core::log::err("uncaught exception") << ex.what();
	}
}
