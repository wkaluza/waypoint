#include "waypoint/waypoint.hpp"

namespace
{

WAYPOINT_AUTORUN(t)
{
  auto g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](auto &ctx)
      {
        ctx.assert(true);
        ctx.assert(false);
      });
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

  auto const results = run_all_tests(t);
  if(results.pass())
  {
    return 1;
  }

  return 0;
}
