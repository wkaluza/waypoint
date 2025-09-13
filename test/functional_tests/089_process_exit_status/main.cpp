#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <format>
#include <vector>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::body_call_std_abort);

  t.test(g1, "Test 2")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::body_call_std_exit_123)
    .teardown(waypoint::test::trivial_test_teardown);

  t.test(g1, "Test 3")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::body_throws_exception);

  t.test(g1, "Test 4")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 5")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::trivial_failing_body);

  t.test(g1, "Test 6")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::body_failing_assertion);

  t.test(g1, "Test 7")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::trivial_failing_body)
    .disable();

  t.test(g1, "Test 8")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::body_long_sleep)
    .timeout_ms(10);

  t.test(g1, "Test 9")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::body_throws_exception_while_noexcept);
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success(), "Expected the run to fail");

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(
    error_count == 0,
    std::format("Expected error_count to be 0, but it is {}", error_count));

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(
    test_count == 9,
    std::format("Expected test_count to be 9, but it is {}", test_count));

  std::vector const expected_statuses = {
    waypoint::TestOutcome::Status::Terminated,
    waypoint::TestOutcome::Status::Terminated,
    waypoint::TestOutcome::Status::Terminated,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Failure,
    waypoint::TestOutcome::Status::Terminated,
    waypoint::TestOutcome::Status::NotRun,
    waypoint::TestOutcome::Status::Timeout,
    waypoint::TestOutcome::Status::Terminated,
  };
  std::vector const expected_disabled_states =
    {false, false, false, false, false, false, true, false, false};
  std::vector const expected_assertion_counts =
    {2U, 2U, 2U, 3U, 3U, 2U, 0U, 2U, 2U};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    auto const expected_status = expected_statuses[i];
    auto const actual_status = outcome.status();
    REQUIRE_IN_MAIN(
      actual_status == expected_status,
      std::vformat(
        "Expected actual_status to be {}, but it is {}",
        std::make_format_args(expected_status, actual_status)));

    auto const expected_disabled_state = expected_disabled_states[i];
    REQUIRE_IN_MAIN(
      outcome.disabled() == expected_disabled_state,
      std::format(
        "Expected outcome.disabled() to be {}",
        expected_disabled_state));

    auto const expected_assertion_count = expected_assertion_counts[i];
    REQUIRE_IN_MAIN(
      outcome.assertion_count() == expected_assertion_count,
      std::format(
        "Expected outcome.assertion_count() to be {}, but it is {}",
        expected_assertion_count,
        outcome.assertion_count()));

    if(outcome.status() == waypoint::TestOutcome::Status::Terminated)
    {
      REQUIRE_IN_MAIN(
        outcome.exit_code() != nullptr,
        "Expected outcome.exit_code() != nullptr");
      REQUIRE_IN_MAIN(*outcome.exit_code() > 0, "*outcome.exit_code() > 0");
    }
    else
    {
      REQUIRE_IN_MAIN(
        outcome.exit_code() == nullptr,
        "Expected outcome.exit_code() == nullptr");
    }
  }

  return 0;
}
