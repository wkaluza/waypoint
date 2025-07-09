#include "impls.hpp"

#include "waypoint/waypoint.hpp"

#include <string>

namespace
{

template<typename T>
void register_test_unique_ptr(
  waypoint::Engine const &t,
  std::string const &suffix)
{
  auto g1 = t.group(("Test group 1 " + suffix).c_str());

  t.test(g1, ("Test 1 " + suffix).c_str())
    .run(
      [](auto &ctx)
      {
        auto pi1 = waypoint::internal::UniquePtr<T>{nullptr};

        ctx.assert(!static_cast<bool>(pi1));
      });

  t.test(g1, ("Test 2 " + suffix).c_str())
    .run(
      [](auto &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UniquePtr{pi1_original};

        ctx.assert(static_cast<bool>(pi1));

        auto const &pi1_payload_ref = *pi1;

        ctx.assert(&pi1_payload_ref == pi1_original);
      });
}

template<typename T>
void register_test_unique_ptr_moveable(
  waypoint::Engine const &t,
  std::string const &suffix)
{
  auto g1 = t.group(("Test group 1 " + suffix).c_str());

  t.test(g1, ("Test 1 " + suffix).c_str())
    .run(
      [](auto &ctx)
      {
        auto pi1 = waypoint::internal::UniquePtrMoveable<T>{nullptr};
        auto pi2 = waypoint::internal::UniquePtrMoveable<T>{nullptr};

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 2 " + suffix).c_str())
    .run(
      [](auto &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UniquePtrMoveable{pi1_original};
        auto pi2 = waypoint::internal::UniquePtrMoveable<T>{nullptr};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 3 " + suffix).c_str())
    .run(
      [](auto &ctx)
      {
        auto pi1 = waypoint::internal::UniquePtrMoveable<T>{nullptr};
        auto pi2 = waypoint::internal::UniquePtrMoveable{new T{}};

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 4" + suffix).c_str())
    .run(
      [](auto &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UniquePtrMoveable{pi1_original};
        auto pi2 = waypoint::internal::UniquePtrMoveable{new T{}};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 5 " + suffix).c_str())
    .run(
      [](auto &ctx)
      {
        auto pi1 = waypoint::internal::UniquePtrMoveable<T>{nullptr};
        waypoint::internal::UniquePtrMoveable const pi2{std::move(pi1)};

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 6 " + suffix).c_str())
    .run(
      [](auto &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UniquePtrMoveable{pi1_original};
        waypoint::internal::UniquePtrMoveable const pi2{std::move(pi1)};

        // NOLINTNEXTLINE use after move
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });
}

} // namespace

WAYPOINT_AUTORUN(waypoint::Engine const &t)
{
  register_test_unique_ptr<waypoint::internal::AssertionOutcome_impl>(
    t,
    "AssertionOutcome_impl");
  register_test_unique_ptr<waypoint::internal::Context_impl>(t, "Context_impl");
  register_test_unique_ptr<waypoint::internal::Engine_impl>(t, "Engine_impl");
  register_test_unique_ptr<waypoint::internal::Group_impl>(t, "Group_impl");
  register_test_unique_ptr<waypoint::internal::RunResult_impl>(
    t,
    "RunResult_impl");
  register_test_unique_ptr<waypoint::internal::Test_impl>(t, "Test_impl");
  register_test_unique_ptr<waypoint::internal::TestOutcome_impl>(
    t,
    "TestOutcome_impl");

  register_test_unique_ptr_moveable<waypoint::internal::Registrar_impl>(
    t,
    "Registrar_impl");
}

auto main() -> int
{
  auto t = waypoint::make_default_engine();

  auto const results = run_all_tests(t);
  if(!results.success())
  {
    return 1;
  }

  return 0;
}
