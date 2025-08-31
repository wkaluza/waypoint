#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <format>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 2").run(waypoint::test::trivial_test_body);
  t.test(g1, "Test 2").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 3").run(waypoint::test::trivial_test_body);
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = waypoint::run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success(), "Expected the run to fail");
  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(
    error_count == 1,
    std::format("Expected error_count to be 1, but it is {}", error_count));

  auto const *const error_message = results.error(0);
  REQUIRE_STRING_EQUAL_IN_MAIN(
    error_message,
    R"(Group "Test group 1" contains duplicate test "Test 2")",
    std::format("Unexpected string value: {}", error_message));

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(
    test_count == 0,
    std::format("Expected test_count to be 0, but it is {}", test_count));

  return 0;
}
