#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <cstring>

WAYPOINT_AUTORUN(waypoint::Engine const &t)
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
  auto const t = waypoint::make_default_engine();

  auto const results = waypoint::run_all_tests(t);

  REQUIRE_IN_MAIN(results.success());
  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 5);

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &test_outcome = results.test_outcome(i);
    REQUIRE_IN_MAIN(!test_outcome.disabled());
    REQUIRE_IN_MAIN(
      test_outcome.status() == waypoint::TestOutcome::Status::Success);
    REQUIRE_IN_MAIN(test_outcome.assertion_count() == 2);
    auto const &assertion_outcome1 = test_outcome.assertion_outcome(0);
    REQUIRE_IN_MAIN(assertion_outcome1.passed());
    REQUIRE_IN_MAIN(assertion_outcome1.message() == nullptr);
    auto const &assertion_outcome2 = test_outcome.assertion_outcome(1);
    REQUIRE_IN_MAIN(assertion_outcome2.passed());
    REQUIRE_IN_MAIN(
      std::strcmp(assertion_outcome2.message(), "body assertion message") == 0);
  }

  return 0;
}
