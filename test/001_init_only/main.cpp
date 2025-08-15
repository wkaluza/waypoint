#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const result = waypoint::run_all_tests_in_process(t);
  REQUIRE_IN_MAIN(result.success());

  return 0;
}
