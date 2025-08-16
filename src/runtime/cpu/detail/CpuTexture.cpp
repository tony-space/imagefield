#include <imf/runtime/cpu/CpuTexture.hpp>
#include <imf/core/pixel_transform.hpp>


namespace imf::runtime::cpu
{

CpuTexture::CpuTexture(const core::TextureData& hostData, core::TextureFormat deviceFormat)
{
	const auto dstSize = core::calc_image_size(deviceFormat, hostData.dim, kRowAlignment, kPlaneAlignment);

	m_data = std::make_unique<uint8_t[]>(dstSize.volumeByteSize);
	m_dim = hostData.dim;
	m_format = deviceFormat;

	core::convert_pixels(hostData, deviceFormat, kRowAlignment, kPlaneAlignment, m_data.get(), dstSize.volumeByteSize);
}

glm::uvec3 CpuTexture::dim() const
{
	return m_dim;
}

const core::TextureData CpuTexture::readMap() const
{
	core::TextureData result;

	result.data = m_data.get();
	result.dim = m_dim;
	result.format = m_format;
	result.rowAlignment = kRowAlignment;
	result.planeAlignment = kPlaneAlignment;

	return result;
}

void CpuTexture::unmap() const noexcept
{
}

}
