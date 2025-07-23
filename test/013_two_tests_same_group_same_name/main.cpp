#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <cstring>

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 1").run(waypoint::test::trivial_test_body);
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  // We expect the call to run_all_tests to fail
  // due to initialization errors
  auto const result = waypoint::run_all_tests(t);

  // We expect the run to fail
  if(result.success())
  {
    return 1;
  }

  if(result.error_count() != 1)
  {
    return 1;
  }

  if(
    std::strcmp(
      result.error(0),
      R"(Group "Test group 1" contains duplicate test "Test 1")") != 0)
  {
    return 1;
  }

  if(result.test_count() != 0)
  {
    return 1;
  }

  return 0;
}
