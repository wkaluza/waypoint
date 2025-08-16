#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1").run(waypoint::test::body_call_std_abort);

  t.test(g1, "Test 2").run(waypoint::test::body_call_std_abort);

  t.test(g1, "Test 3").run(waypoint::test::body_call_std_abort);
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success());

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 3);

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    REQUIRE_IN_MAIN(
      outcome.status() == waypoint::TestOutcome::Status::Terminated);
    REQUIRE_IN_MAIN(outcome.assertion_count() == 1);
  }

  return 0;
}
