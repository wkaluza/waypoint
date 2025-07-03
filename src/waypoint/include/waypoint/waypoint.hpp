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
struct remove_reference
{
  using type = T;
};

template<typename T>
struct remove_reference<T &>
{
  using type = T;
};

template<typename T>
struct remove_reference<T &&>
{
  using type = T;
};

template<typename T>
using remove_reference_t = typename remove_reference<T>::type;

template<typename T>
struct remove_cv
{
  using type = T;
};

template<typename T>
struct remove_cv<T const>
{
  using type = T;
};

template<typename T>
struct remove_cv<T volatile>
{
  using type = T;
};

template<typename T>
struct remove_cv<T const volatile>
{
  using type = T;
};

template<typename T>
using remove_cv_t = typename remove_cv<T>::type;

template<typename T>
struct remove_cv_ref
{
  using type = remove_cv_t<remove_reference_t<T>>;
};

template<typename T>
using remove_cv_ref_t = typename remove_cv_ref<T>::type;

template<typename T, typename U>
struct is_same
{
  constexpr static bool value = false;
};

template<typename T>
struct is_same<T, T>
{
  constexpr static bool value = true;
};

template<typename T, typename U>
constexpr bool is_same_v = is_same<T, U>::value;

class FunctionBase
{
public:
  virtual ~FunctionBase() = default;
  FunctionBase() = default;
  FunctionBase(FunctionBase const &other) = delete;
  FunctionBase(FunctionBase &&other) noexcept = default;
  auto operator=(FunctionBase const &other) -> FunctionBase & = delete;
  auto operator=(FunctionBase &&other) noexcept -> FunctionBase & = delete;

  virtual void invoke(Context &ctx) = 0;
};

template<typename F>
class Function final : public FunctionBase
{
public:
  explicit Function(F const &f) :
    fn_(f)
  {
  }

  // NOLINTNEXTLINE rvalue reference used without std::move
  explicit Function(F &&f) :
    fn_(static_cast<remove_reference_t<F> &&>(f))
  {
  }

  void invoke(Context &ctx) override
  {
    this->fn_(ctx);
  }

private:
  F fn_;
};

class TestBody
{
public:
  ~TestBody();

  template<typename F>
  requires requires { !is_same_v<remove_reference_t<F>, TestBody>; }
  // NOLINTNEXTLINE forwarding reference used without std::forward
  explicit TestBody(F &&f) :
    fn_{new Function<remove_reference_t<F>>(static_cast<F &&>(f))}
  {
  }

  TestBody(TestBody const &other) = delete;

  TestBody(TestBody &&other) noexcept;

  auto operator=(TestBody const &other) -> TestBody & = delete;
  auto operator=(TestBody &&other) noexcept -> TestBody & = delete;

  void operator()(Context &ctx) const;

private:
  FunctionBase *fn_;
};

template<typename T>
class UniquePtr
{
public:
  ~UniquePtr();
  UniquePtr() = delete;
  explicit UniquePtr(T *ptr);
  UniquePtr(UniquePtr const &other) = delete;
  auto operator=(UniquePtr const &other) -> UniquePtr & = delete;
  UniquePtr(UniquePtr &&other) noexcept = delete;
  auto operator=(UniquePtr &&other) noexcept -> UniquePtr & = delete;

  explicit operator bool() const;
  auto operator->() const -> T *;
  auto operator*() const -> T &;

private:
  T *ptr_;
};

extern template class UniquePtr<AssertionOutcome_impl>;
extern template class UniquePtr<Context_impl>;
extern template class UniquePtr<Engine_impl>;
extern template class UniquePtr<Group_impl>;
extern template class UniquePtr<RunResult_impl>;
extern template class UniquePtr<Test_impl>;
extern template class UniquePtr<TestOutcome_impl>;

} // namespace waypoint::internal

namespace waypoint
{

// defined in core_actions.cpp
[[nodiscard]]
auto make_default_engine() -> Engine;
// defined in core_actions.cpp
[[nodiscard]]
auto run_all_tests(Engine &t) -> RunResult;

class AssertionOutcome
{
public:
  ~AssertionOutcome();
  AssertionOutcome(AssertionOutcome const &other) = delete;
  AssertionOutcome(AssertionOutcome &&other) noexcept = delete;
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

  internal::UniquePtr<internal::AssertionOutcome_impl> const impl_;

  friend class internal::Engine_impl;
};

class TestOutcome
{
public:
  ~TestOutcome();
  TestOutcome(TestOutcome const &other) = delete;
  TestOutcome(TestOutcome &&other) noexcept = delete;
  auto operator=(TestOutcome const &other) -> TestOutcome & = delete;
  auto operator=(TestOutcome &&other) noexcept -> TestOutcome & = delete;

  [[nodiscard]]
  auto group_name() const -> char const *;
  [[nodiscard]]
  auto group_id() const -> unsigned long long;
  [[nodiscard]]
  auto test_name() const -> char const *;
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

  internal::UniquePtr<internal::TestOutcome_impl> const impl_;

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

  internal::UniquePtr<internal::Group_impl> const impl_;

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

  template<typename F>
  // NOLINTNEXTLINE forwarding reference used without std::forward
  auto run(F &&body) -> Test &
  {
    register_body(internal::TestBody{static_cast<F &&>(body)});

    return *this;
  }

private:
  explicit Test(internal::Test_impl *impl);

  void register_body(internal::TestBody &&body) const;

  internal::UniquePtr<internal::Test_impl> const impl_;

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

  internal::UniquePtr<internal::Context_impl> const impl_;

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

  internal::UniquePtr<internal::RunResult_impl> const impl_;

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

  internal::UniquePtr<internal::Engine_impl> const impl_;

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
