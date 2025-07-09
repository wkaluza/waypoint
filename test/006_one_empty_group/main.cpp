#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto g1 = t.group("Test group 1");
}

auto main() -> int
{
  auto t = waypoint::make_default_engine();

  auto const result = waypoint::run_all_tests(t);
  if(!result.success())
  {
    return 1;
  }

  return 0;
}
