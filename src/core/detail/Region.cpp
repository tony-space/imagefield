#include <imf/core/Region.hpp>

#include <mapbox/earcut.hpp>
#include <array>
#include <stdexcept>

namespace mapbox::util
{

template <>
struct nth<0, glm::vec2>
{
	inline static auto get(const glm::vec2& t)
	{
		return t.x;
	};
};
template <>
struct nth<1, glm::vec2> {
	inline static auto get(const glm::vec2& t)
	{
		return t.y;
	};
};

}

namespace imf::core
{

Region::Region(const BoundingBox& box)
{
	assert(box.finite());
	if (!box.finite())
	{
		throw std::invalid_argument("box must be finite");
	}

	const auto points =
	{
		box.vec2<0>(),
		box.vec2<3>(),
		box.vec2<2>(),
		box.vec2<1>(),

		box.vec2<0>()
	};

	m_multiPolygon = multi_polygon_t
	{
		polygon_t { points }
	};
	m_cachedBox = box;
}

Region::Region(multi_polygon_t p) :
	m_multiPolygon(std::move(p))
{

}

const Region::Triangulation& Region::triangles() const
{
	if (m_cachedTriangulation)
	{
		return *m_cachedTriangulation;
	}

	Triangulation result;

	std::vector<std::vector<glm::vec2>> rings;
	std::vector<Triangulation::index_type> indices;

	for (const polygon_t& polygon : m_multiPolygon)
	{
		rings.resize(1 + polygon.inners().size());
		rings.front() = polygon.outer();

		std::copy(polygon.inners().begin(), polygon.inners().end(), rings.begin() + 1);

		for (auto& ring : rings)
		{
			ring.pop_back();
		}

		indices = mapbox::earcut<Triangulation::index_type>(rings);

		const auto bias = static_cast<Triangulation::index_type>(result.vertices.size());
		for (std::size_t i = 0, len = indices.size(); i < len; i = i + 3)
		{
			result.indices.emplace_back(Triangulation::index_triple
			{
				bias + indices[i + 0],
				bias + indices[i + 1],
				bias + indices[i + 2]
			});
		}

		for (auto& ring : rings)
		{
			std::copy(ring.begin(), ring.end(), std::back_inserter(result.vertices));
		}
	}

	m_cachedTriangulation = std::move(result);
	return *m_cachedTriangulation;
}

const BoundingBox& Region::boundingBox() const noexcept
{
	if (!m_cachedBox)
	{
		BoundingBox result;

		boost::geometry::for_each_point(m_multiPolygon, [&](const glm::vec2& v)
		{
			result.add(v);
		});

		m_cachedBox = result;
	}

	return *m_cachedBox;
}

const Region::multi_polygon_t& Region::multiPolygon() const noexcept
{
	return m_multiPolygon;
}

bool Region::empty() const noexcept
{
	return m_multiPolygon.empty();
}

Region::operator bool() const noexcept
{
	return !empty();
}

bool Region::trivialRectangle() const noexcept
{
	if (m_multiPolygon.size() != 1) return false;

	const auto& polygon = m_multiPolygon[0];

	if (polygon.inners().size() != 0) return false;
	if (polygon.outer().size() != 5) return false;
	if (polygon.outer().front() != polygon.outer().back()) return false;

	const auto& polygonBoundingBox = boundingBox();
	for (const auto& p : polygon.outer())
	{
		bool anyOf =
			polygonBoundingBox.vec2<0>() == p ||
			polygonBoundingBox.vec2<1>() == p ||
			polygonBoundingBox.vec2<2>() == p ||
			polygonBoundingBox.vec2<3>() == p;

		if (!anyOf) return false;
	}

	return true;
}

Region shape_difference(const Region& lhs, const Region& rhs)
{
	Region::multi_polygon_t result;

	boost::geometry::difference(lhs.m_multiPolygon, rhs.m_multiPolygon, result);

	return { std::move(result) };
}

Region shape_intersection(const Region& lhs, const Region& rhs)
{
	Region::multi_polygon_t result;

	boost::geometry::intersection(lhs.m_multiPolygon, rhs.m_multiPolygon, result);

	return { std::move(result) };
}

Region shape_symmetric_difference(const Region& lhs, const Region& rhs)
{
	Region::multi_polygon_t result;

	boost::geometry::sym_difference(lhs.m_multiPolygon, rhs.m_multiPolygon, result);

	return { std::move(result) };
}

Region shape_union(const Region& lhs, const Region& rhs)
{
	Region::multi_polygon_t result;

	boost::geometry::union_(lhs.m_multiPolygon, rhs.m_multiPolygon, result);

	return { std::move(result) };
}

}
