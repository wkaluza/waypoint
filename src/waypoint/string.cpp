#include "string.hpp"

#include <string>
#include <utility>

namespace waypoint::internal
{

class String_impl
{
public:
  ~String_impl();
  String_impl();
  String_impl(String_impl const &other);
  String_impl(String_impl &&other) noexcept;
  auto operator=(String_impl const &other) -> String_impl &;
  auto operator=(String_impl &&other) noexcept -> String_impl &;

  explicit String_impl(char const *str);

  [[nodiscard]]
  auto get_string() const -> std::string const &;

private:
  std::string str_;
};

String_impl::~String_impl() = default;

String_impl::String_impl() = default;
String_impl::String_impl(String_impl const &other) = default;
String_impl::String_impl(String_impl &&other) noexcept = default;
auto String_impl::operator=(String_impl const &other)
  -> String_impl & = default;
auto String_impl::operator=(String_impl &&other) noexcept
  -> String_impl & = default;

String_impl::String_impl(char const *str) :
  str_{str}
{
}

auto String_impl::get_string() const -> std::string const &
{
  return str_;
}

} // namespace waypoint::internal

namespace waypoint
{

String::~String()
{
  delete this->impl_;
}

String::String() :
  impl_{new internal::String_impl{}}
{
}

String::String(String const &other) :
  impl_{new internal::String_impl{*other.impl_}}
{
}

String::String(String &&other) noexcept :
  impl_{std::exchange(other.impl_, nullptr)}
{
}

auto String::operator=(String const &other) -> String &
{
  if(&other == this)
  {
    return *this;
  }

  auto temp = other;

  this->impl_ = std::exchange(temp.impl_, nullptr);

  return *this;
}

auto String::operator=(String &&other) noexcept -> String &
{
  if(&other == this)
  {
    return *this;
  }

  this->impl_ = std::exchange(other.impl_, nullptr);

  return *this;
}

String::String(char const *str) :
  impl_{new internal::String_impl{str}}
{
}

auto String::operator==(String const &other) const noexcept -> bool
{
  return this->impl_->get_string() == other.impl_->get_string();
}

auto String::operator<(String const &other) const noexcept -> bool
{
  return this->impl_->get_string() < other.impl_->get_string();
}

} // namespace waypoint
