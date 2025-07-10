#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const g1 = t.group("Test group 1");
  auto const g2 = t.group("Test group 1");
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
