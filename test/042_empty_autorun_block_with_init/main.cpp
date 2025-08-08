#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

namespace
{

int x = 0;

} // namespace

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  ++x;
  (void)t;
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const result = waypoint::run_all_tests(t);
  REQUIRE_IN_MAIN(result.success());
  REQUIRE_IN_MAIN(x == 1);

  return 0;
}
