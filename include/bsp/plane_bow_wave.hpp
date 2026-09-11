#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
using PlaneBowWavePoint = std::array<float, 3>;
using PlaneBowWaveTransform = std::array<float, 16>;

// Native descriptor +550h container element: pointer token then model-space
// marker point x/y/z. Tokens are opaque 32-bit host identities, not C++ pointers.
// No default initializer: Lua 007d3608 initializes only effect_definition;
// the model-binding fragment below subsequently writes all three coordinates.
struct PlaneBowWaveDefinition {
    std::uint32_t effect_definition;
    PlaneBowWavePoint model_point;
};
static_assert(sizeof(PlaneBowWaveDefinition) == 0x10);
static_assert(offsetof(PlaneBowWaveDefinition, model_point) == 4);

// Same 10h-byte shape, but the pointer at +0h names a created effect instance.
// Native plane instance +A3Ch owns a vector of these records.
struct PlaneBowWaveInstance {
    std::uint32_t effect_instance;
    PlaneBowWavePoint model_point;
};
static_assert(sizeof(PlaneBowWaveInstance) == 0x10);

struct PlaneBowWaveMarkerHost {
    virtual ~PlaneBowWaveMarkerHost() = default;
    // 00718000: search model +68h/+6Ch by exact name and numeric id; select
    // the first point of the resulting marker's +48h/+4Ch point vector.
    // Native requires a present marker with a nonempty point vector. The host
    // must report a failed requirement rather than synthesize a zero point.
    virtual PlaneBowWavePoint first_model_marker_point(
        const char* name, std::uint32_t one_based_id) = 0;
};

// Interior fragment 007d4812..007d4953 of descriptor post-model setup
// 007d3e60 (native ECX=descriptor). Rewrites points only, in index order.
void bind_plane_bow_wave_points_007d4812(PlaneBowWaveDefinition* records,
    std::size_t count, PlaneBowWaveMarkerHost& host);

struct PlaneBowWaveEffectHost {
    virtual ~PlaneBowWaveEffectHost() = default;
    // 007d59c8 -> 00868420: native ECX=return-handle slot,
    // EDX=(*(00e188a8)+19ECh), stack=(retained definition, matrix*, 0, 0).
    // Returns one owned temporary reference, or zero. Host folds the native
    // by-value definition/result reference traffic into this boundary.
    virtual std::uint32_t create_effect(std::uint32_t definition,
        const PlaneBowWaveTransform& transform,
        std::uint32_t argument_3, std::uint32_t argument_4) = 0;
    // 007d5a49 writes instance byte +9h. Its general meaning is unresolved.
    virtual void set_effect_byte_09(std::uint32_t instance, std::uint8_t value) = 0;
    // 007d5a5c -> 007d1dc0: append to plane instance +A3Ch. The host acquires
    // its own reference if nonzero. Null records are appended too.
    virtual void append_bow_wave(const PlaneBowWaveInstance& record) = 0;
    // Release the returned temporary after append, including an exception.
    virtual void release_effect(std::uint32_t instance) noexcept = 0;
};

// 007d5890..007d5ab7, native ECX=plane instance, no stack args, RET.
// Reads definitions through instance+538h -> descriptor+550h. Appends without
// clearing existing runtime records. Records/count must remain stable while
// hosts run; initialized model points and valid records are preconditions.
// New C++ interface: not a drop-in ABI or a replacement effect manager.
void create_plane_bow_waves_007d5890(const PlaneBowWaveDefinition* records,
    std::size_t count, PlaneBowWaveEffectHost& host);
}
