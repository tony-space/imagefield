#pragma once

#include <cstdint>

namespace imf::core
{

enum class TextureFormat : std::uint8_t
{
	R8,
	RG8,
	RGB8,
	RGBA8,

	R32F,
	RG32F,
	RGB32F,
	RGBA32F,
};

}
