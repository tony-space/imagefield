#pragma once

namespace imf::core
{

using unique_id_t = int;

constexpr unique_id_t empty_unique_id{ 0 };

unique_id_t make_unique_id() noexcept;

}
