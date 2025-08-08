#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](auto const &ctx)
      {
        auto const maybe_value = waypoint::test::get_env(
          "WAYPOINT_INTERNAL_RUNNING_TEST_XTSyiOp7QMFW8P2H");
        if(ctx.assume(maybe_value.has_value()))
        {
          ctx.assert(maybe_value.value() == "123");
        }
      });
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(results.success());

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 1);
  auto const maybe_value =
    waypoint::test::get_env("WAYPOINT_INTERNAL_RUNNING_TEST_XTSyiOp7QMFW8P2H");
  REQUIRE_IN_MAIN(maybe_value.has_value());
  REQUIRE_IN_MAIN(maybe_value.value() == "123");

  return 0;
}
