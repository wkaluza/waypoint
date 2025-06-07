#include "impls.hpp"
#include "waypoint.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <ranges>
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
  constexpr std::size_t arbitrary_seed = 0x0123'4567'89ab'cdef;
  constexpr std::size_t arbitrary_constant = 0x1234;
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

auto get_shuffled_indices(Engine &t) -> std::vector<std::size_t>
{
  auto rng = get_random_number_generator();

  auto const number_of_tests = internal::get_impl(t).test_bodies().size();

  auto indices = std::views::iota(static_cast<std::size_t>(0)) |
    std::views::take(number_of_tests) |
    std::ranges::to<std::vector>();
  std::ranges::shuffle(indices, rng);

  return indices;
}

} // namespace

auto run_all_tests(Engine &t) -> Result
{
  auto const indices = get_shuffled_indices(t);

  auto const &bodies = internal::get_impl(t).test_bodies();
  for(auto const i : indices)
  {
    auto context = internal::get_impl(t).make_context(bodies[i].test_id());
    bodies[i].body()(context);
  }

  return internal::get_impl(t).generate_results();
}

} // namespace waypoint
