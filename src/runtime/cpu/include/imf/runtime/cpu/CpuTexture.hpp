#pragma once

#include <imf/core/ITexture.hpp>

#include <memory>

namespace imf::runtime::cpu
{

class CpuTexture : public core::ITexture
{
public:
	constexpr static std::size_t kRowAlignment = 32;
	constexpr static std::size_t kPlaneAlignment = 32;

	CpuTexture(const core::TextureData& hostData, core::TextureFormat deviceFormat);
	CpuTexture(const CpuTexture&) = delete;
	CpuTexture(CpuTexture&&) = delete;

	CpuTexture& operator=(const CpuTexture&) = delete;
	CpuTexture& operator=(CpuTexture&&) = delete;

	glm::uvec3 dim() const override;
private:
	std::unique_ptr<std::uint8_t[]> m_data;
	core::TextureFormat m_format;
	glm::uvec3 m_dim;
};

}
