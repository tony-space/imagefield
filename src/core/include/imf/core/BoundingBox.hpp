#pragma once

#include <imf/core/glm.hpp>

#include <array>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <stdexcept>

namespace imf::core
{

class BoundingBox
{
public:
	constexpr static auto kEpsilon = 1.0f / 128.0f;
	constexpr static auto kPrecision = 1.0f / kEpsilon;
	constexpr static auto positive_infinity = std::numeric_limits<glm::vec2::value_type>::infinity();
	constexpr static auto negative_infinity = -std::numeric_limits<glm::vec2::value_type>::infinity();

	BoundingBox() = default;
	BoundingBox(const BoundingBox&) = default;
	BoundingBox(BoundingBox&&) noexcept = default;

	BoundingBox& operator= (const BoundingBox&) = default;
	BoundingBox& operator= (BoundingBox&&) noexcept = default;

	constexpr BoundingBox(std::initializer_list<glm::vec2> points)
	{
		for (const auto& p : points)
		{
			add(p);
		}
	}
	
	constexpr BoundingBox(const glm::uvec2& dim) :
		m_min(0.0f, 0.0f),
		m_max(dim)
	{

	}

	constexpr BoundingBox(float width, float height) :
		m_min(0.0f, 0.0f),
		m_max(width, height)
	{
	}

	template<size_t idx>
	[[nodiscard]] constexpr glm::vec2 vec2() const noexcept
	{
		static_assert(idx < 4);

		if constexpr (idx == 0)
			return m_min;
		else if constexpr (idx == 1)
			return { m_max.x, m_min.y };
		else if constexpr (idx == 2)
			return m_max;
		else if constexpr (idx == 3)
			return { m_min.x, m_max.y };
	}

	[[nodiscard]] constexpr std::array<glm::vec3, 4> homogenousCorners() const noexcept
	{
		return
		{
			glm::vec3{ vec2<0>(), 1.0f },
			glm::vec3{ vec2<1>(), 1.0f },
			glm::vec3{ vec2<2>(), 1.0f },
			glm::vec3{ vec2<3>(), 1.0f },
		};
	}

	[[nodiscard]] constexpr glm::vec2 min() const noexcept
	{
		return m_min;
	}
	
	[[nodiscard]] constexpr glm::vec2 max() const noexcept
	{
		return m_max;
	}
	
	[[nodiscard]] constexpr glm::vec2 center() const noexcept
	{
		return (m_max + m_min) * 0.5f;
	}
	
	[[nodiscard]] constexpr glm::vec2 size() const noexcept
	{
		return m_max - m_min;
	}

	constexpr void add(const glm::vec2& point) noexcept
	{
		m_min = glm::min(m_min, point);
		m_max = glm::max(m_max, point);
	}

	[[nodiscard]] constexpr bool inside(const glm::vec2& point) const noexcept
	{
		auto outside = false;

		outside = outside || point.x > (m_max.x + kEpsilon);
		outside = outside || point.y > (m_max.y + kEpsilon);
		outside = outside || point.x < (m_min.x - kEpsilon);
		outside = outside || point.y < (m_min.y - kEpsilon);

		return !outside;
	}

	[[nodiscard]] constexpr bool overlaps(const BoundingBox& other) const noexcept
	{
		const auto center1 = center();
		const auto center2 = other.center();

		auto delta = glm::abs(center1 - center2) * 2.0f;
		auto totalSize = size() + other.size();

		return (delta.x < totalSize.x) && (delta.y < totalSize.y);
	}

	[[nodiscard]] constexpr bool finite() const noexcept
	{
		if (glm::any(glm::isinf(m_min))) return false;
		if (glm::any(glm::isinf(m_max))) return false;

		return true;
	}
	
	[[nodiscard]] constexpr glm::uvec2 textureSize() const
	{
		assert(finite());
		if (!finite())
		{
			throw std::runtime_error("bounding box is infinite");
		}

		return glm::ceil(size() * 0.5f) * 2.0f;
	}

	[[nodiscard]] constexpr BoundingBox transform(const glm::mat3& m) const noexcept
	{
		return
		{
			transform(m, vec2<0>()),
			transform(m, vec2<1>()),
			transform(m, vec2<2>()),
			transform(m, vec2<3>()),
		};
	}
	
	[[nodiscard]] static glm::vec2 transform(const glm::mat3& homogenousMat, glm::vec2 v)
	{
		const auto p = homogenousMat * glm::vec3(v, 1.0f);
		v = glm::vec2(p.x / p.z, p.y / p.z);
		v = glm::round(v * kPrecision) * kEpsilon;
		return v;
	}

	[[nodiscard]] static BoundingBox fromOrigin(glm::vec2 origin, glm::uvec2 size)
	{
		return BoundingBox
		{
			origin,
			origin + glm::vec2(size)
		};
	}

	glm::vec2 m_min{ positive_infinity };
	glm::vec2 m_max{ negative_infinity };
};

}
