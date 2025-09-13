#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <format>
#include <vector>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1").run(waypoint::test::body_call_std_abort);

  t.test(g1, "Test 2").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 3").run(waypoint::test::trivial_failing_body);

  t.test(g1, "Test 4").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 5").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 6").run(waypoint::test::trivial_test_body);
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
    test_count == 6,
    std::format("Expected test_count to be 6, but it is {}", test_count));

  std::vector const statuses = {
    waypoint::TestOutcome::Status::Terminated,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Failure,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Success};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    auto const expected_status = statuses[i];
    auto const actual_status = outcome.status();
    REQUIRE_IN_MAIN(
      actual_status == expected_status,
      std::vformat(
        "Expected actual_status to be {}, but it is {}",
        std::make_format_args(expected_status, actual_status)));

    if(outcome.status() == waypoint::TestOutcome::Status::Terminated)
    {
      REQUIRE_IN_MAIN(
        outcome.assertion_count() == 1,
        std::format(
          "Expected outcome.assertion_count() to be 1, but it is {}",
          outcome.assertion_count()));
    }
    else
    {
      REQUIRE_IN_MAIN(
        outcome.assertion_count() == 2,
        std::format(
          "Expected outcome.assertion_count() to be 2, but it is {}",
          outcome.assertion_count()));
    }
  }

  return 0;
}
