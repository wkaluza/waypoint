#pragma once

namespace waypoint
{

class Context;
class Engine;
class Group;
class Result;
class Test;

} // namespace waypoint

namespace waypoint::internal
{

class Context_impl;
class Engine_impl;
class Group_impl;
class Result_impl;
class Test_impl;

// defined in get_impl.cpp
auto get_impl(Engine const &engine) -> Engine_impl &;
// defined in get_impl.cpp
auto get_impl(Group const &group) -> Group_impl &;
// defined in get_impl.cpp
auto get_impl(Test const &test) -> Test_impl &;
// defined in get_impl.cpp
auto get_impl(Context const &context) -> Context_impl &;
// defined in get_impl.cpp
auto get_impl(Result const &result) -> Result_impl &;

} // namespace waypoint::internal

namespace waypoint
{

// defined in core_actions.cpp
[[nodiscard]]
auto initialize(Engine &t) -> bool;
// defined in core_actions.cpp
[[nodiscard]]
auto run_all_tests(Engine &t) -> Result;

using BodyFnPtr = void (*)(Context &);

class Group
{
public:
  ~Group();
  Group(Group const &other) = delete;
  Group(Group &&other) noexcept;
  auto operator=(Group const &other) -> Group & = delete;
  auto operator=(Group &&other) noexcept -> Group &;

private:
  explicit Group(Engine &engine);

  internal::Group_impl *impl_;

  friend class internal::Engine_impl;
  friend auto internal::get_impl(Group const &group) -> internal::Group_impl &;
};

class Test
{
public:
  ~Test();
  Test(Test const &other) = delete;
  Test(Test &&other) noexcept;
  auto operator=(Test const &other) -> Test & = delete;
  auto operator=(Test &&other) noexcept -> Test &;

  auto run(BodyFnPtr const &body) -> Test &;

private:
  explicit Test(Engine &engine);

  internal::Test_impl *impl_;

  friend class internal::Engine_impl;
  friend auto internal::get_impl(Test const &test) -> internal::Test_impl &;
};

class Context
{
public:
  ~Context();
  Context(Context const &other) = delete;
  Context(Context &&other) noexcept = delete;
  auto operator=(Context const &other) -> Context & = delete;
  auto operator=(Context &&other) noexcept -> Context & = delete;

  void assert(bool condition) const;

private:
  explicit Context(internal::Context_impl *impl);

  internal::Context_impl *impl_;

  friend class internal::Engine_impl;
  friend auto internal::get_impl(Context const &context)
    -> internal::Context_impl &;
};

class Result
{
public:
  ~Result();
  Result(Result const &other) = delete;
  Result(Result &&other) noexcept = delete;
  auto operator=(Result const &other) -> Result & = delete;
  auto operator=(Result &&other) noexcept -> Result & = delete;

  [[nodiscard]]
  auto pass() const -> bool;

private:
  explicit Result(internal::Result_impl *impl);

  internal::Result_impl *impl_;

  friend class internal::Engine_impl;
  friend auto internal::get_impl(Result const &result)
    -> internal::Result_impl &;
};

class Engine
{
public:
  ~Engine();
  Engine();
  Engine(Engine const &other) = delete;
  Engine(Engine &&other) noexcept = delete;
  auto operator=(Engine const &other) -> Engine & = delete;
  auto operator=(Engine &&other) noexcept -> Engine & = delete;

  auto group(char const *name) const -> Group;
  auto test(Group const &group, char const *name) const -> Test;

private:
  internal::Engine_impl *impl_;

  friend auto internal::get_impl(Engine const &engine)
    -> internal::Engine_impl &;
};

} // namespace waypoint

#define X_INTERNAL_WAYPOINT_MERGE(name, counter, line) name##_##counter##_##line

#define X_INTERNAL_WAYPOINT_TEST_NAME(counter, line) \
  X_INTERNAL_WAYPOINT_MERGE(WAYPOINT_TEST, counter, line)
#define X_INTERNAL_WAYPOINT_TEST_PTR_NAME(counter, line) \
  X_INTERNAL_WAYPOINT_MERGE(WAYPOINT_TEST_PTR, counter, line)

#define X_INTERNAL_WAYPOINT_AUTORUN_IMPL(engine, counter, line) \
  void X_INTERNAL_WAYPOINT_TEST_NAME(counter, line)( \
    waypoint::Engine & (engine)); \
  void (*X_INTERNAL_WAYPOINT_TEST_PTR_NAME(counter, line))(waypoint::Engine &) \
    __attribute__((used, section("waypoint_tests"))) = \
      X_INTERNAL_WAYPOINT_TEST_NAME(counter, line); \
  void X_INTERNAL_WAYPOINT_TEST_NAME(counter, line)(waypoint::Engine & (engine))

#define WAYPOINT_TESTS(engine) \
  X_INTERNAL_WAYPOINT_AUTORUN_IMPL(engine, __COUNTER__, __LINE__)
