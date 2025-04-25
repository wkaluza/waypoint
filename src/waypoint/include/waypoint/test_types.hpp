#pragma once

#include "string.hpp"

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

private:
  String name_;
};

class Test
{
public:
  ~Test();
  Test(Test const &other);
  Test(Test &&other) noexcept;
  auto operator=(Test const &other) -> Test &;
  auto operator=(Test &&other) noexcept -> Test &;

  Test(TestGroup group, String name);

private:
  TestGroup group_;
  String name_;
};

} // namespace waypoint
