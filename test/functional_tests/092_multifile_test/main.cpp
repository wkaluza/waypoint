#include "waypoint/waypoint.hpp"

#include <iostream>

namespace test
{

extern int y;

} // namespace test

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = waypoint::run_all_tests_in_process(t);
  if(!results.success())
  {
    std::cerr << "Expected the run to succeed" << std::endl;

    return 1;
  }

  if(::test::y != 6)
  {
    std::cerr << "Incorrect value of y == " << ::test::y << " instead of 6"
              << std::endl;

    return 1;
  }

  return 0;
}
