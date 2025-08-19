#pragma once

#include <any>
#include <limits>
#include <vector>

namespace imf::core
{

class Registers
{
public:
	struct register_id_t
	{
		std::size_t value{ (std::numeric_limits<std::size_t>::max)() };

		constexpr explicit register_id_t(std::size_t v) : value(v) {}

		register_id_t() = default;
		register_id_t(const register_id_t&) = default;
		register_id_t(register_id_t&&) = default;
		register_id_t& operator=(const register_id_t&) = default;
		register_id_t& operator=(register_id_t&&) = default;

		constexpr bool empty() const noexcept
		{
			return value == (std::numeric_limits<std::size_t>::max)();
		}

		friend constexpr bool operator==(const Registers::register_id_t& lhs, const Registers::register_id_t& rhs) noexcept { return lhs.value == rhs.value; }
		friend constexpr bool operator!=(const Registers::register_id_t& lhs, const Registers::register_id_t& rhs) noexcept { return !(lhs == rhs); }
		friend constexpr bool operator< (const Registers::register_id_t& lhs, const Registers::register_id_t& rhs) noexcept { return lhs.value < rhs.value; }
		friend constexpr bool operator> (const Registers::register_id_t& lhs, const Registers::register_id_t& rhs) noexcept { return rhs < lhs; }
		friend constexpr bool operator<=(const Registers::register_id_t& lhs, const Registers::register_id_t& rhs) noexcept { return !(lhs > rhs); }
		friend constexpr bool operator>=(const Registers::register_id_t& lhs, const Registers::register_id_t& rhs) noexcept { return !(lhs < rhs); }
	};

	const std::any& get(const register_id_t& id) const
	{
		return m_registers.at(id.value);
	}

	std::any& get(const register_id_t& id)
	{
		return m_registers.at(id.value);
	}

	void set(const register_id_t& id, std::any&& value)
	{
		m_registers.resize(std::max(m_registers.size(), id.value + 1));
		m_registers[id.value] = std::move(value);
	}

	void set(const register_id_t& id, const std::any& value)
	{
		m_registers.resize(std::max(m_registers.size(), id.value + 1));
		m_registers[id.value] = value;
	}

	void reset(const register_id_t& id)
	{
		m_registers.at(id.value).reset();
	}

	void clear() noexcept
	{
		m_registers.clear();
	}
private:
	std::vector<std::any> m_registers;
};

}
