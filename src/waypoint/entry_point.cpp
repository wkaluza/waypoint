#include "waypoint/waypoint.hpp"

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = waypoint::run_all_tests(t);

  if(results.error_count() > 0)
  {
    return 1;
  }

  if(!results.success())
  {
    return 1;
  }

  return 0;
}
