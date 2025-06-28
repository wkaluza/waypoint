#pragma once

#include "types.hpp"
#include "waypoint.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace waypoint
{

class Engine;

} // namespace waypoint

namespace waypoint::internal
{

class AssertionOutcome_impl
{
public:
  std::string const group_name;
  std::string const test_name;
  std::string const message;
  bool const passed;
  unsigned long long const index;
};

class TestOutcome_impl
{
public:
  explicit TestOutcome_impl(
    TestId id,
    std::vector<AssertionOutcome> assertion_outcomes,
    std::string group_name,
    std::string test_name,
    unsigned long long index);

  [[nodiscard]]
  auto get_id() const -> unsigned long long;
  [[nodiscard]]
  auto get_assertion_count() const -> unsigned long long;
  [[nodiscard]]
  auto get_assertion_outcome(unsigned long long index) const
    -> AssertionOutcome const &;
  [[nodiscard]]
  auto get_group_name() const -> std::string const &;
  [[nodiscard]]
  auto get_test_name() const -> std::string const &;
  [[nodiscard]]
  auto get_index() const -> unsigned long long;

private:
  std::vector<AssertionOutcome> assertion_outcomes_;
  TestId test_id_;
  std::string group_name_;
  std::string test_name_;
  unsigned long long test_index_;
};

class TestRecord
{
public:
  TestRecord(BodyFnPtr body, TestId test_id);

  [[nodiscard]]
  auto test_id() const -> TestId;
  [[nodiscard]]
  auto body() const -> BodyFnPtr;

  [[nodiscard]]
  auto operator<(TestRecord const &other) const -> bool;

private:
  BodyFnPtr body_;
  TestId test_id_;
};

class AssertionRecord
{
public:
  explicit AssertionRecord(
    bool condition,
    TestId test_id,
    AssertionIndex index,
    std::optional<std::string> maybe_message);

  [[nodiscard]]
  auto passed() const -> bool;
  [[nodiscard]]
  auto test_id() const -> TestId;
  [[nodiscard]]
  auto index() const -> AssertionIndex;
  [[nodiscard]]
  auto message() const -> std::optional<std::string>;

private:
  bool condition_;
  TestId test_id_;
  AssertionIndex index_;
  std::optional<std::string> maybe_message_;
};

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

class Test_impl
{
public:
  explicit Test_impl(Engine &engine);

  [[nodiscard]]
  auto get_engine() const -> Engine &;
  void set_id(TestId test_id);
  [[nodiscard]]
  auto get_id() const -> TestId;

private:
  Engine &engine_;
  TestId id_;
};

class Context_impl
{
public:
  explicit Context_impl(Engine &engine, TestId test_id);

  [[nodiscard]]
  auto get_engine() const -> Engine &;
  [[nodiscard]]
  auto generate_assertion_index() -> AssertionIndex;

  [[nodiscard]]
  auto test_id() const -> TestId;

private:
  Engine &engine_;
  TestId test_id_;
  AssertionIndex assertion_index_;
};

class Engine_impl
{
public:
  Engine_impl();

private:
  using GroupName = std::string;
  using TestName = std::string;

  enum class ErrorType : std::uint8_t
  {
    Init_DuplicateTestInGroup
  };

  struct Error
  {
    ErrorType type;
    std::string message;
  };

public:
  void initialize(Engine &engine);
  void register_test_body(BodyFnPtr body, TestId test_id);
  [[nodiscard]]
  auto test_bodies() -> std::vector<TestRecord> &;
  [[nodiscard]]
  auto generate_results() const -> RunResult;
  void register_assertion(
    bool condition,
    TestId test_id,
    AssertionIndex index,
    std::optional<std::string> maybe_message);
  [[nodiscard]]
  auto has_errors() const -> bool;
  [[nodiscard]]
  auto register_group(GroupName const &group_name) -> GroupId;
  [[nodiscard]]
  auto register_test(GroupId group_id, TestName const &test_name) -> TestId;
  [[nodiscard]]
  auto make_group(GroupId group_id) const -> Group;
  [[nodiscard]]
  auto make_test(TestId test_id) const -> Test;
  [[nodiscard]]
  auto get_group_id(TestId id) const -> GroupId;
  [[nodiscard]]
  auto get_group_id(Group const &group) const -> GroupId;
  [[nodiscard]]
  auto get_group_name(GroupId id) const -> std::string;
  [[nodiscard]]
  auto get_test_name(TestId id) const -> std::string;
  void set_test_index(TestId test_id, unsigned long long index);
  [[nodiscard]]
  auto get_test_index(TestId test_id) const -> unsigned long long;
  [[nodiscard]]
  auto test_count() const -> unsigned long long;
  [[nodiscard]]
  auto make_test_outcome(TestId test_id) const -> TestOutcome;
  void report_error(ErrorType type, std::string const &message);
  void report_duplicate_test(
    GroupName const &group_name,
    TestName const &test_name);
  [[nodiscard]]
  auto get_assertions() const -> std::vector<AssertionRecord>;
  [[nodiscard]]
  auto make_context(TestId test_id) const -> Context;
  void set_shuffled_body_ptrs();
  [[nodiscard]]
  auto get_shuffled_body_ptrs() const
    -> std::vector<TestRecord const *> const &;

private:
  Engine *engine_;
  GroupId group_id_counter_;
  TestId test_id_counter_;
  std::unordered_map<GroupName, GroupId> group_name2id_map_;
  std::unordered_map<GroupId, GroupName> group_id2name_map_;
  std::unordered_map<TestId, TestName> test_id2name_map_;
  std::unordered_map<TestId, GroupId> test_id2group_id_;
  std::unordered_map<TestId, unsigned long long> test_id2index_;
  std::vector<std::unordered_map<TestName, TestId>> test_id_maps_;
  std::vector<Error> errors_;
  std::vector<AssertionRecord> assertions_;
  std::vector<TestRecord> bodies_;
  std::vector<TestRecord const *> shuffled_body_ptrs_;
};

class RunResult_impl
{
public:
  RunResult_impl();

  [[nodiscard]]
  auto has_errors() const -> bool;
  [[nodiscard]]
  auto has_failing_assertions() const -> bool;

  void initialize(Engine const &engine);

  [[nodiscard]]
  auto test_outcome_count() const -> unsigned long long;
  [[nodiscard]]
  auto get_test_outcome(unsigned long long index) const -> TestOutcome const &;

private:
  bool has_failing_assertions_;
  bool has_errors_;
  std::vector<TestOutcome> test_outcomes_;
};

} // namespace waypoint::internal
