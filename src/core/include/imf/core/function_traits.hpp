#pragma once

#include <utility>

namespace imf::core
{

template <typename ...Types>
struct type_list_t
{
	constexpr static auto size = sizeof...(Types);
};

template<typename...>
struct function_traits;

template<typename Callable>
struct function_traits<Callable> : function_traits<decltype(&Callable::operator())>
{
};

template<typename Class, typename Rtype, typename ...Args>
struct function_traits<Rtype(Class::*)(Args...) const>
{
	using return_type = Rtype;
	using class_type = Class;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool const_v = true;
	static constexpr bool noexcept_v = false;
	static constexpr bool method_v = true;
};

template<typename Class, typename Rtype, typename ...Args>
struct function_traits<Rtype(Class::*)(Args...)>
{
	using return_type = Rtype;
	using class_type = Class;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool const_v = false;
	static constexpr bool noexcept_v = false;
	static constexpr bool method_v = true;
};

template<typename Rtype, typename ...Args>
struct function_traits<Rtype(Args...)>
{
	using return_type = Rtype;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool noexcept_v = false;
	static constexpr bool method_v = false;
};

template<typename Rtype, typename ...Args>
struct function_traits<Rtype(&)(Args...)>
{
	using return_type = Rtype;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool noexcept_v = false;
	static constexpr bool method_v = false;
};

template<typename Rtype, typename ...Args>
struct function_traits<Rtype(*)(Args...)>
{
	using return_type = Rtype;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool noexcept_v = false;
	static constexpr bool method_v = false;
};

//noexcept part

template<typename Class, typename Rtype, typename ...Args>
struct function_traits<Rtype(Class::*)(Args...) const noexcept>
{
	using return_type = Rtype;
	using class_type = Class;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool const_v = true;
	static constexpr bool noexcept_v = true;
	static constexpr bool method_v = true;
};

template<typename Class, typename Rtype, typename ...Args>
struct function_traits<Rtype(Class::*)(Args...) noexcept>
{
	using return_type = Rtype;
	using class_type = Class;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool const_v = false;
	static constexpr bool noexcept_v = true;
	static constexpr bool method_v = true;
};

template<typename Rtype, typename ...Args>
struct function_traits<Rtype(Args...) noexcept>
{
	using return_type = Rtype;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool noexcept_v = true;
	static constexpr bool method_v = false;
};

template<typename Rtype, typename ...Args>
struct function_traits<Rtype(&)(Args...) noexcept>
{
	using return_type = Rtype;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool noexcept_v = true;
	static constexpr bool method_v = false;
};

template<typename Rtype, typename ...Args>
struct function_traits<Rtype(*)(Args...) noexcept>
{
	using return_type = Rtype;
	using args_types = type_list_t<Args...>;
	static constexpr std::size_t args_count = sizeof...(Args);
	static constexpr bool noexcept_v = true;
	static constexpr bool method_v = false;
};

}
