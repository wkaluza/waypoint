#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  [[maybe_unused]]
  auto const g1 = t.group("Test group 1");
  [[maybe_unused]]
  auto const g2 = t.group("Test group 2");
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
