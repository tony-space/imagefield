#pragma once

#include <imf/core/unique_id_t.hpp>

namespace imf::core
{

class GraphNode;

class DataFlow
{
public:
	DataFlow(GraphNode* _producer, unique_id_t _type);

	DataFlow(const DataFlow&) = delete;
	DataFlow(DataFlow&&) = delete;

	DataFlow& operator=(const DataFlow&) = delete;
	DataFlow& operator=(DataFlow&&) = delete;

	unique_id_t id() const noexcept;
	std::shared_ptr<GraphNode> producer() noexcept;
	std::shared_ptr<const GraphNode> producer() const noexcept;
	unique_id_t dataType() const noexcept;

private:

	GraphNode* m_producer;
	unique_id_t m_id{ make_unique_id() };
	unique_id_t m_dataType;
};

}