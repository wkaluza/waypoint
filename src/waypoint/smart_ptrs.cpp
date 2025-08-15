#include "impls.hpp"
#include "waypoint.hpp"

namespace waypoint::internal
{

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
template class UniquePtr<ContextInProcess_impl>;
template class UniquePtr<ContextChildProcess_impl>;
template class UniquePtr<TestRun_impl>;
template class UniquePtr<Group_impl>;
template class UniquePtr<Test_impl>;
template class UniquePtr<TestOutcome_impl>;

} // namespace waypoint::internal
