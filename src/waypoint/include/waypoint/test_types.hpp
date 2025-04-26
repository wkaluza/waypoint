#pragma once

#include "string.hpp"
#include "types.hpp"

namespace waypoint
{

class TestGroup
{
public:
  ~TestGroup();
  TestGroup(TestGroup const &other);
  TestGroup(TestGroup &&other) noexcept;
  auto operator=(TestGroup const &other) -> TestGroup &;
  auto operator=(TestGroup &&other) noexcept -> TestGroup &;

  explicit TestGroup(String name);

  auto operator==(TestGroup const &other) const noexcept -> bool;
  auto operator<(TestGroup const &other) const noexcept -> bool;

private:
  String name_;
};

class TestEngine;

class Test
{
public:
  ~Test();
  Test(Test const &other);
  Test(Test &&other) noexcept;
  auto operator=(Test const &other) -> Test &;
  auto operator=(Test &&other) noexcept -> Test &;

  Test(TestGroup group, String name, TestEngine &engine);

  auto operator==(Test const &other) const noexcept -> bool;
  auto operator<(Test const &other) const noexcept -> bool;

  auto run(Body fn) -> Test &;

private:
  String name_;
  TestGroup group_;
  TestEngine &engine_;
};

} // namespace waypoint
