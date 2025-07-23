#include "test_helpers.hpp"

#include "waypoint/waypoint.hpp"

namespace waypoint::test
{

int x = x_init;

void trivial_test_body(waypoint::Context const &ctx)
{
  ctx.assert(true);
}

} // namespace waypoint::test
