#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-clock singleton requires MSVC Win32.
#endif

#include "bsp/native_resource_record_vector.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {

// One actual 1Ch owner and its actual nested vector header. The getter starts
// this object's lifetime once by default placement construction, before the
// seven native stores. No initializer or store defines time_18: do not read it
// until the genuine B19A10 writer has run. This is not a projected clock.
struct NativeParticleClockStorage {
    std::uint32_t profile_00;
    std::uint32_t profile_04;
    NativeResourceRecordVectorStorage records_08;
    std::uint32_t word_14;
    float time_18;
};
static_assert(sizeof(void*) == 4 && sizeof(float) == 4);
static_assert(sizeof(NativeResourceRecordVectorStorage) == 0x0c);
static_assert(alignof(NativeResourceRecordVectorStorage) == 4);
static_assert(offsetof(NativeResourceRecordVectorStorage, data_00) == 0);
static_assert(offsetof(NativeResourceRecordVectorStorage, count_04) == 4);
static_assert(offsetof(NativeResourceRecordVectorStorage, capacity_08) == 8);
static_assert(sizeof(NativeParticleClockStorage) == 0x1c);
static_assert(alignof(NativeParticleClockStorage) == 4);
static_assert(offsetof(NativeParticleClockStorage, profile_00) == 0);
static_assert(offsetof(NativeParticleClockStorage, profile_04) == 4);
static_assert(offsetof(NativeParticleClockStorage, records_08) == 8);
static_assert(offsetof(NativeParticleClockStorage, word_14) == 0x14);
static_assert(offsetof(NativeParticleClockStorage, time_18) == 0x18);
static_assert(std::is_standard_layout_v<NativeParticleClockStorage>);
static_assert(std::is_trivially_default_constructible_v<NativeParticleClockStorage>);
static_assert(std::is_trivially_destructible_v<NativeParticleClockStorage>);
static_assert(std::is_trivially_default_constructible_v<NativeResourceRecordVectorStorage>);

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
    void* volatile& actual_particle_publication_00f8d420);

} // namespace bsp
