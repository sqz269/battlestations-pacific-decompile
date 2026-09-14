#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-clock singleton requires MSVC Win32.
#endif

#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual 1Ch owner storage. The native constructor does not write payload_18;
// a caller may observe its allocation preimage before the clock setter runs.
struct NativeParticleClockStorage {
    std::uint32_t profile_00;
    std::uint32_t profile_04;
    std::uint32_t word_08;
    std::uint32_t word_0c;
    std::uint32_t word_10;
    std::uint32_t word_14;
    std::uint32_t payload_18;
};
static_assert(sizeof(NativeParticleClockStorage) == 0x1c);
static_assert(offsetof(NativeParticleClockStorage, payload_18) == 0x18);

// Complete source behavior of 004DE4B0[199]. Original entry takes no input,
// returns EAX and uses RET. Source adds stable borrowed references to the
// application's actual mutable manager and particle publication cells.
// The first nonnull particle read returns captured. On a miss, capture the
// first manager's raw +10 lock, enter/increment, then recheck publication.
// Allocate 1Ch only on a second miss. Publish even a null allocation, obtain
// the manager again, and register a freshly read publication. Release the
// originally captured lock and return a final publication read.
NativeParticleClockStorage* get_native_particle_clock_singleton_004de4b0(
    void* volatile& actual_manager_publication_01090aa0,
    NativeParticleClockStorage* volatile& actual_particle_publication_00f8d420);

} // namespace bsp
