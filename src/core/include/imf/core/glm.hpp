#pragma once

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/constants.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/round.hpp>
#include <glm/gtx/component_wise.hpp>

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

inline glm::mat3 translate(float x, float y)
{
	return glm::mat3
	(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		x, y, 1.0f
	);
}

inline glm::mat3 translate(glm::vec2 t)
{
	return translate(t.x, t.y);
}

inline glm::mat3 scale(float x, float y)
{
	return glm::mat3
	(
		x, 0.0f, 0.0f,
		0.0f, y, 0.0f,
		0.0f, 0.0f, 1.0f
	);
}

inline glm::mat3 scale(glm::vec2 s)
{
	return scale(s.x, s.y);
}

inline glm::vec2 projectToPlane(const glm::mat3& homogenous, const glm::vec2& v)
{
	const auto p = homogenous * glm::vec3(v, 1.0f);
	return p.xy() / p.z;
}

inline glm::mat4x2 projectToPlane(const glm::mat3& homogenous, glm::mat4x2 v)
{
	v[0] = projectToPlane(homogenous, v[0]);
	v[1] = projectToPlane(homogenous, v[1]);
	v[2] = projectToPlane(homogenous, v[2]);
	v[3] = projectToPlane(homogenous, v[3]);

	return v;
}

}
