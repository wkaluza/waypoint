#include "waypoint.hpp"

#include <algorithm>
#include <cstdint>
#include <format>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

extern char __start_waypoint_tests;
extern char __stop_waypoint_tests;

namespace
{

struct AutorunSectionBoundaries
{
  std::uintptr_t begin;
  std::uintptr_t end;
};

auto get_autorun_section_boundaries() -> AutorunSectionBoundaries
{
  auto const begin = reinterpret_cast<std::uintptr_t>(&__start_waypoint_tests);
  auto const end = reinterpret_cast<std::uintptr_t>(&__stop_waypoint_tests);

  return {begin, end};
}

} // namespace

namespace waypoint
{

using Id = unsigned long long;
using GroupId = Id;
using TestId = Id;

auto get_impl(Engine const &engine) -> Engine_impl &
{
  return *engine.impl_;
}

auto get_impl(Group const &group) -> Group_impl &
{
  return *group.impl_;
}

auto get_impl(Test const &test) -> Test_impl &
{
  return *test.impl_;
}

auto get_impl(Context const &context) -> Context_impl &
{
  return *context.impl_;
}

auto get_impl(Result const &result) -> Result_impl &
{
  return *result.impl_;
}

class Group_impl
{
public:
  Group_impl();

  void set_id(GroupId id);
  [[nodiscard]]
  auto get_id() const -> GroupId;

private:
  GroupId id_;
};

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

class Test_impl
{
public:
  explicit Test_impl(Engine &engine);

  [[nodiscard]]
  auto get_engine() const -> Engine &;
  void set_id(TestId test_id);

private:
  Engine &engine_;
  TestId id_;
};

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

class Assertion
{
public:
  explicit Assertion(bool condition);

  [[nodiscard]]
  auto get_condition() const -> bool;

private:
  bool condition_;
};

Assertion::Assertion(bool condition) :
  condition_{condition}
{
}

auto Assertion::get_condition() const -> bool
{
  return this->condition_;
}

class Engine_impl
{
public:
  explicit Engine_impl(Engine &engine);

private:
  using GroupName = std::string;
  using TestName = std::string;

public:
  void register_test_body(Body const &body);
  [[nodiscard]]
  auto test_bodies() -> std::vector<Body>;
  [[nodiscard]]
  auto generate_results() const -> Result;
  void register_assertion(bool condition);
  [[nodiscard]]
  auto verify() const -> bool;
  auto register_group(GroupName const &group_name) -> GroupId;
  auto register_test(GroupId group_id, TestName const &test_name) -> TestId;
  auto get_group(GroupId group_id) const -> Group;
  auto get_test(TestId test_id) const -> Test;
  void report_duplicate_test(
    GroupName const &group_name,
    TestName const &test_name);
  [[nodiscard]]
  auto get_assertions() const -> std::vector<Assertion>;
  [[nodiscard]]
  auto make_context() const -> Context;

private:
  Engine &engine_;
  GroupId group_id_counter_{0};
  TestId test_id_counter_{0};
  std::unordered_map<GroupName, GroupId> group_name2id_map_;
  std::unordered_map<GroupId, GroupName> group_id2name_map_;
  std::unordered_map<TestId, TestName> test_id2name_map_;
  std::vector<std::unordered_map<TestName, TestId>> test_id_maps_;
  std::vector<std::string> errors_;
  std::vector<Assertion> assertions_;
  std::vector<Body> bodies_;
};

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

auto Engine_impl::get_assertions() const -> std::vector<Assertion>
{
  return this->assertions_;
}

auto Engine_impl::make_context() const -> Context
{
  return Context{this->engine_};
}

auto Engine_impl::verify() const -> bool
{
  return this->errors_.empty();
}

void Engine_impl::register_test_body(Body const &body)
{
  this->bodies_.push_back(body);
}

auto Engine_impl::test_bodies() -> std::vector<Body>
{
  return this->bodies_;
}

auto Engine_impl::generate_results() const -> Result
{
  return Result{this->engine_};
}

void Engine_impl::register_assertion(bool condition)
{
  this->assertions_.emplace_back(condition);
}

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

auto Test::run(Body const &body) -> Test &
{
  get_impl(this->impl_->get_engine()).register_test_body(body);

  return *this;
}

class Context_impl
{
public:
  explicit Context_impl(Engine &engine);

  [[nodiscard]]
  auto get_engine() const -> Engine &;

private:
  Engine &engine_;
};

Context_impl::Context_impl(Engine &engine) :
  engine_{engine}
{
}

auto Context_impl::get_engine() const -> Engine &
{
  return engine_;
}

auto initialize(Engine &t) -> bool
{
  auto const section = get_autorun_section_boundaries();
  auto const begin = section.begin;
  auto const end = section.end;

  for(auto fn_ptr = begin; fn_ptr < end; fn_ptr += sizeof(void (*)(Engine &)))
  {
    if(auto const autorun_fn = *reinterpret_cast<void (**)(Engine &)>(fn_ptr);
       autorun_fn != nullptr)
    {
      autorun_fn(t);
    }
  }

  bool const success = get_impl(t).verify();

  return success;
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

[[nodiscard]]
auto run_all_tests(Engine &t) -> Result
{
  for(Body const &body : get_impl(t).test_bodies())
  {
    auto ctx = get_impl(t).make_context();
    body(ctx);
  }

  return get_impl(t).generate_results();
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
  get_impl(impl_->get_engine()).register_assertion(condition);
}

Context::~Context()
{
  delete impl_;
}

Context::Context(Engine &engine) :
  impl_{new Context_impl{engine}}
{
}

class Result_impl
{
public:
  explicit Result_impl(Engine const &engine);

  [[nodiscard]]
  auto has_failing_assertions() const -> bool;

private:
  bool has_failing_assertions_;
};

Result_impl::Result_impl(Engine const &engine)
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

Result::~Result()
{
  delete impl_;
}

Result::Result(Engine &engine) :
  impl_{new Result_impl{engine}}
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
