#include <imf/core/log.hpp>

namespace imf::core::log
{

void log_stream::flush()
{
	m_device.flush();
}

log_stream& log_stream::operator<< (char c)
{
	m_device.write(&c, 1);
	return *this;
}

log_stream& log_stream::operator<< (std::string_view message)
{
	m_device.write(message.data(), message.size());
	return *this;
}
 
}
