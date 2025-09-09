#include <imf/core/core.hpp>

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
		cpuRuntime->saveImage(std::move(result), "out.png");
	}
	catch (const std::exception& ex)
	{
		imf::core::log::err("uncaught exception") << ex.what();
	}

}
