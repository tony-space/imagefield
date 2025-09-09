#pragma once

namespace imf::core
{

enum class ComponentSwizzle : unsigned char
{
	Red,
	Green,
	Blue,
	Alpha,
	One,
	Zero
};

struct ComponentMapping
{
	ComponentSwizzle red{ ComponentSwizzle::Red };
	ComponentSwizzle green{ ComponentSwizzle::Green };
	ComponentSwizzle blue{ ComponentSwizzle::Blue };
	ComponentSwizzle alpha{ ComponentSwizzle::Alpha };

	friend bool operator == (ComponentMapping lhs, ComponentMapping rhs) noexcept;
};

inline bool operator == (ComponentMapping lhs, ComponentMapping rhs) noexcept
{
	return
		lhs.red == rhs.red &&
		lhs.green == rhs.green &&
		lhs.blue == rhs.blue &&
		lhs.alpha == rhs.alpha;

}

constexpr inline ComponentMapping kLuminanceSwizzle
{
	ComponentSwizzle::Red,
	ComponentSwizzle::Red,
	ComponentSwizzle::Red,
	ComponentSwizzle::One
};

constexpr inline ComponentMapping kRedSwizzle
{
	ComponentSwizzle::Red,
	ComponentSwizzle::Red,
	ComponentSwizzle::Red,
	ComponentSwizzle::Red
};

constexpr inline ComponentMapping kGreenSwizzle
{
	ComponentSwizzle::Green,
	ComponentSwizzle::Green,
	ComponentSwizzle::Green,
	ComponentSwizzle::Green
};

constexpr inline ComponentMapping kBlueSwizzle
{
	ComponentSwizzle::Blue,
	ComponentSwizzle::Blue,
	ComponentSwizzle::Blue,
	ComponentSwizzle::Blue
};

constexpr inline ComponentMapping kAlphaSwizzle
{
	ComponentSwizzle::Alpha,
	ComponentSwizzle::Alpha,
	ComponentSwizzle::Alpha,
	ComponentSwizzle::Alpha
};

}
