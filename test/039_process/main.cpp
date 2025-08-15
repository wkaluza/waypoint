#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <cstring>
#include <vector>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](auto const &ctx)
      {
        if(!ctx.assume(true) || ctx.assume(true, "body assertion message"))
        {
        }
      });
  t.test(g1, "Test 2").run(waypoint::test::trivial_test_body);
  t.test(g1, "Test 3").run(waypoint::test::trivial_test_body);
  t.test(g1, "Test 4").run(waypoint::test::trivial_failing_body);
  t.test(g1, "Test 5").run(waypoint::test::trivial_test_body);
  t.test(g1, "Test 6").run(waypoint::test::trivial_test_body).disable();
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = waypoint::run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success());
  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 6);

  std::vector<waypoint::TestOutcome::Status> const statuses{
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::Failure,
    waypoint::TestOutcome::Status::Success,
    waypoint::TestOutcome::Status::NotRun};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &test_outcome = results.test_outcome(i);
    auto const expected_status = statuses[i];
    REQUIRE_IN_MAIN(test_outcome.status() == expected_status);
    if(expected_status == waypoint::TestOutcome::Status::Success)
    {
      REQUIRE_IN_MAIN(!test_outcome.disabled());
      REQUIRE_IN_MAIN(test_outcome.assertion_count() == 2);
      auto const &assertion_outcome1 = test_outcome.assertion_outcome(0);
      REQUIRE_IN_MAIN(assertion_outcome1.passed());
      REQUIRE_IN_MAIN(assertion_outcome1.message() == nullptr);
      auto const &assertion_outcome2 = test_outcome.assertion_outcome(1);
      REQUIRE_IN_MAIN(assertion_outcome2.passed());
      REQUIRE_IN_MAIN(
        std::strcmp(assertion_outcome2.message(), "body assertion message") ==
        0);
    }
    else if(expected_status == waypoint::TestOutcome::Status::Failure)
    {
      REQUIRE_IN_MAIN(!test_outcome.disabled());
      REQUIRE_IN_MAIN(test_outcome.assertion_count() == 2);
      auto const &assertion_outcome1 = test_outcome.assertion_outcome(0);
      REQUIRE_IN_MAIN(!assertion_outcome1.passed());
      REQUIRE_IN_MAIN(assertion_outcome1.message() == nullptr);
      auto const &assertion_outcome2 = test_outcome.assertion_outcome(1);
      REQUIRE_IN_MAIN(!assertion_outcome2.passed());
      REQUIRE_IN_MAIN(
        std::strcmp(
          assertion_outcome2.message(),
          "failing body assertion message") == 0);
    }
    else if(expected_status == waypoint::TestOutcome::Status::NotRun)
    {
      REQUIRE_IN_MAIN(test_outcome.disabled());
      REQUIRE_IN_MAIN(test_outcome.assertion_count() == 0);
    }
  }

  return 0;
}
