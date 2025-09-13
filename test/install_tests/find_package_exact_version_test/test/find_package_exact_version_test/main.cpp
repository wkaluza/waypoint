#include "stub_library/stub_library.hpp"
#include "waypoint/waypoint.hpp"

#include <iostream>

namespace
{

int x = 0;

} // namespace

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("aaa1");
  t.test(g1, "bbb1")
    .run(
      [](auto const &ctx)
      {
        ++x;
        ctx.assert(stub::the_answer() == 42);
      });

  t.test(g1, "bbb2")
    .run(
      [](auto const &ctx)
      {
        ++x;
        ctx.assert(stub::the_answer() == 42);
      });

  t.test(g1, "bbb3")
    .run(
      [](auto const &ctx)
      {
        ++x;
        ctx.assert(stub::the_answer() == 42);
      });
}

auto main() -> int
{
  auto const run = waypoint::TestRun::create();

  auto const result = waypoint::run_all_tests_in_process(run);

  if(!result.success())
  {
    std::cerr << "Expected the run to pass, but it failed" << std::endl;

    return 1;
  }

  if(x != 3)
  {
    std::cerr << "Incorrect value of x == " << x << " instead of 3"
              << std::endl;

    return 1;
  }

  return 0;
}
