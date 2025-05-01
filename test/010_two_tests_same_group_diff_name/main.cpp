#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_TESTS(t)
{
  auto g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](waypoint::Context &ctx)
      {
        ctx.assert(true);
      });

  t.test(g1, "Test 2")
    .run(
      [](waypoint::Context &ctx)
      {
        ctx.assert(true);
      });
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
