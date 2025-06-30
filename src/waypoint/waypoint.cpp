#include "waypoint.hpp"

#include "impls.hpp"

#include <optional>

namespace waypoint
{

AssertionOutcome::~AssertionOutcome() = default;

AssertionOutcome::AssertionOutcome(
  internal::AssertionOutcome_impl *const impl) :
  impl_{internal::UniquePtr{impl}}
{
}

AssertionOutcome::AssertionOutcome(AssertionOutcome &&other) noexcept = default;

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

TestOutcome::TestOutcome(TestOutcome &&other) noexcept = default;

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

auto Test::run(BodyFnPtr const &body) -> Test &
{
  internal::get_impl(this->impl_->get_engine())
    .register_test_body(body, this->impl_->get_id());

  return *this;
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

namespace
{

// NOLINTNEXTLINE param may be const
WAYPOINT_AUTORUN(t)
{
  (void)t;
}

} // namespace
