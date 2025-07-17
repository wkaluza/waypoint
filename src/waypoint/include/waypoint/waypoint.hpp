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
class UnsafeSharedPtr
{
public:
  ~UnsafeSharedPtr()
  {
    if(this->ref_count_ != nullptr)
    {
      *this->ref_count_ -= 1;
    }

    if(this->ref_count_ == nullptr || *this->ref_count_ == 0)
    {
      delete this->ptr_;
      delete this->ref_count_;
    }
  }

  UnsafeSharedPtr() = delete;

  template<typename U>
  explicit UnsafeSharedPtr(U *ptr)
    : ptr_{ptr},
      ref_count_{ptr_ == nullptr ? nullptr : new unsigned long long{1}}
  {
  }

  UnsafeSharedPtr(UnsafeSharedPtr const &other)
    : ptr_{other.ptr_},
      ref_count_{other.ref_count_}
  {
    if(this->ref_count_ == nullptr)
    {
      return;
    }

    *this->ref_count_ += 1;
  }

  auto operator=(UnsafeSharedPtr const &other) -> UnsafeSharedPtr &
  {
    if(this == &other)
    {
      return *this;
    }

    if(this->ptr_ == other.ptr_)
    {
      return *this;
    }

    if(this->ref_count_ != nullptr)
    {
      *this->ref_count_ -= 1;
    }

    if(this->ref_count_ == nullptr || *this->ref_count_ == 0)
    {
      delete this->ptr_;
      delete this->ref_count_;
    }

    this->ptr_ = other.ptr_;
    this->ref_count_ = other.ref_count_;

    if(this->ref_count_ == nullptr)
    {
      return *this;
    }

    *this->ref_count_ += 1;

    return *this;
  }

  UnsafeSharedPtr(UnsafeSharedPtr &&other) noexcept
    : ptr_{other.ptr_},
      ref_count_{other.ref_count_}
  {
    other.ptr_ = nullptr;
    other.ref_count_ = nullptr;
  }

  auto operator=(UnsafeSharedPtr &&other) noexcept -> UnsafeSharedPtr &
  {
    if(this == &other)
    {
      return *this;
    }

    if(this->ptr_ == other.ptr_)
    {
      return *this;
    }

    if(this->ref_count_ != nullptr)
    {
      *this->ref_count_ -= 1;
    }

    if(this->ref_count_ == nullptr || *this->ref_count_ == 0)
    {
      delete this->ptr_;
      delete this->ref_count_;
    }

    this->ref_count_ = other.ref_count_;
    this->ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    other.ref_count_ = nullptr;

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
  unsigned long long *ref_count_;
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

  Function(Function const &other) = default;
  Function(Function &&other) noexcept = default;
  auto operator=(Function const &other) -> Function & = default;
  auto operator=(Function &&other) noexcept -> Function & = default;

  template<typename F>
  requires requires { !is_same_v<F, Function<R(Context const &)>>; }
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward,google-explicit-constructor)
  Function(F &&f)
    : callable_{UnsafeSharedPtr<callable_interface>{
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

  UnsafeSharedPtr<callable_interface> callable_;
};

template<>
class Function<void(Context const &)>
{
public:
  ~Function() = default;

  Function()
    : callable_{static_cast<callable_interface *>(nullptr)}
  {
  }

  Function(Function const &other) = default;
  Function(Function &&other) noexcept = default;
  auto operator=(Function const &other) -> Function & = default;
  auto operator=(Function &&other) noexcept -> Function & = default;

  template<typename F>
  requires requires { !is_same_v<F, Function<void(Context const &)>>; }
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward,google-explicit-constructor)
  Function(F &&f)
    : callable_{UnsafeSharedPtr<callable_interface>{
        new callable<F>{internal::forward<F>(f)}}}
  {
  }

  explicit operator bool() const
  {
    return static_cast<bool>(this->callable_);
  }

  void operator()(Context const &ctx) const
  {
    this->callable_->invoke(ctx);
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

  UnsafeSharedPtr<callable_interface> callable_;
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

  Function(Function const &other) = default;
  Function(Function &&other) noexcept = default;
  auto operator=(Function const &other) -> Function & = default;
  auto operator=(Function &&other) noexcept -> Function & = default;

  template<typename F>
  requires requires {
    !is_same_v<F, Function<void(Context const &, FixtureT &)>>;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward,google-explicit-constructor)
  Function(F &&f)
    : callable_{UnsafeSharedPtr<callable_interface>{
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

  UnsafeSharedPtr<callable_interface> callable_;
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
extern template class UniquePtr<Context_impl>;
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

  auto group(char const *name) const -> Group;
  auto test(Group const &group, char const *name) const -> waypoint::Test;

private:
  explicit Engine(internal::Engine_impl *impl);

  void register_test_assembly(
    internal::TestAssembly f,
    unsigned long long test_id) const;

  internal::UniquePtr<internal::Engine_impl> const impl_;

  friend auto internal::get_impl(Engine const &engine)
    -> internal::Engine_impl &;

  friend auto make_default_engine() -> Engine;

  template<typename FixtureT>
  friend class internal::Registrar;
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

} // namespace waypoint

namespace waypoint::internal
{

template<typename FixtureT>
class Registrar
{
public:
  ~Registrar()
  {
    if(static_cast<bool>(this->body_))
    {
      this->engine_.register_test_assembly(
        TestAssembly{[setup = move(this->setup_),
                      body = move(this->body_),
                      teardown = move(this->teardown_)](Context const &ctx)
                     {
                       FixtureT fixture = setup(ctx);
                       body(ctx, fixture);
                       if(static_cast<bool>(teardown))
                       {
                         teardown(ctx, fixture);
                       }
                     }},
        this->test_id_);
    }
  }

  Registrar(Registrar const &other) = delete;
  Registrar(Registrar &&other) noexcept = default;
  auto operator=(Registrar const &other) -> Registrar & = delete;
  auto operator=(Registrar &&other) noexcept -> Registrar & = delete;

  void register_setup(NonVoidSetup<FixtureT> f)
  {
    this->setup_ = move(f);
  }

  void register_body(TestBodyWithFixture<FixtureT> f)
  {
    this->body_ = move(f);
  }

  void register_teardown(TeardownWithFixture<FixtureT> f)
  {
    this->teardown_ = move(f);
  }

private:
  Registrar(Engine const &engine, unsigned long long const test_id)
    : engine_{engine},
      test_id_{test_id},
      setup_{},
      body_{},
      teardown_{}
  {
  }

  Engine const &engine_;
  unsigned long long test_id_;
  NonVoidSetup<FixtureT> setup_;
  TestBodyWithFixture<FixtureT> body_;
  TeardownWithFixture<FixtureT> teardown_;

  friend class waypoint::Test;
};

template<>
class Registrar<void>
{
public:
  ~Registrar()
  {
    if(static_cast<bool>(this->body_))
    {
      this->engine_.register_test_assembly(
        TestAssembly{[setup = move(this->setup_),
                      body = move(this->body_),
                      teardown = move(this->teardown_)](Context const &ctx)
                     {
                       if(static_cast<bool>(setup))
                       {
                         setup(ctx);
                       }
                       body(ctx);
                       if(static_cast<bool>(teardown))
                       {
                         teardown(ctx);
                       }
                     }},
        this->test_id_);
    }
  }

  Registrar(Registrar const &other) = delete;
  Registrar(Registrar &&other) noexcept = default;
  auto operator=(Registrar const &other) -> Registrar & = delete;
  auto operator=(Registrar &&other) noexcept -> Registrar & = delete;

  void register_setup(VoidSetup f)
  {
    this->setup_ = move(f);
  }

  void register_body(TestBodyNoFixture f)
  {
    this->body_ = move(f);
  }

  void register_teardown(TeardownNoFixture f)
  {
    this->teardown_ = move(f);
  }

private:
  Registrar(Engine const &engine, unsigned long long const test_id)
    : engine_{engine},
      test_id_{test_id}
  {
  }

  Engine const &engine_;
  unsigned long long test_id_;
  VoidSetup setup_;
  TestBodyNoFixture body_;
  TeardownNoFixture teardown_;

  friend class waypoint::Test;
};

} // namespace waypoint::internal

namespace waypoint
{

// defined in core_actions.cpp
[[nodiscard]]
auto make_default_engine() -> Engine;
// defined in core_actions.cpp
[[nodiscard]]
auto run_all_tests(Engine const &t) -> RunResult;

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
  void teardown(F &&f) &&
  {
    this->registrar_.register_teardown(internal::forward<F>(f));
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
  ~Test3() = default;
  Test3() = delete;
  Test3(Test3 const &other) = delete;
  Test3(Test3 &&other) noexcept = delete;
  auto operator=(Test3 const &other) -> Test3 & = delete;
  auto operator=(Test3 &&other) noexcept -> Test3 & = delete;

  template<typename F>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  void teardown(F &&f) &&
  {
    this->registrar_.register_teardown(internal::forward<F>(f));
  }

private:
  explicit Test3(internal::Registrar<void> registrar)
    : registrar_{internal::move(registrar)}
  {
  }

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
  auto run(F &&f) && -> waypoint::Test3<FixtureT>
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
  ~Test2() = default;
  Test2() = delete;
  Test2(Test2 const &other) = delete;
  Test2(Test2 &&other) noexcept = delete;
  auto operator=(Test2 const &other) -> Test2 & = delete;
  auto operator=(Test2 &&other) noexcept -> Test2 & = delete;

  template<typename F>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto run(F &&f) && -> waypoint::Test3<void>
  {
    this->registrar_.register_body(internal::forward<F>(f));

    return waypoint::Test3<void>{internal::move(this->registrar_)};
  }

private:
  explicit Test2(internal::Registrar<void> registrar)
    : registrar_{internal::move(registrar)}
  {
  }

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
  auto setup(F &&f) && -> internal::enable_if_t<
    !internal::is_void_v<internal::setup_invoke_result_t<F>>,
    waypoint::Test2<internal::setup_invoke_result_t<F>>>
  {
    auto registrar = this->make_registrar<internal::setup_invoke_result_t<F>>();

    registrar.register_setup(internal::forward<F>(f));

    return waypoint::Test2<internal::setup_invoke_result_t<F>>{
      internal::move(registrar)};
  }

  template<typename F>
  [[nodiscard]]
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto setup(F &&f) && -> internal::enable_if_t<
    internal::is_void_v<internal::setup_invoke_result_t<F>>,
    waypoint::Test2<void>>
  {
    auto registrar = this->make_registrar<void>();

    registrar.register_setup(internal::forward<F>(f));

    return waypoint::Test2<internal::setup_invoke_result_t<F>>{
      internal::move(registrar)};
  }

  template<typename F>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto run(F &&f) && -> waypoint::Test3<void>
  {
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
