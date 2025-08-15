#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  [[maybe_unused]]
  auto const g1 = t.group("Test group 1");
  [[maybe_unused]]
  auto const g2 = t.group("Test group 2");
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const result = waypoint::run_all_tests(t);
  REQUIRE_IN_MAIN(result.success());

  return 0;
}
