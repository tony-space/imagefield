#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/node/GraphNodeBase.hpp>

namespace imf::core
{

class AffineTransformNode : public GraphNodeBase<AffineTransformNode, 2>
{
public:
	constexpr static std::string_view operation_name = "AffineTransform";
	
	constexpr static std::string_view input_names[] =
	{
		"image",
		"transformMat"
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
	
};

}

DeclareGraphNode(AffineTransformNode);
