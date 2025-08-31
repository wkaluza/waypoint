#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <format>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1").run(waypoint::test::trivial_test_body);
  t.test(g1, "Test 2").run(waypoint::test::trivial_test_body);
  t.test(g1, "Test 3").run(waypoint::test::trivial_test_body);
  t.test(g1, "Test 4").run(waypoint::test::trivial_test_body);
  t.test(g1, "Test 5").run(waypoint::test::trivial_test_body);
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = waypoint::run_all_tests(t);

  REQUIRE_IN_MAIN(results.success(), "Expected the run to succeed");
  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(
    error_count == 0,
    std::format("Expected error_count to be 0, but it is {}", error_count));

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(
    test_count == 5,
    std::format("Expected test_count to be 5, but it is {}", test_count));

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &test_outcome = results.test_outcome(i);
    REQUIRE_IN_MAIN(
      !test_outcome.disabled(),
      "Expected test_outcome.disabled() to be false");
    constexpr auto expected_status = waypoint::TestOutcome::Status::Success;
    auto const actual_status = test_outcome.status();
    REQUIRE_IN_MAIN(
      actual_status == expected_status,
      std::vformat(
        "Expected actual_status to be {}, but it is {}",
        std::make_format_args(expected_status, actual_status)));
    REQUIRE_IN_MAIN(
      test_outcome.assertion_count() == 2,
      std::format(
        "Expected test_outcome.assertion_count() to be 2, but it is {}",
        test_outcome.assertion_count()));
    auto const &assertion_outcome1 = test_outcome.assertion_outcome(0);
    REQUIRE_IN_MAIN(
      assertion_outcome1.passed(),
      "Expected assertion_outcome1.passed() to be true");
    REQUIRE_IN_MAIN(
      assertion_outcome1.message() == nullptr,
      "Expected assertion_outcome1.message() == nullptr");
    auto const &assertion_outcome2 = test_outcome.assertion_outcome(1);
    REQUIRE_IN_MAIN(
      assertion_outcome2.passed(),
      "Expected assertion_outcome2.passed() to be true");
    REQUIRE_STRING_EQUAL_IN_MAIN(
      assertion_outcome2.message(),
      "body assertion message",
      std::format("Unexpected string value: {}", assertion_outcome2.message()));
  }

  return 0;
}
