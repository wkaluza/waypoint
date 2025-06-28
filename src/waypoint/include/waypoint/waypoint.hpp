#pragma once

namespace waypoint
{

class Context;
class Engine;
class Group;
class RunResult;
class Test;
class TestOutcome;

} // namespace waypoint

namespace waypoint::internal
{

class AssertionOutcome_impl;
class Context_impl;
class Engine_impl;
class Group_impl;
class RunResult_impl;
class Test_impl;
class TestOutcome_impl;

// defined in get_impl.cpp
auto get_impl(Engine const &engine) -> Engine_impl &;

template<typename T>
class UniquePtr
{
public:
  ~UniquePtr()
  {
    delete ptr_;
  }

  UniquePtr() = delete;

  explicit UniquePtr(T *ptr) :
    ptr_{ptr}
  {
  }

  UniquePtr(UniquePtr const &other) = delete;
  auto operator=(UniquePtr const &other) -> UniquePtr & = delete;

  UniquePtr(UniquePtr &&other) noexcept :
    ptr_{other.ptr_}
  {
    other.ptr_ = nullptr;
  }

  auto operator=(UniquePtr &&other) noexcept -> UniquePtr &
  {
    delete ptr_;

    ptr_ = other.ptr_;
    other.ptr_ = nullptr;

    return *this;
  }

  explicit operator bool() const
  {
    return ptr_ != nullptr;
  }

  auto operator->() -> T *
  {
    return ptr_;
  }

  auto operator->() const -> T *
  {
    return ptr_;
  }

  auto operator*() -> T &
  {
    return *ptr_;
  }

  auto operator*() const -> T &
  {
    return *ptr_;
  }

private:
  T *ptr_;
};

} // namespace waypoint::internal

namespace waypoint
{

// defined in core_actions.cpp
[[nodiscard]]
auto make_default_engine() -> Engine;
// defined in core_actions.cpp
[[nodiscard]]
auto run_all_tests(Engine &t) -> RunResult;

using BodyFnPtr = void (*)(Context &);

class AssertionOutcome
{
public:
  ~AssertionOutcome();
  AssertionOutcome(AssertionOutcome const &other) = delete;
  AssertionOutcome(AssertionOutcome &&other) noexcept;
  auto operator=(AssertionOutcome const &other) -> AssertionOutcome & = delete;
  auto operator=(AssertionOutcome &&other) noexcept
    -> AssertionOutcome & = delete;

  [[nodiscard]]
  auto group() const -> char const *;
  [[nodiscard]]
  auto test() const -> char const *;
  [[nodiscard]]
  auto message() const -> char const *;
  [[nodiscard]]
  auto passed() const -> bool;
  [[nodiscard]]
  auto index() const -> unsigned long long;

private:
  explicit AssertionOutcome(internal::AssertionOutcome_impl *impl);

  internal::UniquePtr<internal::AssertionOutcome_impl> impl_;

  friend class internal::Engine_impl;
};

class TestOutcome
{
public:
  ~TestOutcome();
  TestOutcome(TestOutcome const &other) = delete;
  TestOutcome(TestOutcome &&other) noexcept;
  auto operator=(TestOutcome const &other) -> TestOutcome & = delete;
  auto operator=(TestOutcome &&other) noexcept -> TestOutcome & = delete;

  [[nodiscard]]
  auto test_name() const -> char const *;
  [[nodiscard]]
  auto group_name() const -> char const *;
  [[nodiscard]]
  auto test_id() const -> unsigned long long;
  [[nodiscard]]
  auto test_index() const -> unsigned long long;
  [[nodiscard]]
  auto assertion_count() const -> unsigned long long;
  [[nodiscard]]
  auto assertion_outcome(unsigned long long index) const
    -> AssertionOutcome const &;

private:
  explicit TestOutcome(internal::TestOutcome_impl *impl);

  internal::UniquePtr<internal::TestOutcome_impl> impl_;

  friend class internal::Engine_impl;
};

class Group
{
public:
  ~Group();
  Group(Group const &other) = delete;
  Group(Group &&other) noexcept = delete;
  auto operator=(Group const &other) -> Group & = delete;
  auto operator=(Group &&other) noexcept -> Group & = delete;

private:
  explicit Group(internal::Group_impl *impl);

  internal::UniquePtr<internal::Group_impl> impl_;

  friend class internal::Engine_impl;
};

class Test
{
public:
  ~Test();
  Test(Test const &other) = delete;
  Test(Test &&other) noexcept = delete;
  auto operator=(Test const &other) -> Test & = delete;
  auto operator=(Test &&other) noexcept -> Test & = delete;

  auto run(BodyFnPtr const &body) -> Test &;

private:
  explicit Test(internal::Test_impl *impl);

  internal::UniquePtr<internal::Test_impl> impl_;

  friend class internal::Engine_impl;
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
  void assert(bool condition, char const *message) const;

private:
  explicit Context(internal::Context_impl *impl);

  internal::UniquePtr<internal::Context_impl> impl_;

  friend class internal::Engine_impl;
};

class RunResult
{
public:
  ~RunResult();
  RunResult(RunResult const &other) = delete;
  RunResult(RunResult &&other) noexcept = delete;
  auto operator=(RunResult const &other) -> RunResult & = delete;
  auto operator=(RunResult &&other) noexcept -> RunResult & = delete;

  [[nodiscard]]
  auto success() const -> bool;
  [[nodiscard]]
  auto test_count() const -> unsigned long long;
  [[nodiscard]]
  auto test_outcome(unsigned long long index) const -> TestOutcome const &;

private:
  explicit RunResult(internal::RunResult_impl *impl);

  internal::UniquePtr<internal::RunResult_impl> impl_;

  friend class internal::Engine_impl;
};

class Engine
{
public:
  ~Engine();
  Engine(Engine const &other) = delete;
  Engine(Engine &&other) noexcept = delete;
  auto operator=(Engine const &other) -> Engine & = delete;
  auto operator=(Engine &&other) noexcept -> Engine & = delete;

  auto group(char const *name) const -> Group;
  auto test(Group const &group, char const *name) const -> Test;

private:
  explicit Engine(internal::Engine_impl *impl);

  internal::UniquePtr<internal::Engine_impl> impl_;

  friend auto internal::get_impl(Engine const &engine)
    -> internal::Engine_impl &;

  friend auto make_default_engine() -> Engine;
};

} // namespace waypoint

#define _INTERNAL_WAYPOINT_AUTORUN_IMPL2_(engine, section_name, counter, line) \
  void _INTERNAL_WAYPOINT_TEST##_##counter##_##line( \
    waypoint::Engine &(engine)); \
  void (*_INTERNAL_WAYPOINT_TEST_PTR##_##counter##_##line)(waypoint::Engine &) \
    __attribute__((used, section(#section_name))) = \
      _INTERNAL_WAYPOINT_TEST##_##counter##_##line; \
  void _INTERNAL_WAYPOINT_TEST##_##counter##_##line(waypoint::Engine &(engine))

#define _INTERNAL_WAYPOINT_AUTORUN_IMPL1_(engine, section_name, counter, line) \
  _INTERNAL_WAYPOINT_AUTORUN_IMPL2_(engine, section_name, counter, line)

#define WAYPOINT_AUTORUN(engine) \
  _INTERNAL_WAYPOINT_AUTORUN_IMPL1_( \
    engine, \
    waypoint_tests, \
    __COUNTER__, \
    __LINE__)
