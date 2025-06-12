#include "impls.hpp"
#include "waypoint.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

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

auto make_default_engine() -> Engine
{
  auto *impl = new internal::Engine_impl{};

  return Engine{impl};
}

auto initialize(Engine &t) -> bool
{
  auto const section = get_autorun_section_boundaries();
  auto const begin = section.begin;
  auto const end = section.end;

  for(auto fn_ptr = begin; fn_ptr < end; fn_ptr += sizeof(void (*)(Engine &)))
  {
    if(auto const autorun_fn = *reinterpret_cast<void (**)(Engine &)>(fn_ptr);
       autorun_fn != nullptr)
    {
      autorun_fn(t);
    }
  }

  bool const success = internal::get_impl(t).verify();

  return success;
}

namespace
{

auto get_random_number_generator() -> std::mt19937_64
{
  constexpr std::size_t arbitrary_constant = 0x1234;
  constexpr std::size_t arbitrary_seed = 0x0123'4567'89ab'cdef;

  using knuth_lcg = std::linear_congruential_engine<
    std::uint64_t,
    6'364'136'223'846'793'005U,
    1'442'695'040'888'963'407U,
    0U>;
  knuth_lcg seed_rng(arbitrary_seed);
  seed_rng.discard(arbitrary_constant);

  std::vector<std::uint64_t> seeds(624);
  std::ranges::generate(seeds, seed_rng);
  std::seed_seq seq(seeds.begin(), seeds.end());
  std::mt19937_64 rng(seq);
  rng.discard(arbitrary_constant);

  return rng;
}

auto get_body_ptrs(Engine &t) -> std::vector<internal::TestBodyRecord const *>
{
  auto const &bodies = internal::get_impl(t).test_bodies();
  std::vector<internal::TestBodyRecord const *> body_ptrs(bodies.size());

  std::ranges::transform(
    bodies,
    body_ptrs.begin(),
    [](auto const &body)
    {
      return &body;
    });

  std::ranges::sort(
    body_ptrs,
    [](auto *a, auto *b)
    {
      return *a < *b;
    });

  return body_ptrs;
}

auto get_shuffled_body_ptrs(Engine &t)
  -> std::vector<internal::TestBodyRecord const *>
{
  auto body_ptrs = get_body_ptrs(t);

  auto rng = get_random_number_generator();

  std::ranges::shuffle(body_ptrs, rng);

  return body_ptrs;
}

} // namespace

auto run_all_tests(Engine &t) -> Result
{
  auto const body_ptrs = get_shuffled_body_ptrs(t);
  std::ranges::for_each(
    body_ptrs,
    [&t](auto const *ptr)
    {
      auto context = internal::get_impl(t).make_context(ptr->test_id());
      // Run test
      ptr->body()(context);
    });

  return internal::get_impl(t).generate_results();
}

} // namespace waypoint
