#pragma once

#include <imf/core/EvaluationContext.hpp>
#include <imf/core/iterator_range.hpp>
#include <imf/core/TypeID.hpp>

#include <any>
#include <variant>

namespace imf::core
{

struct source_operand
{
	using runtime_value_location_t = EvaluationContext::element_id_t;
	using compile_time_value_t = std::any;
	using value_t = std::variant<compile_time_value_t, runtime_value_location_t>;

	explicit source_operand(const runtime_value_location_t& location, const TypeID& _dataType) :
		variant(location), dataType(_dataType)
	{
	}

	explicit source_operand(compile_time_value_t value, const TypeID& _dataType) :
		variant(std::move(value)), dataType(_dataType)
	{
	}

	template <typename T>
	explicit source_operand(T&& value) : source_operand(compile_time_value_t(std::forward<T>(value)), TypeID::make<std::decay_t<T>>())
	{

	}

	source_operand(const source_operand&) = default;
	source_operand(source_operand&&) = default;

	source_operand& operator=(const source_operand&) = default;
	source_operand& operator=(source_operand&&) = default;

	constexpr bool constant() const noexcept
	{
		return std::holds_alternative<compile_time_value_t>(variant);
	}

	template<typename T>
	const T& value() const
	{
		return std::any_cast<const T&>(std::get<compile_time_value_t>(variant));
	}

	value_t variant;
	TypeID dataType;
};
using source_operand_iterator_t = const source_operand*;
using source_operands_range = iterator_range<source_operand_iterator_t>;

struct destination_operand
{
	EvaluationContext::element_id_t location;
	TypeID dataType;

	destination_operand() = default;

	explicit destination_operand(EvaluationContext::element_id_t _location, const TypeID& _dataType) :
		location(_location), dataType(_dataType)
	{
	}

	destination_operand(const destination_operand&) = default;
	destination_operand(destination_operand&&) = default;

	destination_operand& operator=(const destination_operand&) = default;
	destination_operand& operator=(destination_operand&&) = default;
};
using destination_operand_iterator_t = const destination_operand*;
using destination_operands_range = iterator_range<destination_operand_iterator_t>;

inline const std::any& fetch_operand(const EvaluationContext& evalCtx, const source_operand& operand)
{
	struct visitor
	{
		const EvaluationContext& evalCtx;

		const std::any& operator() (const std::any& value) const noexcept
		{
			return value;
		}
		const std::any& operator() (EvaluationContext::element_id_t registerId) const noexcept
		{
			return evalCtx.get(registerId);
		}
	};

	return std::visit(visitor{ evalCtx }, operand.variant);
}

template<typename T>
const std::decay_t<T>& fetch_operand(const EvaluationContext& evalCtx, const source_operand& operand)
{
	return std::any_cast<const std::decay_t<T>&>(fetch_operand(evalCtx, operand));
}

}
