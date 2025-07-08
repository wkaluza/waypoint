#include "waypoint/waypoint.hpp"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class CopyableImmovable
{
public:
  ~CopyableImmovable() = default;
  CopyableImmovable() = default;
  CopyableImmovable(CopyableImmovable const &other) = default;
  CopyableImmovable(CopyableImmovable &&other) noexcept = delete;
  auto operator=(CopyableImmovable const &other)
    -> CopyableImmovable & = default;
  auto operator=(CopyableImmovable &&other) noexcept
    -> CopyableImmovable & = delete;

  int foo{42};
};

WAYPOINT_AUTORUN(t)
{
  int const *const captured0 = nullptr;
  std::string const captured1 = "42";
  std::vector<int> const captured2 = {1, 2, 3, 4};
  std::function<int()> const captured3 = []()
  {
    return 123;
  };
  auto captured4 = std::make_unique<int>(42);
  auto captured5 = CopyableImmovable{};

  auto g1 = t.group("Capturing lambdas as test bodies");

  t.test(g1, "Basic capture")
    .run(
      [captured0, captured1, captured2, captured3](waypoint::Context &ctx)
      {
        ctx.assert(captured0 == nullptr);
        ctx.assert(captured1 == "42");
        ctx.assert(captured2.size() == 4);
        ctx.assert(captured3() == 123);
      });

  t.test(g1, "Movable & non-copyable capture")
    .run(
      [captured4 = std::move(captured4)](waypoint::Context &ctx)
      {
        ctx.assert(*captured4 == 42);
      });

  t.test(g1, "Copyable & non-movable capture")
    .run(
      [captured5](waypoint::Context &ctx)
      {
        ctx.assert(captured5.foo == 42);
      });
}

auto main() -> int
{
  auto t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);
  if(results.success())
  {
    return 0;
  }

  return 1;
}
