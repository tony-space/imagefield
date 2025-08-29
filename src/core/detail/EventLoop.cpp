#include <imf/core/EventLoop.hpp>

namespace imf::core
{

EventLoop::~EventLoop() noexcept
{
	enqueue([this]
	{
		m_running = false;
	});

	m_thread.join();
}

void EventLoop::enqueue(callable_t&& callable) noexcept
{
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		m_writeBuffer.emplace_back(std::move(callable));
	}
	m_condVar.notify_one();
}

void EventLoop::synchronize() noexcept
{
	enqueueSync([] {});
}

std::thread::id EventLoop::threadId() const noexcept
{
	return m_thread.get_id();
}

void EventLoop::threadFunc() noexcept
{
	std::vector<callable_t> readBuffer;

	while (m_running)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_condVar.wait(lock, [this]
			{
				return !m_writeBuffer.empty();
			});
			std::swap(readBuffer, m_writeBuffer);
		}

		for (auto& func : readBuffer)
		{
			func();
			func = nullptr;
		}

		readBuffer.clear();
	}
}

}
