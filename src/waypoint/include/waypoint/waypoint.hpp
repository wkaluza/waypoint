#pragma once

namespace waypoint
{

class TestEngine
{
};

void initialize(TestEngine &t);

} // namespace waypoint

namespace waypoint::internal
{

using AutorunFn = void (*)(TestEngine &);

} // namespace waypoint::internal

#define X_INTERNAL_MERGE(name, counter, line) name##_##counter##_##line

#define X_INTERNAL_WAYPOINTTESTS_impl(engine, counter, line) \
  void X_INTERNAL_MERGE(WAYPOINTTESTS, counter, line)(waypoint::TestEngine &); \
  waypoint::internal::AutorunFn \
    __attribute__((used, section("waypoint_tests"))) X_INTERNAL_MERGE( \
      WAYPOINTTESTSPTR, \
      counter, \
      line) = X_INTERNAL_MERGE(WAYPOINTTESTS, counter, line); \
  void X_INTERNAL_MERGE(WAYPOINTTESTS, counter, line)( \
    waypoint::TestEngine & (engine))

#define WAYPOINT_TESTS(engine) \
  X_INTERNAL_WAYPOINTTESTS_impl(engine, __COUNTER__, __LINE__)
