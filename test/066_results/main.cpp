#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <cstring>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true, "Condition must be true");
      });

  t.test(g1, "Test 2")
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true, "Condition must be true");
      });

  t.test(g1, "Test 3")
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true, "Condition must be true");
      });

  auto const g2 = t.group("Test group 2");

  t.test(g2, "Test 4")
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true, "Condition must be true");
      });

  t.test(g2, "Test 5")
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true, "Condition must be true");
      });

  t.test(g2, "Test 6")
    .run(
      [](auto const &ctx)
      {
        ctx.assert(true, "Condition must be true");
      });
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);
  REQUIRE_IN_MAIN(results.success());

  auto const test_count = results.test_count();
  REQUIRE_IN_MAIN(test_count == 6);

  std::vector<unsigned long long> const test_indices{
    3,
    1,
    5,
    4,
    2,
    0,
  };

  for(unsigned i = 0; i < test_count; ++i)
  {
    auto const &test_outcome = results.test_outcome(i);
    REQUIRE_IN_MAIN(test_outcome.test_index() == test_indices.at(i));

    auto const group_name = std::invoke(
      [i]() -> std::string
      {
        return i < 3 ? "Test group 1" : "Test group 2";
      });
    REQUIRE_IN_MAIN(test_outcome.group_name() == group_name);

    auto const test_name = std::invoke(
      [i]()
      {
        std::ostringstream stream;

        stream << "Test " << i + 1;

        return stream.str();
      });
    REQUIRE_IN_MAIN(test_outcome.test_name() == test_name);

    auto const assertion_count = test_outcome.assertion_count();
    REQUIRE_IN_MAIN(assertion_count == 1);

    auto const &assertion_outcome = test_outcome.assertion_outcome(0);

    REQUIRE_IN_MAIN(assertion_outcome.group() == group_name);
    REQUIRE_IN_MAIN(assertion_outcome.test() == test_name);
    REQUIRE_IN_MAIN(
      std::strcmp(assertion_outcome.message(), "Condition must be true") == 0);
    REQUIRE_IN_MAIN(assertion_outcome.passed());
    REQUIRE_IN_MAIN(assertion_outcome.index() == 0);
  }

  return 0;
}
