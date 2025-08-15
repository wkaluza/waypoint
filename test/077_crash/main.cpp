#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <vector>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 2").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 3").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 4").run(waypoint::test::body_call_std_abort);

  t.test(g1, "Test 5").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 6").run(waypoint::test::trivial_test_body);
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success());

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 6);

  std::vector const statuses = {
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Crashed,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Success};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    auto const expected_status = statuses[i];
    REQUIRE_IN_MAIN(outcome.status() == expected_status);

    if(outcome.status() == waypoint::TestOutcome::Status::Crashed)
    {
      REQUIRE_IN_MAIN(outcome.assertion_count() == 1);
    }
    else
    {
      REQUIRE_IN_MAIN(outcome.assertion_count() == 2);
    }
  }

  return 0;
}
