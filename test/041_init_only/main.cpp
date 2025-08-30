#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const result = waypoint::run_all_tests(t);
  REQUIRE_IN_MAIN(result.success(), "Expected the run to succeed");

  return 0;
}
