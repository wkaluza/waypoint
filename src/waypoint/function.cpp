#include "waypoint.hpp"

namespace waypoint::internal
{

Function<void(Context const &)>::~Function() = default;

Function<void(Context const &)>::Function()
  : callable_{static_cast<callable_interface *>(nullptr)}
{
}

Function<void(Context const &)>::Function(Function &&other) noexcept = default;
auto Function<void(Context const &)>::operator=(Function &&other) noexcept
  -> Function & = default;

Function<void(Context const &)>::operator bool() const
{
  return static_cast<bool>(this->callable_);
}

void Function<void(Context const &)>::operator()(Context const &ctx) const
{
  this->callable_->invoke(ctx);
}

Function<void(Context const &)>::callable_interface::~callable_interface() =
  default;
Function<void(Context const &)>::callable_interface::callable_interface() =
  default;

} // namespace waypoint::internal
