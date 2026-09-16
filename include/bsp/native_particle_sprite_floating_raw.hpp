#pragma once
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_type_loading.hpp"
#include "bsp/native_particle_type_property_raw.hpp"
#include <optional>

namespace bsp {
// Borrow the application's SAME actual string publications, shared F8C2C8
// scratch, builder numeric/CRT cells and F8D344 runtime parameter pool. Property
// providers retain their existing actual texture/cache/renderer domains.
struct NativeParticleSpriteFloatingRawContext {
    NativeParticleTypePropertyRawContext& properties;
    NativeParticleParameterBuilderRawContext& builder;
    NativeParticleTypeParameterRawContext& parameters;
    char* actual_text_scratch_00f8c2c8;
};

// One caller-retained invocation. Header and builder storage precedes its
// property child, so failed cache/provider children keep their referenced
// storage alive. The native builder kind+C starts unwritten: supply its actual
// incoming residue explicitly; later lines reuse the resulting current word.
// Completed property frames may be replaced; a failed child is retained until
// its existing obligations are externally resolved. No destructor rollback.
struct NativeParticleSpriteFloatingRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    explicit NativeParticleSpriteFloatingRawAcquired(std::int32_t initial_builder_kind) noexcept;
    NativeParticleSpriteFloatingRawAcquired(const NativeParticleSpriteFloatingRawAcquired&) = delete;
    NativeParticleSpriteFloatingRawAcquired& operator=(const NativeParticleSpriteFloatingRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    // Actual4h pooled headers. Only line is initialized at native entry.
    std::uint32_t line, name, key_token, property_suffix;
    std::uint32_t percentage_token, percentage_suffix, curve, curve_suffix;
    std::uint32_t percentage_bits;
    NativeParticleParameterBuilderStorage builder_storage;
    std::optional<NativeParticleTypePropertyRawAcquired> property;
};

// Complete B08AC0..B08F55 and B07D60..B08202. Native ECX definition,
// stacked actual TextBuffer, AL true/RET4 on normal EOF or closing brace.
// Sprite clears byte+64 before reading; Floating leaves it unchanged. Both
// write runtime parameters at +80/+84/+88, and Sprite Size invokes B08870 to
// publish its bound+8C. Genuine raw providers; native seven-state ownership,
// unarmed percentage-token cleanup and x87 percentage stores are preserved.
bool load_native_sprite_particle_definition_00b08ac0(void*, void*,
    NativeParticleSpriteFloatingRawContext&, NativeParticleSpriteFloatingRawAcquired&);
bool load_native_floating_particle_definition_00b07d60(void*, void*,
    NativeParticleSpriteFloatingRawContext&, NativeParticleSpriteFloatingRawAcquired&);
// New C++ interfaces, not original ABI/FH3/SEH/CRT/fault or gameplay proof.
// No native parameter publication is rolled back on failure or replacement.
} // namespace bsp
