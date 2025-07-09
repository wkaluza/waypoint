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
class Registrar_impl;
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

template<typename T>
// NOLINTNEXTLINE missing std::forward
constexpr auto move(T &&t) noexcept -> remove_reference_t<T> &&
{
  return static_cast<remove_reference_t<T> &&>(t);
}

template<typename T>
constexpr auto forward(remove_reference_t<T> &t) noexcept -> T &&
{
  return static_cast<T &&>(t);
}

template<typename T>
// NOLINTNEXTLINE missing std::move
constexpr auto forward(remove_reference_t<T> &&t) noexcept -> T &&
{
  return static_cast<T &&>(t);
}

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

template<typename T>
struct is_void
{
  constexpr static bool value = false;
};

template<>
struct is_void<void>
{
  constexpr static bool value = true;
};

template<typename T>
constexpr bool is_void_v = is_void<T>::value;

template<bool B, typename T = void>
struct enable_if
{
};

template<typename T>
struct enable_if<true, T>
{
  using type = T;
};

template<bool B, typename T>
using enable_if_t = typename enable_if<B, T>::type;

template<typename T>
auto declval() -> T;

template<typename F, typename... Args>
auto invoke_impl(F &&f, Args &&...args)
  -> decltype(forward<F>(f)(forward<Args>(args)...));

template<typename F, typename... Args>
struct invoke_result
{
  using type = decltype(invoke_impl(declval<F>(), declval<Args>()...));
};

template<typename F, typename... Args>
using invoke_result_t = typename invoke_result<F, Args...>::type;

template<typename F>
using setup_invoke_result_t = typename invoke_result<F, Context &>::type;

template<typename T>
class Function;

template<typename R, typename... Args>
class Function<R(Args...)>
{
public:
  ~Function()
  {
    delete callable_;
  }

  Function() = delete;
  Function(Function const &other) = delete;
  auto operator=(Function const &other) -> Function & = delete;

  Function(Function &&other) noexcept
    : callable_{other.callable_}
  {
    other.callable_ = nullptr;
  }

  auto operator=(Function &&other) noexcept -> Function &
  {
    // No need to handle self-assignment
    this->callable_ = other.callable_;
    other.callable_ = nullptr;

    return *this;
  }

  template<typename F>
  requires requires { !is_same_v<F, Function<R(Args...)>>; }
  // NOLINTNEXTLINE missing std::forward, missing explicit
  Function(F &&f)
    : callable_{new callable<F>{internal::forward<F>(f)}}
  {
  }

  auto operator()(Args &&...args) const -> R
  {
    return this->callable_->invoke(internal::forward<Args>(args)...);
  }

private:
  class callable_interface
  {
  public:
    virtual ~callable_interface() = default;
    callable_interface() = default;
    callable_interface(callable_interface const &other) = default;
    callable_interface(callable_interface &&other) noexcept = default;
    auto operator=(callable_interface const &other)
      -> callable_interface & = delete;
    auto operator=(callable_interface &&other) noexcept
      -> callable_interface & = delete;

    virtual auto invoke(Args &&...args) -> R = 0;
  };

  template<typename F>
  class callable final : public callable_interface
  {
  public:
    ~callable() override = default;
    callable() = delete;
    auto operator=(callable const &other) -> callable & = delete;
    auto operator=(callable &&other) noexcept -> callable & = delete;

    callable(callable const &other)
      : fn_{other.fn_}
    {
    }

    callable(callable &&other) noexcept
      : fn_{internal::move(other.fn_)}
    {
    }

    // NOLINTNEXTLINE missing std::move
    explicit callable(F &&f)
      : fn_{internal::forward<F>(f)}
    {
    }

    auto invoke(Args &&...args) -> R override
    {
      return fn_(internal::move<Args>(args)...);
    }

  private:
    F fn_;
  };

  callable_interface *callable_;
};

template<typename... Args>
class Function<void(Args...)>
{
public:
  ~Function()
  {
    delete callable_;
  }

  Function() = delete;
  Function(Function const &other) = delete;
  auto operator=(Function const &other) -> Function & = delete;

  Function(Function &&other) noexcept
    : callable_{other.callable_}
  {
    other.callable_ = nullptr;
  }

  auto operator=(Function &&other) noexcept -> Function &
  {
    // No need to handle self-assignment
    this->callable_ = other.callable_;
    other.callable_ = nullptr;

    return *this;
  }

  template<typename F>
  requires requires { !is_same_v<F, Function<void(Args...)>>; }
  // NOLINTNEXTLINE missing std::forward, missing explicit
  Function(F &&f)
    : callable_{new callable<F>{internal::forward<F>(f)}}
  {
  }

  void operator()(Args &&...args) const
  {
    this->callable_->invoke(internal::forward<Args>(args)...);
  }

private:
  class callable_interface
  {
  public:
    virtual ~callable_interface() = default;
    callable_interface() = default;
    callable_interface(callable_interface const &other) = default;
    callable_interface(callable_interface &&other) noexcept = default;
    auto operator=(callable_interface const &other)
      -> callable_interface & = delete;
    auto operator=(callable_interface &&other) noexcept
      -> callable_interface & = delete;

    virtual void invoke(Args &&...args) = 0;
  };

  template<typename F>
  class callable final : public callable_interface
  {
  public:
    ~callable() override = default;
    callable() = delete;
    auto operator=(callable const &other) -> callable & = delete;
    auto operator=(callable &&other) noexcept -> callable & = delete;

    callable(callable const &other)
      : fn_{other.fn_}
    {
    }

    callable(callable &&other) noexcept
      : fn_{internal::move(other.fn_)}
    {
    }

    // NOLINTNEXTLINE missing std::move
    explicit callable(F &&f)
      : fn_{internal::move(f)}
    {
    }

    void invoke(Args &&...args) override
    {
      fn_(internal::forward<Args>(args)...);
    }

  private:
    F fn_;
  };

  callable_interface *callable_;
};

using VoidSetup = Function<void(Context &)>;
using TestBodyNoFixture = Function<void(Context &)>;

template<typename FixtureT>
using NonVoidSetup = Function<FixtureT(Context &)>;
template<typename FixtureT>
using TestBodyWithFixture = Function<void(Context &, FixtureT)>;

template<typename T>
class UniquePtrMoveable
{
public:
  ~UniquePtrMoveable();
  UniquePtrMoveable() = delete;
  explicit UniquePtrMoveable(T *ptr);
  UniquePtrMoveable(UniquePtrMoveable const &other) = delete;
  auto operator=(UniquePtrMoveable const &other)
    -> UniquePtrMoveable & = delete;
  UniquePtrMoveable(UniquePtrMoveable &&other) noexcept;
  auto operator=(UniquePtrMoveable &&other) noexcept -> UniquePtrMoveable &;

  explicit operator bool() const;
  auto operator->() const -> T *;
  auto operator*() const -> T &;

private:
  T *ptr_;
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

extern template class UniquePtrMoveable<Registrar_impl>;

class Registrar
{
public:
  ~Registrar();

  Registrar();
  Registrar(Registrar const &other) = delete;
  Registrar(Registrar &&other) noexcept;
  auto operator=(Registrar const &other) -> Registrar & = delete;
  auto operator=(Registrar &&other) noexcept -> Registrar &;

  void register_body(unsigned long long test_id, TestBodyNoFixture f) const;

private:
  explicit Registrar(Registrar_impl *impl);

  UniquePtrMoveable<Registrar_impl> impl_;

  friend class Engine_impl;
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

template<typename FixtureT>
class Test2;

class Test
{
public:
  ~Test();
  Test(Test const &other) = delete;
  Test(Test &&other) noexcept = delete;
  auto operator=(Test const &other) -> Test & = delete;
  auto operator=(Test &&other) noexcept -> Test & = delete;

  template<typename F>
  // NOLINTNEXTLINE missing std::forward
  auto setup(F &&f) -> internal::enable_if_t<
    internal::is_void_v<internal::setup_invoke_result_t<F>>,
    Test2<void>>
  {
    return this->make_Test2<void>(internal::forward<F>(f));
  }

  template<typename F>
  // NOLINTNEXTLINE missing std::forward
  auto setup(F &&f) -> internal::enable_if_t<
    !internal::is_void_v<internal::setup_invoke_result_t<F>>,
    Test2<internal::setup_invoke_result_t<F>>>
  {
    return this->make_Test2<internal::setup_invoke_result_t<F>>(
      internal::forward<F>(f));
  }

  template<typename F>
  // NOLINTNEXTLINE missing std::forward
  void run(F &&f) const
  {
    this->registrar().register_body(
      this->test_id(),
      internal::TestBodyNoFixture{internal::forward<F>(f)});
  }

private:
  explicit Test(internal::Test_impl *impl);

  [[nodiscard]]
  auto registrar() const -> internal::Registrar;
  [[nodiscard]]
  auto test_id() const -> unsigned long long;

  template<typename R, typename F>
  [[nodiscard]]
  // NOLINTNEXTLINE missing std::forward
  auto make_Test2(F &&f)
  {
    return Test2<R>{
      internal::forward<F>(f),
      this->registrar(),
      this->test_id()};
  }

  internal::UniquePtr<internal::Test_impl> const impl_;

  friend class internal::Engine_impl;
  template<typename FixtureT>
  friend class Test2;
};

template<typename FixtureT>
class Test2
{
public:
  ~Test2() = default;
  Test2() = delete;
  Test2(Test2 const &other) = delete;
  Test2(Test2 &&other) noexcept = delete;
  auto operator=(Test2 const &other) -> Test2 & = delete;
  auto operator=(Test2 &&other) noexcept -> Test2 & = delete;

  Test2(
    internal::NonVoidSetup<FixtureT> setup,
    internal::Registrar registrar,
    unsigned long long const test_id)
    : setup_{internal::move(setup)},
      registrar_{internal::move(registrar)},
      test_id_{test_id}
  {
  }

  template<typename F>
  // NOLINTNEXTLINE missing std::forward
  void run(F &&f)
  {
    this->registrar_.register_body(
      this->test_id_,
      internal::TestBodyNoFixture{
        [setup = internal::move(this->setup_),
         test_body = internal::forward<F>(f)](Context &ctx)
        {
          test_body(ctx, setup(ctx));
        }});
  }

private:
  internal::NonVoidSetup<FixtureT> setup_;
  internal::Registrar registrar_;
  unsigned long long test_id_;
};

template<>
class Test2<void>
{
public:
  ~Test2() = default;
  Test2() = delete;
  Test2(Test2 const &other) = delete;
  Test2(Test2 &&other) noexcept = delete;
  auto operator=(Test2 const &other) -> Test2 & = delete;
  auto operator=(Test2 &&other) noexcept -> Test2 & = delete;

  Test2(
    internal::VoidSetup setup,
    internal::Registrar registrar,
    unsigned long long const test_id)
    : setup_{internal::move(setup)},
      registrar_{internal::move(registrar)},
      test_id_{test_id}
  {
  }

  template<typename F>
  // NOLINTNEXTLINE missing std::forward
  void run(F &&f)
  {
    this->registrar_.register_body(
      this->test_id_,
      internal::TestBodyNoFixture{
        [setup = internal::move(this->setup_),
         test_body = internal::forward<F>(f)](Context &ctx)
        {
          setup(ctx);
          test_body(ctx);
        }});
  }

private:
  internal::VoidSetup setup_;
  internal::Registrar registrar_;
  unsigned long long test_id_;
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
  static void _INTERNAL_WAYPOINT_TEST##_##counter##_##line( \
    waypoint::Engine &(engine)); \
  static void (*_INTERNAL_WAYPOINT_TEST_PTR##_##counter##_##line)( \
    waypoint::Engine &) __attribute__((used, section(#section_name))) = \
    _INTERNAL_WAYPOINT_TEST##_##counter##_##line; \
  static void _INTERNAL_WAYPOINT_TEST##_##counter##_##line( \
    waypoint::Engine &(engine))

#define _INTERNAL_WAYPOINT_AUTORUN_IMPL1_(engine, section_name, counter, line) \
  _INTERNAL_WAYPOINT_AUTORUN_IMPL2_(engine, section_name, counter, line)

#define WAYPOINT_AUTORUN(engine) \
  _INTERNAL_WAYPOINT_AUTORUN_IMPL1_( \
    engine, \
    waypoint_tests, \
    __COUNTER__, \
    __LINE__)
