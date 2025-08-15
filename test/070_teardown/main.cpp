#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 1'000'000);
        waypoint::test::x = 1;
      })
    .teardown(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 1);
        waypoint::test::x = 1'000'000;
      });

  t.test(g1, "Test 2")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 1'000'000);
        waypoint::test::x = 2;
      })
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 2);
        waypoint::test::x = 3;
      })
    .teardown(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 3);
        waypoint::test::x = 1'000'000;
      });

  t.test(g1, "Test 3")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 1'000'000);
        waypoint::test::x = 4;

        return 42;
      })
    .run(
      [](waypoint::Context const &ctx, int &fixture)
      {
        ctx.assert(waypoint::test::x == 4);
        ctx.assert(fixture == 42);
        waypoint::test::x = 5;
        ++fixture;
      })
    .teardown(
      [](waypoint::Context const &ctx, int const &fixture)
      {
        ctx.assert(waypoint::test::x == 5);
        ctx.assert(fixture == 43);
        waypoint::test::x = 1'000'000;
      });

  t.test(g1, "Test 4")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 1'000'000);
        waypoint::test::x = 6;

        return 42;
      })
    .run(
      [](waypoint::Context const &ctx, int fixture)
      {
        ctx.assert(waypoint::test::x == 6);
        ctx.assert(fixture == 42);
        ++fixture;
        ctx.assert(fixture == 43);
        waypoint::test::x = 7;
      })
    .teardown(
      [](waypoint::Context const &ctx, int const fixture)
      {
        ctx.assert(waypoint::test::x == 7);
        ctx.assert(fixture == 42);
        waypoint::test::x = 1'000'000;
      });

  t.test(g1, "Test 5")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 1'000'000);
        waypoint::test::x = 8;

        auto f = waypoint::test::X{};
        f.foo = 123;

        return f;
      })
    .run(
      [](waypoint::Context const &ctx, waypoint::test::X const &fixture)
      {
        ctx.assert(waypoint::test::x == 8);
        ctx.assert(fixture.foo == 123);
        waypoint::test::x = 9;
      })
    .teardown(
      [](waypoint::Context const &ctx, waypoint::test::X const &fixture)
      {
        ctx.assert(waypoint::test::x == 9);
        ctx.assert(fixture.foo == 123);
        waypoint::test::x = 1'000'000;
      });

  t.test(g1, "Test 6")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 1'000'000);
        waypoint::test::x = 10;

        auto f = waypoint::test::X{};
        f.foo = 123;

        return f;
      })
    .run(
      [](waypoint::Context const &ctx, waypoint::test::X &fixture)
      {
        ctx.assert(waypoint::test::x == 10);
        ctx.assert(fixture.foo == 123);
        ++fixture.foo;
        waypoint::test::x = 11;
      })
    .teardown(
      [](waypoint::Context const &ctx, waypoint::test::X const &fixture)
      {
        ctx.assert(waypoint::test::x == 11);
        ctx.assert(fixture.foo == 124);
        waypoint::test::x = 1'000'000;
      });
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);
  REQUIRE_IN_MAIN(results.success());

  return 0;
}
