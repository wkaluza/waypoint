#pragma once

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

private:
  internal::Vector_impl<T> *impl_;
};

class Test;
extern template class Vector<Test>;
class TestGroup;
extern template class Vector<TestGroup>;

} // namespace waypoint
