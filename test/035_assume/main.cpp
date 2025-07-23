#include "impls.hpp"

#include "waypoint/waypoint.hpp"

#include <cstring>
#include <string>
#include <vector>

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](auto const &ctx)
      {
        if(ctx.assume(true, "message 1"))
        {
          ctx.assert(false, "message 2");

          if(!ctx.assume(true, "message 3"))
          {
            ctx.assert(false, "message 4");

            return;
          }

          ctx.assert(false, "message 5");
          ctx.assert(true, "message 6");

          if(!ctx.assume(false))
          {
            ctx.assert(false, "message 7");
            ctx.assert(true, "message 8");

            return;
          }

          ctx.assert(true, "message 9");
        }
      });
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);

  // We expect the run to fail
  if(results.success())
  {
    return 1;
  }

  auto const error_count = results.error_count();
  if(error_count != 0)
  {
    return 1;
  }

  auto const test_count = results.test_count();
  if(test_count != 1)
  {
    return 1;
  }

  auto const &test_outcome = results.test_outcome(0);
  auto const assertion_count = test_outcome.assertion_count();
  if(assertion_count != 8)
  {
    return 1;
  }

  for(unsigned i = 0; i < assertion_count; ++i)
  {
    auto const &assertion_outcome = test_outcome.assertion_outcome(i);
    std::vector<std::string> messages = {
      "message 1",
      "message 2",
      "message 3",
      "message 5",
      "message 6",
      waypoint::internal::NO_ASSERTION_MESSAGE,
      "message 7",
      "message 8"};

    char const *actual_message = assertion_outcome.message();
    char const *expected_message = messages[i].c_str();
    if(std::strcmp(actual_message, expected_message) != 0)
    {
      return 1;
    }

    std::vector<bool> outcomes =
      {true, false, true, false, true, false, false, true};
    if(assertion_outcome.passed() != outcomes[i])
    {
      return 1;
    }
  }

  return 0;
}
