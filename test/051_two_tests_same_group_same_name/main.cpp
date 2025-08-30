#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <cstring>
#include <format>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1").run(waypoint::test::trivial_test_body);

  t.test(g1, "Test 1").run(waypoint::test::trivial_test_body);
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  // We expect the call to run_all_tests to fail
  // due to initialization errors
  auto const result = waypoint::run_all_tests(t);

  REQUIRE_IN_MAIN(!result.success(), "Expected the run to fail");
  REQUIRE_IN_MAIN(
    result.error_count() == 1,
    std::format(
      "Expected result.error_count() to be 1, but it is {}",
      result.error_count()));
  REQUIRE_IN_MAIN(
    std::strcmp(
      result.error(0),
      R"(Group "Test group 1" contains duplicate test "Test 1")") == 0,
    std::format("Unexpected string value: {}", result.error(0)));
  REQUIRE_IN_MAIN(
    result.test_count() == 0,
    std::format(
      "Expected result.test_count() to be 0, but it is {}",
      result.test_count()));

  return 0;
}
