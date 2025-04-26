#include "vector.hpp"

#include "test_types.hpp"
#include "types.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace waypoint::internal
{

template<typename T>
class Vector_impl
{
public:
  Vector_impl();

  [[nodiscard]]
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

template<typename T>
auto Vector<T>::contains_duplicates() const -> bool
{
  auto vec = this->impl_->get_vector();

  std::sort(vec.begin(), vec.end());
  auto it = std::adjacent_find(vec.begin(), vec.end());

  return it != vec.end();
}

template<typename T>
Vector<T>::Iterator::Iterator(T *const ptr) :
  ptr_{ptr}
{
}

template<typename T>
auto Vector<T>::begin() const -> Iterator
{
  return Iterator{this->impl_->get_vector().data()};
}

template<typename T>
auto Vector<T>::end() const -> Iterator
{
  return Iterator{
    this->impl_->get_vector().data() + this->impl_->get_vector().size()};
}

template<typename T>
auto Vector<T>::Iterator::operator*() const -> T &
{
  return *ptr_;
}

template<typename T>
auto Vector<T>::Iterator::operator->() -> T *
{
  return ptr_;
}

template<typename T>
auto Vector<T>::Iterator::operator++() -> Iterator &
{
  ++ptr_;
  return *this;
}

template<typename T>
auto Vector<T>::Iterator::operator++(int) -> Iterator
{
  Iterator temp = *this;
  ++(*this);
  return temp;
}

template<typename T>
auto Vector<T>::Iterator::operator==(Iterator const &other) const -> bool
{
  return ptr_ == other.ptr_;
}

template<typename T>
auto Vector<T>::Iterator::operator!=(Iterator const &other) const -> bool
{
  return !(*this == other);
}

template class Vector<Test>;
template class Vector<TestGroup>;
template class Vector<Body>;

} // namespace waypoint
