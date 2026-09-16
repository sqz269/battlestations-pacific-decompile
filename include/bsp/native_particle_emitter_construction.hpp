#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/native_particle_type_lifetime.hpp"

namespace bsp {

inline constexpr std::size_t native_particle_emitter_base_size = 0x80;
inline constexpr std::size_t native_particle_cone_emitter_size = 0x94;
inline constexpr std::size_t native_particle_sphere_emitter_size = 0x8c;
inline constexpr std::size_t native_particle_smartarea_emitter_size = 0x90;

// One physical lifetime domain supplies both construction-failure string
// cleanup and the future derived/base destruction chain. The referenced
// NativeStringRawPoolContext and F8D344 NativeWeakHandlePool are the actual
// application domains; this context owns neither and adds no callbacks.
struct NativeParticleEmitterConstructionContext {
    NativeParticleTypeLifetimeContext& lifetime;
};

// Complete AFA280. Original ECX actual80h owner; stack(name header pointer,
// word10,word70,flag14), EAX owner, RET10. The actual8h name is borrowed and
// remains caller-owned. Only flag14's low byte is read. All unlisted bytes,
// including derived parameter slots, retain their allocation preimage.
//
// This overload composes the genuine raw string pool. It coexists with the
// older NativeParticleDefinitionBindings overload in native_particle_definition.hpp.
void* construct_native_particle_definition_00afa280(void* actual_owner,
    const void* actual_name8h, std::uint32_t word10, std::uint32_t word70,
    std::uint32_t flag14, NativeParticleEmitterConstructionContext&);

// Complete one-call derived bodies. Original ABI is the same RET10 interface;
// each installs its physical profile only after AFA280 returns successfully.
void* construct_native_particle_cone_definition_00b03940(void* actual_owner,
    const void* actual_name8h, std::uint32_t word10, std::uint32_t word70,
    std::uint32_t flag14, NativeParticleEmitterConstructionContext&);
void* construct_native_particle_sphere_definition_00b02b90(void* actual_owner,
    const void* actual_name8h, std::uint32_t word10, std::uint32_t word70,
    std::uint32_t flag14, NativeParticleEmitterConstructionContext&);
void* construct_native_particle_smartarea_definition_00b01cb0(void* actual_owner,
    const void* actual_name8h, std::uint32_t word10, std::uint32_t word70,
    std::uint32_t flag14, NativeParticleEmitterConstructionContext&);

// These are source-level compositions, not binary replacements for the native
// FH3 entrypoints. AF9FB0 allocation/type selection/parser dispatch is separate.

} // namespace bsp
