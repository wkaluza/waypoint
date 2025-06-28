#include "waypoint/waypoint.hpp"

#include <utility>

namespace
{

WAYPOINT_AUTORUN(t)
{
  auto g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](auto &ctx)
      {
        auto pi1 = waypoint::internal::UniquePtr<int>{nullptr};
        auto pi2 = waypoint::internal::UniquePtr<int>{nullptr};

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, "Test 2")
    .run(
      [](auto &ctx)
      {
        auto *pi1_original = new int{42};
        auto pi1 = waypoint::internal::UniquePtr{pi1_original};
        auto pi2 = waypoint::internal::UniquePtr<int>{nullptr};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_ref = *pi2;

        ctx.assert(&pi2_ref == pi1_original);
      });

  t.test(g1, "Test 3")
    .run(
      [](auto &ctx)
      {
        auto pi1 = waypoint::internal::UniquePtr<int>{nullptr};
        auto pi2 = waypoint::internal::UniquePtr{new int{123}};

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, "Test 4")
    .run(
      [](auto &ctx)
      {
        auto *pi1_original = new int{42};
        auto pi1 = waypoint::internal::UniquePtr{pi1_original};
        auto pi2 = waypoint::internal::UniquePtr{new int{123}};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_ref = *pi2;

        ctx.assert(&pi2_ref == pi1_original);
      });

  t.test(g1, "Test 5")
    .run(
      [](auto &ctx)
      {
        auto pi1 = waypoint::internal::UniquePtr<int>{nullptr};
        waypoint::internal::UniquePtr const pi2{std::move(pi1)};

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, "Test 6")
    .run(
      [](auto &ctx)
      {
        auto *pi1_original = new int{42};
        auto pi1 = waypoint::internal::UniquePtr{pi1_original};
        waypoint::internal::UniquePtr const pi2{std::move(pi1)};

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_ref = *pi2;

        ctx.assert(&pi2_ref == pi1_original);
      });
}

} // namespace

auto main() -> int
{
  auto t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);
  if(!results.success())
  {
    return 1;
  }

  return 0;
}
