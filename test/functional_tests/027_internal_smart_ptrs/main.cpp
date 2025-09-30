// Copyright (c) 2025 Wojciech Kałuża
// SPDX-License-Identifier: MIT
// For license details, see LICENSE file

#include "impls.hpp"

#include "test_helpers/test_helpers.hpp"
#include "waypoint/waypoint.hpp"

#include <string>

namespace
{

template<typename T>
void register_test_unique_ptr(
  waypoint::TestRun const &t,
  std::string const &suffix)
{
  auto const g1 = t.group(("Test group 1 " + suffix).c_str());

  t.test(g1, ("Test 1 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 = waypoint::internal::UniquePtr<T>{static_cast<T *>(nullptr)};

        ctx.assert(!static_cast<bool>(pi1));
      });

  t.test(g1, ("Test 2 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UniquePtr<T>{pi1_original};

        ctx.assert(static_cast<bool>(pi1));

        auto const &pi1_payload_ref = *pi1;

        ctx.assert(&pi1_payload_ref == pi1_original);
      });
}

template<typename T>
void register_test_moveable_unique_ptr(
  waypoint::TestRun const &t,
  std::string const &suffix)
{
  auto const g1 = t.group(("Test group 1 " + suffix).c_str());

  t.test(g1, ("Test 1 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::MoveableUniquePtr<T>{static_cast<T *>(nullptr)};
        auto pi2 =
          waypoint::internal::MoveableUniquePtr<T>{static_cast<T *>(nullptr)};

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 2 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::MoveableUniquePtr<T>{pi1_original};
        auto pi2 =
          waypoint::internal::MoveableUniquePtr<T>{static_cast<T *>(nullptr)};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 3 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::MoveableUniquePtr<T>{static_cast<T *>(nullptr)};
        auto pi2 = waypoint::internal::MoveableUniquePtr<T>{new T{}};

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 4" + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::MoveableUniquePtr<T>{pi1_original};
        auto pi2 = waypoint::internal::MoveableUniquePtr<T>{new T{}};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 5 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::MoveableUniquePtr<T>{static_cast<T *>(nullptr)};
        waypoint::internal::MoveableUniquePtr<T> const pi2{std::move(pi1)};

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 6 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::MoveableUniquePtr<T>{pi1_original};
        waypoint::internal::MoveableUniquePtr<T> const pi2{std::move(pi1)};

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });
}

} // namespace

WAYPOINT_AUTORUN(waypoint::TestRun const &t)
{
  register_test_unique_ptr<waypoint::internal::AssertionOutcome_impl>(
    t,
    "AssertionOutcome_impl");
  register_test_unique_ptr<waypoint::internal::AutorunFunctionPtrVector_impl>(
    t,
    "AutorunFunctionPtrVector_impl");
  register_test_unique_ptr<waypoint::internal::ContextInProcess_impl>(
    t,
    "ContextInProcess_impl");
  register_test_unique_ptr<waypoint::internal::ContextChildProcess_impl>(
    t,
    "ContextChildProcess_impl");
  register_test_unique_ptr<waypoint::internal::TestRun_impl>(t, "TestRun_impl");
  register_test_unique_ptr<waypoint::internal::Group_impl>(t, "Group_impl");
  register_test_unique_ptr<waypoint::internal::Test_impl>(t, "Test_impl");
  register_test_unique_ptr<waypoint::internal::TestOutcome_impl>(
    t,
    "TestOutcome_impl");

  register_test_moveable_unique_ptr<waypoint::internal::TestRunResult_impl>(
    t,
    "TestRunResult_impl");
}

auto main() -> int
{
  auto const t = waypoint::TestRun::create();

  auto const results = run_all_tests_in_process(t);
  REQUIRE_IN_MAIN(results.success(), "Expected the run to succeed");

  return 0;
}
