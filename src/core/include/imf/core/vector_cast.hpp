#pragma once

#include <imf/core/glm.hpp>

namespace imf::core
{
template<typename T>
constexpr glm::vec4 to_vec4_norm(const glm::vec<1, T, glm::defaultp>& orig)
{
	auto x = static_cast<float>(orig.x);
	if constexpr (!std::is_same_v<float, T>)
	{
		x = x / static_cast<float>(std::numeric_limits<T>::max());
	}

	return { x, 0.0f, 0.0f, 1.0f };
}

template<typename T>
constexpr glm::vec4 to_vec4_norm(const glm::vec<2, T, glm::defaultp>& orig)
{
	auto x = static_cast<float>(orig.x);
	auto y = static_cast<float>(orig.y);
	if constexpr (!std::is_same_v<float, T>)
	{
		x = x / static_cast<float>(std::numeric_limits<T>::max());
		y = y / static_cast<float>(std::numeric_limits<T>::max());
	}

	return { x, y, 0.0f, 1.0f };
}

template<typename T>
constexpr glm::vec4 to_vec4_norm(const glm::vec<3, T, glm::defaultp>& orig)
{
	auto x = static_cast<float>(orig.x);
	auto y = static_cast<float>(orig.y);
	auto z = static_cast<float>(orig.z);
	if constexpr (!std::is_same_v<float, T>)
	{
		x = x / static_cast<float>(std::numeric_limits<T>::max());
		y = y / static_cast<float>(std::numeric_limits<T>::max());
		z = z / static_cast<float>(std::numeric_limits<T>::max());
	}

	return { x, y, z, 1.0f };
}

template<typename T>
constexpr glm::vec4 to_vec4_norm(const glm::vec<4, T, glm::defaultp>& orig)
{
	auto x = static_cast<float>(orig.x);
	auto y = static_cast<float>(orig.y);
	auto z = static_cast<float>(orig.z);
	auto w = static_cast<float>(orig.w);
	if constexpr (!std::is_same_v<float, T>)
	{
		x = x / static_cast<float>(std::numeric_limits<T>::max());
		y = y / static_cast<float>(std::numeric_limits<T>::max());
		z = z / static_cast<float>(std::numeric_limits<T>::max());
		w = w / static_cast<float>(std::numeric_limits<T>::max());
	}

	return { x, y, z, w };
}

namespace detail
{
template<typename>
struct from_vec4_norm;

template<typename T>
struct from_vec4_norm<glm::vec<1, T, glm::defaultp>>
{
	using result_type = glm::vec<1, T, glm::defaultp>;
	constexpr static result_type invoke(glm::vec4 orig)
	{
		if constexpr (!std::is_same_v<float, T>)
		{
			orig = glm::min(orig, glm::vec4{ 1.0f });
			if constexpr (std::is_unsigned_v<T>)
			{
				orig = glm::max(orig, glm::vec4{ 0.0f });
			}

			const auto x = static_cast<T>(glm::round(orig.x * static_cast<float>(std::numeric_limits<T>::max())));
			return result_type{ x };
		}
		else
		{
			return result_type{ orig.x };
		}
	}
};

template<typename T>
struct from_vec4_norm<glm::vec<2, T, glm::defaultp>>
{
	using result_type = glm::vec<2, T, glm::defaultp>;
	constexpr static result_type invoke(glm::vec4 orig)
	{
		if constexpr (!std::is_same_v<float, T>)
		{
			orig = glm::min(orig, glm::vec4{ 1.0f });
			if constexpr (std::is_unsigned_v<T>)
			{
				orig = glm::max(orig, glm::vec4{ 0.0f });
			}

			const auto x = static_cast<T>(glm::round(orig.x * static_cast<float>(std::numeric_limits<T>::max())));
			const auto y = static_cast<T>(glm::round(orig.y * static_cast<float>(std::numeric_limits<T>::max())));
			return result_type{ x, y };
		}
		else
		{
			return result_type{ orig.x, orig.y };
		}
	}
};

template<typename T>
struct from_vec4_norm<glm::vec<3, T, glm::defaultp>>
{
	using result_type = glm::vec<3, T, glm::defaultp>;
	constexpr static result_type invoke(glm::vec4 orig)
	{
		if constexpr (!std::is_same_v<float, T>)
		{
			orig = glm::min(orig, glm::vec4{ 1.0f });
			if constexpr (std::is_unsigned_v<T>)
			{
				orig = glm::max(orig, glm::vec4{ 0.0f });
			}

			const auto x = static_cast<T>(glm::round(orig.x * static_cast<float>(std::numeric_limits<T>::max())));
			const auto y = static_cast<T>(glm::round(orig.y * static_cast<float>(std::numeric_limits<T>::max())));
			const auto z = static_cast<T>(glm::round(orig.z * static_cast<float>(std::numeric_limits<T>::max())));
			return result_type{ x, y, z };
		}
		else
		{
			return result_type{ orig.x, orig.y, orig.z };
		}
	}
};

template<typename T>
struct from_vec4_norm<glm::vec<4, T, glm::defaultp>>
{
	using result_type = glm::vec<4, T, glm::defaultp>;
	constexpr static result_type invoke(glm::vec4 orig)
	{
		if constexpr (!std::is_same_v<float, T>)
		{
			orig = glm::min(orig, glm::vec4{ 1.0f });
			if constexpr (std::is_unsigned_v<T>)
			{
				orig = glm::max(orig, glm::vec4{ 0.0f });
			}

			const auto x = static_cast<T>(glm::round(orig.x * static_cast<float>(std::numeric_limits<T>::max())));
			const auto y = static_cast<T>(glm::round(orig.y * static_cast<float>(std::numeric_limits<T>::max())));
			const auto z = static_cast<T>(glm::round(orig.z * static_cast<float>(std::numeric_limits<T>::max())));
			const auto w = static_cast<T>(glm::round(orig.w * static_cast<float>(std::numeric_limits<T>::max())));
			return result_type{ x, y, z, w };
		}
		else
		{
			return result_type{ orig.x, orig.y, orig.z, orig.w };
		}
	}
};

}

template<typename T>
constexpr T from_vec4_norm(const glm::vec4& orig)
{
	return detail::from_vec4_norm<T>::invoke(orig);
}
}
