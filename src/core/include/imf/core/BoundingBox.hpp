#pragma once

#include <imf/core/glm.hpp>

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <limits>

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
	template<size_t idx>
	constexpr glm::vec2 vec2() const noexcept
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
	constexpr bool finite() const noexcept
	{
		if (glm::any(glm::isinf(m_min))) return false;
		if (glm::any(glm::isinf(m_max))) return false;

		return true;
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

	glm::vec2 m_min{ positive_infinity };
	glm::vec2 m_max{ negative_infinity };
};

}
