#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/register/point.hpp>

#include <imf/core/BoundingBox.hpp>

#include <optional>
#include <vector>

BOOST_GEOMETRY_REGISTER_POINT_2D(glm::vec2, glm::vec2::value_type, boost::geometry::cs::cartesian, x, y)

namespace imf::core
{

class Region
{
public:
	using polygon_t = boost::geometry::model::polygon<glm::vec2>;
	using segment_t = boost::geometry::model::referring_segment<glm::vec2>;
	using multi_polygon_t = boost::geometry::model::multi_polygon<polygon_t>;
	using linestring_t = boost::geometry::model::linestring<glm::vec2>;

	struct Triangulation
	{
		using index_triple = glm::uvec3;
		using index_type = index_triple::value_type;

		std::vector<glm::vec2> vertices;
		std::vector<index_triple> indices;
	};

	Region(const BoundingBox&);
	Region(multi_polygon_t);
	Region(const Region&) = default;
	Region(Region&&) noexcept = default;

	Region& operator=(const Region&) = default;
	Region& operator=(Region&&) noexcept = default;

	const Triangulation& triangles() const;
	void transformPoints(const glm::mat3& homogenousMat) noexcept;
	const BoundingBox& boundingBox() const noexcept;
	const multi_polygon_t& multiPolygon() const noexcept;

	bool empty() const noexcept;
	operator bool() const noexcept;
	bool trivialRectangle() const noexcept;



	friend Region shape_difference(const Region& lhs, const Region& rhs);
	friend Region shape_intersection(const Region& lhs, const Region& rhs);
	friend Region shape_symmetric_difference(const Region& lhs, const Region& rhs);
	friend Region shape_union(const Region& lhs, const Region& rhs);
	friend Region shape_make_convex_and_expand_box(const Region& lhs, const glm::vec2& boxSize);

	template <typename Iterator>
	static Region make_convex(Iterator beginIt, Iterator endIt);
	static Region make_convex(std::initializer_list<glm::vec2> l)
	{
		return make_convex(std::cbegin(l), std::cend(l));
	}

	template <typename Iterator>
	static Region make_concave(Iterator beginIt, Iterator endIt);
	static Region make_concave(std::initializer_list<glm::vec2> l)
	{
		return make_concave(std::cbegin(l), std::cend(l));
	}

private:
	multi_polygon_t m_multiPolygon;
	mutable std::optional<BoundingBox> m_cachedBox;
	mutable std::optional<Triangulation> m_cachedTriangulation;
};

template <typename Iterator>
Region Region::make_convex(Iterator beginIt, Iterator endIt)
{
	auto lineString = linestring_t(beginIt, endIt);
	auto hull = polygon_t();
	boost::geometry::convex_hull(lineString, hull);

	return multi_polygon_t{ std::move(hull) };
}

template <typename Iterator>
Region Region::make_concave(Iterator beginIt, Iterator endIt)
{
	auto polygon = polygon_t();

	polygon.outer().resize(std::distance(beginIt, endIt) + 1);

	std::copy(beginIt, endIt, polygon.outer().begin());
	polygon.outer().back() = *beginIt;

	boost::geometry::correct(polygon);
	if (!boost::geometry::is_valid(polygon))
	{
		throw std::invalid_argument("polygon self-intersects");
	}

	return multi_polygon_t{ std::move(polygon) };
}

Region shape_difference(const Region& lhs, const Region& rhs);
Region shape_intersection(const Region& lhs, const Region& rhs);
Region shape_symmetric_difference(const Region& lhs, const Region& rhs);
Region shape_union(const Region& lhs, const Region& rhs);
Region shape_make_convex_and_expand_box(const Region& lhs, const glm::vec2& boxSize);

}
