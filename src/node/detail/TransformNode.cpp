#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/node/GraphNodeBase.hpp>
#include <imf/core/node/PlaceholderNode.hpp>

namespace imf::core
{

class TransformNode : public GraphNodeBase<TransformNode, 2>
{
public:
	constexpr static std::string_view operation_name = "Transform";
	
	constexpr static std::string_view input_names[] =
	{
		"image",
		"matrix"
	};

	constexpr static TypeID input_types[] =
	{
		TypeID::make<Image>(),
		TypeID::make<glm::mat3>()
	};

	constexpr static std::string_view output_names[] =
	{
		"output"
	};

	constexpr static TypeID output_types[] =
	{
		TypeID::make<Image>(),
	};

	TransformNode()
	{
		m_inputs[1] = PlaceholderNode::make_constant(glm::mat3(1.0f))->outputs().front().sharedPtr();
	}
	
};

}

DeclareGraphNode(TransformNode);
