// Copyright (c) 2025 Wojciech Kałuża
// SPDX-License-Identifier: MIT
// For license details, see LICENSE file

#pragma once

#include <source_location>
#include <string_view>

namespace waypoint::internal
{

void assert(
  bool condition,
  std::string_view message,
  std::source_location loc = std::source_location::current());

} // namespace waypoint::internal
