#include "waypoint/waypoint.hpp"

#include <functional>
#include <string>
#include <vector>

namespace
{

WAYPOINT_AUTORUN(t)
{
  std::string const captured1 = "42";
  std::vector<int> const captured2 = {1, 2, 3, 4};
  std::function<int()> const captured3 = []()
  {
    return 123;
  };

  auto g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [=](waypoint::Context &ctx)
      {
        ctx.assert(captured1 == "42");
        ctx.assert(captured2.size() == 4);
        ctx.assert(captured3() == 123);
      });
}

} // namespace

auto main() -> int
{
  auto t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);
  if(results.success())
  {
    return 0;
  }

  return 1;
}
