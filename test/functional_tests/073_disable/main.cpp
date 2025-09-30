// Copyright (c) 2025 Wojciech Kałuża
// SPDX-License-Identifier: MIT
// For license details, see LICENSE file

#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <format>
#include <vector>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1").run(waypoint::test::increment_x_test_body).disable();

  t.test(g1, "Test 2").run(waypoint::test::increment_x_test_body).disable(true);

  t.test(g1, "Test 3")
    .run(waypoint::test::increment_x_test_body)
    .disable(false);

  t.test(g1, "Test 4")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::increment_x_test_body)
    .disable();

  t.test(g1, "Test 5")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::increment_x_test_body)
    .disable(true);

  t.test(g1, "Test 6")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::increment_x_test_body)
    .disable(false);

  t.test(g1, "Test 7")
    .setup(waypoint::test::int_fixture_test_setup)
    .run(waypoint::test::int_fixture_increment_x_test_body)
    .disable();

  t.test(g1, "Test 8")
    .setup(waypoint::test::int_fixture_test_setup)
    .run(waypoint::test::int_fixture_increment_x_test_body)
    .disable(true);

  t.test(g1, "Test 9")
    .setup(waypoint::test::int_fixture_test_setup)
    .run(waypoint::test::int_fixture_increment_x_test_body)
    .disable(false);

  t.test(g1, "Test 10")
    .run(waypoint::test::increment_x_test_body)
    .teardown(waypoint::test::trivial_test_teardown)
    .disable();

  t.test(g1, "Test 11")
    .run(waypoint::test::increment_x_test_body)
    .teardown(waypoint::test::trivial_test_teardown)
    .disable(true);

  t.test(g1, "Test 12")
    .run(waypoint::test::increment_x_test_body)
    .teardown(waypoint::test::trivial_test_teardown)
    .disable(false);

  t.test(g1, "Test 13")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::increment_x_test_body)
    .teardown(waypoint::test::trivial_test_teardown)
    .disable();

  t.test(g1, "Test 14")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::increment_x_test_body)
    .teardown(waypoint::test::trivial_test_teardown)
    .disable(true);

  t.test(g1, "Test 15")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::increment_x_test_body)
    .teardown(waypoint::test::trivial_test_teardown)
    .disable(false);

  t.test(g1, "Test 16")
    .setup(waypoint::test::int_fixture_test_setup)
    .run(waypoint::test::int_fixture_increment_x_test_body)
    .teardown(waypoint::test::int_fixture_teardown)
    .disable();

  t.test(g1, "Test 17")
    .setup(waypoint::test::int_fixture_test_setup)
    .run(waypoint::test::int_fixture_increment_x_test_body)
    .teardown(waypoint::test::int_fixture_teardown)
    .disable(true);

  t.test(g1, "Test 18")
    .setup(waypoint::test::int_fixture_test_setup)
    .run(waypoint::test::int_fixture_increment_x_test_body)
    .teardown(waypoint::test::int_fixture_teardown)
    .disable(false);
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(results.success(), "Expected the run to succeed");

  // Not valid when testing in child process
  // REQUIRE_IN_MAIN(
  // waypoint::test::x == waypoint::test::x_init + 6,
  // std::format(
  // "Incorrect value of x == {} instead of {}",
  // waypoint::test::x,
  // waypoint::test::x_init + 6));

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(
    error_count == 0,
    std::format("Incorrect error_count == {} instead of 0", error_count));

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(
    test_count == 18,
    std::format("Incorrect test_count == {} instead of 18", test_count));

  std::vector const expected_disabled_states = {
    true,
    true,
    false,
    true,
    true,
    false,
    true,
    true,
    false,
    true,
    true,
    false,
    true,
    true,
    false,
    true,
    true,
    false};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &test_outcome = results.test_outcome(i);
    REQUIRE_IN_MAIN(
      test_outcome.disabled() == expected_disabled_states[i],
      std::format(
        "Expected test_outcome.disabled() to return {}, instead got {}",
        expected_disabled_states[i],
        test_outcome.disabled()));
  }

  return 0;
}
