#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_TESTS(t)
{
  [[maybe_unused]]
  auto g1 = t.group("Test group 1");
  [[maybe_unused]]
  auto g2 = t.group("Test group 2");
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
