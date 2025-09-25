#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/node/GraphNodeBase.hpp>

#include <imf/core/glm.hpp>

namespace imf::core
{

class Image;

class DeltaImageNode : public GraphNodeBase<DeltaImageNode, 2>
{
public:
	constexpr static std::string_view operation_name = "DeltaImage";

	constexpr static std::string_view input_names[] =
	{
		"image1",
		"image2"
	};

	constexpr static TypeID input_types[] =
	{
		TypeID::make<Image>(),
		TypeID::make<Image>()
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

DeclareGraphNode(DeltaImageNode);
