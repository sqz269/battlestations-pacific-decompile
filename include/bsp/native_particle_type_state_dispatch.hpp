#pragma once
#include <cstdint>

namespace bsp {
struct NativeParticleTypeStateAccess;
struct NativeParticleSpriteStateAccess;
struct NativeParticleObjectStateAccess;
struct NativeParticleTracerStateAccess;
struct NativeParticleEmissionStateAccess;

// Borrow the application's existing domains. A recognized target requires its
// binding; unknown CURRENT virtual18 targets remain the caller's real service.
// This is host composition, not another recovered native function or profile.
struct NativeParticleTypeStateDispatch {
    const NativeParticleTypeStateAccess* common;
    const NativeParticleSpriteStateAccess* sprite;
    const NativeParticleObjectStateAccess* object;
    const NativeParticleTracerStateAccess* tracer;
};
bool dispatch_known_native_particle_type_state(
    const NativeParticleTypeStateDispatch&, const NativeParticleEmissionStateAccess&,
    void* actual_definition, std::uint32_t captured_target,
    void* actual_state, const void* actual_record);
} // namespace bsp
