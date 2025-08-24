#include "NaiveGraphCompiler.hpp"

namespace imf::runtime::cpu
{
using namespace core;

namespace detail
{

template<typename Visited, typename Func>
static void recurseTraversalPre(Visited& visited, const core::GraphNode& curNode, const Func& func)
{
	auto [_, inserted] = visited.emplace(curNode.instanceId());

	if (!inserted)
	{
		return;
	}

	func(curNode);

	for (const auto& input : curNode.inputs())
	{
		auto producer = input->producer();
		recurseTraversalPre(visited, *producer, func);
	}
}

template<typename Visited, typename Func>
static void recurseTraversalPost(Visited& visited, const core::GraphNode& curNode, const Func& func)
{
	auto [_, inserted] = visited.emplace(curNode.instanceId());

	if (!inserted)
	{
		return;
	}

	for (const auto& input : curNode.inputs())
	{
		auto producer = input->producer();
		recurseTraversalPost(visited, *producer, func);
	}

	func(curNode);
}

template<typename Visited, typename PreFunc, typename PostFunc>
static void recurseTraversalBoth(Visited& visited, const core::GraphNode& curNode, const PreFunc& preFunc, const PostFunc& postFunc)
{
	auto [_, inserted] = visited.emplace(curNode.instanceId());

	if (!inserted)
	{
		return;
	}

	preFunc(curNode);

	for (const auto& input : curNode.inputs())
	{
		auto producer = input->producer();
		recurseTraversalBoth(visited, *producer, preFunc, postFunc);
	}

	postFunc(curNode);
}

}

ExecutionPlan NaiveGraphCompiler::build(const iterator_range<const std::shared_ptr<const SinkNode>*>& sinks)
{
	validateTopology(sinks);

	return {};
}

void NaiveGraphCompiler::validateTopology(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks)
{
	auto visited = std::set<unique_id_t>{};
	auto breadcrumbs = std::set<unique_id_t>{};
	for (const auto& sink : sinks)
	{
		if (sink == nullptr)
		{
			throw std::invalid_argument("sink is nullptr");
		}

		detail::recurseTraversalBoth(visited, *sink, [&](const core::GraphNode& curNode)
		{
			breadcrumbs.insert(curNode.instanceId());

			for (std::size_t i = 0, len = curNode.inputs().size(); i < len; ++i)
			{
				const auto& inputFlow = curNode.inputs()[i];
				if (inputFlow == nullptr)
				{
					throw std::invalid_argument
					(
						std::string(curNode.operationName())
						+ ':'
						+ std::string(curNode.inputNames()[i])
						+ " is nullptr"
					);
				}

				if (breadcrumbs.count(inputFlow->producer()->instanceId()))
				{
					throw std::invalid_argument
					(
						std::string(curNode.operationName())
						+ " loops itself"
					);
				}
			}
		}, [&](const core::GraphNode& curNode)
		{
			breadcrumbs.erase(curNode.instanceId());
		});
	}
}

}

