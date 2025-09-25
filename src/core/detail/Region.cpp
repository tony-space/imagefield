#include <imf/core/BoundingBox.hpp>
#include <imf/core/Region.hpp>

#if _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)
#pragma warning(disable : 4244)
#pragma warning(disable : 4100)
#elif __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-lambda-capture"
#endif

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/register/point.hpp>

#if _MSC_VER
#pragma warning(pop)
#elif __clang__
#pragma clang diagnostic pop
#endif

#include <mapbox/earcut.hpp>
#include <array>
#include <stdexcept>

BOOST_GEOMETRY_REGISTER_POINT_2D(glm::vec2, glm::vec2::value_type, boost::geometry::cs::cartesian, x, y)

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

class RegionImpl : public Region
{
public:
	using polygon_t = boost::geometry::model::polygon<glm::vec2>;
	using segment_t = boost::geometry::model::referring_segment<glm::vec2>;
	using multi_polygon_t = boost::geometry::model::multi_polygon<polygon_t>;
	using linestring_t = boost::geometry::model::linestring<glm::vec2>;

	RegionImpl(multi_polygon_t multiPolygon);
	RegionImpl(const BoundingBox&);
	RegionImpl(const RegionImpl&) = default;
	RegionImpl(RegionImpl&&) noexcept = default;
	
	std::shared_ptr<Region> copy() const override;

	const Triangulation& triangles() const override;
	const BoundingBox& boundingBox() const noexcept override;
	bool empty() const noexcept override;
	operator bool() const noexcept override;
	bool trivialRectangle() const noexcept override;
	void transformPoints(const glm::mat3& homogenousMat) noexcept override;

	friend std::shared_ptr<Region> region_difference(const Region& lhs, const Region& rhs);
	friend std::shared_ptr<Region> region_intersection(const Region& lhs, const Region& rhs);

private:
	bool anyOfPointsImpl(bool(*pFn)(const void* ctx, const glm::vec2&), const void* ctx) const override;

	multi_polygon_t m_multiPolygon;
	mutable std::optional<BoundingBox> m_cachedBox;
	mutable std::optional<Triangulation> m_cachedTriangulation;
};

RegionImpl::RegionImpl(multi_polygon_t multiPolygon) : m_multiPolygon(std::move(multiPolygon))
{

}

RegionImpl::RegionImpl(const BoundingBox& box)
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

std::shared_ptr<Region> RegionImpl::copy() const
{
	return std::make_shared<RegionImpl>(m_multiPolygon);
}

const Region::Triangulation& RegionImpl::triangles() const
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

const BoundingBox& RegionImpl::boundingBox() const noexcept
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

bool RegionImpl::empty() const noexcept
{
	return m_multiPolygon.empty();
}

RegionImpl::operator bool() const noexcept
{
	return !empty();
}

bool RegionImpl::trivialRectangle() const noexcept
{
	if (m_multiPolygon.size() != 1)
	{
		return false;
	}

	const auto& polygon = m_multiPolygon.front();

	if (polygon.inners().size() != 0)
	{
		return false;
	}
	if (polygon.outer().size() != 5)
	{
		return false;
	}

	const auto trivialBox = BoundingBox(1.0f, 1.0f);
	const auto& ring = polygon.outer();
	if (trivialBox.vec2<0>() != ring[0]) return false;
	if (trivialBox.vec2<3>() != ring[1]) return false;
	if (trivialBox.vec2<2>() != ring[2]) return false;
	if (trivialBox.vec2<1>() != ring[3]) return false;
	if (trivialBox.vec2<0>() != ring[4]) return false;

	return true;
}

void RegionImpl::transformPoints(const glm::mat3& homogenousMat) noexcept
{
	m_cachedBox.reset();
	m_cachedTriangulation.reset();

	boost::geometry::for_each_point(m_multiPolygon, [&](glm::vec2& v)
	{
		v = BoundingBox::transform(homogenousMat, v);
	});

	boost::geometry::correct(m_multiPolygon);
}

bool RegionImpl::anyOfPointsImpl(bool(*pFn)(const void* ctx, const glm::vec2&), const void* ctx) const
{
	for (const auto& polygon : m_multiPolygon)
	{
		for (const auto& point : polygon.outer())
		{
			if (pFn(ctx, point))
			{
				return true;
			}
		}

		for (const auto& ring : polygon.inners())
		{
			for (const auto& point : ring)
			{
				if (pFn(ctx, point))
				{
					return true;
				}
			}
		}
	}

	return false;
}

std::shared_ptr<Region> Region::make(const BoundingBox& box)
{
	return std::make_shared<RegionImpl>(box);
}

std::shared_ptr<Region> region_difference(const Region& lhs, const Region& rhs)
{
	RegionImpl::multi_polygon_t result;

	boost::geometry::difference(static_cast<const RegionImpl&>(lhs).m_multiPolygon, static_cast<const RegionImpl&>(rhs).m_multiPolygon, result);

	if (result.empty())
	{
		return nullptr;
	}

	return std::make_shared<RegionImpl>(std::move(result));
}

std::shared_ptr<Region> region_intersection(const Region& lhs, const Region& rhs)
{
	RegionImpl::multi_polygon_t result;

	boost::geometry::intersection(static_cast<const RegionImpl&>(lhs).m_multiPolygon, static_cast<const RegionImpl&>(rhs).m_multiPolygon, result);

	if (result.empty())
	{
		return nullptr;
	}

	return std::make_shared<RegionImpl>(std::move(result));
}

}
