#pragma once

#include <imf/core/unique_id_t.hpp>

namespace imf::core
{

template <typename T>
struct type_id
{
	using type = T;
	static const unique_id_t value;
};

template <typename T>
unique_id_t type_id_v = type_id<T>::value;


template <typename T>
const unique_id_t type_id<T>::value = make_unique_id();

}
