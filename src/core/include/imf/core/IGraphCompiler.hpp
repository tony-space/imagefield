#pragma once

#include <imf/core/iterator_range.hpp>
#include <imf/core/ExecutionPlan.hpp>

namespace imf::core
{

class SinkNode;

struct IGraphCompiler
{
	virtual ~IGraphCompiler() = default;
	virtual ExecutionPlan build(const iterator_range<const std::shared_ptr<const SinkNode>*>& sinks) = 0;
};

}
