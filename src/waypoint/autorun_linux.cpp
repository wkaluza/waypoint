#include "autorun.hpp"

#include <cstdint>
#include <utility>

extern char __start_waypoint_tests;
extern char __stop_waypoint_tests;

namespace waypoint::internal
{

auto get_autorun_section_boundaries()
  -> std::pair<std::uintptr_t, std::uintptr_t>
{
  auto const begin = reinterpret_cast<std::uintptr_t>(&__start_waypoint_tests);
  auto const end = reinterpret_cast<std::uintptr_t>(&__stop_waypoint_tests);

  return {begin, end};
}

} // namespace waypoint::internal
