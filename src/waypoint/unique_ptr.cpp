#include "impls.hpp"
#include "waypoint.hpp"

#include <utility>

namespace waypoint::internal
{

template<typename T>
UniquePtr<T>::~UniquePtr()
{
  delete ptr_;
}

template<typename T>
UniquePtr<T>::UniquePtr(T *ptr) :
  ptr_{ptr}
{
}

template<typename T>
UniquePtr<T>::UniquePtr(UniquePtr &&other) noexcept :
  ptr_{std::exchange(other.ptr_, nullptr)}
{
}

template<typename T>
auto UniquePtr<T>::operator=(UniquePtr &&other) noexcept -> UniquePtr &
{
  if(this == &other)
  {
    return *this;
  }

  delete ptr_;

  ptr_ = std::exchange(other.ptr_, nullptr);

  return *this;
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
auto UniquePtr<T>::operator*() -> T &
{
  return *ptr_;
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

} // namespace waypoint::internal
