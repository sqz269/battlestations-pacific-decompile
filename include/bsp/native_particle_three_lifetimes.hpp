#pragma once
#include <cstdint>

namespace bsp {
struct NativeParticleTypeLifetimeContext;

// Complete ECX-owner/RET bodies. Stamp the actual base-family profile, dispose
// and return captured nonnull parameter slots in the original order, then run
// the genuine common80h destructor. Remaining owner bytes are not initialized.
// Axial: D5DF30, +8C then +90. Floating: D5DFB0, +84/+80/+88.
// Sprite: D5DFF4, +84/+80/+88. Base cleanup is armed only after the first load.
void destroy_native_axial_particle_type_00b05940(void*, NativeParticleTypeLifetimeContext&);
void destroy_native_floating_particle_type_00b07730(void*, NativeParticleTypeLifetimeContext&);
void destroy_native_sprite_particle_type_00b088b0(void*, NativeParticleTypeLifetimeContext&);

// Six distinct original scalar entries: ECX owner, stack flags, EAX same owner,
// RET4. Each calls its family destructor and frees the owner iff flags bit0.
void* delete_native_axial_particle_definition_00b008c0(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);
void* delete_native_axial_particle_base_00b05ce0(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);
void* delete_native_floating_particle_definition_00b008e0(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);
void* delete_native_floating_particle_base_00b07c60(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);
void* delete_native_sprite_particle_definition_00b00900(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);
void* delete_native_sprite_particle_base_00b089c0(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);

// These owning source interfaces borrow the canonical parameter/string pools.
// A second exception during C++ unwind terminates. They are not original binary
// entry thunks or a claim of FH3/SEH/fault, concurrency, or gameplay equivalence.
} // namespace bsp
