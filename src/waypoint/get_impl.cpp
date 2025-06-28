#include "waypoint.hpp"

namespace waypoint::internal
{

auto get_impl(Engine const &engine) -> Engine_impl &
{
  return *engine.impl_;
}

} // namespace waypoint::internal
