#include <stdexcept>

#include <imf/core/ThreadPool.hpp>

namespace imf::core
{

ThreadPool::ThreadPool() : ThreadPool((std::max)(std::thread::hardware_concurrency(), 1u))
{

}

ThreadPool::ThreadPool(std::size_t workers) : m_workers(workers)
{
	if (workers == 0)
	{
		throw std::invalid_argument("workers number is null");
	}
}

}
