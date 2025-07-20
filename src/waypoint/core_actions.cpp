#include "impls.hpp"
#include "waypoint.hpp"

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <vector>

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

void populate_test_indices(waypoint::Engine const &t)
{
  waypoint::internal::get_impl(t).set_shuffled_test_record_ptrs();
  auto const &shuffled_ptrs =
    waypoint::internal::get_impl(t).get_shuffled_test_record_ptrs();
  for(unsigned long long i = 0; i < shuffled_ptrs.size(); ++i)
  {
    waypoint::internal::get_impl(t).set_test_index(
      shuffled_ptrs[i]->test_id(),
      i);
  }
}

void initialize(waypoint::Engine const &t)
{
  auto const section = get_autorun_section_boundaries();
  auto const begin = section.begin;
  auto const end = section.end;

  using AutorunFunctionPtr = void (*)(waypoint::Engine const &);

  std::vector<AutorunFunctionPtr> functions;

  for(auto fn_ptr = begin; fn_ptr < end; fn_ptr += sizeof(AutorunFunctionPtr))
  {
    functions.push_back(*reinterpret_cast<AutorunFunctionPtr *>(fn_ptr));
  }

  auto is_not_null = [](auto *ptr)
  {
    return ptr != nullptr;
  };

  for(auto const fn_ptr : functions | std::views::filter(is_not_null))
  {
    fn_ptr(t);
  }

  populate_test_indices(t);
}

} // namespace

namespace waypoint
{

auto make_default_engine() noexcept -> Engine
{
  auto *impl = new internal::Engine_impl{};

  return Engine{impl};
}

auto run_all_tests(Engine const &t) noexcept -> RunResult
{
  initialize(t);
  if(internal::get_impl(t).has_errors())
  {
    // Initialization had errors, skip running tests and emit results
    return internal::get_impl(t).generate_results();
  }

  std::ranges::for_each(
    internal::get_impl(t).get_shuffled_test_record_ptrs(),
    [&t](auto const *ptr) noexcept
    {
      auto context = internal::get_impl(t).make_context(ptr->test_id());
      // Run test
      ptr->test_assembly()(context);
    });

  return internal::get_impl(t).generate_results();
}

} // namespace waypoint
