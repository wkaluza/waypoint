#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

namespace
{

int x = 0;

} // namespace

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  ++x;
  (void)t;
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const result = waypoint::run_all_tests_in_process(t);
  REQUIRE_IN_MAIN(result.success());
  REQUIRE_IN_MAIN(x == 1);

  return 0;
}
