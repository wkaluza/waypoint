#include "waypoint/waypoint.hpp"

namespace test
{

extern int y;

} // namespace test

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](waypoint::Context const &ctx)
      {
        ::test::y += 1;
        ctx.assert(true);
      });
}
