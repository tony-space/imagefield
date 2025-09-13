#include <imf/core/core.hpp>
#include <imf/core/node/node.hpp>

int main()
{
	try
	{
		using namespace imf::core;

		auto cpuRuntime = make_runtime("cpu");

		auto scalePlaceholder1 = PlaceholderNode::make_variable(8.0f);
		auto scalePlaceholder2 = PlaceholderNode::make_variable(4.0f);
		auto functorNode = FunctorNode::make([](float factor1, float factor2)
		{
			return glm::vec2(factor1, factor2);
		}, scalePlaceholder1->outputs().front(), scalePlaceholder2->outputs().front());
	
		auto toLinear = make_graph_node("ColorSpaceConvert");
		toLinear->setInput("image", TypeQualifier::Variable, cpuRuntime->loadImage("../assets/png/lenna.png"));
		toLinear->setInput("sourceColorSpace", TypeQualifier::Constant, color::ColorSpace::sRGB);
		toLinear->setInput("targetColorSpace", TypeQualifier::Constant, color::ColorSpace::Linear_sRGB);

		auto bicubic = make_graph_node("BicubicUpscale");
		bicubic->setInput("image", toLinear->outputs().front());
		bicubic->setInput("scale", functorNode->outputs().front());

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
