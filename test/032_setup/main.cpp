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
  auto g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1'000'000);
        x = 1;
      })
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1);
        x = 1'000'000;
      });

  t.test(g1, "Test 2")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1'000'000);
        x = 2;

        return 42;
      })
    .run(
      [](waypoint::Context const &ctx, int const fixture)
      {
        ctx.assert(x == 2);
        ctx.assert(fixture == 42);
        x = 1'000'000;
      });

  t.test(g1, "Test 3")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 1'000'000);
        x = 3;

        auto f = X{};
        f.foo = 123;

        return f;
      })
    .run(
      [](waypoint::Context const &ctx, X const fixture)
      {
        ctx.assert(x == 3);
        ctx.assert(fixture.foo == 123);
        x = 1'000'000;
      });

  auto const void_shared_setup = [](waypoint::Context const &ctx)
  {
    ctx.assert(x == 1'000'000);
    x = 111;
  };

  t.test(g1, "Test 4")
    .setup(void_shared_setup)
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 111);
        x = 1'000'000;
      });

  t.test(g1, "Test 5")
    .setup(void_shared_setup)
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(x == 111);
        x = 1'000'000;
      });

  auto const non_void_shared_setup = [](waypoint::Context const &ctx)
  {
    ctx.assert(x == 1'000'000);
    x = 112;

    return X{42};
  };

  t.test(g1, "Test 6")
    .setup(non_void_shared_setup)
    .run(
      [](waypoint::Context const &ctx, X const fixture)
      {
        ctx.assert(x == 112);
        ctx.assert(fixture.foo == 42);
        x = 1'000'000;
      });

  t.test(g1, "Test 7")
    .setup(non_void_shared_setup)
    .run(
      [](waypoint::Context const &ctx, X const fixture)
      {
        ctx.assert(x == 112);
        ctx.assert(fixture.foo == 42);
        x = 1'000'000;
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
