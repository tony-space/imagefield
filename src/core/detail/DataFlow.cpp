#include <imf/core/DataFlow.hpp>
#include <imf/core/GraphNode.hpp>

namespace imf::core
{

std::shared_ptr<const DataFlow> DataFlow::sharedPtr() const noexcept
{
	return { m_producer->shared_from_this(), this };
}

}
