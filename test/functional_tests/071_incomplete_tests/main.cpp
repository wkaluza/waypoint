// Copyright (c) 2025 Wojciech Kałuża
// SPDX-License-Identifier: MIT
// For license details, see LICENSE file

#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <format>

// False positives from clang-tidy (according to Valgrind)
// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  (void)t.test(g1, "Test 1");

  t.test(g1, "Test 2")
    .run(
      waypoint::test::body_factory_no_fixture(
        waypoint::test::x_init,
        waypoint::test::x_init));

  (void)t.test(g1, "Test 3")
    .setup(waypoint::test::void_setup_factory(waypoint::test::x_init));

  t.test(g1, "Test 4")
    .setup(waypoint::test::setup_factory_fixture<int>(42, 77))
    .run(
      waypoint::test::body_factory_fixture<int>(
        waypoint::test::x_init,
        42,
        77));

  (void)t.test(g1, "Test 5")
    .setup(
      waypoint::test::setup_factory_fixture<int>(42, waypoint::test::x_init));

  t.test(g1, "Test 6")
    .setup(waypoint::test::setup_factory_fixture<waypoint::test::X>(42, 66))
    .run(
      waypoint::test::body_factory_fixture<waypoint::test::X>(
        waypoint::test::x_init,
        42,
        66));

  (void)t.test(g1, "Test 7")
    .setup(
      waypoint::test::setup_factory_fixture<waypoint::test::X>(
        42,
        waypoint::test::x_init));
}

// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success(), "Expected the run to fail");

  auto const count = results.error_count();

  REQUIRE_IN_MAIN(
    count == 4,
    std::format("Expected count to be 4, but it is {}", count));
  REQUIRE_STRING_EQUAL_IN_MAIN(
    results.error(0),
    R"(Test "Test 1" in group "Test group 1" is incomplete. )"
    R"(Call the run(...) method to fix this.)",
    std::format("Unexpected string value: {}", results.error(0)));
  REQUIRE_STRING_EQUAL_IN_MAIN(
    results.error(1),
    R"(Test "Test 3" in group "Test group 1" is incomplete. )"
    R"(Call the run(...) method to fix this.)",
    std::format("Unexpected string value: {}", results.error(1)));
  REQUIRE_STRING_EQUAL_IN_MAIN(
    results.error(2),
    R"(Test "Test 5" in group "Test group 1" is incomplete. )"
    R"(Call the run(...) method to fix this.)",
    std::format("Unexpected string value: {}", results.error(2)));
  REQUIRE_STRING_EQUAL_IN_MAIN(
    results.error(3),
    R"(Test "Test 7" in group "Test group 1" is incomplete. )"
    R"(Call the run(...) method to fix this.)",
    std::format("Unexpected string value: {}", results.error(3)));
  REQUIRE_IN_MAIN(
    results.test_count() == 0,
    std::format(
      "Expected results.test_count() to be 0, but it is {}",
      results.test_count()));

  return 0;
}
