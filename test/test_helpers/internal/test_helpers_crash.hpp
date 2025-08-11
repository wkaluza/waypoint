#pragma once

namespace waypoint::test
{

[[noreturn]]
void call_std_exit_123();
[[noreturn]]
void call_std_abort();
[[noreturn]]
void failing_assertion();
void throws_exception();
void throws_exception_while_noexcept() noexcept;

} // namespace waypoint::test
