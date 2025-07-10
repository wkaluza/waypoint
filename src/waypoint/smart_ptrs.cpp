#include "impls.hpp"
#include "waypoint.hpp"

namespace waypoint::internal
{

template<typename T>
UniquePtrMoveable<T>::~UniquePtrMoveable()
{
  delete ptr_;
}

template<typename T>
UniquePtrMoveable<T>::UniquePtrMoveable(UniquePtrMoveable &&other) noexcept
  : ptr_{other.ptr_}
{
  other.ptr_ = nullptr;
}

template<typename T>
auto UniquePtrMoveable<T>::operator=(UniquePtrMoveable &&other) noexcept
  -> UniquePtrMoveable &
{
  if(this == &other)
  {
    return *this;
  }

  delete this->ptr_;

  this->ptr_ = other.ptr_;
  other.ptr_ = nullptr;

  return *this;
}

template<typename T>
UniquePtrMoveable<T>::UniquePtrMoveable(T *ptr)
  : ptr_{ptr}
{
}

template<typename T>
UniquePtrMoveable<T>::operator bool() const
{
  return ptr_ != nullptr;
}

template<typename T>
auto UniquePtrMoveable<T>::operator->() const -> T *
{
  return ptr_;
}

template<typename T>
auto UniquePtrMoveable<T>::operator*() const -> T &
{
  return *ptr_;
}

template<typename T>
UniquePtr<T>::~UniquePtr()
{
  delete ptr_;
}

template<typename T>
UniquePtr<T>::UniquePtr(T *ptr)
  : ptr_{ptr}
{
}

template<typename T>
UniquePtr<T>::operator bool() const
{
  return ptr_ != nullptr;
}

template<typename T>
auto UniquePtr<T>::operator->() const -> T *
{
  return ptr_;
}

template<typename T>
auto UniquePtr<T>::operator*() const -> T &
{
  return *ptr_;
}

template class UniquePtr<AssertionOutcome_impl>;
template class UniquePtr<Context_impl>;
template class UniquePtr<Engine_impl>;
template class UniquePtr<Group_impl>;
template class UniquePtr<RunResult_impl>;
template class UniquePtr<Test_impl>;
template class UniquePtr<TestOutcome_impl>;

template class UniquePtrMoveable<Registrar_impl>;

} // namespace waypoint::internal
