// Copyright (c) 2025 Wojciech Kałuża
// SPDX-License-Identifier: MIT
// For license details, see LICENSE file

#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <format>

namespace
{

int x = 0;

} // namespace

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  ++x;
  (void)t;
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const result = waypoint::run_all_tests_in_process(t);
  REQUIRE_IN_MAIN(result.success(), "Expected the run to succeed");
  REQUIRE_IN_MAIN(x == 1, std::format("Expected x to be 1, but it is {}", x));

  return 0;
}
