#include "NaiveGraphCompiler.hpp"

#include <imf/runtime/cpu/CpuOperationFactory.hpp>
#include <imf/runtime/cpu/CpuRuntime.hpp>

#include <imf/core/node/PlaceholderNode.hpp>
#include <imf/core/node/SinkNode.hpp>
#include <imf/core/node/ReferenceNode.hpp>
#include <imf/core/node/ChildGraphBase.hpp>

#include <imf/core/log.hpp>

#include <boost/container/small_vector.hpp>

namespace imf::runtime::cpu
{

namespace detail
{

template<typename Func>
static void for_each_input(const core::GraphNode& curNode, const Func& func)
{
	for (const auto& input : curNode.inputs())
	{
		if (input == nullptr)
		{
			const auto idx = std::distance(curNode.inputs().begin(), &input);
			throw std::invalid_argument
			(
				std::string(curNode.operationName())
				+ ':'
				+ std::string(curNode.inputNames()[idx])
				+ " is nullptr"
			);
		}

		const auto producer = &input->producer();

		if (const auto* childGraph = dynamic_cast<const core::IChildGraph*>(producer))
		{
			auto idx = std::distance(producer->outputs().begin(), input.get());
			const auto& referenceOutput = childGraph->outputReferences()[idx]->outputs().front();
			func(referenceOutput);
		}
		else
		{
			func(*input);
		}
	}
}

template<typename Visited, typename Func>
static void recurseTraversalPre(Visited& visited, const core::GraphNode& curNode, const Func& func)
{
	auto [_, inserted] = visited.emplace(curNode.instanceId());

	if (!inserted)
	{
		return;
	}

	func(curNode);

	for_each_input(curNode, [&](const core::DataFlow& input)
	{
		const auto& producer = input.producer();
		recurseTraversalPre(visited, producer, func);
	});
}

template<typename Visited, typename Func>
static void recurseTraversalPost(Visited& visited, const core::GraphNode& curNode, const Func& func)
{
	auto [_, inserted] = visited.emplace(curNode.instanceId());

	if (!inserted)
	{
		return;
	}

	for_each_input(curNode, [&](const core::DataFlow& input)
	{
		const auto& producer = input.producer();
		recurseTraversalPost(visited, producer, func);
	});

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

	for_each_input(curNode, [&](const core::DataFlow& input)
	{
		const auto& producer = input.producer();
		recurseTraversalBoth(visited, producer, preFunc, postFunc);
	});

	postFunc(curNode);
}

}

core::ExecutionPlan NaiveGraphCompiler::build(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks)
{
	core::log::info("compiler") << "compiling graph for " << m_runtime.platform() << " platform using NaiveGraphCompiler";

	validateTopologyStage(sinks);
	scanAllFlowsPhase(sinks);
	mainProcessingStage(sinks);

	core::log::info("compiler") << "compilation complete";
	core::log::info("compiler") << "\t|-instructons: " << m_instructions.size();
	core::log::info("compiler") << "\t|-active placeholders: " << m_placeholderLocations.size();
	core::log::info("compiler") << "\t|-active sinks: " << m_sinkLocations.size();

	return { m_runtime.shared_from_this(), std::move(m_instructions), std::move(m_placeholderLocations), std::move(m_sinkLocations) };
}

void NaiveGraphCompiler::validateTopologyStage(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks)
{
	core::log::info("compiler") << "\t|-validating graph topology";

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
			
			detail::for_each_input(curNode, [&](const core::DataFlow& inputFlow)
			{
				if (breadcrumbs.count(inputFlow.producer().instanceId()))
				{
					throw std::invalid_argument
					(
						std::string(curNode.operationName())
						+ " loops itself"
					);
				}
			});
		}, [&](const core::GraphNode& curNode)
		{
			breadcrumbs.erase(curNode.instanceId());
		});
	}
}

void NaiveGraphCompiler::scanAllFlowsPhase(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks)
{
	core::log::info("compiler") << "\t|-indexing graph edges";

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

				detail::for_each_input(curNode, [&](const core::DataFlow& input)
				{
					auto& flowInfo = m_flows.at(&input);

					flowInfo.usages++;
					constantOutput = constantOutput && flowInfo.constant();
				});

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
	core::log::info("compiler") << "\t|-sorting topologically";
	auto visited = std::set<core::unique_id_t>{};

	for (const auto& sink : sinks)
	{
		detail::recurseTraversalPost(visited, *sink, [this](const core::GraphNode& curNode)
		{
			if (auto placeholderNode = curNode.as<core::PlaceholderNode>())
			{
				processPlaceholderNode(*placeholderNode);
			}
			else if (auto sinkNode = curNode.as<core::SinkNode>())
			{
				processSinkNode(*sinkNode);
			}
			else if (auto referenceNode = curNode.as<core::ReferenceNode>())
			{
				processReferenceNode(*referenceNode);
			}
			else
			{
				processRegularNode(curNode);
			}
		});
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

void NaiveGraphCompiler::processSinkNode(const core::SinkNode& sinkNode)
{
	const auto& flow = sinkNode.inputs().front();
	auto& flowInfo = m_flows.at(flow.get());

	core::EvaluationContext::element_id_t location;
	if (flowInfo.constant())
	{
		location = m_evalCtxAllocator.allocate();

		auto destination = core::destination_operand{ location, flow->dataType() };
		auto source = core::source_operand{ flowInfo.value(), flow->dataType() };

		auto execution = make_operation("Move",
			m_runtime,
			core::destination_operands_range(&destination, &destination + 1),
			core::source_operands_range(&source, &source + 1));
		m_instructions.emplace_back(std::move(execution));
	}
	else
	{
		location = flowInfo.location();
	}
	m_sinkLocations[sinkNode.instanceId()] = { location, flow->dataType() };
}

void NaiveGraphCompiler::processRegularNode(const core::GraphNode& curNode)
{
	static constexpr std::size_t kPreallocatedSize = 8;
	boost::container::small_vector<core::source_operand, kPreallocatedSize> sources;
	boost::container::small_vector<core::destination_operand, kPreallocatedSize> destinations;

	detail::for_each_input(curNode, [&](const core::DataFlow& input)
	{
		sources.emplace_back(convertFlowToOperand(&input));
	});

	const auto constantInputs = std::all_of(sources.begin(), sources.end(), [](const core::source_operand& operand)
	{
		return operand.constant();
	});

	if (constantInputs)
	{
		core::log::info("compiler") << "\t|-Found constant node " << curNode.operationName();
	}

	for (const auto& output : curNode.outputs())
	{
		if (constantInputs)
		{
			destinations.emplace_back(core::EvaluationContext::element_id_t(destinations.size()), output.dataType());
		}
		else
		{
			// TODO what if no one uses one of the outputs?
			auto& outputFlowInfo = m_flows.at(&output);
			outputFlowInfo.setLocation(m_evalCtxAllocator.allocate());
			destinations.emplace_back(outputFlowInfo.location(), output.dataType());
		}
	}

	auto backendOperation = make_operation
	(
		curNode.operationName(),
		m_runtime,
		core::destination_operands_range(destinations.data(), destinations.data() + destinations.size()),
		core::source_operands_range(sources.data(), sources.data() + sources.size())
	);

	if (constantInputs)
	{
		core::log::info("compiler") << "\t\t|-precomputing constant result";

		core::EvaluationContext fakeContext;
		backendOperation->execute(fakeContext);

		auto outputs = curNode.outputs();
		for (std::size_t i = 0, len = outputs.size(); i != len; ++i)
		{
			auto& outputFlowInfo = m_flows.at(&outputs[i]);
			std::any executionResult = std::move(fakeContext.get(destinations[i].location));
			outputFlowInfo.setValue(std::move(executionResult));
		}
	}
	else
	{
		m_instructions.emplace_back(std::move(backendOperation));
	}
}

void NaiveGraphCompiler::processReferenceNode(const core::ReferenceNode& referenceNode)
{
	const auto* curReferenceNode = &referenceNode;

	while (curReferenceNode)
	{
		const core::DataFlow* referenceInput = curReferenceNode->inputs().front().get();

		auto flowIt = m_flows.find(referenceInput);
		assert(flowIt->second.usages > 0);
		flowIt->second.usages--;

		curReferenceNode = referenceInput->producer().as<core::ReferenceNode>();
	}
}

NaiveGraphCompiler::FlowInfo& NaiveGraphCompiler::resolveReference(const core::ReferenceNode& referenceNode)
{
	const auto* curReferenceNode = &referenceNode;
	auto flowIt = m_flows.end();

	while (curReferenceNode)
	{
		const core::DataFlow* referenceInput = curReferenceNode->inputs().front().get();
		flowIt = m_flows.find(referenceInput);
		assert(flowIt->second.usages == 0);
		curReferenceNode = referenceInput->producer().as<core::ReferenceNode>();
	}

	return flowIt->second;
}

core::source_operand NaiveGraphCompiler::convertFlowToOperand(const core::DataFlow* flow)
{
	auto& flowInfo = [&]() -> FlowInfo&
	{
		if (const auto* referenceNode = flow->producer().as<core::ReferenceNode>())
		{
			return resolveReference(*referenceNode);
		}
		else
		{
			return m_flows.at(flow);
		}
	}();

	if (flowInfo.constant())
	{
		if (flowInfo.usages == 0)
		{
			return core::source_operand{ std::move(flowInfo.value()), flow->dataType() };
		}
		else
		{
			return core::source_operand{ flowInfo.value(), flow->dataType() };
		}
	}
	else
	{
		if (flowInfo.usages == 0)
		{
			m_evalCtxAllocator.free(flowInfo.location());
		}
		return core::source_operand{ flowInfo.location(), flow->dataType() };
	}

}

}

