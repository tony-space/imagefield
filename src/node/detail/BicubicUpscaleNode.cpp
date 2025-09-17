#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/node/GraphNodeBase.hpp>
#include <imf/core/node/PlaceholderNode.hpp>

#include <imf/core/Image.hpp>
#include <imf/core/glm.hpp>

namespace imf::core
{

class BicubicUpscaleNode final : public GraphNodeBase<BicubicUpscaleNode, 2>
{
public:
	constexpr static std::string_view operation_name = "BicubicUpscale";

	constexpr static std::string_view input_names[] =
	{
		"image",
		"scale",
	};

	constexpr static TypeID input_types[] =
	{
		TypeID::make<Image>(),
		TypeID::make<glm::vec2>(),
	};

	constexpr static std::string_view output_names[] =
	{
		"output"
	};

	constexpr static TypeID output_types[] =
	{
		TypeID::make<Image>(),
	};

	BicubicUpscaleNode()
	{
		m_inputs[1] = PlaceholderNode::make_constant(glm::vec2(1.0f))->outputs().front().sharedPtr();
	}
};

}

DeclareGraphNode(BicubicUpscaleNode);
