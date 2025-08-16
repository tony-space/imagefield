#pragma once

#include <imf/core/ITexture.hpp>

#include <stdexcept>

namespace imf::core
{

constexpr inline std::size_t texture_pixel_size(TextureFormat format)
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

constexpr inline std::size_t align_size(std::size_t bytesSize, std::size_t alignment)
{
	return (bytesSize + alignment - 1) / alignment * alignment;
}

struct ImageSize
{
	std::size_t pixelByteSize;
	std::size_t rowByteSize;
	std::size_t planeByteSize;
	std::size_t volumeByteSize;
};

ImageSize calc_image_size(TextureFormat format, glm::uvec3 dim, std::size_t rowAlignment, std::size_t planeAlignment);

void convert_pixels(const TextureData& source, TextureFormat dstFormat, std::size_t dstRowAlignment, std::size_t dstPlaneAlignment, void* dstLocation, std::size_t dstBytesSize);

}
