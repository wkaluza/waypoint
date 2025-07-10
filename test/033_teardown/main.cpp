#include "waypoint/waypoint.hpp"

namespace
{

int x = 1'000'000;

struct X
{
  int foo;
};

} // namespace

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .teardown(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1);
        x = 1'000'000;
      })
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1'000'000);
        x = 1;
      });

  t.test(g1, "Test 2")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1'000'000);
        x = 2;
      })
    .teardown(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 3);
        x = 1'000'000;
      })
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 2);
        x = 3;
      });

  t.test(g1, "Test 3")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1'000'000);
        x = 4;

        return 42;
      })
    .teardown(
      [](waypoint::Context const &ctx, int const &fixture)
      {
        ctx.assert(x == 5);
        ctx.assert(fixture == 43);
        x = 1'000'000;
      })
    .run(
      [](waypoint::Context const &ctx, int &fixture)
      {
        ctx.assert(x == 4);
        ctx.assert(fixture == 42);
        x = 5;
        ++fixture;
      });

  t.test(g1, "Test 4")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1'000'000);
        x = 6;

        return 42;
      })
    .teardown(
      [](waypoint::Context const &ctx, int const fixture)
      {
        ctx.assert(x == 7);
        ctx.assert(fixture == 42);
        x = 1'000'000;
      })
    .run(
      [](waypoint::Context const &ctx, int fixture)
      {
        ctx.assert(x == 6);
        ctx.assert(fixture == 42);
        ++fixture;
        ctx.assert(fixture == 43);
        x = 7;
      });

  t.test(g1, "Test 5")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1'000'000);
        x = 8;

        auto f = X{};
        f.foo = 123;

        return f;
      })
    .teardown(
      [](waypoint::Context const &ctx, X const &fixture)
      {
        ctx.assert(x == 9);
        ctx.assert(fixture.foo == 123);
        x = 1'000'000;
      })
    .run(
      [](waypoint::Context const &ctx, X const &fixture)
      {
        ctx.assert(x == 8);
        ctx.assert(fixture.foo == 123);
        x = 9;
      });

  t.test(g1, "Test 6")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1'000'000);
        x = 10;

        auto f = X{};
        f.foo = 123;

        return f;
      })
    .teardown(
      [](waypoint::Context const &ctx, X const &fixture)
      {
        ctx.assert(x == 11);
        ctx.assert(fixture.foo == 124);
        x = 1'000'000;
      })
    .run(
      [](waypoint::Context const &ctx, X &fixture)
      {
        ctx.assert(x == 10);
        ctx.assert(fixture.foo == 123);
        ++fixture.foo;
        x = 11;
      });
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);
  if(results.success())
  {
    return 0;
  }

  return 1;
}
