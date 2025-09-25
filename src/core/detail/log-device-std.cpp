#include <imf/core/log.hpp>

#include <iostream>

namespace imf::core::log
{

class IOstreamDevice : public log_stream::IDevice
{
public:
	IOstreamDevice()
	{
		std::cout << "-- Image field --";
	}
	
	~IOstreamDevice()
	{
		std::cout << std::endl;
	}

	virtual void putc(char c) override
	{
		std::cout.put(c);
	}

	virtual void write(const char* s, size_t n) override
	{
		std::cout.write(s, n);
	}

	virtual void flush() override
	{
		std::cout.flush();
	}
private:
};

static IOstreamDevice& iostream_device()
{
	static IOstreamDevice device;
	return device;
}

log_stream& info(std::string_view tag)
{
	static log_stream infoStream{ iostream_device() };

	infoStream << "\n[" << tag << "]\t";
	
	return infoStream;
}

log_stream& err(std::string_view tag)
{
	static log_stream errStream{ iostream_device() };

	errStream << "\n[" << tag << "]\t";

	return errStream;
}

}
