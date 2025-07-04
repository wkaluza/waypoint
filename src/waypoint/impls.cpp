#include "impls.hpp"

#include "types.hpp"
#include "waypoint.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace waypoint::internal
{

AssertionOutcome_impl::AssertionOutcome_impl() :
  passed{},
  index{}
{
}

void AssertionOutcome_impl::initialize(
  std::string group_name,
  std::string test_name,
  std::string message,
  bool const passed,
  unsigned long long const index)
{
  this->group_name = std::move(group_name);
  this->test_name = std::move(test_name);
  this->message = std::move(message);
  this->passed = passed;
  this->index = index;
}

TestOutcome_impl::TestOutcome_impl() :
  test_id_{},
  group_id_{},
  test_index_{}
{
}

void TestOutcome_impl::initialize(
  TestId const test_id,
  GroupId const group_id,
  std::vector<std::unique_ptr<AssertionOutcome>> assertion_outcomes,
  std::string group_name,
  std::string test_name,
  unsigned long long const index)
{
  this->test_id_ = test_id;
  this->group_id_ = group_id;
  this->assertion_outcomes_ = std::move(assertion_outcomes);
  this->group_name_ = std::move(group_name);
  this->test_name_ = std::move(test_name);
  this->test_index_ = index;
}

auto TestOutcome_impl::get_test_name() const -> std::string const &
{
  return this->test_name_;
}

auto TestOutcome_impl::get_test_id() const -> unsigned long long
{
  return this->test_id_;
}

auto TestOutcome_impl::get_group_name() const -> std::string const &
{
  return this->group_name_;
}

auto TestOutcome_impl::get_group_id() const -> unsigned long long
{
  return this->group_id_;
}

auto TestOutcome_impl::get_assertion_count() const -> unsigned long long
{
  return this->assertion_outcomes_.size();
}

auto TestOutcome_impl::get_assertion_outcome(
  unsigned long long const index) const -> AssertionOutcome const &
{
  return *this->assertion_outcomes_[index];
}

auto TestOutcome_impl::get_index() const -> unsigned long long
{
  return this->test_index_;
}

TestRecord::TestRecord(TestBody body, TestId const test_id) :
  body_(std::move(body)),
  test_id_{test_id}
{
}

auto TestRecord::test_id() const -> TestId
{
  return this->test_id_;
}

auto TestRecord::body() const -> TestBody const &
{
  return this->body_;
}

AssertionRecord::AssertionRecord(
  bool const condition,
  TestId const test_id,
  AssertionIndex const index,
  std::optional<std::string> maybe_message) :
  condition_{condition},
  test_id_{test_id},
  index_{index},
  maybe_message_{std::move(maybe_message)}
{
}

auto AssertionRecord::passed() const -> bool
{
  return this->condition_;
}

auto AssertionRecord::test_id() const -> TestId
{
  return this->test_id_;
}

auto AssertionRecord::index() const -> AssertionIndex
{
  return this->index_;
}

auto AssertionRecord::message() const -> std::optional<std::string>
{
  return this->maybe_message_;
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

Test_impl::Test_impl() :
  engine_{},
  id_{}
{
}

void Test_impl::initialize(Engine &engine, TestId const id)
{
  this->engine_ = &engine;
  this->id_ = id;
}

auto Test_impl::get_engine() const -> Engine &
{
  return *engine_;
}

auto Test_impl::get_id() const -> TestId
{
  return this->id_;
}

Context_impl::Context_impl() :
  engine_{},
  test_id_{},
  assertion_index_{}
{
}

void Context_impl::initialize(Engine &engine, TestId const test_id)
{
  this->engine_ = &engine;
  this->test_id_ = test_id;
  this->assertion_index_ = 0;
}

auto Context_impl::get_engine() const -> Engine &
{
  return *engine_;
}

auto Context_impl::generate_assertion_index() -> AssertionIndex
{
  return assertion_index_++;
}

auto Context_impl::test_id() const -> TestId
{
  return this->test_id_;
}

Engine_impl::Engine_impl() :
  engine_{nullptr},
  group_id_counter_{0},
  test_id_counter_{0}
{
}

auto Engine_impl::make_group(GroupId const group_id) const -> Group
{
  auto *impl = new Group_impl{};

  impl->set_id(group_id);

  return Group{impl};
}

auto Engine_impl::make_test(TestId const test_id) const -> Test
{
  auto *impl = new Test_impl{};

  impl->initialize(*this->engine_, test_id);

  return Test{impl};
}

auto Engine_impl::get_group_id(TestId const id) const -> GroupId
{
  return this->test_id2group_id_.at(id);
}

auto Engine_impl::get_group_id(Group const &group) const -> GroupId
{
  return group.impl_->get_id();
}

auto Engine_impl::get_group_name(GroupId const id) const -> std::string
{
  return this->group_id2name_map_.at(id);
}

auto Engine_impl::get_test_name(TestId const id) const -> std::string
{
  return this->test_id2name_map_.at(id);
}

void Engine_impl::set_test_index(
  TestId const test_id,
  unsigned long long const index)
{
  this->test_id2test_index_[test_id] = index;
}

auto Engine_impl::get_test_index(TestId const test_id) const
  -> unsigned long long
{
  return this->test_id2test_index_.at(test_id);
}

auto Engine_impl::test_count() const -> unsigned long long
{
  return test_id_counter_;
}

auto Engine_impl::make_test_outcome(TestId const test_id) const
  -> std::unique_ptr<TestOutcome>
{
  std::vector<std::unique_ptr<AssertionOutcome>> assertion_outcomes;

  auto assertions = this->get_assertions() |
    std::ranges::views::filter(
                      [test_id](auto const &assertion)
                      {
                        return assertion.test_id() == test_id;
                      });

  for(auto const &assertion : assertions)
  {
    auto maybe_message = assertion.message();
    auto *assertion_impl = new AssertionOutcome_impl{};

    assertion_impl->initialize(
      this->get_group_name(this->get_group_id(test_id)),
      this->get_test_name(test_id),
      maybe_message.has_value() ? maybe_message.value() : "[EMPTY]",
      assertion.passed(),
      assertion.index());

    assertion_outcomes.emplace_back(
      std::unique_ptr<AssertionOutcome>(new AssertionOutcome{assertion_impl}));
  }

  auto *impl = new TestOutcome_impl{};

  impl->initialize(
    test_id,
    this->get_group_id(test_id),
    std::move(assertion_outcomes),
    this->get_group_name(this->get_group_id(test_id)),
    this->get_test_name(test_id),
    this->get_test_index(test_id));

  return std::unique_ptr<TestOutcome>(new TestOutcome{impl});
}

auto Engine_impl::register_test(
  GroupId const group_id,
  TestName const &test_name) -> TestId
{
  auto &test_names = test_names_per_group_[group_id];
  if(test_names.contains(test_name))
  {
    auto const &group_name = this->group_id2name_map_[group_id];
    this->report_duplicate_test_name(group_name, test_name);
  }

  test_names.insert(test_name);

  auto const test_id = test_id_counter_++;

  test_id2group_id_[test_id] = group_id;

  this->test_id2name_map_[test_id] = test_name;

  return test_id;
}

auto Engine_impl::register_group(GroupName const &group_name) -> GroupId
{
  auto const group_id = group_id_counter_++;
  group_id2name_map_[group_id] = group_name;

  return group_id;
}

void Engine_impl::report_error(ErrorType type, std::string const &message)
{
  this->errors_.emplace_back(type, message);
}

void Engine_impl::report_duplicate_test_name(
  GroupName const &group_name,
  TestName const &test_name)
{
  this->report_error(
    ErrorType::Init_DuplicateTestInGroup,
    std::format(
      R"(Group "{}" contains duplicate test "{}")",
      group_name,
      test_name));
}

auto Engine_impl::get_assertions() const -> std::vector<AssertionRecord>
{
  return this->assertions_;
}

auto Engine_impl::make_context(TestId const test_id) const -> Context
{
  auto *impl = new Context_impl{};

  impl->initialize(*this->engine_, test_id);

  return Context{impl};
}

namespace
{

auto get_random_number_generator() -> std::mt19937_64
{
  constexpr std::size_t arbitrary_constant = 0x1234;
  constexpr std::size_t arbitrary_seed = 0x0123'4567'89ab'cdef;

  using knuth_lcg = std::linear_congruential_engine<
    std::uint64_t,
    6'364'136'223'846'793'005U,
    1'442'695'040'888'963'407U,
    0U>;
  knuth_lcg seed_rng(arbitrary_seed);
  seed_rng.discard(arbitrary_constant);

  std::vector<std::uint64_t> seeds(624);
  std::ranges::generate(seeds, seed_rng);
  std::seed_seq seq(seeds.begin(), seeds.end());
  std::mt19937_64 rng(seq);
  rng.discard(arbitrary_constant);

  return rng;
}

auto get_body_ptrs(Engine const &t) -> std::vector<TestRecord const *>
{
  auto const &bodies = get_impl(t).test_bodies();
  std::vector<TestRecord const *> body_ptrs(bodies.size());

  std::ranges::transform(
    bodies,
    body_ptrs.begin(),
    [](auto &body)
    {
      return &body;
    });

  std::ranges::sort(
    body_ptrs,
    [](auto *a, auto *b)
    {
      return a->test_id() < b->test_id();
    });

  return body_ptrs;
}

auto get_shuffled_body_ptrs_(Engine const &t) -> std::vector<TestRecord const *>
{
  auto body_ptrs = get_body_ptrs(t);

  auto rng = get_random_number_generator();

  std::ranges::shuffle(body_ptrs, rng);

  return body_ptrs;
}

} // namespace

void Engine_impl::set_shuffled_body_ptrs()
{
  this->shuffled_body_ptrs_ = get_shuffled_body_ptrs_(*this->engine_);
}

auto Engine_impl::get_shuffled_body_ptrs() const
  -> std::vector<TestRecord const *> const &
{
  return this->shuffled_body_ptrs_;
}

auto Engine_impl::has_errors() const -> bool
{
  return !this->errors_.empty();
}

void Engine_impl::initialize(Engine &engine)
{
  this->engine_ = &engine;
}

void Engine_impl::register_test_body(TestBody &&body, TestId const test_id)
{
  this->bodies_.emplace_back(std::move(body), test_id);
}

auto Engine_impl::test_bodies() -> std::vector<TestRecord> &
{
  return this->bodies_;
}

auto Engine_impl::generate_results() const -> RunResult
{
  auto *impl = new RunResult_impl{};

  impl->initialize(*this->engine_);

  return RunResult{impl};
}

void Engine_impl::register_assertion(
  bool condition,
  TestId const test_id,
  AssertionIndex const index,
  std::optional<std::string> maybe_message)
{
  this->assertions_
    .emplace_back(condition, test_id, index, std::move(maybe_message));
}

RunResult_impl::RunResult_impl() :
  has_failing_assertions_{false},
  has_errors_{false}
{
}

void RunResult_impl::initialize(Engine const &engine)
{
  this->has_failing_assertions_ = std::invoke(
    [&engine]()
    {
      auto const &assertions = get_impl(engine).get_assertions();

      auto const it = std::ranges::find_if(
        assertions,
        [](auto const &assertion)
        {
          return !assertion.passed();
        });

      return it != assertions.end();
    });

  this->has_errors_ = std::invoke(
    [&engine]()
    {
      return get_impl(engine).has_errors();
    });

  this->test_outcomes_ = std::invoke(
    [&engine]()
    {
      unsigned long long const n = get_impl(engine).test_count();

      std::vector<std::unique_ptr<TestOutcome>> output;
      output.reserve(n);

      for(unsigned long long id = 0; id < n; ++id)
      {
        output.emplace_back(get_impl(engine).make_test_outcome(id));
      }

      std::ranges::sort(
        output,
        [](auto const &a, auto const &b)
        {
          return a->test_id() < b->test_id();
        });

      return output;
    });
}

auto RunResult_impl::has_errors() const -> bool
{
  return this->has_errors_;
}

auto RunResult_impl::has_failing_assertions() const -> bool
{
  return this->has_failing_assertions_;
}

auto RunResult_impl::test_outcome_count() const -> unsigned long long
{
  return this->test_outcomes_.size();
}

auto RunResult_impl::get_test_outcome(unsigned long long const index) const
  -> TestOutcome const &
{
  return *this->test_outcomes_[index];
}

} // namespace waypoint::internal
