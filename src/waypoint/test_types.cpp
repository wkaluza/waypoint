#include "test_types.hpp"

#include "string.hpp"
#include "types.hpp"
#include "waypoint.hpp"

#include <utility>

namespace waypoint
{

TestGroup::~TestGroup() = default;
TestGroup::TestGroup(TestGroup const &other) = default;
TestGroup::TestGroup(TestGroup &&other) noexcept = default;
auto TestGroup::operator=(TestGroup const &other) -> TestGroup & = default;
auto TestGroup::operator=(TestGroup &&other) noexcept -> TestGroup & = default;

TestGroup::TestGroup(String name) :
  name_(std::move(name))
{
}

auto TestGroup::operator==(TestGroup const &other) const noexcept -> bool
{
  return this->name_ == other.name_;
}

auto TestGroup::operator<(TestGroup const &other) const noexcept -> bool
{
  return this->name_ < other.name_;
}

Test::~Test() = default;
Test::Test(Test const &other) = default;
Test::Test(Test &&other) noexcept = default;

// NOLINTNEXTLINE
auto Test::operator=(Test const &other) -> Test &
{
  this->name_ = other.name_;
  this->group_ = other.group_;
  this->engine_ = other.engine_;

  return *this;
}

auto Test::operator=(Test &&other) noexcept -> Test &
{
  this->name_ = std::move(other.name_);
  this->group_ = std::move(other.group_);
  this->engine_ = other.engine_;

  return *this;
}

Test::Test(TestGroup group, String name, TestEngine &engine) :
  name_(std::move(name)),
  group_(std::move(group)),
  engine_(engine)
{
}

auto Test::operator==(Test const &other) const noexcept -> bool
{
  return this->name_ == other.name_ && this->group_ == other.group_;
}

auto Test::operator<(Test const &other) const noexcept -> bool
{
  return this->name_ < other.name_ && this->group_ < other.group_;
}

auto Test::run(Body fn) -> Test &
{
  engine_.register_test_body(fn);

  return *this;
}

} // namespace waypoint
