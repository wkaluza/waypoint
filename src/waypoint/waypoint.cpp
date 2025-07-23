#include "waypoint.hpp"

#include "impls.hpp"

#include <optional>
#include <utility>

namespace waypoint
{

AssertionOutcome::~AssertionOutcome() = default;

AssertionOutcome::AssertionOutcome(internal::AssertionOutcome_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

auto AssertionOutcome::group() const noexcept -> char const *
{
  return this->impl_->group_name.c_str();
}

auto AssertionOutcome::test() const noexcept -> char const *
{
  return this->impl_->test_name.c_str();
}

auto AssertionOutcome::message() const noexcept -> char const *
{
  return this->impl_->message.c_str();
}

auto AssertionOutcome::passed() const noexcept -> bool
{
  return this->impl_->passed;
}

auto AssertionOutcome::index() const noexcept -> unsigned long long
{
  return this->impl_->index;
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
  unsigned long long const test_id) const
{
  this->impl_->register_test_assembly(std::move(f), test_id);
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
