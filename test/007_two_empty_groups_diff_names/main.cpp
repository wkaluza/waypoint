#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_AUTORUN(t)
{
  [[maybe_unused]]
  auto g1 = t.group("Test group 1");
  [[maybe_unused]]
  auto g2 = t.group("Test group 2");
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
