#pragma once

#include <imf/core/glm.hpp>

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

inline bool operator < (TextureFormat lhs, TextureFormat rhs)
{
	return static_cast<std::uint8_t>(lhs) < static_cast<std::uint8_t>(rhs);
}

constexpr inline std::size_t texture_format_size(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::R8: return 1;
	case TextureFormat::RG8: return 2;
	case TextureFormat::RGB8: return 3;
	case TextureFormat::RGBA8: return 4;
	case TextureFormat::R32F: return 4;
	case TextureFormat::RG32F: return 8;
	case TextureFormat::RGB32F: return 12;
	case TextureFormat::RGBA32F: return 16;
	}

	throw std::runtime_error("Unknown texture format");
}

struct TextureData
{
	TextureFormat format;
	glm::uvec3 dim;
	std::size_t rowAlignment{ 1 };
	std::size_t planeAlignment{ 1 };
	const void* data;
};

struct ITexture
{
	virtual ~ITexture() = default;
	virtual glm::uvec3 dim() const = 0;
};

}
