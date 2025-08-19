#pragma once

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>
#include <glm/gtc/constants.hpp>

namespace imf::core
{

inline glm::mat3 rotate_rad(float radians)
{
	auto sin = glm::sin(radians);
	auto cos = glm::cos(radians);

	return glm::mat3(
		cos, sin, 0.0f,
		-sin, cos, 0.0f,
		0.0f, 0.0f, 1.0f
	);
}

inline glm::mat3 rotate_deg(float deg)
{
	auto radians = deg / 180.0f * glm::pi<float>();
	return rotate_rad(radians);
}

inline glm::mat3 translate(glm::vec2 t)
{
	return glm::mat3(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		t.x, t.y, 1.0f
	);
}

inline glm::mat3 scale(glm::vec2 s)
{
	return glm::mat3(
		s.x, 0.0f, 0.0f,
		0.0f, s.y, 0.0f,
		0.0f, 0.0f, 1.0f
	);
}

}
