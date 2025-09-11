#include <imf/core/core.hpp>

int main()
{
	try
	{
		using namespace imf::core;

		auto cpuRuntime = make_runtime("cpu");
	
		auto toLinear = make_graph_node("ColorSpaceConvert");
		toLinear->setInput("image", TypeQualifier::Variable, cpuRuntime->loadImage("../assets/png/noise128.png"));
		toLinear->setInput("sourceColorSpace", TypeQualifier::Constant, color::ColorSpace::sRGB);
		toLinear->setInput("targetColorSpace", TypeQualifier::Constant, color::ColorSpace::Linear_sRGB);

		auto bicubic = make_graph_node("BicubicUpscale");
		bicubic->setInput("image", toLinear->outputs().front());
		bicubic->setInput("scale", TypeQualifier::Constant, glm::vec2(8.0f, 8.0f));

		auto toGamma = make_graph_node("ColorSpaceConvert");
		toGamma->setInput("image", bicubic->outputs().front());
		toGamma->setInput("sourceColorSpace", TypeQualifier::Constant, color::ColorSpace::Linear_sRGB);
		toGamma->setInput("targetColorSpace", TypeQualifier::Constant, color::ColorSpace::sRGB);

		auto sink = SinkNode::make(toGamma->outputs().front());
		
		auto executor = GraphExecutor::make(cpuRuntime, sink);

		auto executionResult = executor->run();
		auto result = std::any_cast<Image&&>(std::move(executionResult.front().value));
		cpuRuntime->saveImage(std::move(result), "out.png");
	}
	catch (const std::exception& ex)
	{
		imf::core::log::err("uncaught exception") << ex.what();
	}

}
