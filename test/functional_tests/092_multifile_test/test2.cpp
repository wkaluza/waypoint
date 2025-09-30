// Copyright (c) 2025 Wojciech Kałuża
// SPDX-License-Identifier: MIT
// For license details, see LICENSE file

#include "waypoint/waypoint.hpp"

namespace test
{

extern int y;

} // namespace test

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 2")
    .run(
      [](waypoint::Context const &ctx)
      {
        ::test::y += 2;
        ctx.assert(true);
      });
}
