#pragma once

#include <memory>
#include <optional>
#include <vector>

namespace imf::core
{

class BoundingBox;

class Region
{
public:
	struct Triangulation
	{
		using index_triple = glm::uvec3;
		using index_type = index_triple::value_type;

		std::vector<glm::vec2> vertices;
		std::vector<index_triple> indices;
	};

	virtual ~Region() = default;
	virtual std::shared_ptr<Region> copy() const = 0;

	virtual const Triangulation& triangles() const = 0;
	virtual const BoundingBox& boundingBox() const noexcept = 0;

	virtual bool empty() const noexcept = 0;
	virtual operator bool() const noexcept = 0;
	virtual bool trivialRectangle() const noexcept = 0;
	virtual void transformPoints(const glm::mat3& homogenousMat) noexcept = 0;

	template<typename Func>
	bool anyOfPoints(const Func& func) const
	{
		const Func* funcOrObjPtr = &func;

		return anyOfPointsImpl(+[](const void* ctx, const glm::vec2& v)
		{
			const Func& func = *static_cast<const Func*>(ctx);
			return func(v);
		}, funcOrObjPtr);
	}

	template<typename Func>
	bool allOfPoints(const Func& func) const
	{
		return !anyOfPoints([&](const glm::vec2& v)
		{
			return !func(v);
		});
	}

	static std::shared_ptr<Region> make(const BoundingBox&);
private:
	virtual bool anyOfPointsImpl(bool(*pFn)(const void* ctx, const glm::vec2&), const void* ctx) const = 0;
};

std::shared_ptr<Region> region_difference(const Region& lhs, const Region& rhs);
std::shared_ptr<Region> region_intersection(const Region& lhs, const Region& rhs);

}
