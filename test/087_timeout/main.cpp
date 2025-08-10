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
    .setup(waypoint::test::int_fixture_test_setup)
    .run(
      [](auto const &ctx, auto & /*fixture*/)
      {
        ctx.assert(true);
        std::this_thread::sleep_for(std::chrono::years{1});
        ctx.assert(true);
      })
    .timeout_ms(50);

  t.test(g1, "Test 2")
    .setup(waypoint::test::int_fixture_test_setup)
    .run(
      [](auto const &ctx, auto & /*fixture*/)
      {
        ctx.assert(true);
        std::this_thread::sleep_for(std::chrono::years{1});
        ctx.assert(true);
      })
    .teardown(waypoint::test::int_fixture_teardown)
    .timeout_ms(50);

  t.test(g1, "Test 3")
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true);
        std::this_thread::sleep_for(std::chrono::years{1});
        ctx.assert(true);
      })
    .timeout_ms(50);

  t.test(g1, "Test 4")
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true);
        std::this_thread::sleep_for(std::chrono::years{1});
        ctx.assert(true);
      })
    .timeout_ms(1'000'000)
    .disable();
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success());

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 4);

  std::vector<waypoint::TestOutcome::Status> const expected_statuses = {
    waypoint::TestOutcome::Status::Timeout,
    waypoint::TestOutcome::Status::Timeout,
    waypoint::TestOutcome::Status::Timeout,
    waypoint::TestOutcome::Status::NotRun};
  std::vector<bool> const expected_disabled_states =
    {false, false, false, true};
  std::vector<unsigned> const expected_assertion_counts = {2, 2, 1, 0};

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    auto const expected_status = expected_statuses[i];
    REQUIRE_IN_MAIN(outcome.status() == expected_status);

    auto const expected_disabled_state = expected_disabled_states[i];
    REQUIRE_IN_MAIN(outcome.disabled() == expected_disabled_state);

    auto const expected_assertion_count = expected_assertion_counts[i];
    REQUIRE_IN_MAIN(outcome.assertion_count() == expected_assertion_count);
  }

  return 0;
}
