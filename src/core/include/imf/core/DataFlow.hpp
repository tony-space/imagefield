#pragma once

#include <imf/core/TypeID.hpp>

#include <memory>

namespace imf::core
{

class GraphNode;

class DataFlow
{
public:
	DataFlow(GraphNode& _producer, TypeID _type) :
		m_producer(&_producer),
		m_dataType(std::move(_type))
	{
	}

	DataFlow(const DataFlow&) = delete;
	DataFlow(DataFlow&&) noexcept = default;

	DataFlow& operator=(const DataFlow&) = delete;
	DataFlow& operator=(DataFlow&&) noexcept = default;

	unique_id_t id() const noexcept { return m_id; }
	GraphNode& producer() noexcept { return *m_producer; }
	const GraphNode& producer() const noexcept { return *m_producer; }
	const TypeID& dataType() const noexcept { return m_dataType; }

	std::shared_ptr<const DataFlow> sharedPtr() const noexcept;

private:
	GraphNode* m_producer;
	unique_id_t m_id{ make_unique_id() };
	TypeID m_dataType;
};

}
