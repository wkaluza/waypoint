#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(t)
{
  auto g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](auto &ctx)
      {
        ctx.assert(false);
        ctx.assert(true);
      });
}

auto main() -> int
{
  auto t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);
  if(results.success())
  {
    return 1;
  }

  return 0;
}
