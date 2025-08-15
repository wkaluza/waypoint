#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <vector>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(waypoint::test::body_long_sleep)
    .teardown(waypoint::test::trivial_test_teardown)
    .timeout_ms(50);

  t.test(g1, "Test 2")
    .setup(waypoint::test::trivial_test_setup)
    .run(waypoint::test::body_long_sleep)
    .teardown(waypoint::test::trivial_test_teardown)
    .timeout_ms(50);
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success());

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 2);

  std::vector const expected_assertion_counts = {1U, 2U};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    REQUIRE_IN_MAIN(outcome.status() == waypoint::TestOutcome::Status::Timeout);
    auto const expected_assertion_count = expected_assertion_counts[i];
    REQUIRE_IN_MAIN(outcome.assertion_count() == expected_assertion_count);
  }

  return 0;
}
