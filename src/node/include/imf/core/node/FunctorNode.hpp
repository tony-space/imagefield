#pragma once

#include <imf/core/EvaluationContext.hpp>
#include <imf/core/function_traits.hpp>
#include <imf/core/GraphNode.hpp>
#include <imf/core/operand.hpp>

#include <functional>
#include <boost/container/small_vector.hpp>

namespace imf::core
{

class FunctorNode final : public GraphNode
{
public:
	// prevents base class function hiding
	using GraphNode::setInput;

	constexpr static std::size_t kPreallocatedSize = 8;
	template <typename T> using small_vector_t = boost::container::small_vector<T, kPreallocatedSize>;
	//template <typename T> using small_vector_t = std::vector<T>;

	using functor_t = std::function<void(EvaluationContext&, destination_operands_range, source_operands_range)>;

	template <typename Func, typename... Args>
	FunctorNode(Func&& func, Args&& ...args);
	FunctorNode
	(
		functor_t functor,
		small_vector_t<TypeID> inputTypes,
		small_vector_t<TypeID> outputTypes,
		small_vector_t<std::shared_ptr<const DataFlow>> inputFlows
	);

	std::string_view operationName() const noexcept override;
	iterator_range<const std::string_view*> inputNames() const noexcept override;
	iterator_range<const TypeID*> inputTypes() const noexcept override;
	iterator_range<const std::shared_ptr<const DataFlow>*> inputs() const noexcept override;
	iterator_range<const std::string_view*> outputNames() const noexcept override;
	iterator_range<const TypeID*> outputTypes() const noexcept override;
	iterator_range<const DataFlow*> outputs() const noexcept override;
	void setInput(const std::string_view& name, const DataFlow& flow) override;

	template <typename... Args>
	static std::shared_ptr<GraphNode> make(Args&& ...args)
	{
		return std::make_shared<FunctorNode>(std::forward<Args>(args)...);
	}
private:

	small_vector_t<std::string> m_inputNames;
	small_vector_t<std::string_view> m_inputNamesView;
	small_vector_t<TypeID> m_inputTypes;
	small_vector_t<std::shared_ptr<const DataFlow>> m_inputFlows;

	small_vector_t<std::string> m_outputNames;
	small_vector_t<std::string_view> m_outputNamesView;
	small_vector_t<TypeID> m_outputTypes;
	small_vector_t<DataFlow> m_outputFlows;

};

namespace detail
{

template <typename TypeList>
struct apply_operands;

template <typename Head, typename ...Tail>
struct apply_operands<type_list_t<Head, Tail...>>
{
	template <typename Func>
	static auto invoke(const Func& func, EvaluationContext& evalCtx, source_operand_iterator_t operand)
	{
		const std::any& operandVal = core::fetch_operand(evalCtx, *operand);
		const Head& headOperand = std::any_cast<const Head&>(operandVal);

		// std::bind copies the function object, which sometimes has no copy constructor
		// auto bound = std::bind(func, std::cref(headOperand));

		auto bound = [&func, &headOperand](auto&& ...args)
		{
			return func(headOperand, std::forward<decltype(args)>(args)...);
		};

		if constexpr (sizeof...(Tail) == 0)
		{
			return bound();
		}
		else
		{
			return apply_operands<type_list_t<Tail...>>::invoke(bound, evalCtx, ++operand);
		}
	}
};

template <typename TypesList>
struct type_list_to_vector;

template <typename ...Types>
struct type_list_to_vector<type_list_t<Types...>>
{
	static FunctorNode::small_vector_t<TypeID> invoke()
	{
		auto result = FunctorNode::small_vector_t<TypeID>();
		result.reserve(1 + sizeof...(Types));

		result.emplace_back(TypeID::make<FunctorNode::functor_t>());
		(result.emplace_back(TypeID::make<std::decay_t<Types>>()), ...);

		return result;
	}
};

template <typename Func>
FunctorNode::functor_t make_functor(Func&& func)
{
	using signature_t = function_traits<std::decay_t<Func>>;

	using arguments_list_t = typename signature_t::args_types;
	using return_type = typename signature_t::return_type;

	FunctorNode::functor_t functor = [func = std::forward<Func>(func)](EvaluationContext& ctx, destination_operands_range dstRange, source_operands_range srcRange)
	{
		return_type result = apply_operands<arguments_list_t>::invoke(func, ctx, srcRange.begin());
		ctx.set(dstRange.front().location, std::move(result));
	};

	return functor;
}

template <typename Func>
FunctorNode::small_vector_t<TypeID> make_input_types()
{
	using signature_t = function_traits<std::decay_t<Func>>;
	using arguments_list_t = typename signature_t::args_types;

	return type_list_to_vector<arguments_list_t>::invoke();
}

template <typename Func>
FunctorNode::small_vector_t<TypeID> make_output_types()
{
	using signature_t = function_traits<std::decay_t<Func>>;
	using return_type = typename signature_t::return_type;

	return { TypeID::make<return_type>() };
}

template <typename... Args>
FunctorNode::small_vector_t<std::shared_ptr<const DataFlow>> make_input_flows(Args&& ...args)
{
	return { args.sharedPtr()... };
}

}


template <typename Func, typename... Args>
FunctorNode::FunctorNode(Func&& func, Args&& ...args) : FunctorNode
(
	detail::make_functor<Func>(std::forward<Func>(func)),
	detail::make_input_types<Func>(),
	detail::make_output_types<Func>(),
	detail::make_input_flows(std::forward<Args>(args)...)
)
{
}

}

