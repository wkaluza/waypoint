#include "impls.hpp"

#include "get_impl.hpp"
#include "ids.hpp"
#include "waypoint.hpp"

#include <algorithm>
#include <format>
#include <vector>

namespace waypoint
{

TestBodyRecord::TestBodyRecord(BodyFnPtr const body, TestId const test_id) :
  body_(body),
  test_id_{test_id}
{
}

auto TestBodyRecord::test_id() const -> TestId
{
  return this->test_id_;
}

auto TestBodyRecord::body() const -> BodyFnPtr
{
  return this->body_;
}

AssertionRecord::AssertionRecord(bool const condition, TestId const test_id) :
  condition_{condition},
  test_id_{test_id}
{
}

auto AssertionRecord::get_condition() const -> bool
{
  return this->condition_;
}

auto AssertionRecord::test_id() const -> TestId
{
  return this->test_id_;
}

Group_impl::Group_impl() :
  id_{}
{
}

void Group_impl::set_id(GroupId const id)
{
  this->id_ = id;
}

auto Group_impl::get_id() const -> GroupId
{
  return this->id_;
}

Test_impl::Test_impl(Engine &engine) :
  engine_{engine},
  id_{}
{
}

auto Test_impl::get_engine() const -> Engine &
{
  return engine_;
}

void Test_impl::set_id(TestId const test_id)
{
  this->id_ = test_id;
}

auto Test_impl::get_id() const -> TestId
{
  return this->id_;
}

Context_impl::Context_impl(Engine &engine, TestId const test_id) :
  engine_{engine},
  test_id_{test_id}
{
}

auto Context_impl::get_engine() const -> Engine &
{
  return engine_;
}

auto Context_impl::test_id() const -> TestId
{
  return this->test_id_;
}

void Context_impl::set_test_id(TestId test_id)
{
  this->test_id_ = test_id;
}

Engine_impl::Engine_impl(Engine &engine) :
  engine_{engine}
{
}

auto Engine_impl::get_group(GroupId const group_id) const -> Group
{
  Group group{this->engine_};

  get_impl(group).set_id(group_id);

  return group;
}

auto Engine_impl::get_test(TestId const test_id) const -> Test
{
  Test test{this->engine_};

  get_impl(test).set_id(test_id);

  return test;
}

auto Engine_impl::register_test(
  GroupId const group_id,
  TestName const &test_name) -> TestId
{
  if(test_id_maps_.size() <= group_id)
  {
    test_id_maps_.resize(group_id + 1);
  }

  auto &test_id_map = test_id_maps_[group_id];
  if(test_id_map.contains(test_name))
  {
    auto const &group_name = this->group_id2name_map_[group_id];
    this->report_duplicate_test(group_name, test_name);
  }
  else
  {
    test_id_map[test_name] = test_id_counter_++;
  }

  auto const test_id = test_id_map[test_name];

  this->test_id2name_map_[test_id] = test_name;

  return test_id;
}

auto Engine_impl::register_group(GroupName const &group_name) -> GroupId
{
  if(!group_name2id_map_.contains(group_name))
  {
    group_name2id_map_[group_name] = group_id_counter_++;
  }

  auto const group_id = group_name2id_map_[group_name];
  group_id2name_map_[group_id] = group_name;

  return group_id;
}

void Engine_impl::report_duplicate_test(
  GroupName const &group_name,
  TestName const &test_name)
{
  this->errors_.push_back(
    std::format(
      R"(Group "{}" contains duplicate test named "{}")",
      group_name,
      test_name));
}

auto Engine_impl::get_assertions() const -> std::vector<AssertionRecord>
{
  return this->assertions_;
}

auto Engine_impl::make_context(TestId const test_id) const -> Context
{
  auto *impl = new Context_impl{this->engine_, test_id};

  return Context{impl};
}

auto Engine_impl::verify() const -> bool
{
  return this->errors_.empty();
}

void Engine_impl::register_test_body(BodyFnPtr body, TestId const test_id)
{
  this->bodies_.emplace_back(body, test_id);
}

auto Engine_impl::test_bodies() -> std::vector<TestBodyRecord>
{
  return this->bodies_;
}

auto Engine_impl::generate_results() const -> Result
{
  auto *impl = new Result_impl{};

  impl->initialize(this->engine_);

  return Result{impl};
}

void Engine_impl::register_assertion(bool condition, TestId const test_id)
{
  this->assertions_.emplace_back(condition, test_id);
}

Result_impl::Result_impl() :
  has_failing_assertions_{false}
{
}

void Result_impl::initialize(Engine &engine)
{
  auto const &assertions = get_impl(engine).get_assertions();

  auto const it = std::ranges::find_if(
    assertions,
    [](auto const &assertion)
    {
      return !assertion.get_condition();
    });

  this->has_failing_assertions_ = it != assertions.end();
}

auto Result_impl::has_failing_assertions() const -> bool
{
  return this->has_failing_assertions_;
}

} // namespace waypoint
