#include "waypoint/waypoint.hpp"

auto main() -> int
{
  auto t = waypoint::make_default_engine();

  auto const result = waypoint::run_all_tests(t);
  if(!result.success())
  {
    return 1;
  }

  return 0;
}
