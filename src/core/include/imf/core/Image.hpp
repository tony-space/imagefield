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
		float lodMin;
		float lodMax;
		float lodBias;

		LodSettings();
	};

	explicit Image
	(
		std::shared_ptr<const ITexture> texture,
		const BoundingBox& boundingBox,
		std::shared_ptr<const Region> localRegion = {},
		ComponentMapping componentMapping = {},
		LodSettings lodSettings = {},
		std::optional<glm::mat3> uvToWorldMat = {}
	);


	Image() = delete;
	Image(const Image&) = default;
	Image(Image&&) = default;

	Image& operator=(const Image&) = default;
	Image& operator=(Image&&) = default;

	// Device-specific memory containing image data
	[[nodiscard]] const auto& texture() const noexcept { return m_texture; }

	// Polygonal domain of definiton in local coordinate system. Outside the region pixels are black and transparent.
	[[nodiscard]] const auto& localRegion() const noexcept { return m_localRegion; }

	// Bounding box in world space. Consider using localRegion()->boundingBox() for local space bounding box.
	[[nodiscard]] const auto& boundingBox() const noexcept { return m_boundingBox; }

	// Defines how texture channels should be interpreted during sampling
	[[nodiscard]] const auto& componentMapping() const noexcept { return m_componentMapping; }

	// Defines level-of-detail settings for texture sampling
	[[nodiscard]] const auto& lodSettings() const noexcept { return m_lodSettings; }

	// Homogenous matrix transforming UV coordinates to world space
	[[nodiscard]] const auto& uvToWorldMat() const noexcept { return m_uvToWorldMat; }

	// Homogenous matrix transforming world space coordinates to UV space
	[[nodiscard]] const auto& worldToUvMat() const noexcept { return m_worldToUvMat; }

	// Returns a copy of this image transformed by the given homogenous matrix in world space
	[[nodiscard]] Image transformed(const glm::mat3& homogenousMat) const;

	// Returns a copy of this image with the given component mapping
	[[nodiscard]] Image setComponentMapping(const ComponentMapping& mapping) const &;
	
	// Sets the component mapping of this image and returns it
	[[nodiscard]] Image setComponentMapping(const ComponentMapping& mapping) &&;

	static glm::mat3 calcUvToWorldMat(const BoundingBox& box) noexcept;
private:
	std::shared_ptr<const ITexture> m_texture;
	std::shared_ptr<const Region> m_localRegion;
	BoundingBox m_boundingBox;
	ComponentMapping m_componentMapping;
	LodSettings m_lodSettings;
	glm::mat3 m_uvToWorldMat;
	glm::mat3 m_worldToUvMat;

	void validateGeometry() const;
};

}
