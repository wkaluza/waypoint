#include "waypoint.hpp"

namespace waypoint::internal
{

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

} // namespace waypoint::internal
