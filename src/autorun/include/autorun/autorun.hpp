#pragma once

#include <cstdint>
#include <utility>

namespace waypoint::internal
{

auto get_autorun_section_boundaries()
  -> std::pair<std::uintptr_t, std::uintptr_t>;

} // namespace waypoint::internal
