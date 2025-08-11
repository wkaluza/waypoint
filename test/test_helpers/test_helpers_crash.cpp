#include "test_helpers_crash.hpp"

#include "coverage/coverage.hpp"

#include <cassert>
#include <cstdlib>
#include <stdexcept>

namespace waypoint::test
{

void call_std_exit_123()
{
  std::exit(123);
}

void call_std_abort()
{
  waypoint::coverage::gcov_dump();
  std::abort();
}

void failing_assertion()
{
  waypoint::coverage::gcov_dump();
  // NOLINTNEXTLINE(misc-static-assert)
  assert(2 + 2 != 4);
#ifdef NDEBUG
  std::abort();
#endif
}

void throws_exception()
{
  waypoint::coverage::gcov_dump();
  throw std::runtime_error("boom");
}

void throws_exception_while_noexcept() noexcept
{
  throws_exception();
}

} // namespace waypoint::test
