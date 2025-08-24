#pragma once

#include <imf/core/IGraphCompiler.hpp>

namespace imf::runtime::cpu
{

class NaiveGraphCompiler : public core::IGraphCompiler
{
public:
	core::ExecutionPlan build(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks) override;
private:

	static void validateTopology(const core::iterator_range<const std::shared_ptr<const core::SinkNode>*>& sinks);

};

}
