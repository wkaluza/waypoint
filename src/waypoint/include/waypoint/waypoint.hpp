#pragma once

#include "string.hpp"
#include "test_types.hpp"
#include "vector.hpp"

namespace waypoint
{

class TestEngine
{
public:
  auto group(String name) -> TestGroup;
  auto test(TestGroup const &group, String name) -> Test;

private:
  Vector<TestGroup> groups_;
  Vector<Test> tests_;
};

void initialize(TestEngine &t);

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
