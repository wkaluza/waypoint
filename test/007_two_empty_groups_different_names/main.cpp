#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_TESTS(t)
{
  auto g1 = t.group("Test group 1");
  auto g2 = t.group("Test group 2");
}

} // namespace

auto main() -> int
{
  return 0;
}
