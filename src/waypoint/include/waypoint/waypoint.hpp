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
class ContextInProcess_impl;
class ContextChildProcess_impl;
class Engine_impl;
class Group_impl;
class RunResult_impl;
class Test_impl;
class TestOutcome_impl;

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
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
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
// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
constexpr auto forward(remove_reference_t<T> &&t) noexcept -> T &&
{
  return static_cast<T &&>(t);
}

template<typename /*T*/, typename /*U*/>
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
  -> decltype(internal::forward<F>(f)(internal::forward<Args>(args)...));

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
class MoveableUniquePtr
{
public:
  ~MoveableUniquePtr()
  {
    delete this->ptr_;
  }

  MoveableUniquePtr() = delete;

  template<typename U>
  explicit MoveableUniquePtr(U *ptr)
    : ptr_{ptr}
  {
  }

  MoveableUniquePtr(MoveableUniquePtr const &other) = delete;
  auto operator=(MoveableUniquePtr const &other)
    -> MoveableUniquePtr & = delete;

  MoveableUniquePtr(MoveableUniquePtr &&other) noexcept
    : ptr_{other.ptr_}
  {
    other.ptr_ = nullptr;
  }

  auto operator=(MoveableUniquePtr &&other) noexcept -> MoveableUniquePtr &
  {
    if(this == &other)
    {
      return *this;
    }

    delete this->ptr_;
    this->ptr_ = other.ptr_;
    other.ptr_ = nullptr;

    return *this;
  }

  explicit operator bool() const
  {
    return ptr_ != nullptr;
  }

  auto operator->() const -> T *
  {
    return ptr_;
  }

  auto operator*() const -> T &
  {
    return *ptr_;
  }

private:
  T *ptr_;
};

template<typename T>
class Function;

template<typename R>
class Function<R(Context const &)>
{
public:
  ~Function() = default;

  Function()
    : callable_{static_cast<callable_interface *>(nullptr)}
  {
  }

  Function(Function const &other) = delete;
  Function(Function &&other) noexcept = default;
  auto operator=(Function const &other) -> Function & = delete;
  auto operator=(Function &&other) noexcept -> Function & = default;

  template<typename F>
  requires requires { !is_same_v<F, Function<R(Context const &)>>; }
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward,google-explicit-constructor)
  Function(F &&f)
    : callable_{MoveableUniquePtr<callable_interface>{
        new callable<F>{internal::forward<F>(f)}}}
  {
  }

  explicit operator bool() const
  {
    return static_cast<bool>(this->callable_);
  }

  auto operator()(Context const &ctx) const -> R
  {
    return this->callable_->invoke(ctx);
  }

private:
  class callable_interface
  {
  public:
    virtual ~callable_interface() = default;
    callable_interface() = default;
    callable_interface(callable_interface const &other) = delete;
    callable_interface(callable_interface &&other) noexcept = delete;
    auto operator=(callable_interface const &other)
      -> callable_interface & = delete;
    auto operator=(callable_interface &&other) noexcept
      -> callable_interface & = delete;

    virtual auto invoke(Context const &ctx) -> R = 0;
  };

  template<typename F>
  class callable final : public callable_interface
  {
  public:
    ~callable() override = default;
    callable() = delete;
    callable(callable const &other) = delete;
    callable(callable &&other) noexcept = delete;
    auto operator=(callable const &other) -> callable & = delete;
    auto operator=(callable &&other) noexcept -> callable & = delete;

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    explicit callable(F &&f)
      : fn_{internal::move(f)}
    {
    }

    auto invoke(Context const &ctx) -> R override
    {
      return this->fn_(ctx);
    }

  private:
    F fn_;
  };

  MoveableUniquePtr<callable_interface> callable_;
};

template<>
class Function<void(Context const &)>
{
public:
  ~Function();
  Function();
  Function(Function const &other) = delete;
  Function(Function &&other) noexcept;
  auto operator=(Function const &other) -> Function & = delete;
  auto operator=(Function &&other) noexcept -> Function &;

  template<typename F>
  requires requires { !is_same_v<F, Function<void(Context const &)>>; }
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward,google-explicit-constructor)
  Function(F &&f)
    : callable_{MoveableUniquePtr<callable_interface>{
        new callable<F>{internal::forward<F>(f)}}}
  {
  }

  explicit operator bool() const;

  void operator()(Context const &ctx) const;

private:
  class callable_interface
  {
  public:
    virtual ~callable_interface();
    callable_interface();
    callable_interface(callable_interface const &other) = delete;
    callable_interface(callable_interface &&other) noexcept = delete;
    auto operator=(callable_interface const &other)
      -> callable_interface & = delete;
    auto operator=(callable_interface &&other) noexcept
      -> callable_interface & = delete;

    virtual void invoke(Context const &ctx) = 0;
  };

  template<typename F>
  class callable final : public callable_interface
  {
  public:
    ~callable() override = default;
    callable() = delete;
    callable(callable const &other) = delete;
    callable(callable &&other) noexcept = delete;
    auto operator=(callable const &other) -> callable & = delete;
    auto operator=(callable &&other) noexcept -> callable & = delete;

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    explicit callable(F &&f)
      : fn_{internal::move(f)}
    {
    }

    void invoke(Context const &ctx) override
    {
      this->fn_(ctx);
    }

  private:
    F fn_;
  };

  MoveableUniquePtr<callable_interface> callable_;
};

template<typename FixtureT>
class Function<void(Context const &, FixtureT &)>
{
public:
  ~Function() = default;

  Function()
    : callable_{static_cast<callable_interface *>(nullptr)}
  {
  }

  Function(Function const &other) = delete;
  Function(Function &&other) noexcept = default;
  auto operator=(Function const &other) -> Function & = delete;
  auto operator=(Function &&other) noexcept -> Function & = default;

  template<typename F>
  requires requires {
    !is_same_v<F, Function<void(Context const &, FixtureT &)>>;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward,google-explicit-constructor)
  Function(F &&f)
    : callable_{MoveableUniquePtr<callable_interface>{
        new callable<F>{internal::forward<F>(f)}}}
  {
  }

  explicit operator bool() const
  {
    return static_cast<bool>(this->callable_);
  }

  void operator()(Context const &ctx, FixtureT &f) const
  {
    this->callable_->invoke(ctx, f);
  }

private:
  class callable_interface
  {
  public:
    virtual ~callable_interface() = default;
    callable_interface() = default;
    callable_interface(callable_interface const &other) = delete;
    callable_interface(callable_interface &&other) noexcept = delete;
    auto operator=(callable_interface const &other)
      -> callable_interface & = delete;
    auto operator=(callable_interface &&other) noexcept
      -> callable_interface & = delete;

    virtual void invoke(Context const &ctx, FixtureT &f) = 0;
  };

  template<typename F>
  class callable final : public callable_interface
  {
  public:
    ~callable() override = default;
    callable() = delete;
    callable(callable const &other) = delete;
    callable(callable &&other) noexcept = delete;
    auto operator=(callable const &other) -> callable & = delete;
    auto operator=(callable &&other) noexcept -> callable & = delete;

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    explicit callable(F &&f)
      : fn_{internal::move(f)}
    {
    }

    void invoke(Context const &ctx, FixtureT &f) override
    {
      this->fn_(ctx, f);
    }

  private:
    F fn_;
  };

  MoveableUniquePtr<callable_interface> callable_;
};

using TestAssembly = Function<void(Context const &)>;

using VoidSetup = Function<void(Context const &)>;
using TestBodyNoFixture = Function<void(Context const &)>;
using TeardownNoFixture = Function<void(Context const &)>;

template<typename FixtureT>
using NonVoidSetup = Function<FixtureT(Context const &)>;
template<typename FixtureT>
using TestBodyWithFixture = Function<void(Context const &, FixtureT &)>;
template<typename FixtureT>
using TeardownWithFixture = Function<void(Context const &, FixtureT &)>;

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
extern template class UniquePtr<ContextInProcess_impl>;
extern template class UniquePtr<ContextChildProcess_impl>;
extern template class UniquePtr<Engine_impl>;
extern template class UniquePtr<Group_impl>;
extern template class UniquePtr<RunResult_impl>;
extern template class UniquePtr<Test_impl>;
extern template class UniquePtr<TestOutcome_impl>;

template<typename FixtureT>
class Registrar;

} // namespace waypoint::internal

namespace waypoint
{

class Engine
{
public:
  ~Engine();
  Engine(Engine const &other) = delete;
  Engine(Engine &&other) noexcept = delete;
  auto operator=(Engine const &other) -> Engine & = delete;
  auto operator=(Engine &&other) noexcept -> Engine & = delete;

  auto group(char const *name) const noexcept -> Group;
  auto test(Group const &group, char const *name) const noexcept
    -> waypoint::Test;

private:
  explicit Engine(internal::Engine_impl *impl);

  void register_test_assembly(
    internal::TestAssembly f,
    unsigned long long test_id,
    bool disabled) const;
  void report_incomplete_test(unsigned long long test_id) const;

  internal::UniquePtr<internal::Engine_impl> const impl_;

  friend auto internal::get_impl(Engine const &engine)
    -> internal::Engine_impl &;

  friend auto make_default_engine() noexcept -> Engine;

  template<typename FixtureT>
  friend class internal::Registrar;
};

class Context
{
public:
  virtual ~Context();
  Context();
  Context(Context const &other) = delete;
  Context(Context &&other) noexcept = delete;
  auto operator=(Context const &other) -> Context & = delete;
  auto operator=(Context &&other) noexcept -> Context & = delete;

  virtual void assert(bool condition) const noexcept = 0;
  virtual void assert(bool condition, char const *message) const noexcept = 0;
  [[nodiscard]]
  virtual auto assume(bool condition) const noexcept -> bool = 0;
  [[nodiscard]]
  virtual auto assume(bool condition, char const *message) const noexcept
    -> bool = 0;
};

class ContextInProcess final : public Context
{
public:
  ~ContextInProcess() override;
  ContextInProcess(ContextInProcess const &other) = delete;
  ContextInProcess(ContextInProcess &&other) noexcept = delete;
  auto operator=(ContextInProcess const &other) -> ContextInProcess & = delete;
  auto operator=(ContextInProcess &&other) noexcept
    -> ContextInProcess & = delete;

  void assert(bool condition) const noexcept override;
  void assert(bool condition, char const *message) const noexcept override;
  [[nodiscard]]
  auto assume(bool condition) const noexcept -> bool override;
  [[nodiscard]]
  auto assume(bool condition, char const *message) const noexcept
    -> bool override;

private:
  explicit ContextInProcess(internal::ContextInProcess_impl *impl);

  internal::UniquePtr<internal::ContextInProcess_impl> const impl_;

  friend class internal::Engine_impl;
};

class ContextChildProcess final : public Context
{
public:
  ~ContextChildProcess() override;
  ContextChildProcess(ContextChildProcess const &other) = delete;
  ContextChildProcess(ContextChildProcess &&other) noexcept = delete;
  auto operator=(ContextChildProcess const &other)
    -> ContextChildProcess & = delete;
  auto operator=(ContextChildProcess &&other) noexcept
    -> ContextChildProcess & = delete;

  void assert(bool condition) const noexcept override;
  void assert(bool condition, char const *message) const noexcept override;
  [[nodiscard]]
  auto assume(bool condition) const noexcept -> bool override;
  [[nodiscard]]
  auto assume(bool condition, char const *message) const noexcept
    -> bool override;

private:
  explicit ContextChildProcess(internal::ContextChildProcess_impl *impl);

  internal::UniquePtr<internal::ContextChildProcess_impl> const impl_;

  friend class internal::Engine_impl;
};

} // namespace waypoint

namespace waypoint::internal
{

template<typename FixtureT>
class Registrar
{
public:
  ~Registrar()
  {
    if(!this->is_active_)
    {
      return;
    }

    if(!static_cast<bool>(this->body_))
    {
      this->report_incomplete_test(this->test_id_);

      return;
    }

    this->engine_.register_test_assembly(
      [setup = move(this->setup_),
       body = move(this->body_),
       teardown = move(this->teardown_)](Context const &ctx) noexcept
      {
        FixtureT fixture = setup(ctx);
        body(ctx, fixture);
        if(static_cast<bool>(teardown))
        {
          teardown(ctx, fixture);
        }
      },
      this->test_id_,
      this->is_disabled_);
  }

  Registrar(Registrar const &other) = delete;

  Registrar(Registrar &&other) noexcept
    : is_active_{other.is_active_},
      engine_{other.engine_},
      test_id_{other.test_id_},
      setup_{move(other.setup_)},
      body_{move(other.body_)},
      teardown_{move(other.teardown_)},
      is_disabled_{false}
  {
    other.is_active_ = false;
  }

  auto operator=(Registrar const &other) -> Registrar & = delete;
  auto operator=(Registrar &&other) noexcept -> Registrar & = delete;

  void register_setup(NonVoidSetup<FixtureT> f)
  {
    this->is_active_ = true;

    this->setup_ = move(f);
  }

  void register_body(TestBodyWithFixture<FixtureT> f)
  {
    this->is_active_ = true;

    this->body_ = move(f);
  }

  void register_teardown(TeardownWithFixture<FixtureT> f)
  {
    this->is_active_ = true;

    this->teardown_ = move(f);
  }

  void disable(bool const is_disabled)
  {
    this->is_disabled_ = is_disabled;
  }

private:
  Registrar(Engine const &engine, unsigned long long const test_id)
    : is_active_{false},
      engine_{engine},
      test_id_{test_id},
      setup_{},
      body_{},
      teardown_{},
      is_disabled_{false}
  {
  }

  void report_incomplete_test(unsigned long long const test_id) const
  {
    this->engine_.report_incomplete_test(test_id);
  }

  bool is_active_;
  Engine const &engine_;
  unsigned long long test_id_;
  NonVoidSetup<FixtureT> setup_;
  TestBodyWithFixture<FixtureT> body_;
  TeardownWithFixture<FixtureT> teardown_;
  bool is_disabled_;

  friend class waypoint::Test;
};

template<>
class Registrar<void>
{
public:
  ~Registrar();

  Registrar(Registrar const &other) = delete;

  Registrar(Registrar &&other) noexcept;

  auto operator=(Registrar const &other) -> Registrar & = delete;
  auto operator=(Registrar &&other) noexcept -> Registrar & = delete;

  void register_setup(VoidSetup f);
  void register_body(TestBodyNoFixture f);
  void register_teardown(TeardownNoFixture f);

  void disable(bool is_disabled);

private:
  Registrar(Engine const &engine, unsigned long long test_id);

  void report_incomplete_test(unsigned long long test_id) const;

  bool is_active_;
  Engine const &engine_;
  unsigned long long test_id_;
  VoidSetup setup_;
  TestBodyNoFixture body_;
  TeardownNoFixture teardown_;
  bool is_disabled_;

  friend class waypoint::Test;
};

} // namespace waypoint::internal

namespace waypoint
{

[[nodiscard]]
auto make_default_engine() noexcept -> Engine;
[[nodiscard]]
auto run_all_tests_in_process(Engine const &t) noexcept -> RunResult;
[[nodiscard]]
auto run_all_tests(Engine const &t) noexcept -> RunResult;

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
  auto group() const noexcept -> char const *;
  [[nodiscard]]
  auto test() const noexcept -> char const *;
  [[nodiscard]]
  auto message() const noexcept -> char const *;
  [[nodiscard]]
  auto passed() const noexcept -> bool;
  [[nodiscard]]
  auto index() const noexcept -> unsigned long long;

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

  enum class Status : unsigned char
  {
    Success,
    Failure,
    NotRun
  };

  [[nodiscard]]
  auto group_name() const noexcept -> char const *;
  [[nodiscard]]
  auto group_id() const noexcept -> unsigned long long;
  [[nodiscard]]
  auto test_name() const noexcept -> char const *;
  [[nodiscard]]
  auto test_id() const noexcept -> unsigned long long;
  [[nodiscard]]
  auto test_index() const noexcept -> unsigned long long;
  [[nodiscard]]
  auto assertion_count() const noexcept -> unsigned long long;
  [[nodiscard]]
  auto assertion_outcome(unsigned long long index) const noexcept
    -> AssertionOutcome const &;
  [[nodiscard]]
  auto disabled() const noexcept -> bool;
  [[nodiscard]]
  auto status() const noexcept -> TestOutcome::Status;

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

template<typename>
class Test3;

template<typename FixtureT>
class Test4
{
public:
  ~Test4() = default;
  Test4() = delete;
  Test4(Test4 const &other) = delete;
  Test4(Test4 &&other) noexcept = delete;
  auto operator=(Test4 const &other) -> Test4 & = delete;
  auto operator=(Test4 &&other) noexcept -> Test4 & = delete;

  void disable() && noexcept
  {
    this->registrar_.disable(true);
  }

  void disable(bool const is_disabled) && noexcept
  {
    this->registrar_.disable(is_disabled);
  }

private:
  explicit Test4(internal::Registrar<FixtureT> registrar)
    : registrar_{internal::move(registrar)}
  {
  }

  internal::Registrar<FixtureT> registrar_;

  template<typename>
  friend class waypoint::Test3;
};

template<typename>
class Test2;

template<typename FixtureT>
class Test3
{
public:
  ~Test3() = default;
  Test3() = delete;
  Test3(Test3 const &other) = delete;
  Test3(Test3 &&other) noexcept = delete;
  auto operator=(Test3 const &other) -> Test3 & = delete;
  auto operator=(Test3 &&other) noexcept -> Test3 & = delete;

  template<typename F>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto teardown(F &&f) && noexcept -> Test4<FixtureT>
  {
    this->registrar_.register_teardown(internal::forward<F>(f));

    return waypoint::Test4<FixtureT>{internal::move(this->registrar_)};
  }

  void disable() && noexcept
  {
    this->registrar_.disable(true);
  }

  void disable(bool const is_disabled) && noexcept
  {
    this->registrar_.disable(is_disabled);
  }

private:
  explicit Test3(internal::Registrar<FixtureT> registrar)
    : registrar_{internal::move(registrar)}
  {
  }

  internal::Registrar<FixtureT> registrar_;

  template<typename>
  friend class waypoint::Test2;
};

template<>
class Test3<void>
{
public:
  ~Test3();
  Test3() = delete;
  Test3(Test3 const &other) = delete;
  Test3(Test3 &&other) noexcept = delete;
  auto operator=(Test3 const &other) -> Test3 & = delete;
  auto operator=(Test3 &&other) noexcept -> Test3 & = delete;

  template<typename F>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto teardown(F &&f) && noexcept -> Test4<void>
  {
    this->registrar_.register_teardown(internal::forward<F>(f));

    return waypoint::Test4<void>{internal::move(this->registrar_)};
  }

  void disable() && noexcept;
  void disable(bool is_disabled) && noexcept;

private:
  explicit Test3(internal::Registrar<void> registrar);

  internal::Registrar<void> registrar_;

  template<typename>
  friend class waypoint::Test2;
  friend class waypoint::Test;
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

  template<typename F>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto run(F &&f) && noexcept -> waypoint::Test3<FixtureT>
  {
    this->registrar_.register_body(internal::forward<F>(f));

    return waypoint::Test3<FixtureT>{internal::move(this->registrar_)};
  }

private:
  explicit Test2(internal::Registrar<FixtureT> registrar)
    : registrar_{internal::move(registrar)}
  {
  }

  internal::Registrar<FixtureT> registrar_;

  friend class waypoint::Test;
};

template<>
class Test2<void>
{
public:
  ~Test2();
  Test2() = delete;
  Test2(Test2 const &other) = delete;
  Test2(Test2 &&other) noexcept = delete;
  auto operator=(Test2 const &other) -> Test2 & = delete;
  auto operator=(Test2 &&other) noexcept -> Test2 & = delete;

  template<typename F>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto run(F &&f) && noexcept -> waypoint::Test3<void>
  {
    this->registrar_.register_body(internal::forward<F>(f));

    return waypoint::Test3<void>{internal::move(this->registrar_)};
  }

private:
  explicit Test2(internal::Registrar<void> registrar);

  internal::Registrar<void> registrar_;

  friend class waypoint::Test;
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
  [[nodiscard]]
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto setup(F &&f) && noexcept -> internal::enable_if_t<
    !internal::is_void_v<internal::setup_invoke_result_t<F>>,
    waypoint::Test2<internal::setup_invoke_result_t<F>>>
  {
    this->mark_complete();

    auto registrar = this->make_registrar<internal::setup_invoke_result_t<F>>();

    registrar.register_setup(internal::forward<F>(f));

    return waypoint::Test2<internal::setup_invoke_result_t<F>>{
      internal::move(registrar)};
  }

  template<typename F>
  [[nodiscard]]
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto setup(F &&f) && noexcept -> internal::enable_if_t<
    internal::is_void_v<internal::setup_invoke_result_t<F>>,
    waypoint::Test2<void>>
  {
    this->mark_complete();

    auto registrar = this->make_registrar<void>();

    registrar.register_setup(internal::forward<F>(f));

    return waypoint::Test2<internal::setup_invoke_result_t<F>>{
      internal::move(registrar)};
  }

  template<typename F>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto run(F &&f) && noexcept -> waypoint::Test3<void>
  {
    this->mark_complete();

    auto registrar = this->make_registrar<void>();

    registrar.register_body(internal::forward<F>(f));

    return waypoint::Test3<void>{internal::move(registrar)};
  }

private:
  explicit Test(internal::Test_impl *impl);

  template<typename T>
  [[nodiscard]]
  auto make_registrar() const -> internal::Registrar<T>
  {
    return internal::Registrar<T>{this->get_engine(), this->test_id()};
  }

  [[nodiscard]]
  auto test_id() const -> unsigned long long;
  [[nodiscard]]
  auto get_engine() const -> Engine const &;
  void mark_complete() const;

  internal::UniquePtr<internal::Test_impl> const impl_;

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
  auto success() const noexcept -> bool;
  [[nodiscard]]
  auto test_count() const noexcept -> unsigned long long;
  [[nodiscard]]
  auto test_outcome(unsigned long long index) const noexcept
    -> TestOutcome const &;
  [[nodiscard]]
  auto error_count() const noexcept -> unsigned long long;
  [[nodiscard]]
  auto error(unsigned long long index) const noexcept -> char const *;

private:
  explicit RunResult(internal::RunResult_impl *impl);

  internal::UniquePtr<internal::RunResult_impl> const impl_;

  friend class internal::Engine_impl;
};

} // namespace waypoint

#define _INTERNAL_WAYPOINT_AUTORUN_IMPL2_(engine, section_name, counter, line) \
  static void _INTERNAL_WAYPOINT_TEST##_##counter##_##line(engine); \
  static void (*_INTERNAL_WAYPOINT_TEST_PTR##_##counter##_##line)(engine) \
    __attribute__((used, section(#section_name))) = \
      _INTERNAL_WAYPOINT_TEST##_##counter##_##line; \
  static void _INTERNAL_WAYPOINT_TEST##_##counter##_##line(engine)

#define _INTERNAL_WAYPOINT_AUTORUN_IMPL1_(engine, section_name, counter, line) \
  _INTERNAL_WAYPOINT_AUTORUN_IMPL2_(engine, section_name, counter, line)

#define WAYPOINT_AUTORUN(engine) \
  _INTERNAL_WAYPOINT_AUTORUN_IMPL1_( \
    engine, \
    waypoint_tests, \
    __COUNTER__, \
    __LINE__)
