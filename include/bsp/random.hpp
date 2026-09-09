#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// Observed native layout. Flag names describe the guard, not an inferred policy.
struct RandomState {
    std::uint32_t index{};
    std::uint32_t words[624]{};
    std::uint8_t guard_allowed{1};
    std::uint8_t guard_enabled{};
    std::uint8_t padding[2]{};
};
static_assert(offsetof(RandomState, words) == 4);
static_assert(offsetof(RandomState, guard_allowed) == 0x9c4);
static_assert(offsetof(RandomState, guard_enabled) == 0x9c5);
static_assert(sizeof(RandomState) == 0x9c8);

void random_construct_default_00bd2e00(RandomState& state);
void random_seed_00bf0cf0(RandomState& state, std::uint32_t seed);
void random_refill_00bf0d20(RandomState& state);
std::uint32_t random_next_u32_00ba2c20(RandomState& state);
}
