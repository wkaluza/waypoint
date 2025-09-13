#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == waypoint::test::x_init);
        waypoint::test::x = 1;
      })
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 1);
        waypoint::test::x = waypoint::test::x_init;
      });

  t.test(g1, "Test 2")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == waypoint::test::x_init);
        waypoint::test::x = 2;

        return 42;
      })
    .run(
      [](waypoint::Context const &ctx, int const &fixture)
      {
        ctx.assert(waypoint::test::x == 2);
        ctx.assert(fixture == 42);
        waypoint::test::x = waypoint::test::x_init;
      });

  t.test(g1, "Test 3")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == waypoint::test::x_init);
        waypoint::test::x = 2;

        return 42;
      })
    .run(
      [](waypoint::Context const &ctx, int fixture)
      {
        ctx.assert(waypoint::test::x == 2);
        ctx.assert(fixture == 42);
        ++fixture;
        ctx.assert(fixture == 43);
        waypoint::test::x = waypoint::test::x_init;
      });

  t.test(g1, "Test 4")
    .setup(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == waypoint::test::x_init);
        waypoint::test::x = 3;

        auto f = waypoint::test::X{};
        f.foo = 123;

        return f;
      })
    .run(
      [](waypoint::Context const &ctx, waypoint::test::X const &fixture)
      {
        ctx.assert(waypoint::test::x == 3);
        ctx.assert(fixture.foo == 123);
        waypoint::test::x = waypoint::test::x_init;
      });

  auto const void_shared_setup = [](waypoint::Context const &ctx)
  {
    ctx.assert(waypoint::test::x == waypoint::test::x_init);
    waypoint::test::x = 111;
  };

  t.test(g1, "Test 5")
    .setup(void_shared_setup)
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 111);
        waypoint::test::x = waypoint::test::x_init;
      });

  t.test(g1, "Test 6")
    .setup(void_shared_setup)
    .run(
      [](waypoint::Context const &ctx)
      {
        ctx.assert(waypoint::test::x == 111);
        waypoint::test::x = waypoint::test::x_init;
      });

  auto const non_void_shared_setup = [](waypoint::Context const &ctx)
  {
    ctx.assert(waypoint::test::x == waypoint::test::x_init);
    waypoint::test::x = 112;

    return waypoint::test::X{42};
  };

  t.test(g1, "Test 7")
    .setup(non_void_shared_setup)
    .run(
      [](waypoint::Context const &ctx, waypoint::test::X const &fixture)
      {
        ctx.assert(waypoint::test::x == 112);
        ctx.assert(fixture.foo == 42);
        waypoint::test::x = waypoint::test::x_init;
      });

  t.test(g1, "Test 8")
    .setup(non_void_shared_setup)
    .run(
      [](waypoint::Context const &ctx, waypoint::test::X &fixture)
      {
        ctx.assert(waypoint::test::x == 112);
        ctx.assert(fixture.foo == 42);
        ++fixture.foo;
        ctx.assert(fixture.foo == 43);
        waypoint::test::x = waypoint::test::x_init;
      });

  auto const g2 = t.group("Test group 2");

  t.test(g2, "Test 1")
    .setup(waypoint::test::void_setup_factory(1))
    .run(waypoint::test::body_factory_no_fixture(1, waypoint::test::x_init));

  t.test(g2, "Test 2")
    .setup(waypoint::test::setup_factory_fixture<int>(42, 2))
    .run(
      waypoint::test::body_factory_fixture<int>(waypoint::test::x_init, 42, 2));

  t.test(g2, "Test 3")
    .setup(waypoint::test::setup_factory_fixture<waypoint::test::X>(123, 3))
    .run(
      waypoint::test::body_factory_fixture<waypoint::test::X>(
        waypoint::test::x_init,
        123,
        3));

  t.test(g2, "Test 4")
    .setup(waypoint::test::void_setup_factory(111))
    .run(waypoint::test::body_factory_no_fixture(111, waypoint::test::x_init));

  t.test(g2, "Test 5")
    .setup(waypoint::test::setup_factory_fixture<waypoint::test::X>(42, 112))
    .run(
      waypoint::test::body_factory_fixture<waypoint::test::X>(
        waypoint::test::x_init,
        42,
        112));
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests(t);
  REQUIRE_IN_MAIN(results.success(), "Expected the run to succeed");

  return 0;
}
