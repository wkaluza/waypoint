#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  int const *const captured0 = nullptr;
  std::string const captured1 = "42";
  std::vector<int> const captured2 = {1, 2, 3, 4};
  std::function<int()> const captured3 = []()
  {
    return 123;
  };
  auto captured4 = std::make_unique<int>(42);
  auto captured5 = waypoint::test::CopyableImmovable{};

  auto const g1 = t.group("Capturing lambdas as test bodies");

  t.test(g1, "No capture")
    .run(
      [](waypoint::Context const &ctx) mutable
      {
        ctx.assert(true);
      });

  t.test(g1, "Basic capture")
    .run(
      [captured0, captured1, captured2, captured3](
        waypoint::Context const &ctx) mutable
      {
        ctx.assert(captured0 == nullptr);
        ctx.assert(captured1 == "42");
        ctx.assert(captured2.size() == 4);
        ctx.assert(captured3() == 123);
      });

  t.test(g1, "Movable & non-copyable capture")
    .run(
      [captured4 = std::move(captured4)](waypoint::Context const &ctx) mutable
      {
        ctx.assert(*captured4 == 42);
      });

  t.test(g1, "Copyable & non-movable capture")
    .run(
      [captured5](waypoint::Context const &ctx) mutable
      {
        ctx.assert(captured5.foo == 42);
      });
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests_in_process(t);
  REQUIRE_IN_MAIN(results.success());

  return 0;
}
