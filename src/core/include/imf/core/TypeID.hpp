#pragma once

#include <imf/core/unique_id_t.hpp>

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
	const unique_id_t value = empty_unique_id;

	template <typename T> static TypeID make() noexcept;
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
