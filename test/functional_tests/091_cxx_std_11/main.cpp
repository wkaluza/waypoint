// Copyright (c) 2025 Wojciech Kałuża
// SPDX-License-Identifier: MIT
// For license details, see LICENSE file

#include "waypoint/waypoint.hpp"

#include <iostream>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(true);
      });
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);
  if(!results.success())
  {
    std::cerr << "Expected the run to succeed" << std::endl;

    return 1;
  }

  return 0;
}
