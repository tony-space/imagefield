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

	virtual const Triangulation& triangles() const = 0;
	virtual const BoundingBox& boundingBox() const noexcept = 0;

	virtual bool empty() const noexcept = 0;
	virtual operator bool() const noexcept = 0;
	virtual bool trivialRectangle() const noexcept = 0;

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

	//friend Region shape_difference(const Region& lhs, const Region& rhs);
	//friend Region shape_intersection(const Region& lhs, const Region& rhs);
	//friend Region shape_symmetric_difference(const Region& lhs, const Region& rhs);
	//friend Region shape_union(const Region& lhs, const Region& rhs);

	static std::shared_ptr<Region> make(const BoundingBox&);
private:
	virtual bool anyOfPointsImpl(bool(*pFn)(const void* ctx, const glm::vec2&), const void* ctx) const = 0;
};

//Region shape_difference(const Region& lhs, const Region& rhs);
//Region shape_intersection(const Region& lhs, const Region& rhs);
//Region shape_symmetric_difference(const Region& lhs, const Region& rhs);
//Region shape_union(const Region& lhs, const Region& rhs);

}
