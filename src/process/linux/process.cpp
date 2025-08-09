#include "process.hpp"

#include "coverage/coverage.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// NOLINTNEXTLINE(modernize-deprecated-headers)
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>

namespace
{

char const *const WAYPOINT_INTERNAL_RUNNER_ENV_NAME =
  "WAYPOINT_INTERNAL_RUNNER_u7fw593A";
char const *const WAYPOINT_INTERNAL_RUNNER_ENV_VALUE =
  "4w7SLEq0b0nUd1wXA6qu8AHW6ShUPrun";
char const *const WAYPOINT_INTERNAL_COMMAND_SOURCE_ENV_NAME =
  "WAYPOINT_INTERNAL_COMMAND_SOURCE_g0j3YuHH";
char const *const WAYPOINT_INTERNAL_RESPONSE_SINK_ENV_NAME =
  "WAYPOINT_INTERNAL_RESPONSE_SINK_suwAYZVy";

auto get_env(std::string const &var_name) -> std::optional<std::string>
{
  auto const *const var_value = ::getenv(var_name.c_str());
  if(var_value == nullptr)
  {
    return std::nullopt;
  }

  return {var_value};
}

void unset_env(std::string const &var_name)
{
  ::unsetenv(var_name.c_str());
}

} // namespace

namespace waypoint::internal
{

class InputPipeEnd_impl
{
public:
  ~InputPipeEnd_impl()
  {
    ::close(this->pipe_);
  }

  explicit InputPipeEnd_impl(int const pipe)
    : pipe_{pipe}
  {
  }

  InputPipeEnd_impl() = delete;
  InputPipeEnd_impl(InputPipeEnd_impl const &other) = delete;
  InputPipeEnd_impl(InputPipeEnd_impl &&other) noexcept = delete;
  auto operator=(InputPipeEnd_impl const &other)
    -> InputPipeEnd_impl & = delete;
  auto operator=(InputPipeEnd_impl &&other) noexcept
    -> InputPipeEnd_impl & = delete;

  [[nodiscard]]
  auto raw_pipe() const -> int
  {
    return this->pipe_;
  }

private:
  int pipe_;
};

InputPipeEnd::~InputPipeEnd() = default;

InputPipeEnd::InputPipeEnd(InputPipeEnd_impl *const impl)
  : impl_{std::unique_ptr<InputPipeEnd_impl>{impl}}
{
}

InputPipeEnd::InputPipeEnd(InputPipeEnd &&other) noexcept = default;

void InputPipeEnd::write(
  unsigned char const *const buffer,
  unsigned long long const count) const
{
  unsigned left_to_transfer = count;
  unsigned transferred = 0;

  while(left_to_transfer > 0)
  {
    auto const transferred_this_time =
      ::write(this->impl_->raw_pipe(), buffer + transferred, left_to_transfer);

    transferred += transferred_this_time;
    left_to_transfer -= transferred_this_time;
  }
}

class OutputPipeEnd_impl
{
public:
  ~OutputPipeEnd_impl()
  {
    ::close(this->pipe_);
  }

  explicit OutputPipeEnd_impl(int const pipe)
    : pipe_{pipe}
  {
  }

  OutputPipeEnd_impl() = delete;
  OutputPipeEnd_impl(OutputPipeEnd_impl const &other) = delete;
  OutputPipeEnd_impl(OutputPipeEnd_impl &&other) noexcept = delete;
  auto operator=(OutputPipeEnd_impl const &other)
    -> OutputPipeEnd_impl & = delete;
  auto operator=(OutputPipeEnd_impl &&other) noexcept
    -> OutputPipeEnd_impl & = delete;

  [[nodiscard]]
  auto raw_pipe() const -> int
  {
    return this->pipe_;
  }

private:
  int pipe_;
};

OutputPipeEnd::~OutputPipeEnd() = default;

OutputPipeEnd::OutputPipeEnd(OutputPipeEnd_impl *const impl)
  : impl_{std::unique_ptr<OutputPipeEnd_impl>{impl}}
{
}

auto OutputPipeEnd::read(
  unsigned char *const buffer,
  unsigned long long const count) const -> OutputPipeEnd::ReadResult
{
  unsigned left_to_transfer = count;
  unsigned transferred = 0;

  while(left_to_transfer > 0)
  {
    auto const transferred_this_time =
      ::read(this->impl_->raw_pipe(), buffer + transferred, left_to_transfer);

    if(transferred_this_time == 0)
    {
      // The other end of the pipe is closed - peer crashed or exited
      return OutputPipeEnd::ReadResult::PipeClosed;
    }

    transferred += transferred_this_time;
    left_to_transfer -= transferred_this_time;
  }

  return OutputPipeEnd::ReadResult::Success;
}

OutputPipeEnd::OutputPipeEnd(OutputPipeEnd &&other) noexcept = default;

auto get_pipes_from_env() noexcept -> std::pair<OutputPipeEnd, InputPipeEnd>
{
  auto const maybe_command_read_pipe =
    get_env(WAYPOINT_INTERNAL_COMMAND_SOURCE_ENV_NAME);
  auto const maybe_response_write_pipe =
    get_env(WAYPOINT_INTERNAL_RESPONSE_SINK_ENV_NAME);

  unset_env(WAYPOINT_INTERNAL_COMMAND_SOURCE_ENV_NAME);
  unset_env(WAYPOINT_INTERNAL_RESPONSE_SINK_ENV_NAME);

  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  auto const raw_command_read_pipe = std::stoi(maybe_command_read_pipe.value());
  auto const raw_response_write_pipe =
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    std::stoi(maybe_response_write_pipe.value());

  auto *command_read_pipe = new OutputPipeEnd_impl{raw_command_read_pipe};
  auto *response_write_pipe = new InputPipeEnd_impl{raw_response_write_pipe};

  return {OutputPipeEnd{command_read_pipe}, InputPipeEnd{response_write_pipe}};
}

auto is_child() -> bool
{
  auto const maybe_value = get_env(WAYPOINT_INTERNAL_RUNNER_ENV_NAME);
  if(!maybe_value.has_value())
  {
    return false;
  }

  unset_env(WAYPOINT_INTERNAL_RUNNER_ENV_NAME);

  return maybe_value.value() == WAYPOINT_INTERNAL_RUNNER_ENV_VALUE;
}

Response::Response(
  Code const code_,
  unsigned long long const test_id_,
  bool const assertion_passed_,
  unsigned long long const assertion_index_,
  std::optional<std::string> assertion_message_)
  : code{code_},
    test_id{test_id_},
    assertion_passed{assertion_passed_},
    assertion_index{assertion_index_},
    assertion_message{std::move(assertion_message_)}
{
}

} // namespace waypoint::internal

namespace
{

auto resolve_path(std::string const &input) noexcept -> std::string
{
  std::vector<char> dest;
  constexpr unsigned long long bufsize = 4'096;
  dest.resize(bufsize);
  std::ranges::fill(dest, 0);

  [[maybe_unused]]
  char const *const ret = ::realpath(input.c_str(), dest.data());

  return {dest.data()};
}

auto get_path_to_current_executable() noexcept -> std::string
{
  std::vector<char> dest;
  constexpr unsigned long long bufsize = 4'096;

  dest.resize(bufsize);
  std::ranges::fill(dest, 0);

  [[maybe_unused]]
  auto const ret = ::readlink("/proc/self/exe", dest.data(), bufsize);

  std::string const path{dest.data()};

  return resolve_path(path);
}

auto create_child_process(
  std::array<int, 2> const &pipe_command,
  std::array<int, 2> const &pipe_response) noexcept -> std::tuple<int, int, int>
{
  auto const fork_ret = ::fork();
  if(fork_ret > 0)
  {
    auto const child_pid = fork_ret;

    ::close(pipe_command[0]);
    ::close(pipe_response[1]);

    return {child_pid, pipe_command[1], pipe_response[0]};
  }

  ::close(pipe_command[1]);
  ::close(pipe_response[0]);

  auto const path_to_exe = get_path_to_current_executable();

  std::array<char const *, 2> const execve_argv = {
    path_to_exe.c_str(),
    nullptr};

  auto const runner_mode_env = std::format(
    "{}={}",
    WAYPOINT_INTERNAL_RUNNER_ENV_NAME,
    WAYPOINT_INTERNAL_RUNNER_ENV_VALUE);
  auto const command_source_env = std::format(
    "{}={}",
    WAYPOINT_INTERNAL_COMMAND_SOURCE_ENV_NAME,
    pipe_command[0]);
  auto const response_sink_env = std::format(
    "{}={}",
    WAYPOINT_INTERNAL_RESPONSE_SINK_ENV_NAME,
    pipe_response[1]);

  std::vector<char const *> execve_envp = {
    runner_mode_env.c_str(),
    command_source_env.c_str(),
    response_sink_env.c_str()};

  // ::environ is declared in <unistd.h>
  for(auto const *e = ::environ; *e != nullptr; ++e)
  {
    execve_envp.push_back(*e);
  }

  execve_envp.push_back(nullptr);

  waypoint::coverage::gcov_dump();

  // GCOV_COVERAGE_58QuSuUgMN8onvKx_EXCL_START
  ::execve(
    path_to_exe.c_str(),
    const_cast<char *const *>(execve_argv.data()),
    const_cast<char *const *>(execve_envp.data()));
  // GCOV_COVERAGE_58QuSuUgMN8onvKx_EXCL_STOP

  std::unreachable();
}

auto create_child_process_with_pipes() noexcept -> std::tuple<int, int, int>
{
  std::array<int, 2> pipe_command{};
  std::array<int, 2> pipe_response{};

  [[maybe_unused]]
  auto const ret1 = ::pipe(pipe_command.data());
  [[maybe_unused]]
  auto const ret2 = ::pipe(pipe_response.data());

  return create_child_process(pipe_command, pipe_response);
}

void wait_for_child_process_end(int const child_pid)
{
  int status = 0;
  [[maybe_unused]]
  auto const ret = ::waitpid(child_pid, &status, 0);
}

} // namespace

namespace waypoint::internal
{

class ChildProcessRAII_impl
{
public:
  ~ChildProcessRAII_impl()
  {
    wait_for_child_process_end(this->child_pid());
  }

  ChildProcessRAII_impl()
  {
    auto const [child_pid, raw_command_write_pipe, raw_response_read_pipe] =
      create_child_process_with_pipes();

    this->child_pid_ = child_pid;
    this->command_write_pipe_ = std::make_unique<InputPipeEnd>(
      new InputPipeEnd_impl{raw_command_write_pipe});
    this->response_read_pipe_ = std::make_unique<OutputPipeEnd>(
      new OutputPipeEnd_impl{raw_response_read_pipe});
  }

  ChildProcessRAII_impl(ChildProcessRAII_impl const &other) = delete;
  ChildProcessRAII_impl(ChildProcessRAII_impl &&other) noexcept = delete;
  auto operator=(ChildProcessRAII_impl const &other)
    -> ChildProcessRAII_impl & = delete;
  auto operator=(ChildProcessRAII_impl &&other) noexcept
    -> ChildProcessRAII_impl & = delete;

  [[nodiscard]]
  auto child_pid() const -> int
  {
    return this->child_pid_;
  }

  [[nodiscard]]
  auto command_write_pipe() const -> InputPipeEnd const &
  {
    return *this->command_write_pipe_;
  }

  [[nodiscard]]
  auto response_read_pipe() const -> OutputPipeEnd const &
  {
    return *this->response_read_pipe_;
  }

private:
  int child_pid_;
  std::unique_ptr<InputPipeEnd> command_write_pipe_;
  std::unique_ptr<OutputPipeEnd> response_read_pipe_;
};

ChildProcessRAII::ChildProcessRAII()
  : impl_{std::make_unique<ChildProcessRAII_impl>()}
{
}

ChildProcessRAII::~ChildProcessRAII() = default;

auto ChildProcessRAII::command_write_pipe() const -> InputPipeEnd const &
{
  return this->impl_->command_write_pipe();
}

auto ChildProcessRAII::response_read_pipe() const -> OutputPipeEnd const &
{
  return this->impl_->response_read_pipe();
}

} // namespace waypoint::internal
