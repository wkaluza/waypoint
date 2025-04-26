#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_TESTS(t)
{
  auto g1 = t.group("Test group 1");
  auto g2 = t.group("Test group 1");
}

} // namespace

auto main() -> int
{
  waypoint::TestEngine t;
  bool const success = initialize(t);
  if(!success)
  {
    return 1;
  }

  return 0;
}
