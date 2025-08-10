#include "waypoint.hpp"

namespace waypoint::internal
{

Registrar<void>::~Registrar()
{
  if(!this->is_active_)
  {
    return;
  }

  if(!static_cast<bool>(this->body_))
  {
    this->report_incomplete_test(this->test_id_);

    return;
  }

  this->engine_.register_test_assembly(
    [setup = move(this->setup_),
     body = move(this->body_),
     teardown = move(this->teardown_)](Context const &ctx) noexcept
    {
      if(static_cast<bool>(setup))
      {
        setup(ctx);
      }
      body(ctx);
      if(static_cast<bool>(teardown))
      {
        teardown(ctx);
      }
    },
    this->test_id_,
    this->timeout_ms_,
    this->is_disabled_);
}

Registrar<void>::Registrar(Registrar &&other) noexcept
  : is_active_{other.is_active_},
    engine_{other.engine_},
    test_id_{other.test_id_},
    timeout_ms_{other.timeout_ms_},
    setup_{move(other.setup_)},
    body_{move(other.body_)},
    teardown_{move(other.teardown_)},
    is_disabled_{false}
{
  other.is_active_ = false;
}

void Registrar<void>::register_setup(VoidSetup f)
{
  this->is_active_ = true;

  this->setup_ = move(f);
}

void Registrar<void>::register_body(TestBodyNoFixture f)
{
  this->is_active_ = true;

  this->body_ = move(f);
}

void Registrar<void>::register_teardown(TeardownNoFixture f)
{
  this->is_active_ = true;

  this->teardown_ = move(f);
}

void Registrar<void>::set_timeout_ms(unsigned long long const timeout_ms)
{
  this->timeout_ms_ = timeout_ms;
}

void Registrar<void>::disable(bool const is_disabled)
{
  this->is_disabled_ = is_disabled;
}

Registrar<void>::Registrar(
  Engine const &engine,
  unsigned long long const test_id)
  : is_active_{false},
    engine_{engine},
    test_id_{test_id},
    timeout_ms_{DEFAULT_TIMEOUT_MS},
    is_disabled_{false}
{
}

void Registrar<void>::report_incomplete_test(
  unsigned long long const test_id) const
{
  this->engine_.report_incomplete_test(test_id);
}

} // namespace waypoint::internal
