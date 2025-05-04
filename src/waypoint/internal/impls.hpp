#pragma once

#include "ids.hpp"
#include "waypoint.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace waypoint
{

class TestBodyRecord
{
public:
  TestBodyRecord(BodyFnPtr body, TestId test_id);

  [[nodiscard]]
  auto test_id() const -> TestId;
  [[nodiscard]]
  auto body() const -> BodyFnPtr;

private:
  BodyFnPtr body_;
  TestId test_id_;
};

class AssertionRecord
{
public:
  AssertionRecord(bool condition, TestId test_id);

  [[nodiscard]]
  auto get_condition() const -> bool;
  [[nodiscard]]
  auto test_id() const -> TestId;

private:
  bool condition_;
  TestId test_id_;
};

class Engine;

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
  auto test_id() const -> TestId;
  void set_test_id(TestId test_id);

private:
  Engine &engine_;
  TestId test_id_;
};

class Engine_impl
{
public:
  explicit Engine_impl(Engine &engine);

private:
  using GroupName = std::string;
  using TestName = std::string;

public:
  void register_test_body(BodyFnPtr body, TestId test_id);
  [[nodiscard]]
  auto test_bodies() -> std::vector<TestBodyRecord>;
  [[nodiscard]]
  auto generate_results() const -> Result;
  void register_assertion(bool condition, TestId test_id);
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
  auto get_assertions() const -> std::vector<AssertionRecord>;
  [[nodiscard]]
  auto make_context(TestId test_id) const -> Context;

private:
  Engine &engine_;
  GroupId group_id_counter_{0};
  TestId test_id_counter_{0};
  std::unordered_map<GroupName, GroupId> group_name2id_map_;
  std::unordered_map<GroupId, GroupName> group_id2name_map_;
  std::unordered_map<TestId, TestName> test_id2name_map_;
  std::vector<std::unordered_map<TestName, TestId>> test_id_maps_;
  std::vector<std::string> errors_;
  std::vector<AssertionRecord> assertions_;
  std::vector<TestBodyRecord> bodies_;
};

class Result_impl
{
public:
  explicit Result_impl(Engine const &engine);

  [[nodiscard]]
  auto has_failing_assertions() const -> bool;

private:
  bool has_failing_assertions_;
};

} // namespace waypoint
