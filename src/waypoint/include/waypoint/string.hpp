#pragma once

namespace waypoint::internal
{

class String_impl;

} // namespace waypoint::internal

namespace waypoint
{

class String
{
public:
  ~String();
  String();
  String(String const &other);
  String(String &&other) noexcept;
  auto operator=(String const &other) -> String &;
  auto operator=(String &&other) noexcept -> String &;

  // NOLINTNEXTLINE non-explicit unary ctor
  String(char const *str);

  auto operator==(String const &other) const noexcept -> bool;
  auto operator<(String const &other) const noexcept -> bool;

private:
  internal::String_impl *impl_;
};

} // namespace waypoint
