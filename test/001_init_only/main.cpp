#include "waypoint/waypoint.hpp"

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
