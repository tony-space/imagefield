#include <imf/core/DataFlow.hpp>
#include <imf/core/GraphNode.hpp>

namespace imf::core
{

DataFlow::DataFlow(GraphNode* _producer, TypeID _type) :
	m_producer(_producer),
	m_dataType(std::move(_type))
{
}

unique_id_t DataFlow::id() const noexcept
{
	return m_id;
}

std::shared_ptr<GraphNode> DataFlow::producer() noexcept
{
	return m_producer->shared_from_this();
}

std::shared_ptr<const GraphNode> DataFlow::producer() const noexcept
{
	return m_producer->shared_from_this();
}

const TypeID& DataFlow::dataType() const noexcept
{
	return m_dataType;
}

std::shared_ptr<const DataFlow> DataFlow::sharedPtr() const noexcept
{
	return { m_producer->shared_from_this(), this };
}

}
