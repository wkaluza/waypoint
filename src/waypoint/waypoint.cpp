#include "waypoint.hpp"

#include "impls.hpp"

#include "autorun/autorun.hpp"
#include "process/process.hpp"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace waypoint::internal
{

auto get_impl(Engine const &engine) -> Engine_impl &
{
  return *engine.impl_;
}

} // namespace waypoint::internal

namespace
{

void populate_test_indices_(waypoint::Engine const &t)
{
  waypoint::internal::get_impl(t).set_shuffled_test_record_ptrs();
  auto const &shuffled_ptrs =
    waypoint::internal::get_impl(t).get_shuffled_test_record_ptrs();
  for(unsigned long long i = 0; i < shuffled_ptrs.size(); ++i)
  {
    waypoint::internal::get_impl(t).set_test_index(
      shuffled_ptrs[i]->test_id(),
      i);
  }
}

void run_test(
  waypoint::Engine const &t,
  unsigned long long const test_index,
  waypoint::internal::InputPipeEnd const &response_write_pipe) noexcept
{
  auto const &impl = waypoint::internal::get_impl(t);
  waypoint::internal::TestRecord *const record =
    impl.get_shuffled_test_record_ptrs().at(test_index);

  auto const ctx =
    impl.make_child_process_context(record->test_id(), response_write_pipe);

  record->test_assembly()(*ctx);
  record->mark_as_run();
}

void execute_command(
  waypoint::Engine const &t,
  waypoint::internal::Command const &command,
  waypoint::internal::InputPipeEnd const &response_write_pipe)
{
  if(command.code == waypoint::internal::Command::Code::RunTest)
  {
    run_test(t, command.test_index, response_write_pipe);
  }
}

void begin_handshake(waypoint::internal::InputPipeEnd const &pipe)
{
  constexpr auto cmd =
    std::to_underlying(waypoint::internal::Command::Code::Attention);

  pipe.write(&cmd, sizeof cmd);
}

void complete_handshake(waypoint::internal::InputPipeEnd const &pipe)
{
  constexpr auto ready =
    std::to_underlying(waypoint::internal::Response::Code::Ready);

  pipe.write(&ready, sizeof ready);
}

void await_handshake_start(waypoint::internal::OutputPipeEnd const &pipe)
{
  unsigned char data = 0;
  pipe.read(&data, sizeof data);
}

void await_handshake_end(waypoint::internal::OutputPipeEnd const &pipe)
{
  unsigned char data = 0;
  pipe.read(&data, sizeof data);
}

auto receive_command(waypoint::internal::OutputPipeEnd const &command_read_pipe)
  -> waypoint::internal::Command
{
  unsigned char code_ =
    std::to_underlying(waypoint::internal::Command::Code::Invalid);
  command_read_pipe.read(&code_, sizeof code_);

  auto const code = static_cast<waypoint::internal::Command::Code>(code_);

  if(code == waypoint::internal::Command::Code::RunTest)
  {
    unsigned long long test_index = 0;
    command_read_pipe.read(
      reinterpret_cast<unsigned char *>(&test_index),
      sizeof test_index);

    return {code, test_index};
  }

  return {code, {}};
}

auto receive_response(
  waypoint::internal::OutputPipeEnd const &response_read_pipe)
  -> waypoint::internal::Response
{
  unsigned char code_ =
    std::to_underlying(waypoint::internal::Response::Code::Invalid);
  response_read_pipe.read(&code_, sizeof code_);

  auto const code = static_cast<waypoint::internal::Response::Code>(code_);

  if(code == waypoint::internal::Response::Code::Assertion)
  {
    unsigned long long test_id = 0;
    response_read_pipe.read(
      reinterpret_cast<unsigned char *>(&test_id),
      sizeof test_id);

    unsigned char passed = 0;
    response_read_pipe.read(&passed, sizeof passed);

    unsigned long long assertion_index = 0;
    response_read_pipe.read(
      reinterpret_cast<unsigned char *>(&assertion_index),
      sizeof assertion_index);

    unsigned char has_message = 0;
    response_read_pipe.read(&has_message, sizeof has_message);
    if(has_message == 0)
    {
      return {code, test_id, passed == 1, assertion_index, std::nullopt};
    }

    unsigned long long message_size = 0;
    response_read_pipe.read(
      reinterpret_cast<unsigned char *>(&message_size),
      sizeof message_size);
    std::string message(message_size, 'X');
    response_read_pipe.read(
      reinterpret_cast<unsigned char *>(message.data()),
      message_size);

    return {code, test_id, passed == 1, assertion_index, {message}};
  }

  if(code == waypoint::internal::Response::Code::TestComplete)
  {
    unsigned long long test_id = 0;
    response_read_pipe.read(
      reinterpret_cast<unsigned char *>(&test_id),
      sizeof test_id);

    return {code, test_id, {}, {}, {}};
  }

  return {code, {}, {}, {}, {}};
}

void send_command(
  waypoint::internal::InputPipeEnd const &command_write_pipe,
  waypoint::internal::Command const &command)
{
  auto const code = std::to_underlying(command.code);
  command_write_pipe.write(&code, sizeof code);
  if(command.code == waypoint::internal::Command::Code::RunTest)
  {
    auto const test_index = command.test_index;
    command_write_pipe.write(
      reinterpret_cast<unsigned char const *>(&test_index),
      sizeof test_index);
  }
}

void send_response(
  waypoint::Engine const &t,
  waypoint::internal::InputPipeEnd const &response_write_pipe,
  unsigned long long const test_index,
  waypoint::internal::Response::Code const &code_)
{
  auto const code = std::to_underlying(code_);
  response_write_pipe.write(&code, sizeof code);
  if(code_ == waypoint::internal::Response::Code::TestComplete)
  {
    waypoint::internal::TestRecord const *const record =
      waypoint::internal::get_impl(t).get_shuffled_test_record_ptrs().at(
        test_index);
    auto const test_id = record->test_id();
    response_write_pipe.write(
      reinterpret_cast<unsigned char const *>(&test_id),
      sizeof test_id);
  }
}

auto is_end_command(waypoint::internal::Command const &command) -> bool
{
  return command.code == waypoint::internal::Command::Code::End;
}

void shut_down_sequence(
  waypoint::internal::InputPipeEnd const &command_write_pipe,
  waypoint::internal::OutputPipeEnd const &response_read_pipe) noexcept
{
  constexpr auto command =
    waypoint::internal::Command{waypoint::internal::Command::Code::End, {}};

  send_command(command_write_pipe, command);

  [[maybe_unused]]
  auto const response = receive_response(response_read_pipe);
}

auto parent_main(
  waypoint::Engine const &t,
  waypoint::internal::InputPipeEnd const &command_write_pipe,
  waypoint::internal::OutputPipeEnd const &response_read_pipe) noexcept
  -> waypoint::RunResult
{
  begin_handshake(command_write_pipe);
  await_handshake_end(response_read_pipe);

  auto const &records =
    waypoint::internal::get_impl(t).get_shuffled_test_record_ptrs();

  for(auto *const record : records)
  {
    if(record->disabled())
    {
      continue;
    }

    auto const test_index =
      waypoint::internal::get_impl(t).get_test_index(record->test_id());

    auto const command = waypoint::internal::Command{
      waypoint::internal::Command::Code::RunTest,
      test_index};
    send_command(command_write_pipe, command);
    record->mark_as_run();

    while(true)
    {
      auto const response = receive_response(response_read_pipe);
      if(response.code == waypoint::internal::Response::Code::Assertion)
      {
        waypoint::internal::get_impl(t).register_assertion(
          response.assertion_passed,
          response.test_id,
          response.assertion_index,
          response.assertion_message);
      }
      if(response.code == waypoint::internal::Response::Code::TestComplete)
      {
        break;
      }
    }
  }

  shut_down_sequence(command_write_pipe, response_read_pipe);

  return waypoint::internal::get_impl(t).generate_results();
}

void child_main(
  waypoint::Engine const &t,
  waypoint::internal::OutputPipeEnd const &command_read_pipe,
  waypoint::internal::InputPipeEnd const &response_write_pipe) noexcept
{
  await_handshake_start(command_read_pipe);
  complete_handshake(response_write_pipe);

  while(true)
  {
    auto const command = receive_command(command_read_pipe);

    execute_command(t, command, response_write_pipe);
    send_response(
      t,
      response_write_pipe,
      command.test_index,
      std::invoke(
        [&command]()
        {
          if(command.code == waypoint::internal::Command::Code::RunTest)
          {
            return waypoint::internal::Response::Code::TestComplete;
          }

          return waypoint::internal::Response::Code::ShuttingDown;
        }));

    if(is_end_command(command))
    {
      break;
    }
  }
}

void initialize(waypoint::Engine const &t) noexcept
{
  auto const section = waypoint::internal::get_autorun_section_boundaries();
  auto const begin = section.first;
  auto const end = section.second;

  using AutorunFunctionPtr = void (*)(waypoint::Engine const &);
  std::vector<AutorunFunctionPtr> functions;

  for(auto fn_ptr = begin; fn_ptr < end; fn_ptr += sizeof(AutorunFunctionPtr))
  {
    functions.push_back(*reinterpret_cast<AutorunFunctionPtr *>(fn_ptr));
  }

  auto is_not_null = [](auto *ptr)
  {
    return ptr != nullptr;
  };

  for(auto const fn_ptr : functions | std::views::filter(is_not_null))
  {
    fn_ptr(t);
  }

  populate_test_indices_(t);
}

} // namespace

namespace waypoint
{

auto make_default_engine() noexcept -> Engine
{
  auto *impl = new internal::Engine_impl{};

  return Engine{impl};
}

auto run_all_tests_in_process(Engine const &t) noexcept -> RunResult
{
  initialize(t);
  if(internal::get_impl(t).has_errors())
  {
    // Initialization had errors, skip running tests and emit results
    return internal::get_impl(t).generate_results();
  }

  std::ranges::for_each(
    internal::get_impl(t).get_shuffled_test_record_ptrs(),
    [&t](auto *const ptr) noexcept
    {
      if(!ptr->disabled())
      {
        auto const context =
          internal::get_impl(t).make_in_process_context(ptr->test_id());

        // Run test
        ptr->test_assembly()(*context);
        ptr->mark_as_run();
      }
    });

  return internal::get_impl(t).generate_results();
}

auto run_all_tests(Engine const &t) noexcept -> RunResult
{
  initialize(t);
  if(internal::get_impl(t).has_errors())
  {
    // Initialization had errors, skip running tests and emit results
    return internal::get_impl(t).generate_results();
  }

  if(waypoint::internal::is_child())
  {
    {
      auto const pipes = waypoint::internal::get_pipes_from_env();

      auto const &command_read_pipe = pipes.first;
      auto const &response_write_pipe = pipes.second;

      child_main(t, command_read_pipe, response_write_pipe);
    }

    std::exit(0);
  }

  waypoint::internal::ChildProcessRAII const child;

  return parent_main(t, child.command_write_pipe(), child.response_read_pipe());
}

AssertionOutcome::~AssertionOutcome() = default;

AssertionOutcome::AssertionOutcome(internal::AssertionOutcome_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

auto AssertionOutcome::group() const noexcept -> char const *
{
  return this->impl_->group_name().c_str();
}

auto AssertionOutcome::test() const noexcept -> char const *
{
  return this->impl_->test_name().c_str();
}

auto AssertionOutcome::message() const noexcept -> char const *
{
  if(this->impl_->message().has_value())
  {
    return this->impl_->message().value().c_str();
  }

  return nullptr;
}

auto AssertionOutcome::passed() const noexcept -> bool
{
  return this->impl_->passed();
}

auto AssertionOutcome::index() const noexcept -> unsigned long long
{
  return this->impl_->index();
}

TestOutcome::~TestOutcome() = default;

TestOutcome::TestOutcome(internal::TestOutcome_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

auto TestOutcome::group_name() const noexcept -> char const *
{
  return this->impl_->get_group_name().c_str();
}

auto TestOutcome::group_id() const noexcept -> unsigned long long
{
  return this->impl_->get_group_id();
}

auto TestOutcome::test_name() const noexcept -> char const *
{
  return this->impl_->get_test_name().c_str();
}

auto TestOutcome::test_id() const noexcept -> unsigned long long
{
  return this->impl_->get_test_id();
}

auto TestOutcome::test_index() const noexcept -> unsigned long long
{
  return this->impl_->get_index();
}

auto TestOutcome::assertion_count() const noexcept -> unsigned long long
{
  return this->impl_->get_assertion_count();
}

auto TestOutcome::assertion_outcome(
  unsigned long long const index) const noexcept -> AssertionOutcome const &
{
  return this->impl_->get_assertion_outcome(index);
}

auto TestOutcome::disabled() const noexcept -> bool
{
  return this->impl_->disabled();
}

auto TestOutcome::status() const noexcept -> TestOutcome::Status
{
  return this->impl_->status();
}

Group::~Group() = default;

Group::Group(internal::Group_impl *const impl)
  : impl_{impl}
{
}

Test3<void>::~Test3() = default;

Test3<void>::Test3(internal::Registrar<void> registrar)
  : registrar_{internal::move(registrar)}
{
}

void Test3<void>::disable() && noexcept
{
  this->registrar_.disable(true);
}

void Test3<void>::disable(bool const is_disabled) && noexcept
{
  this->registrar_.disable(is_disabled);
}

Test2<void>::~Test2() = default;

Test2<void>::Test2(internal::Registrar<void> registrar)
  : registrar_{internal::move(registrar)}
{
}

Test::~Test() = default;

Test::Test(internal::Test_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

auto Test::test_id() const -> unsigned long long
{
  return this->impl_->get_id();
}

auto Test::get_engine() const -> Engine const &
{
  return this->impl_->get_engine();
}

void Test::mark_complete() const
{
  this->impl_->mark_complete();
}

auto Engine::group(char const *name) const noexcept -> Group
{
  auto const group_id = this->impl_->register_group(name);

  return this->impl_->make_group(group_id);
}

auto Engine::test(Group const &group, char const *name) const noexcept -> Test
{
  auto const test_id =
    this->impl_->register_test(this->impl_->get_group_id(group), name);

  return this->impl_->make_test(test_id);
}

void Engine::register_test_assembly(
  internal::TestAssembly f,
  unsigned long long const test_id,
  bool const disabled) const
{
  this->impl_->register_test_assembly(std::move(f), test_id, disabled);
}

void Engine::report_incomplete_test(unsigned long long const test_id) const
{
  this->impl_->report_incomplete_test(test_id);
}

Engine::~Engine() = default;

Engine::Engine(internal::Engine_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
  impl->initialize(*this);
}

Context::~Context() = default;

Context::Context() = default;

ContextInProcess::~ContextInProcess() = default;

ContextInProcess::ContextInProcess(internal::ContextInProcess_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

void ContextInProcess::assert(bool const condition) const noexcept
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      std::nullopt);
}

void ContextInProcess::assert(bool const condition, char const *const message)
  const noexcept
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      message);
}

auto ContextInProcess::assume(bool const condition) const noexcept -> bool
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      std::nullopt);

  return condition;
}

auto ContextInProcess::assume(bool const condition, char const *const message)
  const noexcept -> bool
{
  internal::get_impl(impl_->get_engine())
    .register_assertion(
      condition,
      this->impl_->test_id(),
      this->impl_->generate_assertion_index(),
      message);

  return condition;
}

ContextChildProcess::~ContextChildProcess() = default;

ContextChildProcess::ContextChildProcess(
  internal::ContextChildProcess_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

void ContextChildProcess::assert(bool const condition) const noexcept
{
  auto const index = this->impl_->generate_assertion_index();

  internal::get_impl(impl_->get_engine())
    .register_assertion(condition, this->impl_->test_id(), index, std::nullopt);

  internal::get_impl(impl_->get_engine())
    .transmit_assertion(
      condition,
      this->impl_->test_id(),
      index,
      std::nullopt,
      *this->impl_->response_write_pipe());
}

void ContextChildProcess::assert(
  bool const condition,
  char const *const message) const noexcept
{
  auto const index = this->impl_->generate_assertion_index();

  internal::get_impl(impl_->get_engine())
    .register_assertion(condition, this->impl_->test_id(), index, message);

  internal::get_impl(impl_->get_engine())
    .transmit_assertion(
      condition,
      this->impl_->test_id(),
      index,
      message,
      *this->impl_->response_write_pipe());
}

auto ContextChildProcess::assume(bool const condition) const noexcept -> bool
{
  auto const index = this->impl_->generate_assertion_index();

  internal::get_impl(impl_->get_engine())
    .register_assertion(condition, this->impl_->test_id(), index, std::nullopt);

  internal::get_impl(impl_->get_engine())
    .transmit_assertion(
      condition,
      this->impl_->test_id(),
      index,
      std::nullopt,
      *this->impl_->response_write_pipe());

  return condition;
}

auto ContextChildProcess::assume(
  bool const condition,
  char const *const message) const noexcept -> bool
{
  auto const index = this->impl_->generate_assertion_index();

  internal::get_impl(impl_->get_engine())
    .register_assertion(condition, this->impl_->test_id(), index, message);

  internal::get_impl(impl_->get_engine())
    .transmit_assertion(
      condition,
      this->impl_->test_id(),
      index,
      message,
      *this->impl_->response_write_pipe());

  return condition;
}

RunResult::~RunResult() = default;

RunResult::RunResult(internal::RunResult_impl *const impl)
  : impl_{internal::UniquePtr{impl}}
{
}

auto RunResult::success() const noexcept -> bool
{
  return !this->impl_->has_errors() && !this->impl_->has_failing_assertions();
}

auto RunResult::test_count() const noexcept -> unsigned long long
{
  return this->impl_->test_outcome_count();
}

auto RunResult::test_outcome(unsigned long long const index) const noexcept
  -> TestOutcome const &
{
  return this->impl_->get_test_outcome(index);
}

auto RunResult::error_count() const noexcept -> unsigned long long
{
  return this->impl_->errors().size();
}

auto RunResult::error(unsigned long long const index) const noexcept
  -> char const *
{
  return this->impl_->errors().at(index).c_str();
}

} // namespace waypoint

WAYPOINT_AUTORUN(waypoint::Engine const &)
{
}
