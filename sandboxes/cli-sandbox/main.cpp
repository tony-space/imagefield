#include <imf/core/core.hpp>

int main()
{
	try
	{
		using namespace imf::core;

		auto cpuRuntime = make_runtime("cpu");
		auto image = cpuRuntime->loadImage("assets/png/lenna.png");
		
	}
	catch (const std::exception& ex)
	{
		imf::core::log::err("uncaught exception") << ex.what();
	}
	
}
