#include "waypoint.hpp"

#include "autorun.hpp"
#include "impls.hpp"

#include <algorithm>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

namespace waypoint::internal
{

auto get_impl(Engine const &engine) -> Engine_impl &
{
  return *engine.impl_;
}

} // namespace waypoint::internal

namespace
{

void populate_test_indices_(waypoint::Engine const &t)
{
  waypoint::internal::get_impl(t).set_shuffled_test_record_ptrs();
  auto const &shuffled_ptrs =
    waypoint::internal::get_impl(t).get_shuffled_test_record_ptrs();
  for(unsigned long long i = 0; i < shuffled_ptrs.size(); ++i)
  {
    waypoint::internal::get_impl(t).set_test_index(
      shuffled_ptrs[i]->test_id(),
      i);
  }
}

void initialize(waypoint::Engine const &t) noexcept
{
  auto const section = waypoint::internal::get_autorun_section_boundaries();
  auto const begin = section.first;
  auto const end = section.second;

  using AutorunFunctionPtr = void (*)(waypoint::Engine const &);
  std::vector<AutorunFunctionPtr> functions;

  for(auto fn_ptr = begin; fn_ptr < end; fn_ptr += sizeof(AutorunFunctionPtr))
  {
    functions.push_back(*reinterpret_cast<AutorunFunctionPtr *>(fn_ptr));
  }

  auto is_not_null = [](auto *ptr)
  {
    return ptr != nullptr;
  };

  for(auto const fn_ptr : functions | std::views::filter(is_not_null))
  {
    fn_ptr(t);
  }

  populate_test_indices_(t);
}

} // namespace

namespace waypoint
{

auto make_default_engine() noexcept -> Engine
{
  auto *impl = new internal::Engine_impl{};

  return Engine{impl};
}

auto run_all_tests(Engine const &t) noexcept -> RunResult
{
  initialize(t);
  if(internal::get_impl(t).has_errors())
  {
    // Initialization had errors, skip running tests and emit results
    return internal::get_impl(t).generate_results();
  }

  std::ranges::for_each(
    internal::get_impl(t).get_shuffled_test_record_ptrs(),
    [&t](auto *const ptr) noexcept
    {
      auto context = internal::get_impl(t).make_context(ptr->test_id());
      // Run test
      if(!ptr->disabled())
      {
        ptr->test_assembly()(context);
        ptr->mark_as_run();
      }
    });

  return internal::get_impl(t).generate_results();
}

AssertionOutcome::~AssertionOutcome() = default;

AssertionOutcome::AssertionOutcome(internal::AssertionOutcome_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

auto AssertionOutcome::group() const noexcept -> char const *
{
  return this->impl_->group_name().c_str();
}

auto AssertionOutcome::test() const noexcept -> char const *
{
  return this->impl_->test_name().c_str();
}

auto AssertionOutcome::message() const noexcept -> char const *
{
  return this->impl_->message().c_str();
}

auto AssertionOutcome::passed() const noexcept -> bool
{
  return this->impl_->passed();
}

auto AssertionOutcome::index() const noexcept -> unsigned long long
{
  return this->impl_->index();
}

TestOutcome::~TestOutcome() = default;

TestOutcome::TestOutcome(internal::TestOutcome_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

auto TestOutcome::group_name() const noexcept -> char const *
{
  return this->impl_->get_group_name().c_str();
}

auto TestOutcome::group_id() const noexcept -> unsigned long long
{
  return this->impl_->get_group_id();
}

auto TestOutcome::test_name() const noexcept -> char const *
{
  return this->impl_->get_test_name().c_str();
}

auto TestOutcome::test_id() const noexcept -> unsigned long long
{
  return this->impl_->get_test_id();
}

auto TestOutcome::test_index() const noexcept -> unsigned long long
{
  return this->impl_->get_index();
}

auto TestOutcome::assertion_count() const noexcept -> unsigned long long
{
  return this->impl_->get_assertion_count();
}

auto TestOutcome::assertion_outcome(
  unsigned long long const index) const noexcept -> AssertionOutcome const &
{
  return this->impl_->get_assertion_outcome(index);
}

auto TestOutcome::disabled() const noexcept -> bool
{
  return this->impl_->disabled();
}

auto TestOutcome::status() const noexcept -> TestOutcome::Status
{
  return this->impl_->status();
}

Group::~Group() = default;

Group::Group(internal::Group_impl *const impl)
  : impl_{impl}
{
}

Test3<void>::~Test3() = default;

Test3<void>::Test3(internal::Registrar<void> registrar)
  : registrar_{internal::move(registrar)}
{
}

void Test3<void>::disable() && noexcept
{
  this->registrar_.disable(true);
}

void Test3<void>::disable(bool const is_disabled) && noexcept
{
  this->registrar_.disable(is_disabled);
}

Test2<void>::~Test2() = default;

Test2<void>::Test2(internal::Registrar<void> registrar)
  : registrar_{internal::move(registrar)}
{
}

Test::~Test() = default;

Test::Test(internal::Test_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

auto Test::test_id() const -> unsigned long long
{
  return this->impl_->get_id();
}

auto Test::get_engine() const -> Engine const &
{
  return this->impl_->get_engine();
}

void Test::mark_complete() const
{
  this->impl_->mark_complete();
}

auto Engine::group(char const *name) const noexcept -> Group
{
  auto const group_id = this->impl_->register_group(name);

  return this->impl_->make_group(group_id);
}

auto Engine::test(Group const &group, char const *name) const noexcept -> Test
{
  auto const test_id =
    this->impl_->register_test(this->impl_->get_group_id(group), name);

  return this->impl_->make_test(test_id);
}

void Engine::register_test_assembly(
  internal::TestAssembly f,
  unsigned long long const test_id,
  bool const disabled) const
{
  this->impl_->register_test_assembly(std::move(f), test_id, disabled);
}

void Engine::report_incomplete_test(unsigned long long const test_id) const
{
  this->impl_->report_incomplete_test(test_id);
}

Engine::~Engine() = default;

Engine::Engine(internal::Engine_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
  impl->initialize(*this);
}

void Context::assert(bool const condition) const noexcept
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      std::nullopt);
}

void Context::assert(bool const condition, char const *const message)
  const noexcept
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      message);
}

auto Context::assume(bool const condition) const noexcept -> bool
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      std::nullopt);

  return condition;
}

auto Context::assume(bool const condition, char const *const message)
  const noexcept -> bool
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      message);

  return condition;
}

Context::~Context() = default;

Context::Context(internal::Context_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

RunResult::~RunResult() = default;

RunResult::RunResult(internal::RunResult_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

auto RunResult::success() const noexcept -> bool
{
  return !this->impl_->has_errors() && !this->impl_->has_failing_assertions();
}

auto RunResult::test_count() const noexcept -> unsigned long long
{
  return this->impl_->test_outcome_count();
}

auto RunResult::test_outcome(unsigned long long const index) const noexcept
  -> TestOutcome const &
{
  return this->impl_->get_test_outcome(index);
}

auto RunResult::error_count() const noexcept -> unsigned long long
{
  return this->impl_->errors().size();
}

auto RunResult::error(unsigned long long const index) const noexcept
  -> char const *
{
  return this->impl_->errors().at(index).c_str();
}

} // namespace waypoint

WAYPOINT_AUTORUN(waypoint::Engine const &)
{
}
