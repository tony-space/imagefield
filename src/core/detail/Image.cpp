#include <imf/core/Image.hpp>

namespace imf::core
{

Image::Image
(
	std::shared_ptr<const ITexture> /*texture*/,
	BoundingBox /*boundingBox*/,
	std::shared_ptr<const Region> /*region*/,
	ComponentMapping /*componentMapping*/,
	LodSettings /*lodSettings*/,
	std::optional<glm::mat3> /*uvToWorldMat*/
)
{

}

}
