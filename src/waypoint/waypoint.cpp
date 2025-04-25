#include "waypoint.hpp"

#include "string.hpp"
#include "test_types.hpp"
#include "vector.hpp"

#include <cstdint>
#include <utility>

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

auto initialize(TestEngine &t) -> bool
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

  return true;
}

auto TestEngine::group(String name) -> TestGroup
{
  groups_.push_back(TestGroup{std::move(name)});

  return groups_.back();
}

auto TestEngine::test(TestGroup const &group, String name) -> Test
{
  tests_.push_back(Test{group, std::move(name)});

  return tests_.back();
}

} // namespace waypoint

namespace
{

// NOLINTNEXTLINE param may be const
WAYPOINT_TESTS(t)
{
  (void)t;
}

} // namespace
