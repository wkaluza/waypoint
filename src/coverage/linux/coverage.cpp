// Copyright (c) 2025 Wojciech Kałuża
// SPDX-License-Identifier: MIT
// For license details, see LICENSE file

#include "coverage.hpp"

#ifdef WAYPOINT_INTERNAL_COVERAGE_IOm5lSCCB6p0j19
extern "C" void __gcov_dump();
#endif

namespace waypoint::coverage
{

void gcov_dump()
{
#ifdef WAYPOINT_INTERNAL_COVERAGE_IOm5lSCCB6p0j19
  ::__gcov_dump();
#endif
} // GCOV_COVERAGE_58QuSuUgMN8onvKx_EXCL_LINE

} // namespace waypoint::coverage
