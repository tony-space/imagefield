#pragma once

namespace imf::core
{

class EvaluationContext;

struct IBackendOperation
{
	virtual ~IBackendOperation() = default;
	virtual void execute(EvaluationContext& evalContext) = 0;
};

}
