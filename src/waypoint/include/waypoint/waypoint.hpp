#pragma once

#include "string.hpp"
#include "test_types.hpp"
#include "types.hpp"
#include "vector.hpp"

namespace waypoint
{

class TestContext
{
public:
  TestContext();

  void assert(bool condition);

  [[nodiscard]]
  auto has_failure() const -> bool;

private:
  void register_assertion_failure();

  bool has_failure_;
};

class TestResult
{
public:
  TestResult();

  [[nodiscard]]
  auto pass() const -> bool;

  void register_test_outcome(TestContext const &ctx);

  void set_failure(bool fail);
  [[nodiscard]]
  auto has_failure() const -> bool;

private:
  bool has_failure_;
};

class TestEngine
{
public:
  auto group(String name) -> TestGroup;
  auto test(TestGroup const &group, String name) -> Test;

  [[nodiscard]]
  auto verify() const -> bool;

  [[nodiscard]]
  auto test_bodies() const -> Vector<Body>;
  void register_test_body(Body body);

private:
  Vector<TestGroup> groups_;
  Vector<Test> tests_;
  Vector<Body> bodies_;
};

[[nodiscard]]
auto initialize(TestEngine &t) -> bool;

[[nodiscard]]
auto run_all_tests(TestEngine &t) -> TestResult;

} // namespace waypoint

namespace waypoint::internal
{

using AutorunFn = void (*)(TestEngine &);

} // namespace waypoint::internal

#define X_INTERNAL_MERGE(name, counter, line) name##_##counter##_##line

#define X_INTERNAL_WAYPOINT_TEST_NAME(counter, line) \
  X_INTERNAL_MERGE(WAYPOINT_TEST, counter, line)
#define X_INTERNAL_WAYPOINT_TEST_PTR_NAME(counter, line) \
  X_INTERNAL_MERGE(WAYPOINT_TEST_PTR, counter, line)

#define X_INTERNAL_WAYPOINT_AUTORUN_impl(engine, counter, line) \
  void X_INTERNAL_WAYPOINT_TEST_NAME(counter, line)(waypoint::TestEngine &); \
  waypoint::internal::AutorunFn \
    __attribute__((used, section("waypoint_tests"))) \
    X_INTERNAL_WAYPOINT_TEST_PTR_NAME(counter, line) = \
      X_INTERNAL_WAYPOINT_TEST_NAME(counter, line); \
  void X_INTERNAL_WAYPOINT_TEST_NAME(counter, line)( \
    waypoint::TestEngine & (engine))

#define WAYPOINT_TESTS(engine) \
  X_INTERNAL_WAYPOINT_AUTORUN_impl(engine, __COUNTER__, __LINE__)
