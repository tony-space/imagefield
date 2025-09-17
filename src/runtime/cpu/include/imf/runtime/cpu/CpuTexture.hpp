#pragma once

#include <imf/core/ITexture.hpp>
#include <imf/core/pixel_transform.hpp>

#include <boost/container/small_vector.hpp>

#include <memory>

namespace imf::core
{
class ThreadPool;
}

namespace imf::runtime::cpu
{

class CpuTexture : public core::ITexture, public core::IReadMapTexture
{
public:
	struct MipLevel
	{
		std::unique_ptr<std::uint8_t[]> storage;
		glm::uvec3 dim{0u};
		core::ImageSize size;
	};

	constexpr static std::size_t kRowAlignment = 32;
	constexpr static std::size_t kPlaneAlignment = 32;

	CpuTexture(const core::TextureData& hostData, core::TextureFormat deviceFormat);
	CpuTexture(glm::uvec3 dim, core::TextureFormat deviceFormat);
	CpuTexture(glm::uvec2 dim, core::TextureFormat deviceFormat);
	CpuTexture(const CpuTexture&) = delete;
	CpuTexture(CpuTexture&&) = default;

	CpuTexture& operator=(const CpuTexture&) = delete;
	CpuTexture& operator=(CpuTexture&&) = default;

	glm::uvec3 dim() const noexcept override;
	core::TextureFormat format() const noexcept override;
	const core::TextureData readMap(unsigned level) const override;
	void unmap() const noexcept override;
	void generateMipMaps(core::ThreadPool& pool);
	MipLevel downsample(core::ThreadPool& pool, const MipLevel& inLevel) const;
	const MipLevel& at(unsigned level) const;
	const MipLevel& operator[](unsigned level) const noexcept { return m_mipMaps[level]; }
	
	MipLevel& getMsaaLevel();
	void msaaResolve(core::ThreadPool& pool);
private:
	MipLevel m_msaa4x;
	boost::container::small_vector<MipLevel, 16> m_mipMaps;
	core::TextureFormat m_format;

};

}
