#pragma once

#include <imf/core/glm.hpp>

namespace imf::core
{

struct ITexture
{
	virtual ~ITexture() = default;
	virtual glm::uvec3 dim() const = 0;
};

}
