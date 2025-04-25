#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_TESTS(t)
{
  auto t1 = t.group("Test group 1");
}

} // namespace

auto main() -> int
{
  return 0;
}
