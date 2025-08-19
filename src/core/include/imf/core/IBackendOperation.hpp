#pragma once

namespace imf::core
{

class Registers;

struct IBackendOperation
{
	virtual ~IBackendOperation() = default;
	virtual void execute(Registers& registers) = 0;
};

}
