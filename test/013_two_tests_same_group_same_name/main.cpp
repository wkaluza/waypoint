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

  t.test(g1, "Test 1")
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
  // We expect the call to initialize to fail
  bool const success = initialize(t);

  // Note inverted test result
  if(!success)
  {
    return 0;
  }

  return 1;
}
