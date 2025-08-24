#include "NaiveGraphCompiler.hpp"

#include <imf/runtime/cpu/CpuRuntime.hpp>

#include <imf/core/node/PlaceholderNode.hpp>
#include <imf/core/node/SinkNode.hpp>

namespace imf::runtime::cpu
{

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
		const auto& producer = input->producer();
		recurseTraversalPre(visited, producer, func);
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
		const auto& producer = input->producer();
		recurseTraversalPost(visited, producer, func);
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
		const auto& producer = input->producer();
		recurseTraversalBoth(visited, producer, preFunc, postFunc);
	}

	postFunc(curNode);
}

}

core::ExecutionPlan NaiveGraphCompiler::build(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks)
{
	(void)m_runtime;
	validateTopologyStage(sinks);
	scanAllFlowsPhase(sinks);
	mainProcessingStage(sinks);

	return {};
}

void NaiveGraphCompiler::validateTopologyStage(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks)
{
	auto visited = std::set<core::unique_id_t>{};
	auto breadcrumbs = std::set<core::unique_id_t>{};
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

				if (breadcrumbs.count(inputFlow->producer().instanceId()))
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

void NaiveGraphCompiler::scanAllFlowsPhase(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks)
{
	auto visited = std::set<core::unique_id_t>{};
	for (const auto& sink : sinks)
	{
		detail::recurseTraversalPost(visited, *sink, [this](const core::GraphNode& curNode)
		{
			if (auto placeholderNode = curNode.as<core::PlaceholderNode>())
			{
				const auto& outputFlow = curNode.outputs().front();

				if (placeholderNode->typeQualifier() == core::TypeQualifier::Variable)
				{
					auto [_, inserted] = m_flows.emplace(&outputFlow, FlowInfo::make(m_evalCtxAllocator.allocate()));
					assert(inserted);
					if (!inserted)
					{
						throw std::runtime_error("scanAllFlowsPhase: cannot allocate context for variable placeholder");
					}
				}
				else if (placeholderNode->typeQualifier() == core::TypeQualifier::Constant)
				{
					auto anyValue = placeholderNode->value();
					auto [_, inserted] = m_flows.emplace(&outputFlow, FlowInfo::make(std::move(anyValue)));

					assert(inserted);
					if (!inserted)
					{
						throw std::runtime_error("scanAllFlowsPhase: cannot allocate context for constant placeholder");
					}
				}
			}
			else
			{
				auto constantOutput = true;
				for (auto input : curNode.inputs())
				{
					auto& flowInfo = m_flows.at(input.get());

					flowInfo.usages++;
					constantOutput = constantOutput && flowInfo.constant();
				}

				if (!curNode.as<core::SinkNode>())
				{
					for (const auto& outputFlow : curNode.outputs())
					{
						auto flowInfo = FlowInfo();
						if (constantOutput)
						{
							flowInfo.variant = std::any();
						}
						auto [_, inserted] = m_flows.emplace(&outputFlow, std::move(flowInfo));
						assert(inserted);
						if (!inserted)
						{
							throw std::runtime_error("scanAllFlowsPhase: cannot allocate context for regular node");
						}
					}
				}
			}
		});
	}
}

void NaiveGraphCompiler::mainProcessingStage(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks)
{
	auto visited = std::set<core::unique_id_t>{};

	for (const auto& sink : sinks)
	{
		recurseProcessing(visited, *sink);
	}
}

void NaiveGraphCompiler::recurseProcessing(std::set<core::unique_id_t>& visited, const core::GraphNode& curNode)
{
	for (const auto& input : curNode.inputs())
	{
		recurseProcessing(visited, input->producer());
	}

	if (auto placeholderNode = curNode.as<core::PlaceholderNode>())
	{
		processPlaceholderNode(*placeholderNode);
	}
	else if (auto sinkNode = curNode.as<core::SinkNode>())
	{
		processSinkNode(*sinkNode);
	}
	else
	{
		processRegularNode(curNode);
	}
}

void NaiveGraphCompiler::processPlaceholderNode(const core::PlaceholderNode& placeholderNode)
{
	if (placeholderNode.typeQualifier() == core::TypeQualifier::Constant)
	{
		return;
	}

	const auto& outputFlow = placeholderNode.outputs().front();

	auto location = m_flows.at(&outputFlow).location();
	assert(!location.empty());

	m_placeholderLocations[placeholderNode.instanceId()] = { location, placeholderNode.outputTypes().front() };
}

void NaiveGraphCompiler::processSinkNode(const core::SinkNode&)
{

}

void NaiveGraphCompiler::processRegularNode(const core::GraphNode&)
{

}

}

