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
  waypoint::TestEngine t;
  initialize(t);

  return 0;
}
