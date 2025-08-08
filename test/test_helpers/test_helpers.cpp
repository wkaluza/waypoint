#include "test_helpers.hpp"

#include "waypoint/waypoint.hpp"

#include <cstdlib>
#include <optional>
#include <string>

namespace waypoint::test
{

int x = x_init;

void trivial_test_setup(waypoint::Context const &ctx)
{
  ctx.assert(true);
}

void trivial_test_body(waypoint::Context const &ctx)
{
  ctx.assert(true);
  ctx.assert(true, "body assertion message");
}

void trivial_failing_body(waypoint::Context const &ctx)
{
  ctx.assert(false);
  ctx.assert(false, "failing body assertion message");
}

void trivial_test_teardown(waypoint::Context const &ctx)
{
  ctx.assert(true);
}

void increment_x_test_body(waypoint::Context const &ctx)
{
  ctx.assert(true);

  x += 1;
}

auto int_fixture_test_setup(waypoint::Context const &ctx) -> int
{
  ctx.assert(true);

  return 42;
}

void int_fixture_increment_x_test_body(
  waypoint::Context const &ctx,
  int const &fixture)
{
  ctx.assert(fixture == 42);

  x += 1;
}

void int_fixture_teardown(waypoint::Context const &ctx, int const &fixture)
{
  ctx.assert(fixture == 42);
}

auto get_env(std::string const &var_name) -> std::optional<std::string>
{
  auto const *const var_value = std::getenv(var_name.c_str());
  if(var_value == nullptr)
  {
    return std::nullopt;
  }

  return {var_value};
}

} // namespace waypoint::test
