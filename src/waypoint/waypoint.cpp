#include "waypoint.hpp"

#include <cstdint>

extern char __start_waypoint_tests;
extern char __stop_waypoint_tests;

namespace waypoint::internal
{

struct AutorunSectionBoundaries
{
  std::uintptr_t begin;
  std::uintptr_t end;
};

} // namespace waypoint::internal

namespace
{

auto get_autorun_section_boundaries()
  -> waypoint::internal::AutorunSectionBoundaries
{
  auto const begin = reinterpret_cast<std::uintptr_t>(&__start_waypoint_tests);
  auto const end = reinterpret_cast<std::uintptr_t>(&__stop_waypoint_tests);

  return {begin, end};
}

} // namespace

namespace waypoint
{

void initialize(TestEngine &t)
{
  auto const section = get_autorun_section_boundaries();
  auto const begin = section.begin;
  auto const end = section.end;

  for(auto fn_ptr = begin; fn_ptr < end; fn_ptr += sizeof(internal::AutorunFn))
  {
    if(auto const autorun_fn = *reinterpret_cast<internal::AutorunFn *>(fn_ptr);
       autorun_fn != nullptr)
    {
      autorun_fn(t);
    }
  }
}

} // namespace waypoint

namespace
{

WAYPOINT_TESTS(t)
{
  (void)t;
}

} // namespace
