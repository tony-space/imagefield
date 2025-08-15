#include <imf/runtime/cpu/CpuTexture.hpp>
#include <imf/core/vector_cast.hpp>

namespace imf::runtime::cpu
{

namespace detail
{

static std::size_t align_size(std::size_t bytesSize, std::size_t alignment)
{
	return (bytesSize + alignment - 1) / alignment * alignment;
}

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
static TransformRowFunc get_convert_func(core::TextureFormat to)
{
	switch (to)
	{
	case core::TextureFormat::R8: return &convert_row<InputType, inComp, glm::u8, 1>;
	case core::TextureFormat::RG8: return &convert_row<InputType, inComp, glm::u8, 2>;
	case core::TextureFormat::RGB8: return &convert_row<InputType, inComp, glm::u8, 3>;
	case core::TextureFormat::RGBA8: return &convert_row<InputType, inComp, glm::u8, 4>;

	case core::TextureFormat::R32F: return &convert_row<InputType, inComp, glm::f32, 1>;
	case core::TextureFormat::RG32F: return &convert_row<InputType, inComp, glm::f32, 2>;
	case core::TextureFormat::RGB32F: return &convert_row<InputType, inComp, glm::f32, 3>;
	case core::TextureFormat::RGBA32F: return &convert_row<InputType, inComp, glm::f32, 4>;
	}

	throw std::runtime_error("Unknown 'to' texture format");
}

static TransformRowFunc get_convert_func(core::TextureFormat from, core::TextureFormat to)
{
	switch (from)
	{
	case core::TextureFormat::R8: return get_convert_func<glm::u8, 1>(to);
	case core::TextureFormat::RG8: return get_convert_func<glm::u8, 2>(to);
	case core::TextureFormat::RGB8: return get_convert_func<glm::u8, 3>(to);
	case core::TextureFormat::RGBA8: return get_convert_func<glm::u8, 4>(to);

	case core::TextureFormat::R32F: return get_convert_func<glm::f32, 1>(to);
	case core::TextureFormat::RG32F: return get_convert_func<glm::f32, 2>(to);
	case core::TextureFormat::RGB32F: return get_convert_func<glm::f32, 3>(to);
	case core::TextureFormat::RGBA32F: return get_convert_func<glm::f32, 4>(to);
	}

	throw std::runtime_error("Unknown 'from' texure format");
}

}

CpuTexture::CpuTexture(const core::TextureData& hostData, core::TextureFormat deviceFormat)
{
	const auto srcPixelByteSize = core::texture_format_size(hostData.format);
	const auto srcRowByteSize = detail::align_size(hostData.dim.x * srcPixelByteSize, hostData.rowAlignment);
	const auto srcPlaneByteSize = detail::align_size(srcRowByteSize * hostData.dim.y, hostData.planeAlignment);
	const auto srcVolumeByteSize = srcPlaneByteSize * hostData.dim.z;

	if (srcVolumeByteSize == 0)
	{
		throw std::runtime_error("CpuTexture: invalid host texture data size");
	}

	const auto targetPixelByteSize = core::texture_format_size(deviceFormat);
	const auto targetRowByteSize = detail::align_size(hostData.dim.x * targetPixelByteSize, kRowAlignment);
	const auto targetPlaneByteSize = detail::align_size(targetRowByteSize * hostData.dim.y, kPlaneAlignment);
	const auto targetVolumeByteSize = targetPlaneByteSize * hostData.dim.z;

	if (targetVolumeByteSize == 0)
	{
		throw std::runtime_error("CpuTexture: invalid device texture data size");
	}

	m_data = std::make_unique<std::uint8_t[]>(targetVolumeByteSize);
	m_dim = hostData.dim;
	m_format = deviceFormat;

	detail::TransformRowFunc transformRow = detail::get_convert_func(hostData.format, deviceFormat);

	for (size_t plane = 0; plane < hostData.dim.z; ++plane)
	{
		const auto srcPlaneOffset = plane * srcPlaneByteSize;
		const auto targetPlaneOffset = plane * targetPlaneByteSize;

		for (size_t row = 0; row < hostData.dim.y; ++row)
		{
			const auto srcRowOffset = srcPlaneOffset + row * srcRowByteSize;
			const auto targetRowOffset = targetPlaneOffset + row * targetRowByteSize;
			transformRow(static_cast<const uint8_t*>(hostData.data) + srcRowOffset, m_data.get() + targetRowOffset, hostData.dim.x);
		}
	}
}

glm::uvec3 CpuTexture::dim() const
{
	return m_dim;
}

}
