#pragma once
#include "bsp/native_particle_type_state.hpp"
#include "bsp/native_particle_unit_random.hpp"

namespace bsp {
struct NativePointLightEnvironment;
class NativeStringStorage;
struct NativeParticlePopulationLockStorage;
// Required only when actual definition+64 is nonzero. The environment owns
// the same native PointLight pool/node domains used by emission and cleanup.
// strings is the application's existing temporary-string storage; actual
// native publications use ActualNativeStringPoolStorage. The light's retained
// name uses the environment's checked semantic node-name pool contract.
struct NativeParticleSpriteLightAccess {
    NativePointLightEnvironment& environment;
    NativeStringStorage& strings;
    void* volatile& actual_manager_01090aa0;
    NativeParticlePopulationLockStorage* volatile& actual_lock_0108ff50;
};

// Borrow the SAME application random domain and current native globals used
// by the other actual particle initializers. type.random == unit.random.
struct NativeParticleSpriteStateAccess {
    NativeParticleTypeStateAccess type;
    NativeParticleUnitRandomAccess unit;
    const volatile float* sign_threshold_00ce3800;
    const volatile float* negative_one_00d7a260;
    volatile std::uint32_t* sprite_counter_00f8d388;
    const NativeParticleSpriteLightAccess* lights;
};

// Complete actual Sprite virtual18, B08F60..B094DC. ECX actual90h definition,
// stack(actual6Ch state,actual108h record), RET8; record accepted but unread.
// EDX adds borrowed access. Original EAX is incidental, not a logical result.
// See the doc for physical light services and original FH3 ABI limits.
void __fastcall initialize_native_particle_sprite_state_00b08f60(void* actual_definition,
    const NativeParticleSpriteStateAccess*, void* actual_state, const void* actual_record);
// Complete virtual1C, B007F0..B007FD: subtract1 from CURRENT F8D388 and return1.
// Both stack arguments and definition are unread. No light disposal occurs.
std::uint32_t __fastcall release_native_particle_sprite_state_00b007f0(void* actual_definition,
    const NativeParticleSpriteStateAccess*, void* actual_state, std::uint32_t argument);
} // namespace bsp
