#pragma once

#include "waypoint/waypoint.hpp"

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

  int foo{42};
};

struct X
{
  int foo;
};

constexpr int x_init = 1'000'000;
extern int x;

inline auto void_setup_factory(int const new_x)
{
  return [new_x](waypoint::Context const &ctx)
  {
    ctx.assert(waypoint::test::x == waypoint::test::x_init);

    waypoint::test::x = new_x;
  };
}

inline auto void_body_factory(int const current_x, int const new_x)
{
  return [current_x, new_x](waypoint::Context const &ctx)
  {
    ctx.assert(waypoint::test::x == current_x);

    waypoint::test::x = new_x;
  };
}

template<typename T>
auto setup_factory_fixture(int const fixture_, int const new_x)
{
  return [fixture_, new_x](waypoint::Context const & /*ctx*/)
  {
    waypoint::test::x = new_x;

    return T{fixture_};
  };
}

template<typename T>
auto body_factory_fixture(int const new_x)
{
  return [new_x](waypoint::Context const & /*ctx*/, T & /*fixture*/)
  {
    waypoint::test::x = new_x;
  };
}

} // namespace waypoint::test
