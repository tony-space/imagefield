#pragma once

#include <imf/core/IGraphCompiler.hpp>
#include <imf/core/EvaluationContext.hpp>
#include <imf/core/operand.hpp>

#include <set>
#include <queue>
#include <vector>

namespace imf::core
{
class PlaceholderNode;
class SinkNode;
class ReferenceNode;
}

namespace imf::runtime::cpu
{

class CpuRuntime;

class NaiveGraphCompiler : public core::IGraphCompiler
{
public:
	NaiveGraphCompiler(CpuRuntime& runtime) : m_runtime(runtime) {}
	core::ExecutionPlan build(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks) override;
private:
	class EvaluationContextAllocator
	{
	public:
		core::EvaluationContext::element_id_t allocate()
		{
			if (m_queue.empty())
			{
				return core::EvaluationContext::element_id_t(++m_max.value);
			}

			auto result = m_queue.top();
			m_queue.pop();
			return result;
		}
		void free(core::EvaluationContext::element_id_t v)
		{
			m_queue.push(v);
		}
	private:
		std::priority_queue<
			core::EvaluationContext::element_id_t,
			std::vector<core::EvaluationContext::element_id_t>,
			std::greater<core::EvaluationContext::element_id_t>
		> m_queue;
		core::EvaluationContext::element_id_t m_max;
	};

	struct FlowInfo
	{
		using variant_t = std::variant<core::source_operand::runtime_value_location_t, core::source_operand::compile_time_value_t>;
		variant_t variant{ core::EvaluationContext::element_id_t{} };
		std::size_t usages{ 0 };

		constexpr bool constant() const noexcept
		{
			return std::holds_alternative<core::source_operand::compile_time_value_t>(variant);
		}

		constexpr core::source_operand::runtime_value_location_t location() const
		{
			return std::get<core::source_operand::runtime_value_location_t>(variant);
		}

		core::source_operand::compile_time_value_t& value()
		{
			return std::get<core::source_operand::compile_time_value_t>(variant);
		}

		void setLocation(core::source_operand::runtime_value_location_t locationId) noexcept
		{
			variant = locationId;
		}

		void setValue(core::source_operand::compile_time_value_t v)
		{
			variant = std::move(v);
		}

		static FlowInfo make(core::source_operand::runtime_value_location_t locationId)
		{
			FlowInfo f;
			f.variant = locationId;
			return f;
		}
		static FlowInfo make(core::source_operand::compile_time_value_t compileTimeValue)
		{
			FlowInfo f;
			f.variant = std::move(compileTimeValue);
			return f;
		}
	};

	void validateTopologyStage(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks);
	void scanAllFlowsPhase(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks);

	void mainProcessingStage(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks);

	void processPlaceholderNode(const core::PlaceholderNode& placeholderNode);
	void processSinkNode(const core::SinkNode& sinkNode);
	void processRegularNode(const core::GraphNode& graphNode);
	void processReferenceNode(const core::ReferenceNode& referenceNode);
	FlowInfo& resolveReference(const core::ReferenceNode& referenceNode);

	core::source_operand convertFlowToOperand(const core::DataFlow* flow);

	CpuRuntime& m_runtime;
	EvaluationContextAllocator m_evalCtxAllocator;
	std::map<const core::DataFlow*, FlowInfo> m_flows;

	core::ExecutionPlan::instructions_t m_instructions;
	core::ExecutionPlan::node_locations_t m_placeholderLocations;
	core::ExecutionPlan::node_locations_t m_sinkLocations;
};

}
