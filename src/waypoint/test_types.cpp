#include "test_types.hpp"

#include "string.hpp"

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

Test::~Test() = default;
Test::Test(Test const &other) = default;
Test::Test(Test &&other) noexcept = default;
auto Test::operator=(Test const &other) -> Test & = default;
auto Test::operator=(Test &&other) noexcept -> Test & = default;

Test::Test(TestGroup group, String name) :
  group_(std::move(group)),
  name_(std::move(name))
{
}

} // namespace waypoint
