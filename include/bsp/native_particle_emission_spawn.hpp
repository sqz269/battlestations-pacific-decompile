#pragma once
#include "bsp/native_particle_emitter_lifetime.hpp"
#include <cstdint>

namespace bsp {
struct NativeParticleEmissionStateAccess;
struct NativeParticleEmissionSpawnAccess {
    // Actual CRT floor, including current0109EEA0/MXCSR/x87 dispatch and
    // exceptional-input/control-word behavior. Cdecl(double), ST0 result.
    double (__cdecl* floor_00bf85b0)(double);
    // Native ST0 input (already spilled to float32), EAX integer, pop ST0, RET.
    // Must implement CURRENT0109EEA4 dispatch including the BF7456 low32 path.
    // This signature is an assembly-call contract, not an ordinary C++ call.
    std::int32_t (__cdecl* truncate_st0_00bf7420)();
    // Dispatch captured CURRENT recordA0 definition virtual0C. ECX definition,
    // EDX captured target, stack(record,position,direction), RET0C. The binding
    // must use that target without recapturing it and fill both actual vectors.
    void (__fastcall* definition_virtual0c)(void* actual_definition,
        std::uint32_t captured_target, const void* actual_record,
        float* actual_position, float* actual_direction);
    // Borrow complete B0CA40 bindings: actual particle virtual18, point-light
    // positioning/population and canonical shared lock. B04C80 calls the
    // concrete initializer directly, retaining its six stack words/RET18.
    const NativeParticleEmissionStateAccess* state;
};

// Complete B04C80..B04DE6 through the required actual bindings above. Native
// ECX30h container, stack(definition,record108h,requested,time,elapsed), RET14;
// EDX adds a borrowed access pointer. Return is clamped desired count, even
// when callback changes make fewer emissions occur. Exact x87 spills, signed
// comparisons and current field reloads retained; count/statistic ADDs wrap.
std::int32_t __fastcall spawn_native_particle_emission_00b04c80(
    NativeParticleEmitterContainer*, const NativeParticleEmissionSpawnAccess*,
    void* actual_definition, const void* actual_record,
    float requested, float time, float elapsed);

// Complete AFDAF0..AFDBE6. Native ECX108h record; four stack words
// (position,direction,unused sequence,fraction), RET10. EDX adds access.
// Capture definitionA0 virtual0C, generate vectors, then adjust position by
// current record00/0C with original x87 ordering and binary32 intermediates.
void __fastcall generate_native_particle_spawn_vectors_00afdaf0(
    const void* actual_record, const NativeParticleEmissionSpawnAccess*,
    float* actual_position, float* actual_direction,
    std::int32_t unused_sequence, float fraction);
// Names are descriptive hypotheses. New C++ access ABI; valid actual native
// storage and real bindings are required. No gameplay/FH3 compatibility claim.
} // namespace bsp
