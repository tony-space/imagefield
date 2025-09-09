#pragma once

#include <imf/core/glm.hpp>
#include <imf/core/TextureFormat.hpp>

namespace imf::core
{

constexpr inline bool operator < (TextureFormat lhs, TextureFormat rhs)
{
	return static_cast<std::uint8_t>(lhs) < static_cast<std::uint8_t>(rhs);
}

struct TextureData
{
	TextureFormat format;
	glm::uvec3 dim;
	std::size_t rowAlignment{ 1 };
	std::size_t planeAlignment{ 1 };
	void* data;
};

struct ITexture
{
	virtual ~ITexture() = default;
	virtual glm::uvec3 dim() const noexcept = 0;
	virtual TextureFormat format() const noexcept = 0;
};

struct IReadMapTexture
{
	virtual ~IReadMapTexture() = default;
	[[nodiscard]] virtual const TextureData readMap(unsigned level) const = 0;
	virtual void unmap() const noexcept = 0;

	template <typename Func>
	decltype(auto) mapUnmap(Func&& f, unsigned level) const
	{
		const TextureData textureData = readMap(level);
		const unmapper __unmapper(this);

		return std::invoke(std::forward<Func>(f), textureData);
	
	}

private:
	struct unmapper
	{
		const IReadMapTexture* self;
		unmapper(const IReadMapTexture* s) : self(s) {}
		~unmapper()
		{
			self->unmap();
		}
	};
};

}
