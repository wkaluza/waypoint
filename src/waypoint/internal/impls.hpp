#pragma once

#include "types.hpp"
#include "waypoint.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
  AssertionOutcome_impl();

  void initialize(
    std::string group_name,
    std::string test_name,
    std::string message,
    bool passed,
    unsigned long long index);

  std::string group_name;
  std::string test_name;
  std::string message;
  bool passed;
  unsigned long long index;
};

class TestOutcome_impl
{
public:
  TestOutcome_impl();

  void initialize(
    TestId test_id,
    GroupId group_id,
    std::vector<std::unique_ptr<AssertionOutcome>> assertion_outcomes,
    std::string group_name,
    std::string test_name,
    unsigned long long index,
    bool disabled);

  [[nodiscard]]
  auto get_test_name() const -> std::string const &;
  [[nodiscard]]
  auto get_test_id() const -> unsigned long long;
  [[nodiscard]]
  auto get_group_name() const -> std::string const &;
  [[nodiscard]]
  auto get_group_id() const -> unsigned long long;
  [[nodiscard]]
  auto get_assertion_count() const -> unsigned long long;
  [[nodiscard]]
  auto get_assertion_outcome(unsigned long long index) const
    -> AssertionOutcome const &;
  [[nodiscard]]
  auto get_index() const -> unsigned long long;
  [[nodiscard]]
  auto disabled() const -> bool;

private:
  std::vector<std::unique_ptr<AssertionOutcome>> assertion_outcomes_;
  TestId test_id_;
  GroupId group_id_;
  std::string group_name_;
  std::string test_name_;
  unsigned long long test_index_;
  bool disabled_;
};

class TestRecord
{
public:
  TestRecord(TestAssembly assembly, TestId test_id, bool disabled);

  [[nodiscard]]
  auto test_id() const -> TestId;
  [[nodiscard]]
  auto test_assembly() const -> TestAssembly const &;
  [[nodiscard]]
  auto disabled() const -> bool;

private:
  TestAssembly test_assembly_;
  TestId test_id_;
  bool disabled_;
};

class AssertionRecord
{
public:
  AssertionRecord(
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
  ~Test_impl();
  Test_impl();
  Test_impl(Test_impl const &other) = delete;
  Test_impl(Test_impl &&other) noexcept = delete;
  auto operator=(Test_impl const &other) -> Test_impl & = delete;
  auto operator=(Test_impl &&other) noexcept -> Test_impl & = delete;

  void initialize(Engine const &engine, TestId id);

  [[nodiscard]]
  auto get_engine() const -> Engine const &;
  [[nodiscard]]
  auto get_id() const -> TestId;
  void mark_complete();

private:
  Engine const *engine_;
  TestId id_;
  bool incomplete_;
};

class Context_impl
{
public:
  Context_impl();

  void initialize(Engine const &engine, TestId test_id);

  [[nodiscard]]
  auto get_engine() const -> Engine const &;
  [[nodiscard]]
  auto generate_assertion_index() -> AssertionIndex;
  [[nodiscard]]
  auto test_id() const -> TestId;

private:
  Engine const *engine_;
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
    Init_DuplicateTestInGroup,
    Init_TestHasNoBody
  };

  struct Error
  {
    ErrorType type;
    std::string message;
  };

public:
  void initialize(Engine const &engine);
  void register_test_assembly(
    TestAssembly assembly,
    TestId test_id,
    bool disabled);
  [[nodiscard]]
  auto test_records() -> std::vector<TestRecord> &;
  [[nodiscard]]
  auto generate_results() const -> RunResult;
  void register_assertion(
    bool condition,
    TestId test_id,
    AssertionIndex index,
    std::optional<std::string> maybe_message);
  [[nodiscard]]
  auto errors() const noexcept -> std::vector<std::string>;
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
  auto make_test_outcome(TestId test_id) const noexcept
    -> std::unique_ptr<TestOutcome>;
  void report_error(ErrorType type, std::string const &message);
  void report_duplicate_test_name(
    GroupName const &group_name,
    TestName const &test_name);
  void report_incomplete_test(TestId test_id);
  [[nodiscard]]
  auto get_assertions() const -> std::vector<AssertionRecord>;
  [[nodiscard]]
  auto make_context(TestId test_id) const -> Context;
  void set_shuffled_test_record_ptrs();
  [[nodiscard]]
  auto get_shuffled_test_record_ptrs() const
    -> std::vector<TestRecord const *> const &;
  [[nodiscard]]
  auto is_disabled(TestId test_id) const -> bool;

private:
  Engine const *engine_;
  GroupId group_id_counter_;
  TestId test_id_counter_;
  std::unordered_map<GroupId, GroupName> group_id2name_map_;
  std::unordered_map<TestId, TestName> test_id2name_map_;
  std::unordered_map<TestId, GroupId> test_id2group_id_;
  std::unordered_map<TestId, unsigned long long> test_id2test_index_;
  std::unordered_map<GroupId, std::unordered_set<TestName>>
    test_names_per_group_;
  std::vector<Error> errors_;
  std::vector<AssertionRecord> assertions_;
  std::vector<TestRecord> test_records_;
  std::vector<TestRecord const *> shuffled_test_record_ptrs_;
};

class RunResult_impl
{
public:
  RunResult_impl();

  void initialize(Engine const &engine);

  [[nodiscard]]
  auto errors() const -> std::vector<std::string> const &;
  [[nodiscard]]
  auto has_errors() const -> bool;
  [[nodiscard]]
  auto has_failing_assertions() const -> bool;
  [[nodiscard]]
  auto test_outcome_count() const -> unsigned long long;
  [[nodiscard]]
  auto get_test_outcome(unsigned long long index) const -> TestOutcome const &;

private:
  bool has_failing_assertions_;
  std::vector<std::unique_ptr<TestOutcome>> test_outcomes_;
  std::vector<std::string> errors_;
};

extern char const *NO_ASSERTION_MESSAGE;

} // namespace waypoint::internal
