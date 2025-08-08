#pragma once

#include "waypoint/waypoint.hpp"

#include <optional>
#include <string>

namespace waypoint::test
{

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

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator int() const
  {
    return this->foo;
  }

  int foo{42};
};

struct X
{
  // NOLINTNEXTLINE(google-explicit-constructor)
  operator int() const
  {
    return this->foo;
  }

  int foo;
};

constexpr inline int x_init = 1'000'000;
extern int x;

inline auto void_setup_factory(int const new_x)
{
  return [new_x](waypoint::Context const &ctx)
  {
    ctx.assert(waypoint::test::x == waypoint::test::x_init);

    waypoint::test::x = new_x;
  };
}

inline auto body_factory_no_fixture(int const expected_x, int const new_x)
{
  return [expected_x, new_x](waypoint::Context const &ctx)
  {
    ctx.assert(waypoint::test::x == expected_x);

    waypoint::test::x = new_x;
  };
}

template<typename T>
auto setup_factory_fixture(int const fixture_, int const new_x)
{
  return [fixture_, new_x](waypoint::Context const &ctx)
  {
    ctx.assert(waypoint::test::x == waypoint::test::x_init);

    waypoint::test::x = new_x;

    return T{fixture_};
  };
}

template<typename T>
auto body_factory_fixture(
  int const new_x,
  int const fixture_expected,
  int const x_expected)
{
  return [new_x,
          fixture_expected,
          x_expected](waypoint::Context const &ctx, T const &fixture)
  {
    ctx.assert(x_expected == waypoint::test::x);
    ctx.assert(fixture_expected == fixture);

    waypoint::test::x = new_x;
  };
}

void trivial_test_setup(waypoint::Context const &ctx);
void trivial_test_body(waypoint::Context const &ctx);
void trivial_failing_body(waypoint::Context const &ctx);
void trivial_test_teardown(waypoint::Context const &ctx);
void increment_x_test_body(waypoint::Context const &ctx);

auto int_fixture_test_setup(waypoint::Context const &ctx) -> int;
void int_fixture_increment_x_test_body(
  waypoint::Context const &ctx,
  int const &fixture);
void int_fixture_teardown(waypoint::Context const &ctx, int const &fixture);

auto get_env(std::string const &var_name) -> std::optional<std::string>;

} // namespace waypoint::test

#define REQUIRE_IN_MAIN(condition) \
  if(!(condition)) \
  { \
    return 1; \
  }
