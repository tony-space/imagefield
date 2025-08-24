#pragma once

#include <imf/core/EvaluationContext.hpp>
#include <imf/core/GraphNode.hpp>
#include <imf/core/IBackendOperation.hpp>
#include <imf/core/TypeID.hpp>
#include <imf/core/unique_id_t.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace imf::core
{

struct IRuntime;

class ExecutionPlan
{
public:
	struct NodeInfo
	{
		EvaluationContext::element_id_t location;
		TypeID dataType;
	};

	using node_locations_t = std::map<unique_id_t, NodeInfo>;
	using instructions_t = std::vector<std::unique_ptr<IBackendOperation>>;

	ExecutionPlan() = default;
	ExecutionPlan(const std::shared_ptr<IRuntime>& runtime, instructions_t instructions, node_locations_t placeholderLocations, node_locations_t sinkLocations) :
		m_runtime(runtime),
		m_instructions(std::move(instructions)),
		m_placeholderLocations(std::move(placeholderLocations)),
		m_sinkLocations(std::move(sinkLocations))
	{
	}
	ExecutionPlan(const ExecutionPlan&) = delete;
	ExecutionPlan(ExecutionPlan&&) noexcept = default;
	ExecutionPlan& operator=(const ExecutionPlan&) = delete;
	ExecutionPlan& operator=(ExecutionPlan&&) noexcept = default;

	EvaluationContext::element_id_t placeholderLocation(unique_id_t id) const
	{
		return m_placeholderLocations.at(id).location;
	}
	EvaluationContext::element_id_t sinkLocation(unique_id_t id) const
	{
		return m_sinkLocations.at(id).location;
	}
	TypeID placeholderType(unique_id_t id) const
	{
		return m_placeholderLocations.at(id).dataType;
	}
	TypeID sinkType(unique_id_t id) const
	{
		return m_sinkLocations.at(id).dataType;
	}
	const instructions_t& instructions() const noexcept
	{
		return m_instructions;
	}
	const node_locations_t& placeholders() const noexcept
	{
		return m_placeholderLocations;
	}
	const node_locations_t& sinks() const noexcept
	{
		return m_sinkLocations;
	}
	void clear() noexcept
	{
		m_instructions.clear();
		m_placeholderLocations.clear();
		m_sinkLocations.clear();
	}
private:
	std::shared_ptr<IRuntime> m_runtime;
	instructions_t m_instructions;
	node_locations_t m_placeholderLocations;
	node_locations_t m_sinkLocations;
};

}
