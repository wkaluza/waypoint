#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <vector>

WAYPOINT_AUTORUN(waypoint::Engine const &t)
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

  t.test(g1, "Test 19")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::trivial_failing_body);
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests_in_process(t);

  REQUIRE_IN_MAIN(!results.success());

  REQUIRE_IN_MAIN(waypoint::test::x == waypoint::test::x_init + 6);

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 19);

  std::vector<bool> expected_disabled_states = {
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
    false,
    false};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &test_outcome = results.test_outcome(i);
    REQUIRE_IN_MAIN(test_outcome.disabled() == expected_disabled_states[i]);
  }

  std::vector<waypoint::TestOutcome::Status> expected_statuses{
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Failure};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &test_outcome = results.test_outcome(i);
    REQUIRE_IN_MAIN(test_outcome.status() == expected_statuses[i]);
  }

  return 0;
}
