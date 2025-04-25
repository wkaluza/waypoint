#include "vector.hpp"

#include "test_types.hpp"

#include <utility>
#include <vector>

namespace waypoint::internal
{

template<typename T>
class Vector_impl
{
public:
  Vector_impl();

  auto get_vector() -> std::vector<T> &;

private:
  std::vector<T> vec_;
};

template<typename T>
Vector_impl<T>::Vector_impl() = default;

template<typename T>
auto Vector_impl<T>::get_vector() -> std::vector<T> &
{
  return this->vec_;
}

} // namespace waypoint::internal

namespace waypoint
{

template<typename T>
Vector<T>::~Vector()
{
  delete this->impl_;
}

template<typename T>
Vector<T>::Vector() :
  impl_{new internal::Vector_impl<T>{}}
{
}

template<typename T>
Vector<T>::Vector(Vector<T> const &other) :
  impl_{new internal::Vector_impl<T>{*other.impl_}}
{
}

template<typename T>
Vector<T>::Vector(Vector<T> &&other) noexcept :
  impl_{std::exchange(other.impl_, nullptr)}
{
}

template<typename T>
auto Vector<T>::operator=(Vector const &other) -> Vector &
{
  if(&other == this)
  {
    return *this;
  }

  auto temp = other;

  this->impl_ = std::exchange(temp.impl_, nullptr);

  return *this;
}

template<typename T>
auto Vector<T>::operator=(Vector &&other) noexcept -> Vector &
{
  if(&other == this)
  {
    return *this;
  }

  this->impl_ = std::exchange(other.impl_, nullptr);

  return *this;
}

template<typename T>
void Vector<T>::push_back(T const &elem)
{
  this->impl_->get_vector().push_back(elem);
}

template<typename T>
auto Vector<T>::back() -> T &
{
  return this->impl_->get_vector().back();
}

template class Vector<Test>;
template class Vector<TestGroup>;

} // namespace waypoint
