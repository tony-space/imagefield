#pragma once

#include <imf/core/glm.hpp>

#include <string>

namespace imf::core::color
{

struct ColorSpace
{
	using transfer_function_t = glm::vec3(*)(const glm::vec3&) noexcept;

	std::string name;

	// chromaticity xy coordinates of primaries and white point
	glm::vec2 r;
	glm::vec2 g;
	glm::vec2 b;
	glm::vec2 w;

	// opto-electronic transfer function (OETF)
	transfer_function_t oetf{ &linear };

	// electro-optical transfer function (EOTF)
	transfer_function_t eotf{ &linear };

	constexpr static glm::vec3 linear(const glm::vec3& v)noexcept
	{
		return v;
	}

	static const ColorSpace Linear_sRGB;
	static const ColorSpace sRGB;
};

namespace RGB
{

glm::mat3 from_XYZ(glm::vec2 r, glm::vec2 g, glm::vec2 b, glm::vec2 w) noexcept;

}

namespace XYZ
{

glm::vec3 from_xyY(const glm::vec3& xyY) noexcept;
glm::mat3 from_RGB_matrix(const glm::vec2& r, const glm::vec2& g, const glm::vec2& b, const glm::vec2& w) noexcept;

}

}
