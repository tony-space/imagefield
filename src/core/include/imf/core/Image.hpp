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
		glm::vec2 lodMin;
		glm::vec2 lodMax;
		glm::vec2 lodBias;

		LodSettings();
	};

	/// <summary>
	/// Defines a <b>finite</b> image in world coordinates.
	/// </summary>
	/// <param name="texture">Device-specific memory containing the pixel data</param>
	/// <param name="boundingBox">Defines the AABB of the image in world coordinates</param>
	/// <param name="region">Optionally defines the region of the image to be used. Pixels outside the region are black and transparent</param>
	/// <param name="componentMapping">Optionally defines if the image components must be swizzled during readings</param>
	/// <param name="lodSettings">Optionally defines level-of-detail limits and bias</param>
	/// <param name="uvToWorldMat">Optionally defines conversion from normalized texture coordinates [0-1] to world coordinates</param>
	explicit Image
	(
		std::shared_ptr<const ITexture> texture,
		const BoundingBox& boundingBox,
		std::shared_ptr<const Region> region = {},
		ComponentMapping componentMapping = {},
		LodSettings lodSettings = {},
		std::optional<glm::mat3> uvToWorldMat = {}
	);


	Image() = delete;
	Image(const Image&) = default;
	Image(Image&&) = default;

	Image& operator=(const Image&) = default;
	Image& operator=(Image&&) = default;

	[[nodiscard]] const auto& texture() const noexcept { return m_texture; }
	[[nodiscard]] const auto& region() const noexcept { return m_region; }
	[[nodiscard]] const auto& boundingBox() const noexcept { return m_boundingBox; }
	[[nodiscard]] const auto& componentMapping() const noexcept { return m_componentMapping; }
	[[nodiscard]] const auto& lodSettings() const noexcept { return m_lodSettings; }
	[[nodiscard]] const auto& isotropicLevelOfDetail() const noexcept { return m_isoLevelOfDetail; }
	[[nodiscard]] const auto& uvToWorldMat() const noexcept { return m_uvToWorldMat; }
	[[nodiscard]] const auto& worldToUvMat() const noexcept { return m_worldToUvMat; }

private:
	std::shared_ptr<const ITexture> m_texture;
	std::shared_ptr<const Region> m_region;
	BoundingBox m_boundingBox;
	ComponentMapping m_componentMapping;
	LodSettings m_lodSettings;
	glm::vec2 m_isoLevelOfDetail;
	glm::mat3 m_uvToWorldMat;
	glm::mat3 m_worldToUvMat;

	void validateGeometry() const;
	void initLevelOfDetail();

	static glm::mat3 calcUvToWorldMat(const BoundingBox& extent) noexcept;
};

}
