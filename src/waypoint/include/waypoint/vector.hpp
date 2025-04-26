#pragma once

#include "types.hpp"

namespace waypoint
{
namespace internal
{

template<typename T>
class Vector_impl;

} // namespace internal

template<typename T>
class Vector
{
public:
  ~Vector();
  Vector();
  Vector(Vector const &other);
  Vector(Vector &&other) noexcept;
  auto operator=(Vector const &other) -> Vector &;
  auto operator=(Vector &&other) noexcept -> Vector &;

  void push_back(T const &elem);
  auto back() -> T &;
  [[nodiscard]]
  auto contains_duplicates() const -> bool;

  class Iterator
  {
  public:
    explicit Iterator(T *ptr);

    auto operator*() const -> T &;
    auto operator->() -> T *;
    auto operator++() -> Iterator &;
    auto operator++(int) -> Iterator;
    auto operator==(Iterator const &other) const -> bool;
    auto operator!=(Iterator const &other) const -> bool;

  private:
    T *ptr_;
  };

  [[nodiscard]]
  auto begin() const -> Iterator;
  [[nodiscard]]
  auto end() const -> Iterator;

private:
  internal::Vector_impl<T> *impl_;
};

class Test;
extern template class Vector<Test>;
class TestGroup;
extern template class Vector<TestGroup>;
extern template class Vector<Body>;

} // namespace waypoint
