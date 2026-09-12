#pragma once
#include "bsp/native_point_light_provider.hpp"
#include <cstdint>

namespace bsp {
struct TrackedCriticalSection;
// Actual8h owner published at108FF50. Its one-slot native profileCFDEB4
// contains72CC90, not a RefCounted interface. No host vptr/count is added.
struct NativeParticlePopulationLockStorage {
    std::uint32_t native_profile_00;
    TrackedCriticalSection* section_04;
};
static_assert(sizeof(NativeParticlePopulationLockStorage)==8);
// Borrow the application's actual raw singleton publications. The underlying
// manager getter/register and real tracked section use the existing native
// implementations. Mixed-owner manager destruction still needs the real
// CFDEB4->72CC90 dispatch; this API does not invent a C++ manager/domain.
NativeParticlePopulationLockStorage* get_native_particle_population_lock_0072b740(
    void* volatile& actual_manager_01090aa0,
    NativeParticlePopulationLockStorage* volatile& actual_lock_0108ff50);
NativeParticlePopulationLockStorage* construct_native_particle_population_lock_0072a4f0(
    void* actual8h, NativeParticlePopulationLockStorage* volatile& actual_lock_0108ff50);
void clear_native_particle_population_lock_base_00729420(
    NativeParticlePopulationLockStorage*,
    NativeParticlePopulationLockStorage* volatile& actual_lock_0108ff50) noexcept;
NativeParticlePopulationLockStorage* delete_native_particle_population_lock_0072cc90(
    NativeParticlePopulationLockStorage*, std::uint32_t flags,
    NativeParticlePopulationLockStorage* volatile& actual_lock_0108ff50);

struct NativeParticleEmissionStateAccess {
    NativePointLightPopulationRuntime& population;
    void* volatile& actual_manager_01090aa0;
    NativeParticlePopulationLockStorage* volatile& actual_lock_0108ff50;
    const volatile std::uint32_t& minimum_radius_00d7a238;
    void* context;
    // Actual PARTICLE definition (B00CE0 family), not the six-slot shape
    // emitter definition profiles. Dispatch the already captured virtual18
    // on the same definition, state6Ch and record108h; native RET8.
    void (*particle_virtual18)(void*, void* actual_definition,
        std::uint32_t captured_target, void* actual_state, const void* actual_record);
    // Find its existing canonical owner, not a new PointLight/shadow payload.
    // The caller checks identity, population runtime and physical link binding.
    NativePointLightOwner& (*resolve_light)(void*, void* actual_light);
};
// Full B0CA40..B0CC08. Original ECX actual6Ch state, six stack words
// (particle definition,time,emitter,position,direction,record), RET18. Time is
// unread. Added EDX borrows access. Retain x87 sequential/overlapping copies,
// three-load SSE record vector, current virtual18 and all post-call reloads.
// Uses the established actual PointLight volume fragment and full B7B090
// population under the captured physical section. Original FH3 ABI is separate.
void __fastcall initialize_native_particle_emission_state_00b0ca40(void* actual_state,
    const NativeParticleEmissionStateAccess*, void* actual_definition, float time,
    void* actual_emitter, const float* actual_position, const float* actual_direction,
    const void* actual_record);
} // namespace bsp
