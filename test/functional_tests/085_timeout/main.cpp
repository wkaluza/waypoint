#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <format>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::body_long_sleep)
    .timeout_ms(50);
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
    test_count == 1,
    std::format("Expected test_count to ba 1, but it is {}", test_count));

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    constexpr auto expected_status = waypoint::TestOutcome::Status::Timeout;
    auto const actual_status = outcome.status();
    REQUIRE_IN_MAIN(
      actual_status == expected_status,
      std::vformat(
        "Expected actual_status to be {}, but it is {}",
        std::make_format_args(expected_status, actual_status)));
    REQUIRE_IN_MAIN(
      outcome.assertion_count() == 2,
      std::format(
        "Expected outcome.assertion_count() to be 2, but it is {}",
        outcome.assertion_count()));
  }

  return 0;
}
