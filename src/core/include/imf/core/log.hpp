#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

namespace imf::core::log
{

class log_stream;
using stream_manipulator_t = std::function<log_stream&(log_stream&)>;

class log_stream
{
public:
	struct IDevice
	{
		virtual ~IDevice() = default;

		virtual void flush() = 0;
		virtual void putc(char c) = 0;
		virtual void write(const char* s, size_t n) = 0;
	};

	log_stream() = delete;
	log_stream(IDevice& d) : m_device(d) {};
	log_stream(const log_stream&) = delete;
	log_stream(log_stream&&) noexcept = delete;

	log_stream& operator=(const log_stream&) = delete;
	log_stream& operator=(log_stream&&) noexcept = delete;

	void flush();

	log_stream& operator<< (char);
	log_stream& operator<< (std::string_view message);
	template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true> log_stream& operator<< (T value)
	{
		return *this << std::to_string(value);
	}
private:
	IDevice& m_device;
};

log_stream& info(std::string_view tag);
log_stream& err(std::string_view tag);

}
