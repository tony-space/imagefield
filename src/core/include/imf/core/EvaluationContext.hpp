#pragma once

#include <any>
#include <limits>
#include <vector>

namespace imf::core
{

class EvaluationContext
{
public:
	struct element_id_t
	{
		std::size_t value{ (std::numeric_limits<std::size_t>::max)() };

		constexpr explicit element_id_t(std::size_t v) : value(v) {}

		element_id_t() = default;
		element_id_t(const element_id_t&) = default;
		element_id_t(element_id_t&&) = default;
		element_id_t& operator=(const element_id_t&) = default;
		element_id_t& operator=(element_id_t&&) = default;

		constexpr bool empty() const noexcept
		{
			return value == (std::numeric_limits<std::size_t>::max)();
		}

		friend constexpr bool operator==(const EvaluationContext::element_id_t& lhs, const EvaluationContext::element_id_t& rhs) noexcept { return lhs.value == rhs.value; }
		friend constexpr bool operator!=(const EvaluationContext::element_id_t& lhs, const EvaluationContext::element_id_t& rhs) noexcept { return !(lhs == rhs); }
		friend constexpr bool operator< (const EvaluationContext::element_id_t& lhs, const EvaluationContext::element_id_t& rhs) noexcept { return lhs.value < rhs.value; }
		friend constexpr bool operator> (const EvaluationContext::element_id_t& lhs, const EvaluationContext::element_id_t& rhs) noexcept { return rhs < lhs; }
		friend constexpr bool operator<=(const EvaluationContext::element_id_t& lhs, const EvaluationContext::element_id_t& rhs) noexcept { return !(lhs > rhs); }
		friend constexpr bool operator>=(const EvaluationContext::element_id_t& lhs, const EvaluationContext::element_id_t& rhs) noexcept { return !(lhs < rhs); }
	};

	const std::any& get(const element_id_t& id) const
	{
		return m_elements.at(id.value);
	}

	std::any& get(const element_id_t& id)
	{
		return m_elements.at(id.value);
	}

	void set(const element_id_t& id, std::any&& value)
	{
		m_elements.resize(std::max(m_elements.size(), id.value + 1));
		m_elements[id.value] = std::move(value);
	}

	void set(const element_id_t& id, const std::any& value)
	{
		m_elements.resize(std::max(m_elements.size(), id.value + 1));
		m_elements[id.value] = value;
	}

	void reset(const element_id_t& id)
	{
		m_elements.at(id.value).reset();
	}

	void clear() noexcept
	{
		m_elements.clear();
	}
private:
	std::vector<std::any> m_elements;
};

}
