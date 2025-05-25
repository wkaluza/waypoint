#include "impls.hpp"
#include "waypoint.hpp"

#include <cstdint>

extern char __start_waypoint_tests;
extern char __stop_waypoint_tests;

namespace
{

struct AutorunSectionBoundaries
{
  std::uintptr_t begin;
  std::uintptr_t end;
};

auto get_autorun_section_boundaries() -> AutorunSectionBoundaries
{
  auto const begin = reinterpret_cast<std::uintptr_t>(&__start_waypoint_tests);
  auto const end = reinterpret_cast<std::uintptr_t>(&__stop_waypoint_tests);

  return {begin, end};
}

} // namespace

namespace waypoint
{

auto initialize(Engine &t) -> bool
{
  auto const section = get_autorun_section_boundaries();
  auto const begin = section.begin;
  auto const end = section.end;

  for(auto fn_ptr = begin; fn_ptr < end; fn_ptr += sizeof(void (*)(Engine &)))
  {
    if(auto const autorun_fn = *reinterpret_cast<void (**)(Engine &)>(fn_ptr);
       autorun_fn != nullptr)
    {
      autorun_fn(t);
    }
  }

  bool const success = internal::get_impl(t).verify();

  return success;
}

auto run_all_tests(Engine &t) -> Result
{
  for(internal::TestBodyRecord const &body :
      internal::get_impl(t).test_bodies())
  {
    auto ctx = internal::get_impl(t).make_context(body.test_id());
    body.body()(ctx);
  }

  return internal::get_impl(t).generate_results();
}

} // namespace waypoint
