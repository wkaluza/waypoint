#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

// NOLINTNEXTLINE(misc-include-cleaner)
#include <chrono>
#include <thread>

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
      });
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);

  REQUIRE_IN_MAIN(!results.success());

  auto const error_count = results.error_count();
  REQUIRE_IN_MAIN(error_count == 0);

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 1);

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &outcome = results.test_outcome(i);

    REQUIRE_IN_MAIN(outcome.status() == waypoint::TestOutcome::Status::Timeout);
    REQUIRE_IN_MAIN(outcome.assertion_count() == 1);
  }

  return 0;
}
