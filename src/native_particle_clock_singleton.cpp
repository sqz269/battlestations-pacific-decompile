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
    NativeParticleClockStorage* volatile& actual_particle_publication_00f8d420) {
    NativeParticleClockStorage* const initial = actual_particle_publication_00f8d420;
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
                owner = static_cast<NativeParticleClockStorage*>(allocation);
                auto* const words = reinterpret_cast<volatile std::uint32_t*>(owner);
                words[1] = 0x00ce7d08u;
                words[2] = 0;
                words[3] = 0;
                words[4] = 0;
                words[5] = 0;
                words[0] = 0x00ce7d38u;
                words[1] = 0x00ce7d24u;
                // No store to words[6]: native +18 remains allocation preimage.
            }
            actual_particle_publication_00f8d420 = owner;
            void* const current_manager = get_native_singleton_manager_00415350(
                actual_manager_publication_01090aa0);
            NativeParticleClockStorage* const current_particle =
                actual_particle_publication_00f8d420;
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
    return actual_particle_publication_00f8d420;
}

} // namespace bsp
