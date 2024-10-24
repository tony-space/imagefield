#include <imf/core/unique_id_t.hpp>

#include <atomic>

namespace imf::core
{

static std::atomic<unique_id_t> nextId{ empty_unique_id + 1};

unique_id_t make_unique_id() noexcept
{
	return nextId.fetch_add(1, std::memory_order::memory_order_relaxed);
}

}
