#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1").run(waypoint::test::trivial_test_body);

  auto const g2 = t.group("Test group 2");

  t.test(g2, "Test 1").run(waypoint::test::trivial_test_body);
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const result = waypoint::run_all_tests(t);
  if(!result.success())
  {
    return 1;
  }

  return 0;
}
