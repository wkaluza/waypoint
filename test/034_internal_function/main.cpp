#include "impls.hpp"

#include "waypoint/waypoint.hpp"

#include <functional>
#include <type_traits>
#include <utility>

// Required to wrap std::function in waypoint::internal::Function
// Likely a false positive by GCC 15 - works fine on Clang 20
#pragma GCC diagnostic ignored "-Warray-bounds"

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  auto const default_ctor_group = t.group("Function default ctor");

  t.test(default_ctor_group, "Test 1")
    .run(
      [](waypoint::Context const &ctx)
      {
        waypoint::internal::Function<void()> f1{};
        waypoint::internal::Function<void()> f2{};

        ctx.assert(!static_cast<bool>(f1));
        ctx.assert(!static_cast<bool>(f2));

        f1 = f2;

        ctx.assert(!static_cast<bool>(f1));
        ctx.assert(!static_cast<bool>(f2));

        f1 = std::move(f2);

        ctx.assert(!static_cast<bool>(f1));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f2));

        auto ff = []()
        {
        };
        waypoint::internal::Function<void()> f3{std::move(ff)};

        ctx.assert(static_cast<bool>(f3));

        f3();

        f1 = f3;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(!static_cast<bool>(f2));
        ctx.assert(static_cast<bool>(f3));

        f1();
        f3();

        f2 = std::move(f3);

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f3));

        f1();
        f2();

        // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
        f1 = f3;

        ctx.assert(!static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));
        ctx.assert(!static_cast<bool>(f3));

        f2();

        f2 = std::move(f3);

        ctx.assert(!static_cast<bool>(f1));
        ctx.assert(!static_cast<bool>(f2));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f3));

        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f2)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f3)>>);
      });

  t.test(default_ctor_group, "Test 2")
    .run(
      [](waypoint::Context const &ctx)
      {
        waypoint::internal::Function<int()> f1{};
        waypoint::internal::Function<int()> f2{};

        ctx.assert(!static_cast<bool>(f1));
        ctx.assert(!static_cast<bool>(f2));

        f1 = f2;

        ctx.assert(!static_cast<bool>(f1));
        ctx.assert(!static_cast<bool>(f2));

        f1 = std::move(f2);

        ctx.assert(!static_cast<bool>(f1));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f2));

        auto ff = []()
        {
          return 0;
        };
        waypoint::internal::Function<int()> f3{std::move(ff)};

        ctx.assert(static_cast<bool>(f3));

        ctx.assert(f3() == 0);

        f1 = f3;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(!static_cast<bool>(f2));
        ctx.assert(static_cast<bool>(f3));

        ctx.assert(f1() == 0);
        ctx.assert(f3() == 0);

        f2 = std::move(f3);

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f3));

        ctx.assert(f1() == 0);
        ctx.assert(f2() == 0);

        // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
        f1 = f3;

        ctx.assert(!static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));
        ctx.assert(!static_cast<bool>(f3));

        ctx.assert(f2() == 0);

        f2 = std::move(f3);

        ctx.assert(!static_cast<bool>(f1));
        ctx.assert(!static_cast<bool>(f2));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f3));

        ctx.assert(std::is_same_v<int, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<int, std::invoke_result_t<decltype(f2)>>);
        ctx.assert(std::is_same_v<int, std::invoke_result_t<decltype(f3)>>);
      });

  auto const g1 = t.group("Test group 1");

  t.test(g1, "Test 1")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        auto ff = [&x]()
        {
          ++x;
        };
        waypoint::internal::Function<void()> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f2)>>);

        f1();
        f2();

        ctx.assert(x == 2);
      });

  t.test(g1, "Test 2")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        auto ff = [&x](int const y)
        {
          x += y;
        };
        waypoint::internal::Function<void(int)> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f2), int>>);

        f1(3);
        f2(4);

        ctx.assert(x == 7);
      });

  t.test(g1, "Test 3")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        auto ff = [&x](int const y)
        {
          x += y;

          return y;
        };
        waypoint::internal::Function<int(int)> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f2), int>>);

        ctx.assert(f1(42) == 42);
        ctx.assert(f2(43) == 43);

        ctx.assert(x == 85);
      });

  t.test(g1, "Test 4")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        std::function<void()> ff{[&x]()
                                 {
                                   ++x;
                                 }};
        waypoint::internal::Function<void()> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f2)>>);

        f1();
        f2();

        ctx.assert(x == 2);
      });

  t.test(g1, "Test 5")
    .run(
      [](auto const &ctx)
      {
        int x = 0;
        std::function<void(int)> ff{[&x](int const y)
                                    {
                                      x += y;
                                    }};
        waypoint::internal::Function<void(int)> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f2), int>>);

        f1(5);
        f2(6);

        ctx.assert(x == 11);
      });

  t.test(g1, "Test 6")
    .run(
      [](auto const &ctx)
      {
        int x = 0;
        std::function<int(int)> ff{[&x](int const y)
                                   {
                                     x += y;
                                     return y;
                                   }};
        waypoint::internal::Function<int(int)> const f1{std::move(ff)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const f2 = f1;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f2), int>>);

        ctx.assert(f1(42) == 42);
        ctx.assert(f2(41) == 41);

        ctx.assert(x == 83);
      });

  t.test(g1, "Test 7")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        auto ff1 = []()
        {
        };
        auto ff2 = [&x]()
        {
          x += 3;
        };
        waypoint::internal::Function<void()> f1{std::move(ff1)};
        waypoint::internal::Function<void()> f2{std::move(ff2)};
        waypoint::internal::Function<void()> f3;

        f1();
        f2();

        f1 = f2;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));
        ctx.assert(!static_cast<bool>(f3));

        f1();
        f2();

        ctx.assert(x == 9);

        f1 = f3;
        f2 = std::move(f3);

        ctx.assert(!static_cast<bool>(f1));
        ctx.assert(!static_cast<bool>(f2));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f3));

        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f2)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f3)>>);
      });

  t.test(g1, "Test 8")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        auto ff1 = [&x](int const y)
        {
          x += y;
        };
        auto ff2 = [&x](int const y)
        {
          x += y * y;
        };
        waypoint::internal::Function<void(int)> f1{std::move(ff1)};
        waypoint::internal::Function<void(int)> const f2{std::move(ff2)};

        f1(4);
        f2(5);

        f1 = f2;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f2), int>>);

        f1(3);
        f2(5);

        ctx.assert(x == 63);
      });

  t.test(g1, "Test 9")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        auto ff1 = [&x](int const y)
        {
          x += y;
          return y;
        };
        auto ff2 = [&x](int const y)
        {
          x += y * y;
          return y;
        };
        waypoint::internal::Function<int(int)> f1{std::move(ff1)};
        waypoint::internal::Function<int(int)> const f2{std::move(ff2)};

        f1(2);
        f2(3);

        f1 = f2;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f2), int>>);

        ctx.assert(f1(6) == 6);
        ctx.assert(f2(7) == 7);

        ctx.assert(x == 96);
      });

  t.test(g1, "Test 10")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        std::function<void()> ff1{[]()
                                  {
                                  }};
        std::function<void()> ff2{[&x]()
                                  {
                                    x += 4;
                                  }};
        waypoint::internal::Function<void()> f1{std::move(ff1)};
        waypoint::internal::Function<void()> const f2{std::move(ff2)};

        f1();
        f2();

        f1 = f2;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f2)>>);

        f1();
        f2();

        ctx.assert(x == 12);
      });

  t.test(g1, "Test 11")
    .run(
      [](auto const &ctx)
      {
        int x = 0;
        std::function<void(int)> ff1{[&x](int const y)
                                     {
                                       x += y;
                                     }};
        std::function<void(int)> ff2{[&x](int const y)
                                     {
                                       x += y * 2;
                                     }};
        waypoint::internal::Function<void(int)> f1{std::move(ff1)};
        waypoint::internal::Function<void(int)> const f2{std::move(ff2)};

        f1(4);
        f2(5);

        f1 = f2;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f2), int>>);

        f1(9);
        f2(10);

        ctx.assert(x == 52);
      });

  t.test(g1, "Test 12")
    .run(
      [](auto const &ctx)
      {
        int x = 0;
        std::function<int(int)> ff1{[&x](int const y)
                                    {
                                      x += 2;
                                      return y;
                                    }};
        std::function<int(int)> ff2{[&x](int const y)
                                    {
                                      x += y + 7;
                                      return y;
                                    }};
        waypoint::internal::Function<int(int)> f1{std::move(ff1)};
        waypoint::internal::Function<int(int)> const f2{std::move(ff2)};

        ctx.assert(f1(2) == 2);
        ctx.assert(f2(3) == 3);

        f1 = f2;

        ctx.assert(static_cast<bool>(f1));
        ctx.assert(static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f2), int>>);

        ctx.assert(f1(13) == 13);
        ctx.assert(f2(23) == 23);
        ctx.assert(x == 62);
      });

  t.test(g1, "Test 13")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        auto ff1 = []()
        {
        };
        auto ff2 = [&x]()
        {
          x += 3;
        };
        waypoint::internal::Function<void()> f1{std::move(ff1)};
        waypoint::internal::Function<void()> f2{std::move(ff2)};

        f1();
        f2();

        f1 = std::move(f2);

        ctx.assert(static_cast<bool>(f1));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f2));

        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f2)>>);

        f1();
        f1();

        ctx.assert(x == 9);
      });

  t.test(g1, "Test 14")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        auto ff1 = [&x](int const y)
        {
          x += y;
        };
        auto ff2 = [&x](int const y)
        {
          x += y * y;
        };
        waypoint::internal::Function<void(int)> f1{std::move(ff1)};
        waypoint::internal::Function<void(int)> f2{std::move(ff2)};

        f1(2);
        f2(3);

        f1 = std::move(f2);

        ctx.assert(static_cast<bool>(f1));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f2), int>>);

        f1(3);
        f1(5);

        ctx.assert(x == 45);
      });

  t.test(g1, "Test 15")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        auto ff1 = [&x](int const y)
        {
          x += y;
          return y;
        };
        auto ff2 = [&x](int const y)
        {
          x += y * y;
          return y;
        };
        waypoint::internal::Function<int(int)> f1{std::move(ff1)};
        waypoint::internal::Function<int(int)> f2{std::move(ff2)};

        ctx.assert(f1(1) == 1);
        ctx.assert(f2(2) == 2);

        f1 = std::move(f2);

        ctx.assert(static_cast<bool>(f1));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f2), int>>);

        ctx.assert(f1(6) == 6);
        ctx.assert(f1(7) == 7);

        ctx.assert(x == 90);
      });

  t.test(g1, "Test 16")
    .run(
      [](waypoint::Context const &ctx)
      {
        int x = 0;
        std::function<void()> ff1{[]()
                                  {
                                  }};
        std::function<void()> ff2{[&x]()
                                  {
                                    x += 4;
                                  }};
        waypoint::internal::Function<void()> f1{std::move(ff1)};
        waypoint::internal::Function<void()> f2{std::move(ff2)};

        f1();
        f2();

        f1 = std::move(f2);

        ctx.assert(static_cast<bool>(f1));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f2));

        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f1)>>);
        ctx.assert(std::is_same_v<void, std::invoke_result_t<decltype(f2)>>);

        f1();
        f1();

        ctx.assert(x == 12);
      });

  t.test(g1, "Test 17")
    .run(
      [](auto const &ctx)
      {
        int x = 0;
        std::function<void(int)> ff1{[&x](int const y)
                                     {
                                       x += y;
                                     }};
        std::function<void(int)> ff2{[&x](int const y)
                                     {
                                       x += y * 2;
                                     }};
        waypoint::internal::Function<void(int)> f1{std::move(ff1)};
        waypoint::internal::Function<void(int)> f2{std::move(ff2)};

        f1(5);
        f2(6);

        f1 = std::move(f2);

        ctx.assert(static_cast<bool>(f1));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<void, std::invoke_result_t<decltype(f2), int>>);

        f1(9);
        f1(10);

        ctx.assert(x == 55);
      });

  t.test(g1, "Test 18")
    .run(
      [](auto const &ctx)
      {
        int x = 0;
        std::function<int(int)> ff1{[&x](int const y)
                                    {
                                      x += 2;
                                      return y;
                                    }};
        std::function<int(int)> ff2{[&x](int const y)
                                    {
                                      x += y + 7;
                                      return y;
                                    }};
        waypoint::internal::Function<int(int)> f1{std::move(ff1)};
        waypoint::internal::Function<int(int)> f2{std::move(ff2)};

        ctx.assert(f1(1) == 1);
        ctx.assert(f2(2) == 2);

        f1 = std::move(f2);

        ctx.assert(static_cast<bool>(f1));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(f2));

        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f1), int>>);
        ctx.assert(
          std::is_same_v<int, std::invoke_result_t<decltype(f2), int>>);

        ctx.assert(f1(13) == 13);
        ctx.assert(f1(23) == 23);
        ctx.assert(x == 61);
      });
}

auto main() -> int
{
  auto const t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);
  if(!results.success())
  {
    return 1;
  }

  return 0;
}
