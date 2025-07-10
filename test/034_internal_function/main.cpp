#include "impls.hpp"

#include "waypoint/waypoint.hpp"

#include <functional>
#include <type_traits>
#include <utility>

// Required to wrap std::function in waypoint::internal::Function
// Likely a false positive by GCC 15 - works fine on Clang 20
#pragma GCC diagnostic ignored "-Warray-bounds"

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](waypoint::Context const &ctx)
      {
        auto ff = []()
        {
        };
        waypoint::internal::Function<void()> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f2)>>);

        f1();
        f2();
      });

  t.test(g1, "Test 2")
    .run(
      [](waypoint::Context const &ctx)
      {
        auto ff = [](int)
        {
        };
        waypoint::internal::Function<void(int)> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f2), int>>);

        f1(0);
        f2(0);
      });

  t.test(g1, "Test 3")
    .run(
      [](waypoint::Context const &ctx)
      {
        auto ff = [](int const x)
        {
          return x;
        };
        waypoint::internal::Function<int(int)> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f2), int>>);

        ctx.assert(f1(42) == 42);
        ctx.assert(f2(42) == 42);
      });

  t.test(g1, "Test 4")
    .run(
      [](waypoint::Context const &ctx)
      {
        std::function<void()> ff{[]()
                                 {
                                 }};
        waypoint::internal::Function<void()> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f2)>>);

        f1();
        f2();
      });

  t.test(g1, "Test 5")
    .run(
      [](auto const &ctx)
      {
        std::function<void(int)> ff{[](int)
                                    {
                                    }};
        waypoint::internal::Function<void(int)> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f2), int>>);

        f1(0);
        f2(0);
      });

  t.test(g1, "Test 6")
    .run(
      [](auto const &ctx)
      {
        std::function<int(int)> ff{[](int x)
                                   {
                                     return x;
                                   }};
        waypoint::internal::Function<int(int)> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f2), int>>);

        ctx.assert(f1(42) == 42);
        ctx.assert(f2(42) == 42);
      });
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);
  if(!results.success())
  {
    return 1;
  }

  return 0;
}
