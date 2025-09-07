#pragma once

#include <imf/core/EventLoop.hpp>

namespace imf::core
{

class ThreadPool
{
public:
	ThreadPool();
	ThreadPool(std::size_t workers);
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) noexcept = delete;

	ThreadPool& operator= (const ThreadPool&) = delete;
	ThreadPool& operator= (ThreadPool&&) noexcept = delete;

	std::size_t workers() const noexcept { return m_workers.size(); }
	EventLoop& at(std::size_t idx) noexcept { return m_workers.at(idx); }

	decltype(auto) begin() noexcept { return m_workers.begin(); }
	decltype(auto) end() noexcept { return m_workers.end(); }

	template <typename Func, typename Int>
	void forEachSync(const Func& f, Int items, Int minBatchSize = 1);
private:
	std::vector<EventLoop> m_workers;
};

template <typename Func, typename Int>
void ThreadPool::forEachSync(const Func& f, Int items, Int minBatchSize)
{
	const auto workersCount = static_cast<Int>(m_workers.size());
	const auto workPerThread = (std::max)(minBatchSize, (items + workersCount - 1) / workersCount);

	for (Int i = 0; i < workersCount; i++)
	{
		Int rageStart = i * workPerThread;
		Int rangeEnd = (std::min)(items, rageStart + workPerThread);

		if (rageStart >= rangeEnd)
			break;

		m_workers[i].enqueue([&f, rageStart, rangeEnd]()
		{
			f(rageStart, rangeEnd);
		});
	}

	for (Int i = 0; i < workersCount; i++)
	{
		Int rageStart = i * workPerThread;
		Int rangeEnd = (std::min)(items, rageStart + workPerThread);

		if (rageStart >= rangeEnd)
			break;

		m_workers[i].synchronize();
	}
}

}
