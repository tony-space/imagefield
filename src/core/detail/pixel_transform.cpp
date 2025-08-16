
#include <imf/core/pixel_transform.hpp>
#include <imf/core/vector_cast.hpp>

#include <algorithm>


namespace imf::core
{

namespace detail
{

template<typename InputType, glm::length_t inComp, typename OutputType, glm::length_t outComp>
static void convert_row(const void* pFrom, void* pTo, size_t pixels)
{
	auto from = static_cast<const glm::vec<inComp, InputType, glm::defaultp>*>(pFrom);
	auto to = static_cast<glm::vec<outComp, OutputType, glm::defaultp>*>(pTo);

	std::transform(from, from + pixels, to, [](const auto& pixel)
	{
		return core::from_vec4_norm<glm::vec<outComp, OutputType>>(core::to_vec4_norm(pixel));
	});
}

using TransformRowFunc = void (*)(const void* from, void* to, size_t pixels);

template<typename InputType, glm::length_t inComp>
static TransformRowFunc get_convert_func(TextureFormat to)
{
	switch (to)
	{
	case TextureFormat::R8: return &convert_row<InputType, inComp, glm::u8, 1>;
	case TextureFormat::RG8: return &convert_row<InputType, inComp, glm::u8, 2>;
	case TextureFormat::RGB8: return &convert_row<InputType, inComp, glm::u8, 3>;
	case TextureFormat::RGBA8: return &convert_row<InputType, inComp, glm::u8, 4>;

	case TextureFormat::R32F: return &convert_row<InputType, inComp, glm::f32, 1>;
	case TextureFormat::RG32F: return &convert_row<InputType, inComp, glm::f32, 2>;
	case TextureFormat::RGB32F: return &convert_row<InputType, inComp, glm::f32, 3>;
	case TextureFormat::RGBA32F: return &convert_row<InputType, inComp, glm::f32, 4>;
	}

	throw std::runtime_error("Unknown 'to' texture format");
}

static TransformRowFunc get_convert_func(TextureFormat from, TextureFormat to)
{
	switch (from)
	{
	case TextureFormat::R8: return get_convert_func<glm::u8, 1>(to);
	case TextureFormat::RG8: return get_convert_func<glm::u8, 2>(to);
	case TextureFormat::RGB8: return get_convert_func<glm::u8, 3>(to);
	case TextureFormat::RGBA8: return get_convert_func<glm::u8, 4>(to);

	case TextureFormat::R32F: return get_convert_func<glm::f32, 1>(to);
	case TextureFormat::RG32F: return get_convert_func<glm::f32, 2>(to);
	case TextureFormat::RGB32F: return get_convert_func<glm::f32, 3>(to);
	case TextureFormat::RGBA32F: return get_convert_func<glm::f32, 4>(to);
	}

	throw std::runtime_error("Unknown 'from' texure format");
}

}

ImageSize calc_image_size(TextureFormat format, glm::uvec3 dim, std::size_t rowAlignment, std::size_t planeAlignment)
{
	const auto pixelByteSize = texture_pixel_size(format);
	const auto rowByteSize = align_size(dim.x * pixelByteSize, rowAlignment);
	const auto planeByteSize = align_size(rowByteSize * dim.y, planeAlignment);
	const auto volumeByteSize = planeByteSize * dim.z;

	return { pixelByteSize, rowByteSize, planeByteSize, volumeByteSize };
}

void convert_pixels(const TextureData& source, TextureFormat dstFormat, std::size_t dstRowAlignment, std::size_t dstPlaneAlignment, void* dstLocation, std::size_t dstBytesSize)
{
	if (source.data == nullptr || dstLocation == nullptr)
	{
		throw std::invalid_argument("image cannot be nullptr");
	}

	const auto srcSize = calc_image_size(source.format, source.dim, source.rowAlignment, source.planeAlignment);

	if (srcSize.volumeByteSize == 0)
	{
		throw std::runtime_error("invalid source image data size");
	}

	const auto dstSize = calc_image_size(dstFormat, source.dim, dstRowAlignment, dstPlaneAlignment);

	if (dstSize.volumeByteSize == 0 || dstBytesSize < dstSize.volumeByteSize)
	{
		throw std::runtime_error("invalid destination image data size");
	}

	detail::TransformRowFunc transformRow = detail::get_convert_func(source.format, dstFormat);

	for (size_t plane = 0; plane < source.dim.z; ++plane)
	{
		const auto srcPlaneOffset = plane * srcSize.planeByteSize;
		const auto dstPlaneOffset = plane * dstSize.planeByteSize;

		for (size_t row = 0; row < source.dim.y; ++row)
		{
			const auto srcRowOffset = srcPlaneOffset + row * srcSize.rowByteSize;
			const auto dstRowOffset = dstPlaneOffset + row * dstSize.rowByteSize;
			transformRow(static_cast<const uint8_t*>(source.data) + srcRowOffset, static_cast<uint8_t*>(dstLocation) + dstRowOffset, source.dim.x);
		}
	}
}

}

