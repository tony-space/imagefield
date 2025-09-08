#include <imf/core/node/PlaceholderNode.hpp>
#include <imf/core/node/SinkNode.hpp>

#include <imf/core/GraphExecutor.hpp>
#include <imf/core/GraphNodeFactory.hpp>
#include <imf/core/IGraphCompiler.hpp>
#include <imf/core/log.hpp>

#include <set>

namespace imf::core
{

template<typename Func>
static void recurseTraversal(std::set<unique_id_t>& visited, const GraphNode& curNode, const Func& func)
{
	visited.emplace(curNode.instanceId());

	func(curNode);

	for (auto&& input : curNode.inputs())
	{
		const auto& producer = input->producer();
		if (!visited.count(producer.instanceId()))
		{
			recurseTraversal(visited, producer, func);
		}
	}
}

std::shared_ptr<GraphExecutor> GraphExecutor::make_from_range(std::shared_ptr<IRuntime> runtime, const iterator_range<const std::shared_ptr<const SinkNode>*>& sinks)
{
	if (!runtime) throw std::invalid_argument("empty runtime");

	auto compiler = runtime->compiler();

	//bypassing private ctor
	auto result = std::make_shared<GraphExecutor>(GraphExecutor(std::move(runtime), compiler->build(sinks)));

	std::set<unique_id_t> visited;
	for (const auto& sink : sinks)
	{
		recurseTraversal(visited, *sink, [&](const GraphNode& curNode)
		{
			if (auto placeholder = curNode.as<PlaceholderNode>(); placeholder && placeholder->typeQualifier() == TypeQualifier::Variable)
			{
				result->m_placeholderValues.at(placeholder->instanceId()) = placeholder->value();
			}
		});
	}

	return result;
}

GraphExecutor::GraphExecutor(std::shared_ptr<IRuntime> runtime, ExecutionPlan&& executionPlan) noexcept :
	m_runtime(std::move(runtime)),
	m_executionPlan(std::move(executionPlan))
{
	for (const auto& [placeholderId, _] : m_executionPlan.placeholders())
	{
		m_placeholderValues.insert(std::make_pair(placeholderId, std::any()));
	}
}

std::vector<GraphExecutor::ExecutionResult> GraphExecutor::run()
{
	for (const auto& [placeholderId, nodeInfo] : m_executionPlan.placeholders())
	{
		m_evalContext.set(nodeInfo.location, m_placeholderValues.at(placeholderId));
	}

	for (auto& instruction : m_executionPlan.instructions())
	{
		instruction->execute(m_evalContext);
	}

	const auto& sinks = m_executionPlan.sinks();

	std::vector<ExecutionResult> result;
	result.reserve(sinks.size());

	for (auto&& [nodeId, nodeInfo] : sinks)
	{
		std::any& value = m_evalContext.get(nodeInfo.location);

		result.emplace_back(ExecutionResult
		(
			std::move(value),
			nodeInfo.dataType,
			nodeId
		));
	}

	m_evalContext.clear();

	return result;
}

}
