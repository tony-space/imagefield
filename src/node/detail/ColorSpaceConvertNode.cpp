#include <imf/core/ColorSpace.hpp>
#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/node/GraphNodeBase.hpp>

#include <imf/core/Image.hpp>
#include <imf/core/glm.hpp>

namespace imf::core
{

class ColorSpaceConvertNode : public GraphNodeBase<ColorSpaceConvertNode, 3>
{
public:
	constexpr static std::string_view operation_name = "ColorSpaceConvert";

	constexpr static std::string_view input_names[] =
	{
		"image",
		"sourceColorSpace",
		"targetColorSpace",
	};

	constexpr static TypeID input_types[] =
	{
		TypeID::make<Image>(),
		TypeID::make<color::ColorSpace>(),
		TypeID::make<color::ColorSpace>(),
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

DeclareGraphNode(ColorSpaceConvertNode);
