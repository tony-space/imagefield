#include <imf/core/GraphNodeFactory.hpp>

#include <imf/core/node/ChildGraphBase.hpp>
#include <imf/core/node/PlaceholderNode.hpp>
#include <imf/core/node/FunctorNode.hpp>

#include <imf/core/Image.hpp>
#include <imf/core/glm.hpp>

namespace imf::core
{

class GaussianBlur1DNode final : public GraphNodeBase<GaussianBlur1DNode, 3>
{
public:
	constexpr static std::string_view operation_name = "GaussianBlur1D";

	constexpr static std::string_view input_names[] =
	{
		"image",
		"kernelRadius",
		"horizontal"
	};

	constexpr static TypeID input_types[] =
	{
		TypeID::make<Image>(),
		TypeID::make<unsigned>(),
		TypeID::make<bool>()
	};

	constexpr static std::string_view output_names[] =
	{
		"output"
	};

	constexpr static TypeID output_types[] =
	{
		TypeID::make<Image>(),
	};
};

class GaussianBlurNode : public ChildGraphBase<GaussianBlurNode, 2>
{
public:
	constexpr static std::string_view operation_name = "GaussianBlur";

	constexpr static std::string_view input_names[] =
	{
		"image",
		"radius"
	};
	constexpr static TypeID input_types[] =
	{
		TypeID::make<Image>(),
		TypeID::make<glm::uvec2>()
	};

	constexpr static std::string_view output_names[] =
	{
		"output"
	};

	constexpr static TypeID output_types[] =
	{
		TypeID::make<Image>(),
	};

	static constexpr unsigned kTargetRadius = 5;

	GaussianBlurNode()
	{
		auto getScale = FunctorNode::make([](glm::uvec2 inputRadius)
		{
			glm::vec2 downscale(1.0f);

			if (inputRadius.x > kTargetRadius)
			{
				downscale.x = static_cast<float>(kTargetRadius) / static_cast<float>(inputRadius.x);
			}
			if (inputRadius.y > kTargetRadius)
			{
				downscale.y = static_cast<float>(kTargetRadius) / static_cast<float>(inputRadius.y);
			}

			glm::vec2 upscale = glm::vec2(1.0f) / downscale;

			glm::mat3 downscaleMat = scale(downscale);

			glm::uvec2 kernelRadius(glm::ceil(glm::vec2(inputRadius) * downscale));

			return std::make_tuple(downscaleMat, kernelRadius.x, kernelRadius.y, upscale);

		}, m_inputReferences[1]->outputs().front());

		const auto downscaleTransform = make_graph_node("Transform");
		downscaleTransform->setInput("image", m_inputReferences[0]->outputs().front());
		downscaleTransform->setInput("matrix", getScale->outputs()[0]);

		const auto horizontalBlur = make_graph_node<GaussianBlur1DNode>();
		horizontalBlur->setInput("image", downscaleTransform->outputs().front());
		horizontalBlur->setInput("kernelRadius", getScale->outputs()[1]);
		horizontalBlur->setInput("horizontal", core::TypeQualifier::Constant, true);

		const auto verticalBlur = make_graph_node<GaussianBlur1DNode>();
		verticalBlur->setInput("image", horizontalBlur->outputs().front());
		verticalBlur->setInput("kernelRadius", getScale->outputs()[2]);
		verticalBlur->setInput("horizontal", core::TypeQualifier::Constant, false);

		const auto bicubicUpscale = make_graph_node("BicubicUpscale");
		bicubicUpscale->setInput("image", verticalBlur->outputs().front());
		bicubicUpscale->setInput("scale", getScale->outputs()[3]);

		m_outputReferences[0]->setInput("arg", bicubicUpscale->outputs().front());
	}
};

}

DeclareGraphNode(GaussianBlurNode);
