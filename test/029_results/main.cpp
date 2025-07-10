#include "waypoint/waypoint.hpp"

#include <cstring>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>

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
  if(!results.success())
  {
    return 1;
  }

  auto const test_count = results.test_count();
  if(test_count != 6)
  {
    return 1;
  }

  std::unordered_map<unsigned long long, unsigned long long> const test_indices{
    {0, 3},
    {1, 1},
    {2, 5},
    {3, 4},
    {4, 2},
    {5, 0},
  };

  for(unsigned test_id = 0; test_id < test_count; ++test_id)
  {
    auto const &test_outcome = results.test_outcome(test_id);
    auto const group_id = std::invoke(
      [test_id]() -> unsigned long long
      {
        return test_id < 3 ? 0 : 1;
      });
    if(test_outcome.group_id() != group_id)
    {
      return 1;
    }
    if(test_outcome.test_id() != test_id)
    {
      return 1;
    }
    if(test_outcome.test_index() != test_indices.at(test_outcome.test_id()))
    {
      return 1;
    }

    auto const group_name = std::invoke(
      [test_id]() -> std::string
      {
        return test_id < 3 ? "Test group 1" : "Test group 2";
      });
    if(test_outcome.group_name() != group_name)
    {
      return 1;
    }

    auto const test_name = std::invoke(
      [test_id]()
      {
        std::ostringstream stream;

        stream << "Test " << test_id + 1;

        return stream.str();
      });
    if(test_outcome.test_name() != test_name)
    {
      return 1;
    }

    auto const assertion_count = test_outcome.assertion_count();
    if(assertion_count != 1)
    {
      return 1;
    }

    auto const &assertion_outcome = test_outcome.assertion_outcome(0);

    if(assertion_outcome.group() != group_name)
    {
      return 1;
    }

    if(assertion_outcome.test() != test_name)
    {
      return 1;
    }

    if(std::strcmp(assertion_outcome.message(), "Condition must be true") != 0)
    {
      return 1;
    }

    if(!assertion_outcome.passed())
    {
      return 1;
    }

    if(assertion_outcome.index() != 0)
    {
      return 1;
    }
  }

  return 0;
}
