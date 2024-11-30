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
}

Region::Region(multi_polygon_t p) :
	m_multiPolygon(std::move(p))
{

}

const Region::Triangulation& Region::triangles() const
{
	if (m_cachedTriangulation)
		return *m_cachedTriangulation;

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

		const Triangulation::index_type bias = static_cast<Triangulation::index_type>(result.vertices.size());
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

void Region::transformPoints(const glm::mat3& homogenousMat) noexcept
{
	boost::geometry::for_each_point(m_multiPolygon, [&](glm::vec2& v)
	{
		v = BoundingBox::transform(homogenousMat, v);
	});

	boost::geometry::correct(m_multiPolygon);
	m_cachedBox.reset();

	if (m_cachedTriangulation)
	{
		for (auto& v : m_cachedTriangulation->vertices)
		{
			v = BoundingBox::transform(homogenousMat, v);
		}
	}
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

Region shape_make_convex_and_expand_box(const Region& lhs, const glm::vec2& boxSize)
{
	constexpr static auto kEpsilon = 0.1f;

	Region::polygon_t convex;
	boost::geometry::convex_hull(lhs.m_multiPolygon, convex);

	std::vector<std::pair<glm::vec2, glm::vec2>> edges;

	boost::geometry::for_each_segment(convex, [&](const Region::segment_t& segm)
	{
		const auto dir = segm.second - segm.first;
		const auto center = 0.5f * dir + segm.first;
		const auto normal = glm::vec2(-dir.y, dir.x);

		const std::array<glm::vec2, 4> box1 = {
			segm.first + glm::vec2(boxSize.x, boxSize.y),
			segm.first + glm::vec2(boxSize.x, -boxSize.y),
			segm.first + glm::vec2(-boxSize.x, -boxSize.y),
			segm.first + glm::vec2(-boxSize.x, boxSize.y)
		};
		const std::array<glm::vec2, 4> box2 = {
			segm.second + glm::vec2(boxSize.x, boxSize.y),
			segm.second + glm::vec2(boxSize.x, -boxSize.y),
			segm.second + glm::vec2(-boxSize.x, -boxSize.y),
			segm.second + glm::vec2(-boxSize.x, boxSize.y)
		};

		const auto reducer = [&](const glm::vec2& lhs, const glm::vec2& rhs) -> glm::vec2
		{
			const auto projection1 = glm::dot(lhs, normal);
			const auto projection2 = glm::dot(rhs, normal);

			if (glm::abs(projection1 - projection2) < kEpsilon)
			{
				const auto delta1 = lhs - center;
				const auto delta2 = rhs - center;

				return glm::dot(delta1, delta1) > glm::dot(delta2, delta2) ? lhs : rhs;
			}

			return projection1 > projection2 ? lhs : rhs;
		};

		auto farthest1 = std::reduce(box1.begin() + 1, box1.end(), box1.front(), reducer);
		auto farthest2 = std::reduce(box2.begin() + 1, box2.end(), box2.front(), reducer);

		edges.emplace_back(farthest1, farthest2);
	});


	auto result = Region::multi_polygon_t{ Region::polygon_t { } };

	for (std::size_t i = 0, len = edges.size(); i < len; ++i)
	{
		const auto& edge1 = edges[i];
		const auto& edge2 = edges[(i + 1) % len];

		result.front().outer().emplace_back(edge1.second);

		if (glm::compMax(glm::abs(edge1.second - edge2.first)) > kEpsilon)
		{
			result.front().outer().emplace_back(edge2.first);
		}
	}

	result.front().outer().emplace_back(result.front().outer().front());

	return result;
}

}
