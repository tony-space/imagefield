#include <imf/core/ColorSpace.hpp>
#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/node/GraphNodeBase.hpp>

#include <imf/core/TextureFormat.hpp>
#include <imf/core/Image.hpp>
#include <imf/core/glm.hpp>

namespace imf::core
{

class TextureFormatConvertNode : public GraphNodeBase<TextureFormatConvertNode, 2>
{
public:
	constexpr static std::string_view operation_name = "TextureFormatConvert";

	constexpr static std::string_view input_names[] =
	{
		"image",
		"targetTextureFormat",
	};

	constexpr static TypeID input_types[] =
	{
		TypeID::make<Image>(),
		TypeID::make<TextureFormat>(),
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

DeclareGraphNode(TextureFormatConvertNode);
