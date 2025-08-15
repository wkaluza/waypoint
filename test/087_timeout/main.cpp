#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <vector>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .setup(waypoint::test::int_fixture_test_setup)
    .run(waypoint::test::int_fixture_body_long_sleep)
    .timeout_ms(50);

  t.test(g1, "Test 2")
    .setup(waypoint::test::int_fixture_test_setup)
    .run(waypoint::test::int_fixture_body_long_sleep)
    .teardown(waypoint::test::int_fixture_teardown)
    .timeout_ms(50);

  t.test(g1, "Test 3").run(waypoint::test::body_long_sleep).timeout_ms(50);

  t.test(g1, "Test 4")
    .run(waypoint::test::body_long_sleep)
    .timeout_ms(1'000'000)
    .disable();
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success());

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 4);

  std::vector<waypoint::TestOutcome::Status> const expected_statuses = {
    waypoint::TestOutcome::Status::Timeout,
    waypoint::TestOutcome::Status::Timeout,
    waypoint::TestOutcome::Status::Timeout,
    waypoint::TestOutcome::Status::NotRun};
  std::vector<bool> const expected_disabled_states =
    {false, false, false, true};
  std::vector<unsigned> const expected_assertion_counts = {2, 2, 1, 0};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    auto const expected_status = expected_statuses[i];
    REQUIRE_IN_MAIN(outcome.status() == expected_status);

    auto const expected_disabled_state = expected_disabled_states[i];
    REQUIRE_IN_MAIN(outcome.disabled() == expected_disabled_state);

    auto const expected_assertion_count = expected_assertion_counts[i];
    REQUIRE_IN_MAIN(outcome.assertion_count() == expected_assertion_count);
  }

  return 0;
}
