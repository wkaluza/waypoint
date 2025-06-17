#include "waypoint.hpp"

#include "impls.hpp"

namespace waypoint
{

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
  auto group_id = this->impl_->register_group(name);

  return this->impl_->get_group(group_id);
}

auto Engine::test(Group const &group, char const *name) const -> Test
{
  auto test_id =
    this->impl_->register_test(internal::get_impl(group).get_id(), name);

  return this->impl_->get_test(test_id);
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
    .register_assertion(condition, internal::get_impl(*this).test_id());
}

Context::~Context() = default;

Context::Context(internal::Context_impl *const impl) :
  impl_{internal::UniquePtr{impl}}
{
}

Result::~Result() = default;

Result::Result(internal::Result_impl *const impl) :
  impl_{internal::UniquePtr{impl}}
{
}

auto Result::pass() const -> bool
{
  return !this->impl_->has_failing_assertions();
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
