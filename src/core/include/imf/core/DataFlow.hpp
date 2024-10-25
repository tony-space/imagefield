#pragma once

#include <imf/core/TypeID.hpp>

#include <memory>

namespace imf::core
{

class GraphNode;

class DataFlow
{
public:
	DataFlow(GraphNode* _producer, TypeID _type);

	DataFlow(const DataFlow&) = delete;
	DataFlow(DataFlow&&) = delete;

	DataFlow& operator=(const DataFlow&) = delete;
	DataFlow& operator=(DataFlow&&) = delete;

	unique_id_t id() const noexcept;
	std::shared_ptr<GraphNode> producer() noexcept;
	std::shared_ptr<const GraphNode> producer() const noexcept;
	const TypeID& dataType() const noexcept;

	std::shared_ptr<const DataFlow> sharedPtr() const noexcept;

private:
	GraphNode* m_producer;
	unique_id_t m_id{ make_unique_id() };
	TypeID m_dataType;
};

}
