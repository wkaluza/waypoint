#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(true);
      });

  t.test(g1, "Test 2")
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(true);
      });
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const result = waypoint::run_all_tests(t);
  REQUIRE_IN_MAIN(result.success(), "Expected the run to succeed");

  return 0;
}
