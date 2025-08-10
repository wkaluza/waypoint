#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

// NOLINTNEXTLINE(misc-include-cleaner)
#include <chrono>
#include <thread>
#include <vector>

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true);
        std::this_thread::sleep_for(std::chrono::years{1});
        ctx.assert(true);
      })
    .teardown(waypoint::test::trivial_test_teardown)
    .timeout_ms(50);

  t.test(g1, "Test 2")
    .setup(waypoint::test::trivial_test_setup)
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true);
        std::this_thread::sleep_for(std::chrono::years{1});
        ctx.assert(true);
      })
    .teardown(waypoint::test::trivial_test_teardown)
    .timeout_ms(50);
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success());

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 2);

  std::vector<unsigned> const expected_assertion_counts = {1, 2};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    REQUIRE_IN_MAIN(outcome.status() == waypoint::TestOutcome::Status::Timeout);
    auto const expected_assertion_count = expected_assertion_counts[i];
    REQUIRE_IN_MAIN(outcome.assertion_count() == expected_assertion_count);
  }

  return 0;
}
