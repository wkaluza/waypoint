#include "waypoint.hpp"

#include "impls.hpp"

#include <utility>

namespace waypoint
{

Group::~Group()
{
  delete impl_;
}

Group::Group(Group &&other) noexcept :
  impl_{std::exchange(other.impl_, nullptr)}
{
}

auto Group::operator=(Group &&other) noexcept -> Group &
{
  if(this == &other)
  {
    return *this;
  }

  impl_ = std::exchange(other.impl_, nullptr);

  return *this;
}

Group::Group(Engine & /*engine*/) :
  impl_{new Group_impl{}}
{
}

Test::~Test()
{
  delete impl_;
}

Test::Test(Test &&other) noexcept :
  impl_{std::exchange(other.impl_, nullptr)}
{
}

auto Test::operator=(Test &&other) noexcept -> Test &
{
  if(this == &other)
  {
    return *this;
  }

  impl_ = std::exchange(other.impl_, nullptr);

  return *this;
}

Test::Test(Engine &engine) :
  impl_{new Test_impl{engine}}
{
}

auto Test::run(BodyFnPtr const &body) -> Test &
{
  get_impl(this->impl_->get_engine())
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
  auto test_id = this->impl_->register_test(get_impl(group).get_id(), name);

  return this->impl_->get_test(test_id);
}

Engine::~Engine()
{
  delete impl_;
}

Engine::Engine() :
  impl_{new Engine_impl{*this}}
{
}

void Context::assert(bool const condition)
{
  get_impl(impl_->get_engine())
    .register_assertion(condition, get_impl(*this).test_id());
}

Context::~Context()
{
  delete impl_;
}

Context::Context(Context_impl *const impl) :
  impl_{impl}
{
}

Result::~Result()
{
  delete impl_;
}

Result::Result(Result_impl *const impl) :
  impl_{impl}
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
WAYPOINT_TESTS(t)
{
  (void)t;
}

} // namespace
