#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_AUTORUN(t)
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
  auto t = waypoint::make_default_engine();

  // We expect the call to run_all_tests to fail
  // due to initialization errors
  auto const result = waypoint::run_all_tests(t);

  // Note inverted test result
  if(!result.success())
  {
    return 0;
  }

  return 1;
}
