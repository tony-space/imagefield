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
	inline static const unique_id_t value = make_unique_id();
	constexpr static const unique_id_t* value_ptr = &value;
};

class TypeID
{
public:
	using value_type = const unique_id_t*;

	value_type value = nullptr;

	template <typename T>
	constexpr static TypeID make() noexcept;

	friend inline bool operator==(const TypeID& lhs, const TypeID& rhs) { return lhs.value == rhs.value; }
	friend inline bool operator!=(const TypeID& lhs, const TypeID& rhs) { return !(lhs == rhs); }
};

template <typename T>
constexpr TypeID TypeID::make() noexcept
{
	return
	{
		type_id_t<T>::value_ptr
	};
}

}
