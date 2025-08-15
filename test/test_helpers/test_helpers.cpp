#include "test_helpers.hpp"

#include "test_helpers_crash.hpp"

#include "waypoint/waypoint.hpp"

#include <chrono>
#include <cstdlib>
#include <optional>
#include <string>
#include <thread>

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

void int_fixture_test_body(waypoint::Context const &ctx, int const &fixture)
{
  ctx.assert(fixture == 42);
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

void body_short_sleep(waypoint::Context const &ctx) noexcept
{
  ctx.assert(true);
  std::this_thread::sleep_for(std::chrono::milliseconds{110});
}

void body_long_sleep(waypoint::Context const &ctx) noexcept
{
  ctx.assert(true);
  std::this_thread::sleep_for(std::chrono::years{100});
}

void int_fixture_body_long_sleep(
  waypoint::Context const &ctx,
  int const & /*fixture*/) noexcept
{
  ctx.assert(true);
  std::this_thread::sleep_for(std::chrono::years{100});
}

void body_call_std_exit_123(waypoint::Context const &ctx)
{
  ctx.assert(true);
  call_std_exit_123();
}

void body_call_std_abort(waypoint::Context const &ctx)
{
  ctx.assert(true);
  call_std_abort();
}

void body_failing_assertion(waypoint::Context const &ctx)
{
  ctx.assert(true);
  failing_assertion();
}

void body_throws_exception(waypoint::Context const &ctx)
{
  ctx.assert(true);
  throws_exception();
}

void body_throws_exception_while_noexcept(waypoint::Context const &ctx) noexcept
{
  ctx.assert(true);
  throws_exception_while_noexcept();
}

} // namespace waypoint::test
