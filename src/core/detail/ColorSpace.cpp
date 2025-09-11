#include <imf/core/ColorSpace.hpp>

namespace imf::core::color
{

//
// https://developer.android.com/reference/android/graphics/ColorSpace.Named#LINEAR_SRGB
const ColorSpace ColorSpace::Linear_sRGB
{
	"Linear sRGB",

	{ 0.6400f, 0.3300f },
	{ 0.3000f, 0.6000f },
	{ 0.1500f, 0.0600f },
	{ 0.3127f, 0.3290f },
};

const ColorSpace ColorSpace::sRGB
{
	"sRGB",
	{ 0.6400f, 0.3300f },
	{ 0.3000f, 0.6000f },
	{ 0.1500f, 0.0600f },
	{ 0.3127f, 0.3290f },

	+[](const glm::vec3& v) noexcept
	{
		// An approximation of the sRGB transfer function
		// https://developer.android.com/reference/android/graphics/ColorSpace.Named#SRGB
		return glm::pow(glm::clamp(v, glm::vec3(0.0f), glm::vec3(1.0f)), glm::vec3(1.0f / 2.2f));
	},

	+[](const glm::vec3& v) noexcept
	{
		// An approximation of the sRGB transfer function
		// https://developer.android.com/reference/android/graphics/ColorSpace.Named#SRGB
		return glm::pow(glm::clamp(v, glm::vec3(0.0f), glm::vec3(1.0f)), glm::vec3(2.2f));
	}
};

namespace RGB
{

glm::mat3 from_XYZ(glm::vec2 r, glm::vec2 g, glm::vec2 b, glm::vec2 w) noexcept
{
	return glm::inverse(XYZ::from_RGB_matrix(r, g, b, w));
}

}


namespace XYZ
{

// https://en.wikipedia.org/wiki/CIE_1931_color_space#CIE_xyY_color_space
glm::vec3 from_xyY(const glm::vec3& xyY) noexcept
{
	return glm::vec3
	(
		xyY.x * xyY[2] / xyY.y,
		xyY[2],
		(1 - xyY.x - xyY.y) * xyY[2] / xyY.y
	);
}


glm::mat3 from_RGB_matrix(const glm::vec2& r, const glm::vec2& g, const glm::vec2& b, const glm::vec2& w) noexcept
{
	// Chromaticity values are defined in xy space without luminance value Y.
	// We assume Y=1 for RGB color primaries as well as for the white point.
	// To get a proportion of the color primaries, we need to find the linear combination of the primaries that produces the given white point with luminance Y=1

	glm::mat3 toXYZmat
	{
		XYZ::from_xyY(glm::vec3(r, 1.0f)),
		XYZ::from_xyY(glm::vec3(g, 1.0f)),
		XYZ::from_xyY(glm::vec3(b, 1.0f))
	};

	glm::vec3 proportion = glm::inverse(toXYZmat) * XYZ::from_xyY(glm::vec3(w, 1.0f));

	return glm::mat3
	{
		toXYZmat[0] * proportion.x,
		toXYZmat[1] * proportion.y,
		toXYZmat[2] * proportion.z
	};
}

}

}
