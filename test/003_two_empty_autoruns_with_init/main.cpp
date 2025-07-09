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

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  ++x;
  (void)t;
}

auto main() -> int
{
  auto t = waypoint::make_default_engine();

  auto const result = waypoint::run_all_tests(t);
  if(!result.success())
  {
    return 1;
  }

  if(x != 2)
  {
    return 1;
  }

  return 0;
}
