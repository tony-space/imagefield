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
	virtual glm::uvec3 dim() const = 0;
};

struct IReadMapTexture
{
	virtual ~IReadMapTexture() = default;
	[[nodiscard]] virtual const TextureData readMap() const = 0;
	virtual void unmap() const noexcept = 0;

	template <typename Func>
	decltype(auto) mapUnmap(Func&& f) const
	{
		const TextureData textureData = readMap();
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
