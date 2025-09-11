#include <imf/core/Image.hpp>
#include <imf/core/ITexture.hpp>
#include <imf/core/Region.hpp>

namespace imf::core
{

Image::LodSettings::LodSettings() :
	lodMin(0.0f),
	lodMax(1024.0f),
	lodBias(0.0f)
{
}

Image::Image
(
	std::shared_ptr<const ITexture> texture,
	const BoundingBox& boundingBox,
	std::shared_ptr<const Region> localRegion,
	ComponentMapping componentMapping,
	LodSettings lodSettings,
	std::optional<glm::mat3> uvToWorldMat
) :
	m_texture(std::move(texture)),
	m_localRegion(std::move(localRegion)),
	m_boundingBox(boundingBox),
	m_componentMapping(componentMapping),
	m_lodSettings(lodSettings)
{
	assert(m_texture);
	if (!m_texture)
	{
		throw std::invalid_argument("texture cannot be nullptr");
	}

	assert(m_boundingBox.finite());
	if (!m_boundingBox.finite())
	{
		throw std::invalid_argument("bounding box is infinite");
	}

	if (!m_localRegion)
	{
		m_localRegion = std::make_shared<const Region>(BoundingBox(1.0f, 1.0f));
	}

	if (!uvToWorldMat)
	{
		uvToWorldMat = calcUvToWorldMat(m_boundingBox);
	}

	m_uvToWorldMat = *uvToWorldMat;
	m_worldToUvMat = glm::inverse(m_uvToWorldMat);

	validateGeometry();
}

void Image::validateGeometry() const
{
	const auto consistent = m_localRegion->allOfPoints([&](const glm::vec2& v)
	{
		auto projected = projectToPlane(m_uvToWorldMat, v);
		bool inside = m_boundingBox.inside(projected);
		assert(inside);
		return inside;
	});

	if (!consistent)
	{
		throw std::invalid_argument("projected localRegion must be inside boundingBox");
	}
}

glm::mat3 Image::calcUvToWorldMat(const BoundingBox& box) noexcept
{
	const auto size = box.size();
	const auto t = box.min();
	return glm::mat3
	{
		size.x, 0.0f, 0.0f,
		0.0f, -size.y, 0.0f,
		t.x, t.y + size.y, 1.0f
	};

}

Image Image::transformed(const glm::mat3& homogenousMatrix) const
{
	auto resultBox = BoundingBox{};

	for (auto v : m_boundingBox.homogenousCorners())
	{
		resultBox.add(projectToPlane(homogenousMatrix, v));
	}

	return Image
	(
		m_texture,
		resultBox,
		m_localRegion,
		m_componentMapping,
		m_lodSettings,
		homogenousMatrix * m_uvToWorldMat
	);
}

Image Image::setComponentMapping(const ComponentMapping& mapping) const&
{
	auto copy = *this;
	copy.m_componentMapping = mapping;
	return copy;
}

Image Image::setComponentMapping(const ComponentMapping& mapping)&&
{
	this->m_componentMapping = mapping;
	return std::move(*this);
}

}
