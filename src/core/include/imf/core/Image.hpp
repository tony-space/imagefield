#pragma once

#include <imf/core/BoundingBox.hpp>
#include <imf/core/ComponentMapping.hpp>
#include <imf/core/glm.hpp>

#include <memory>
#include <optional>
#include <utility>
#include <variant>

namespace imf::core
{

struct ITexture;
class Region;

class Image
{
public:
	struct LodSettings
	{
		glm::vec2 lodMin = { 0.0f, 0.0f };
		glm::vec2 lodMax = { 1024.0f, 1024.0f };
		glm::vec2 lodBias = { 0.0f, 0.0f };
	};

	/// <summary>
	/// Defines a <b>finite</b> image in world coordinates.
	/// </summary>
	/// <param name="texture">Device-specific memory containing the pixel data</param>
	/// <param name="boundingBox">Defines the AABB of the image in world coordinates</param>
	/// <param name="region">Optionally defines the region of the image to be used. Pixels outside the region are black and transparent</param>
	/// <param name="componentMapping">Optionally defines if the image components must be swizzled during readings</param>
	/// <param name="lodSettings">Optionally d level-of-detail limits and bias</param>
	/// <param name="uvToWorldMat">Optionally d conversion from normalized texture coordinates [0-1] to world coordinates</param>
	explicit Image
	(
		std::shared_ptr<const ITexture> texture,
		BoundingBox boundingBox,
		std::shared_ptr<const Region> region = {},
		ComponentMapping componentMapping = {},
		LodSettings lodSettings = LodSettings{},
		std::optional<glm::mat3> uvToWorldMat = {}
	);


	Image() = delete;
	Image(const Image&) = default;
	Image(Image&&) = default;

	Image& operator=(const Image&) = default;
	Image& operator=(Image&&) = default;

private:
	std::shared_ptr<const ITexture> m_texture;
	std::shared_ptr<const Region> m_region;
	BoundingBox m_boundingBox;
	ComponentMapping m_componentMapping;
	LodSettings m_lodSettings;
	glm::mat3 m_uvToWorldMat;
	glm::mat3 m_worldToUvMat;
};

}

