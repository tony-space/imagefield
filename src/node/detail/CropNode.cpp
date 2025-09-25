#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/node/GraphNodeBase.hpp>

namespace imf::core
{

class Image;
class BoundingBox;

class CropNode : public GraphNodeBase<CropNode, 2>
{
public:
	constexpr static std::string_view operation_name = "Crop";

	constexpr static std::string_view input_names[] =
	{
		"image",
		"box"
	};

	constexpr static TypeID input_types[] =
	{
		TypeID::make<Image>(),
		TypeID::make<BoundingBox>(),
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

DeclareGraphNode(CropNode);
