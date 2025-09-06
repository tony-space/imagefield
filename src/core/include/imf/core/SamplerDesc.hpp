#pragma once

#include <imf/core/ComponentMapping.hpp>

namespace imf::core
{

enum class AddressMode
{
	Repeat,
	ClampToEdge,
};

enum class MinMagFilter
{
	Nearest,
	Linear
};

enum class MipFilter
{
	Disabled,
	Nearest,
	Linear
};

struct SamplerDesc
{
	MinMagFilter minFilter = MinMagFilter::Linear;
	MinMagFilter magFilter = MinMagFilter::Linear;
	MipFilter mipFilter = MipFilter::Linear;

	AddressMode sAddressMode = AddressMode::ClampToEdge;
	AddressMode tAddressMode = AddressMode::ClampToEdge;
	AddressMode rAddressMode = AddressMode::ClampToEdge;

	ComponentMapping componentMapping;

	unsigned char maxAnisotropy = 8;

	friend bool operator== (const SamplerDesc& lhs, const SamplerDesc& rhs) noexcept;
};

inline bool operator== (const SamplerDesc& lhs, const SamplerDesc& rhs) noexcept
{
	return
		lhs.minFilter == rhs.minFilter &&
		lhs.magFilter == rhs.magFilter &&
		lhs.mipFilter == rhs.mipFilter &&
		lhs.sAddressMode == rhs.sAddressMode &&
		lhs.tAddressMode == rhs.tAddressMode &&
		lhs.rAddressMode == rhs.rAddressMode &&
		lhs.componentMapping == rhs.componentMapping &&
		lhs.maxAnisotropy == rhs.maxAnisotropy;
}

}
