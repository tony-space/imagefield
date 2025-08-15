#include <imf/core/Image.hpp>
#include <imf/core/ITexture.hpp>
#include <imf/core/Region.hpp>

namespace imf::core
{

Image::LodSettings::LodSettings() :
	lodMin(0.0f, 0.0f),
	lodMax(1024.0f, 1024.0f),
	lodBias(0.0f, 0.0f)
{
}

Image::Image
(
	std::shared_ptr<const ITexture> texture,
	const BoundingBox& boundingBox,
	std::shared_ptr<const Region> region,
	ComponentMapping componentMapping,
	LodSettings lodSettings,
	std::optional<glm::mat3> uvToWorldMat
) :
	m_texture(std::move(texture)),
	m_region(std::move(region)),
	m_boundingBox(boundingBox),
	m_componentMapping(componentMapping),
	m_lodSettings(lodSettings)
{
	if (!m_texture)
	{
		throw std::invalid_argument("texture cannot be nullptr");
	}

	if (!m_boundingBox.finite())
	{
		throw std::invalid_argument("bounding box is infinite");
	}

	if (!m_region)
	{
		m_region = std::make_shared<const Region>(m_boundingBox);
	}

	validateGeometry();

	if (!uvToWorldMat)
	{
		uvToWorldMat = calcUvToWorldMat(m_boundingBox);
	}

	m_uvToWorldMat = *uvToWorldMat;
	m_worldToUvMat = glm::inverse(m_uvToWorldMat);

	initLevelOfDetail();

}

void Image::validateGeometry() const
{
	const auto regionBox = m_region->boundingBox();
	if (!m_boundingBox.inside(regionBox.min()) || !m_boundingBox.inside(regionBox.max()))
	{
		throw std::invalid_argument("region must be inside image bounding box");
	}
}

glm::mat3 Image::calcUvToWorldMat(const BoundingBox& extent) noexcept
{
	const auto size = extent.intSize();
	const auto t = extent.min();
	return glm::mat3
	{
		float(size.x), 0.0f, 0.0f,
		0.0f, -float(size.y), 0.0f,
		t.x, t.y + float(size.y), 1.0f
	};

}

void Image::initLevelOfDetail()
{
	// see 3.9.11 Texture Minification
	// https://www.khronos.org/registry/OpenGL/specs/gl/glspec42.core.pdf
	const auto originalDim = m_texture->dim().xy();

	auto jacobian = glm::transpose(glm::mat2
		{
			m_worldToUvMat[0].xy(),
			m_worldToUvMat[1].xy()
		});
	jacobian[0] *= float(originalDim.x);
	jacobian[1] *= float(originalDim.y);

	const auto dUlenSq = glm::dot(jacobian[0], jacobian[0]);
	const auto dVlenSq = glm::dot(jacobian[1], jacobian[1]);

	m_isoLevelOfDetail = 0.5f * glm::log2(glm::vec2(dUlenSq, dVlenSq)); //0.5f * log2(x) == log2(sqrt(x))
}

}
