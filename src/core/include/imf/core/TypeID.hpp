#pragma once

#include <imf/core/unique_id_t.hpp>

#include <functional>
#include <typeinfo>

namespace imf::core
{

template <typename T>
struct type_id_t
{
	using type = T;
	static const unique_id_t value;
};

template <typename T>
unique_id_t type_id_v = type_id_t<T>::value;


template <typename T>
const unique_id_t type_id_t<T>::value = make_unique_id();

class TypeID
{
public:
	using value_type = unique_id_t;

	const value_type value = empty_unique_id;

	template <typename T> static TypeID make() noexcept;

	friend inline bool operator==(const TypeID& lhs, const TypeID& rhs) { return lhs.value == rhs.value; }
	friend inline bool operator!=(const TypeID& lhs, const TypeID& rhs) { return !(lhs == rhs); }
};

template <typename T>
TypeID TypeID::make() noexcept
{
	// type id value is calculated during global initialization phase
	return
	{
		type_id_v<T>
	};
}

}

template<>
struct std::hash<imf::core::TypeID>
{
	std::size_t operator()(const imf::core::TypeID& s) const noexcept
	{
		return std::hash<imf::core::TypeID::value_type>{}(s.value);
	}
};
