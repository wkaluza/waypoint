#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <cstring>

// False positives from clang-tidy (according to Valgrind)
// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  (void)t.test(g1, "Test 1");

  t.test(g1, "Test 2")
    .run(
      waypoint::test::body_factory_no_fixture(
        waypoint::test::x_init,
        waypoint::test::x_init));

  (void)t.test(g1, "Test 3")
    .setup(waypoint::test::void_setup_factory(waypoint::test::x_init));

  t.test(g1, "Test 4")
    .setup(waypoint::test::setup_factory_fixture<int>(42, 77))
    .run(
      waypoint::test::body_factory_fixture<int>(
        waypoint::test::x_init,
        42,
        77));

  (void)t.test(g1, "Test 5")
    .setup(
      waypoint::test::setup_factory_fixture<int>(42, waypoint::test::x_init));

  t.test(g1, "Test 6")
    .setup(waypoint::test::setup_factory_fixture<waypoint::test::X>(42, 66))
    .run(
      waypoint::test::body_factory_fixture<waypoint::test::X>(
        waypoint::test::x_init,
        42,
        66));

  (void)t.test(g1, "Test 7")
    .setup(
      waypoint::test::setup_factory_fixture<waypoint::test::X>(
        42,
        waypoint::test::x_init));
}

// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests_in_process(t);

  // We expect the run to fail
  REQUIRE_IN_MAIN(!results.success());

  auto const count = results.error_count();

  REQUIRE_IN_MAIN(count == 4);
  REQUIRE_IN_MAIN(
    std::strcmp(
      results.error(0),
      R"(Test "Test 1" in group "Test group 1" is incomplete. )"
      R"(Call the run(...) method to fix this.)") == 0);
  REQUIRE_IN_MAIN(
    std::strcmp(
      results.error(1),
      R"(Test "Test 3" in group "Test group 1" is incomplete. )"
      R"(Call the run(...) method to fix this.)") == 0);
  REQUIRE_IN_MAIN(
    std::strcmp(
      results.error(2),
      R"(Test "Test 5" in group "Test group 1" is incomplete. )"
      R"(Call the run(...) method to fix this.)") == 0);
  REQUIRE_IN_MAIN(
    std::strcmp(
      results.error(3),
      R"(Test "Test 7" in group "Test group 1" is incomplete. )"
      R"(Call the run(...) method to fix this.)") == 0);
  REQUIRE_IN_MAIN(results.test_count() == 0);

  return 0;
}
