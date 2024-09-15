#include <imf/core/log.hpp>

#include <iostream>

namespace imf::core::log
{

namespace TerminalColors
{
	constexpr static std::string_view Reset = "\033[0m";
	//constexpr static std::string_view Black = "\033[30m";
	constexpr static std::string_view Red = "\033[31m";
	constexpr static std::string_view Green = "\033[32m";
	//constexpr static std::string_view Yellow = "\033[33m";
	//constexpr static std::string_view Blue = "\033[34m";
	//constexpr static std::string_view Magenta = "\033[35m";
	//constexpr static std::string_view Cyan = "\033[36m";
	//constexpr static std::string_view White = "\033[37m";
	//constexpr static std::string_view BoldBlack = "\033[1m\033[30m";
	//constexpr static std::string_view BoldRed = "\033[1m\033[31m";
	//constexpr static std::string_view BoldGreen = "\033[1m\033[32m";
	//constexpr static std::string_view BoldYellow = "\033[1m\033[33m";
	//constexpr static std::string_view BoldBlue = "\033[1m\033[34m";
	constexpr static std::string_view BoldMagenta = "\033[1m\033[35m";
	//constexpr static std::string_view BoldCyan = "\033[1m\033[36m";
	//constexpr static std::string_view BoldWhite = "\033[1m\033[37m";
};

class IOstreamDevice : public log_stream::IDevice
{
public:
	IOstreamDevice()
	{
		std::cout << TerminalColors::BoldMagenta << "-- Image field --";
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

	iostream_device().write(TerminalColors::Green.data(), TerminalColors::Green.size());
	infoStream << "\n[" << tag << "] ";
	iostream_device().write(TerminalColors::Reset.data(), TerminalColors::Reset.size());

	return infoStream;
}

log_stream& err(std::string_view tag)
{
	static log_stream errStream{ iostream_device() };

	iostream_device().write(TerminalColors::Red.data(), TerminalColors::Red.size());
	errStream << "\n[" << tag << "] ";
	iostream_device().write(TerminalColors::Reset.data(), TerminalColors::Reset.size());

	return errStream;
}

}
