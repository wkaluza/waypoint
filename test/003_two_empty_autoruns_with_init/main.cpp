#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_TESTS(t)
{
  (void)t;
}

WAYPOINT_TESTS(t)
{
  (void)t;
}

} // namespace

auto main() -> int
{
  auto t = waypoint::make_default_engine();

  bool const success = initialize(t);
  if(!success)
  {
    return 1;
  }

  return 0;
}
