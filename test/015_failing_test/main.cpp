#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_TESTS(t)
{
  auto g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](waypoint::TestContext &ctx)
      {
        ctx.assert(false);
      });
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

  auto const results = run_all_tests(t);
  if(results.pass())
  {
    return 1;
  }

  return 0;
}
