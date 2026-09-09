#include "bsp/random.hpp"
#include <cstdlib>

namespace bsp {
void random_seed_00bf0cf0(RandomState& state, std::uint32_t seed) {
    state.words[0] = seed | 1u;
    for (state.index = 1; state.index < 624; ++state.index)
        state.words[state.index] = state.words[state.index - 1] * 69069u;
}

void random_construct_default_00bd2e00(RandomState& state) {
    state.index = 0;
    state.guard_allowed = 1;
    state.guard_enabled = 0;
    random_seed_00bf0cf0(state, 0x1105);
}

void random_refill_00bf0d20(RandomState& state) {
    constexpr std::uint32_t odd[2]{0, 0x9908b0dfu};
    for (state.index = 0; state.index < 227; ++state.index) {
        const auto i = state.index;
        const auto joined = (state.words[i] & 0x80000000u) | (state.words[i + 1] & 0x7fffffffu);
        state.words[i] = state.words[i + 397] ^ odd[joined & 1u] ^ (joined >> 1);
    }
    for (; state.index < 623; ++state.index) {
        const auto i = state.index;
        const auto joined = (state.words[i] & 0x80000000u) | (state.words[i + 1] & 0x7fffffffu);
        state.words[i] = state.words[i - 227] ^ odd[joined & 1u] ^ (joined >> 1);
    }
    const auto joined = (state.words[623] & 0x80000000u) | (state.words[0] & 0x7fffffffu);
    state.words[623] = odd[joined & 1u] ^ state.words[396] ^ (joined >> 1);
    state.index = 0;
}

std::uint32_t random_next_u32_00ba2c20(RandomState& state) {
    // Original deliberately writes 3 to null. Preserve fail-fast intent without C++ UB.
    if (state.guard_enabled && !state.guard_allowed) std::abort();
    if (state.index >= 624) random_refill_00bf0d20(state);
    auto value = state.words[state.index++];
    value ^= value >> 11;
    value ^= (value & 0xff3a58adu) << 7;
    value ^= (value & 0xffffdf8cu) << 15;
    return value ^ (value >> 18);
}
}
