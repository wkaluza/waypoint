// Copyright (c) 2025 Wojciech Kałuża
// SPDX-License-Identifier: MIT
// For license details, see LICENSE file

#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g = t.group("Test group");

  t.test(g, "Test 1")
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(true);
      });

  t.test(g, "Test 2")
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(true);
      });

  t.test(g, "Test 3")
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(true);
      });
}
