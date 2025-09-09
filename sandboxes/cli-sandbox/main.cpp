#include <imf/core/core.hpp>

int main()
{
	try
	{
		using namespace imf::core;

		auto cpuRuntime = make_runtime("cpu");
		
		auto imgPlaceholder = PlaceholderNode::make_variable(cpuRuntime->loadImage("../assets/png/lenna.png"));
		auto transformPlaceholder = PlaceholderNode::make_variable(scale(0.15f, 0.15f) * rotate_deg(5.0f));

		auto toLinear = make_graph_node("ColorSpaceConvert");
		toLinear->setInput("image", imgPlaceholder->outputs().front());
		toLinear->setInput("sourceColorSpace", TypeQualifier::Constant, color::ColorSpace::sRGB);
		toLinear->setInput("targetColorSpace", TypeQualifier::Constant, color::ColorSpace::Linear_sRGB);

		auto affine = make_graph_node("Transform");
		affine->setInput("image", toLinear->outputs().front());
		affine->setInput("matrix", transformPlaceholder->outputs().front());

		auto toGamma = make_graph_node("ColorSpaceConvert");
		toGamma->setInput("image", affine->outputs().front());
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
