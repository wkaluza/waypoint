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
  waypoint::Engine t;
  bool const success = initialize(t);
  if(!success)
  {
    return 1;
  }

  return 0;
}
