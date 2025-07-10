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
void register_test_unique_ptr_moveable(
  waypoint::Engine const &t,
  std::string const &suffix)
{
  auto const g1 = t.group(("Test group 1 " + suffix).c_str());

  t.test(g1, ("Test 1 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::UniquePtrMoveable<T>{static_cast<T *>(nullptr)};
        auto pi2 =
          waypoint::internal::UniquePtrMoveable<T>{static_cast<T *>(nullptr)};

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
        auto pi1 = waypoint::internal::UniquePtrMoveable<T>{pi1_original};
        auto pi2 =
          waypoint::internal::UniquePtrMoveable<T>{static_cast<T *>(nullptr)};

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
          waypoint::internal::UniquePtrMoveable<T>{static_cast<T *>(nullptr)};
        auto pi2 = waypoint::internal::UniquePtrMoveable<T>{new T{}};

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
        auto pi1 = waypoint::internal::UniquePtrMoveable<T>{pi1_original};
        auto pi2 = waypoint::internal::UniquePtrMoveable<T>{new T{}};

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
          waypoint::internal::UniquePtrMoveable<T>{static_cast<T *>(nullptr)};
        waypoint::internal::UniquePtrMoveable<T> const pi2{std::move(pi1)};

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 6 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UniquePtrMoveable<T>{pi1_original};
        waypoint::internal::UniquePtrMoveable<T> const pi2{std::move(pi1)};

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });
}

template<typename T>
void register_test_shared_ptr(
  waypoint::Engine const &t,
  std::string const &suffix)
{
  auto const g1 = t.group(("Test group 1 " + suffix).c_str());

  t.test(g1, ("Test 1 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi2 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};

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
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};

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
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{new T{}};

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
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{new T{}};

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
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        waypoint::internal::UnsafeSharedPtr<T> const pi2{std::move(pi1)};

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 6 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        waypoint::internal::UnsafeSharedPtr<T> const pi2{std::move(pi1)};

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 7 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto const pi2 = pi1;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi1_payload_ref = *pi1;
        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi1_payload_ref == pi1_original);
        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 8 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto *pi2_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{pi2_original};
        pi2 = pi1;
        auto const pi3 = std::move(pi2);

        ctx.assert(static_cast<bool>(pi1));
        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        auto const &pi1_payload_ref = *pi1;
        auto const &pi3_payload_ref = *pi3;

        ctx.assert(&pi1_payload_ref == pi1_original);
        ctx.assert(&pi3_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 9 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto *pi2_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{pi2_original};
        pi2 = pi1;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi1_payload_ref = *pi1;
        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi1_payload_ref == pi1_original);
        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 10" + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{new T{}};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = pi1_ref;
        auto &pi2_ref = pi2;
        pi2 = pi2_ref;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 11 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi2 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = pi1_ref;
        auto &pi2_ref = pi2;
        pi2 = pi2_ref;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        pi2 = pi1;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 12 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = pi1_ref;
        auto &pi2_ref = pi2;
        pi2 = pi2_ref;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));

        pi2 = pi1;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi1_payload_ref = *pi1;
        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi1_payload_ref == pi1_original);
        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 13 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{new T{}};

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = pi1_ref;
        auto &pi2_ref = pi2;
        pi2 = pi2_ref;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        pi2 = pi1;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 14" + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{new T{}};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto &pi1_ref = pi1;
        pi1 = pi1_ref;
        auto &pi2_ref = pi2;
        pi2 = pi2_ref;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        pi2 = pi1;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi1_payload_ref = *pi1;
        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi1_payload_ref == pi1_original);
        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 15 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        waypoint::internal::UnsafeSharedPtr<T> const pi2{pi1};

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
      });

  t.test(g1, ("Test 16 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        waypoint::internal::UnsafeSharedPtr<T> const pi2{pi1};

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));

        auto const &pi1_payload_ref = *pi1;
        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi1_payload_ref == pi1_original);
        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 17 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi2 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi3 = pi2;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        auto &pi1_ref = pi1;
        pi1 = pi1_ref;
        auto &pi2_ref = pi2;
        pi2 = pi2_ref;
        auto &pi3_ref = pi3;
        pi3 = pi3_ref;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        pi2 = pi1;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));
      });

  t.test(g1, ("Test 18 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi3 = pi2;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        auto &pi1_ref = pi1;
        pi1 = pi1_ref;
        auto &pi2_ref = pi2;
        pi2 = pi2_ref;
        auto &pi3_ref = pi3;
        pi3 = pi3_ref;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        pi2 = pi1;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        auto const &pi1_payload_ref = *pi1;
        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi1_payload_ref == pi1_original);
        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 19 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{new T{}};
        auto pi3 = pi2;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        auto &pi1_ref = pi1;
        pi1 = pi1_ref;
        auto &pi2_ref = pi2;
        pi2 = pi2_ref;
        auto &pi3_ref = pi3;
        pi3 = pi3_ref;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        pi2 = pi1;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));
      });

  t.test(g1, ("Test 20" + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{new T{}};
        auto pi3 = pi2;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        auto &pi1_ref = pi1;
        pi1 = pi1_ref;
        auto &pi2_ref = pi2;
        pi2 = pi2_ref;
        auto &pi3_ref = pi3;
        pi3 = pi3_ref;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        pi2 = pi1;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        auto const &pi1_payload_ref = *pi1;
        auto const &pi2_payload_ref = *pi2;
        auto const &pi3_payload_ref = *pi3;

        ctx.assert(&pi1_payload_ref == pi1_original);
        ctx.assert(&pi2_payload_ref == pi1_original);
        ctx.assert(&pi3_payload_ref != pi1_original);
      });

  t.test(g1, ("Test 21 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi2 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi3 = pi2;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);
        auto &pi3_ref = pi3;
        pi3 = std::move(pi3_ref);

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));
      });

  t.test(g1, ("Test 22 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi3 = pi2;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);
        auto &pi3_ref = pi3;
        pi3 = std::move(pi3_ref);

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(!static_cast<bool>(pi3));

        auto const &pi2_payload_ref = *pi2;

        ctx.assert(&pi2_payload_ref == pi1_original);
      });

  t.test(g1, ("Test 23 " + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto pi1 =
          waypoint::internal::UnsafeSharedPtr<T>{static_cast<T *>(nullptr)};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{new T{}};
        auto pi3 = pi2;

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);
        auto &pi3_ref = pi3;
        pi3 = std::move(pi3_ref);

        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(!static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));
      });

  t.test(g1, ("Test 24" + suffix).c_str())
    .run(
      [](auto const &ctx)
      {
        auto *pi1_original = new T{};
        auto pi1 = waypoint::internal::UnsafeSharedPtr<T>{pi1_original};
        auto pi2 = waypoint::internal::UnsafeSharedPtr<T>{new T{}};
        auto pi3 = pi2;

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        auto &pi1_ref = pi1;
        pi1 = std::move(pi1_ref);
        auto &pi2_ref = pi2;
        pi2 = std::move(pi2_ref);
        auto &pi3_ref = pi3;
        pi3 = std::move(pi3_ref);

        ctx.assert(static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        pi2 = std::move(pi1);

        // NOLINTNEXTLINE(bugprone-use-after-move)
        ctx.assert(!static_cast<bool>(pi1));
        ctx.assert(static_cast<bool>(pi2));
        ctx.assert(static_cast<bool>(pi3));

        auto const &pi2_payload_ref = *pi2;
        auto const &pi3_payload_ref = *pi3;

        ctx.assert(&pi2_payload_ref == pi1_original);
        ctx.assert(&pi3_payload_ref != pi1_original);
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

  register_test_shared_ptr<int>(t, "int");
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
