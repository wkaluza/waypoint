#include "waypoint.hpp"

#include "string.hpp"
#include "test_types.hpp"
#include "types.hpp"
#include "vector.hpp"

#include <cstdint>
#include <utility>

extern char __start_waypoint_tests;
extern char __stop_waypoint_tests;

namespace
{

struct AutorunSectionBoundaries
{
  std::uintptr_t begin;
  std::uintptr_t end;
};

auto get_autorun_section_boundaries() -> AutorunSectionBoundaries
{
  auto const begin = reinterpret_cast<std::uintptr_t>(&__start_waypoint_tests);
  auto const end = reinterpret_cast<std::uintptr_t>(&__stop_waypoint_tests);

  return {begin, end};
}

} // namespace

namespace waypoint
{

auto initialize(TestEngine &t) -> bool
{
  auto const section = get_autorun_section_boundaries();
  auto const begin = section.begin;
  auto const end = section.end;

  for(auto fn_ptr = begin; fn_ptr < end; fn_ptr += sizeof(internal::AutorunFn))
  {
    if(auto const autorun_fn = *reinterpret_cast<internal::AutorunFn *>(fn_ptr);
       autorun_fn != nullptr)
    {
      autorun_fn(t);
    }
  }

  bool const success = t.verify();

  return success;
}

auto TestEngine::group(String name) -> TestGroup
{
  groups_.push_back(TestGroup{std::move(name)});

  return groups_.back();
}

auto TestEngine::test(TestGroup const &group, String name) -> Test
{
  Test test{group, std::move(name), *this};
  tests_.push_back(test);

  return test;
}

[[nodiscard]]
auto run_all_tests(TestEngine &t) -> TestResult
{
  TestResult result{};

  for(Body body : t.test_bodies())
  {
    TestContext ctx;
    body(ctx);
    result.register_test_outcome(ctx);
  }

  return result;
}

auto TestEngine::verify() const -> bool
{
  return !this->tests_.contains_duplicates();
}

void TestContext::assert(bool const condition)
{
  if(!condition)
  {
    this->register_assertion_failure();
  }
}

auto TestEngine::test_bodies() const -> Vector<Body>
{
  return this->bodies_;
}

void TestEngine::register_test_body(Body const body)
{
  this->bodies_.push_back(body);
}

TestContext::TestContext() :
  has_failure_{false}
{
}

void TestContext::register_assertion_failure()
{
  this->has_failure_ = true;
}

auto TestContext::has_failure() const -> bool
{
  return this->has_failure_;
}

TestResult::TestResult() :
  has_failure_{false}
{
}

auto TestResult::pass() const -> bool
{
  return !this->has_failure();
}

void TestResult::register_test_outcome(TestContext const &ctx)
{
  this->set_failure(ctx.has_failure());
}

void TestResult::set_failure(bool const fail)
{
  this->has_failure_ = fail;
}

auto TestResult::has_failure() const -> bool
{
  return this->has_failure_;
}

} // namespace waypoint

namespace
{

// NOLINTNEXTLINE param may be const
WAYPOINT_TESTS(t)
{
  (void)t;
}

} // namespace
