#include "waypoint.hpp"

#include "impls.hpp"

#include <optional>
#include <utility>

namespace waypoint::internal
{

Registrar::~Registrar() = default;

Registrar::Registrar() :
  impl_{nullptr}
{
}

Registrar::Registrar(Registrar &&other) noexcept = default;

auto Registrar::operator=(Registrar &&other) noexcept -> Registrar &
{
  // No need to handle self-assignment
  this->impl_ = std::move(other.impl_);

  return *this;
}

Registrar::Registrar(Registrar_impl *impl) :
  impl_{impl}
{
}

void Registrar::register_body(
  unsigned long long const test_id,
  TestBodyNoFixture f) const
{
  get_impl(this->impl_->get_engine()).register_test_body(std::move(f), test_id);
}

} // namespace waypoint::internal

namespace waypoint
{

AssertionOutcome::~AssertionOutcome() = default;

AssertionOutcome::AssertionOutcome(
  internal::AssertionOutcome_impl *const impl) :
  impl_{internal::UniquePtr{impl}}
{
}

auto AssertionOutcome::group() const -> char const *
{
  return this->impl_->group_name.c_str();
}

auto AssertionOutcome::test() const -> char const *
{
  return this->impl_->test_name.c_str();
}

auto AssertionOutcome::message() const -> char const *
{
  return this->impl_->message.c_str();
}

auto AssertionOutcome::passed() const -> bool
{
  return this->impl_->passed;
}

auto AssertionOutcome::index() const -> unsigned long long
{
  return this->impl_->index;
}

TestOutcome::~TestOutcome() = default;

TestOutcome::TestOutcome(internal::TestOutcome_impl *const impl) :
  impl_{internal::UniquePtr{impl}}
{
}

auto TestOutcome::group_name() const -> char const *
{
  return this->impl_->get_group_name().c_str();
}

auto TestOutcome::group_id() const -> unsigned long long
{
  return this->impl_->get_group_id();
}

auto TestOutcome::test_name() const -> char const *
{
  return this->impl_->get_test_name().c_str();
}

auto TestOutcome::test_id() const -> unsigned long long
{
  return this->impl_->get_test_id();
}

auto TestOutcome::test_index() const -> unsigned long long
{
  return this->impl_->get_index();
}

auto TestOutcome::assertion_count() const -> unsigned long long
{
  return this->impl_->get_assertion_count();
}

auto TestOutcome::assertion_outcome(unsigned long long const index) const
  -> AssertionOutcome const &
{
  return this->impl_->get_assertion_outcome(index);
}

Group::~Group() = default;

Group::Group(internal::Group_impl *const impl) :
  impl_{impl}
{
}

Test::~Test() = default;

Test::Test(internal::Test_impl *const impl) :
  impl_{internal::UniquePtr{impl}}
{
}

auto Test::registrar() const -> internal::Registrar
{
  return this->impl_->registrar();
}

auto Test::test_id() const -> unsigned long long
{
  return this->impl_->get_id();
}

auto Engine::group(char const *name) const -> Group
{
  auto const group_id = this->impl_->register_group(name);

  return this->impl_->make_group(group_id);
}

auto Engine::test(Group const &group, char const *name) const -> Test
{
  auto const test_id =
    this->impl_->register_test(this->impl_->get_group_id(group), name);

  return this->impl_->make_test(test_id);
}

Engine::~Engine() = default;

Engine::Engine(internal::Engine_impl *const impl) :
  impl_{internal::UniquePtr{impl}}
{
  impl->initialize(*this);
}

void Context::assert(bool const condition) const
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      std::nullopt);
}

void Context::assert(bool const condition, char const *const message) const
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      message);
}

Context::~Context() = default;

Context::Context(internal::Context_impl *const impl) :
  impl_{internal::UniquePtr{impl}}
{
}

RunResult::~RunResult() = default;

RunResult::RunResult(internal::RunResult_impl *const impl) :
  impl_{internal::UniquePtr{impl}}
{
}

auto RunResult::success() const -> bool
{
  return !this->impl_->has_errors() && !this->impl_->has_failing_assertions();
}

auto RunResult::test_count() const -> unsigned long long
{
  return this->impl_->test_outcome_count();
}

auto RunResult::test_outcome(unsigned long long const index) const
  -> TestOutcome const &
{
  return this->impl_->get_test_outcome(index);
}

} // namespace waypoint

// NOLINTNEXTLINE param may be const
WAYPOINT_AUTORUN(t)
{
  (void)t;
}
