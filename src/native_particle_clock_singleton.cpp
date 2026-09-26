#include "bsp/native_particle_clock_singleton.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <new>

namespace bsp {
namespace {

struct NativeGuard {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGuard) == 8);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

volatile std::uint32_t& tracked_counter(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}

} // namespace

__declspec(noinline) NativeParticleClockStorage*
get_native_particle_clock_singleton_004de4b0(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_particle_publication_00f8d420) {
    NativeParticleClockStorage* const initial =
        static_cast<NativeParticleClockStorage*>(actual_particle_publication_00f8d420);
    if (initial) {
        return initial;
    }

    void* const first_manager = get_native_singleton_manager_00415350(
        actual_manager_publication_01090aa0);
    auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, captured_section};
    if (captured_section) {
        EnterCriticalSection(captured_section);
        auto& depth = tracked_counter(captured_section);
        depth = depth + 1u;
    }

    // Native state 0 arms after Enter and the counter increment. The source
    // catch covers C++ exceptions, not the original private FH3/SEH domain.
    try {
        if (!actual_particle_publication_00f8d420) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x1c, 0x1c});
            NativeParticleClockStorage* owner = nullptr;
            if (allocation) {
                // malloc backing is fresh and aligned for this 4-byte type.
                // Default construction (no () or {}) begins owner/header
                // lifetimes without initializing any scalar representation.
                owner = ::new (allocation) NativeParticleClockStorage;
                volatile auto& actual_owner = *owner;
                actual_owner.profile_04 = 0x00ce7d08u;
                actual_owner.records_08.data_00 = nullptr;
                actual_owner.records_08.count_04 = 0;
                actual_owner.records_08.capacity_08 = 0;
                actual_owner.word_14 = 0;
                actual_owner.profile_00 = 0x00ce7d38u;
                actual_owner.profile_04 = 0x00ce7d24u;
                // No store or read of time_18: native +18 stays unspecified.
            }
            actual_particle_publication_00f8d420 = owner;
            void* const current_manager = get_native_singleton_manager_00415350(
                actual_manager_publication_01090aa0);
            NativeParticleClockStorage* const current_particle =
                static_cast<NativeParticleClockStorage*>(actual_particle_publication_00f8d420);
            register_native_singleton_object_00bd0c30(
                current_manager, nullptr, current_particle);
        }

        if (captured_section) {
            auto& depth = tracked_counter(captured_section);
            depth = depth - 1u;
            LeaveCriticalSection(captured_section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
    return static_cast<NativeParticleClockStorage*>(actual_particle_publication_00f8d420);
}

} // namespace bsp
