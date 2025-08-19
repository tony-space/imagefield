#pragma once

#include <imf/core/node/SinkNode.hpp>

#include <imf/core/DataFlow.hpp>
#include <imf/core/EvaluationContext.hpp>
#include <imf/core/ExecutionPlan.hpp>
#include <imf/core/IRuntime.hpp>

#include <any>
#include <map>
#include <memory>
#include <string>

namespace imf::core
{

class GraphExecutor : public std::enable_shared_from_this<GraphExecutor>
{
public:
	struct ExecutionResult
	{
		std::any value;
		TypeID type;
		unique_id_t sinkNodeId;

		ExecutionResult(std::any&& v, const TypeID& t, unique_id_t id) :
			value(std::move(v)),
			type(t),
			sinkNodeId(id)
		{

		}

		ExecutionResult(const ExecutionResult&) = default;
		ExecutionResult(ExecutionResult&&) noexcept = default;
		ExecutionResult& operator=(const ExecutionResult&) = default;
		ExecutionResult& operator=(ExecutionResult&&) noexcept = default;
	};

	GraphExecutor() = default;
	GraphExecutor(const GraphExecutor&) = delete;
	GraphExecutor(GraphExecutor&&) noexcept = default;
	GraphExecutor& operator=(const GraphExecutor&) = delete;
	GraphExecutor& operator=(GraphExecutor&&) noexcept = default;
	~GraphExecutor() = default;

	template<typename T>
	void setPlaceholderValue(unique_id_t placeholderId, T&& value);

	[[nodiscard]] std::vector<ExecutionResult> run();

	template<typename Sink, typename... Sinks>
	static std::shared_ptr<GraphExecutor> make(std::shared_ptr<IRuntime> runtime, Sink&& head, Sinks&& ...tail)
	{
		const std::shared_ptr<const SinkNode> sinks[] = { std::forward<Sink>(head), std::forward<Sinks>(tail)... };
		return make_from_range(std::move(runtime), iterator_range<const std::shared_ptr<const SinkNode>*>(std::begin(sinks), std::end(sinks)));
	}
	static std::shared_ptr<GraphExecutor> make_from_range(std::shared_ptr<IRuntime> runtime, const iterator_range<const std::shared_ptr<const SinkNode>*>& sinks);
private:
	GraphExecutor(std::shared_ptr<IRuntime>, ExecutionPlan&&) noexcept;

	std::shared_ptr<IRuntime> m_runtime;
	ExecutionPlan m_executionPlan;
	EvaluationContext m_evalContext;

	std::map<unique_id_t, std::any> m_placeholderValues;
	std::map<unique_id_t, std::any> m_sinkValues;

};

template<typename T>
void GraphExecutor::setPlaceholderValue(unique_id_t placeholderId, T&& value)
{
	if (m_executionPlan.placeholders().at(placeholderId).dataType != TypeID::make<std::decay_t<T>>())
	{
		throw std::invalid_argument("placeholder type mismatch");
	}

	m_placeholderValues.at(placeholderId) = std::forward<T>(value);
}

}
