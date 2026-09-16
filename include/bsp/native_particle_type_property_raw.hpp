#pragma once
#include "bsp/native_particle_type_texture_raw.hpp"

namespace bsp {
struct NativeStringRawPoolContext;

// Same actual pool as texture->names.strings and all reached native children.
// Texture is required only on that recognized branch. Profile pointers borrow
// current original table cells through +10h, not copied or callable tables.
// Final definition profiles are installed by B00CE0 and derived constructors.
struct NativeParticleTypePropertyRawContext {
    NativeStringRawPoolContext& strings;
    NativeParticleTypeTextureRawContext* texture;
    const volatile std::uint32_t* sprite_profile_00d5dd18;
    const volatile std::uint32_t* axial_profile_00d5dcc0;
    const volatile std::uint32_t* floating_profile_00d5dcec;
    const volatile std::uint32_t* object_profile_00d5db00;
    const volatile std::uint32_t* tracer_profile_00d5e048;
};

// One caller-retained invocation. Native header storage precedes the retained
// texture/cache child so it outlives child teardown. Failed provider children
// must remain alive until their existing obligations are externally resolved.
// No destructor rollback, replay, additional texture reference or string owner.
struct NativeParticleTypePropertyRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    NativeParticleTypePropertyRawAcquired() = default;
    NativeParticleTypePropertyRawAcquired(const NativeParticleTypePropertyRawAcquired&) = delete;
    NativeParticleTypePropertyRawAcquired& operator=(const NativeParticleTypePropertyRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    std::uint32_t native_local10;
    std::uint32_t native_local14;
    std::uint32_t native_argument28;
    std::uint32_t consumed_name8h[2];
    const char* captured_layer{}; // Diagnostic identity; stale after return.
    NativeParticleTypeTextureRawAcquired texture;
};

// Complete B015C0..B01C11,1618B. ECX definition, stacked actual4h suffix;
// RET4/AL recognized. Twelve properties: Texture, Layer, Shader, nine flags.
// Native states0/1/3/4 and state2 unwind map, captured-pointer returns, consumed
// Layer header, current slot10 capture, concrete five shader bodies and retained
// B01350 child. Unknown reached profiles/targets are explicit source boundaries.
// Existing host API remains separate. New source ABI, not native FH3/SEH,
// unrestricted faults/CRT behavior, concurrent mutation or gameplay proof.
bool load_native_particle_type_property_00b015c0(void* actual_definition,
    const void* actual_suffix, NativeParticleTypePropertyRawContext&,
    NativeParticleTypePropertyRawAcquired&);
} // namespace bsp
