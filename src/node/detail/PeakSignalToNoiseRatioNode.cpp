#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/node/GraphNodeBase.hpp>

namespace imf::core
{

class Image;

class PeakSignalToNoiseRatioNode : public GraphNodeBase<PeakSignalToNoiseRatioNode, 1>
{
public:
	constexpr static std::string_view operation_name = "PeakSignalToNoiseRatio";

	constexpr static std::string_view input_names[] =
	{
		"image",
	};

	constexpr static TypeID input_types[] =
	{
		TypeID::make<Image>(),
	};

	constexpr static std::string_view output_names[] =
	{
		"output"
	};

	constexpr static TypeID output_types[] =
	{
		TypeID::make<float>(),
	};

};

}

DeclareGraphNode(PeakSignalToNoiseRatioNode);
