#pragma once
#include "bsp/native_particle_emission_spawn.hpp"
#include "bsp/native_particle_record_children.hpp"

namespace bsp {
struct NativeParticleUnitRandomAccess;

// Required borrowed bindings to the SAME native owners/current scalar storage.
// No independent random state, cached constants, substitute records or successful
// virtual dispatch fallback is created. The canonical emission callback must
// dispatch its already-captured current target, including non-cone overrides.
struct NativeParticleConeDefinitionAccess {
    void* (__cdecl* operator_new_00bf681b)(std::uint32_t);
    const NativeParticleUnitRandomAccess* unit_random;
    const NativeParticleEmissionSpawnAccess* emission;
    const volatile double* percent_00d7a220;
    const volatile float* maximum_time_00ce3d08;
    const volatile double* angle_scale_00d5daf8;
    const volatile double* azimuth_scale_00ce3828;
    const volatile float* zero_00d7a218;
    void (__cdecl* free_00bf65ac)(void*);
    const volatile std::uint32_t* one_00d7a24c;
};

// Complete B03970..B039E0 through the common actual108h AFE0A0 constructor
// and original allocation domain. Native ECX definition, stack(model,time),
// RET8, EAX actual record/null. EDX adds this borrowed access ABI.
void* __fastcall allocate_native_particle_cone_record_00b03970(void*,
    const NativeParticleConeDefinitionAccess*, NativeNodeStorage*, float time);

// Complete B03B60..B03EBE. Native ECX actual94h definition,
// stack(actual108h record,position,velocity), RET0C. EDX adds access.
// Preserves x87 FSIN/FCOS, spills, unordered comparisons, current curve loads,
// primary random draw count and forward/aliased position-before-velocity writes.
void __fastcall generate_native_particle_cone_vectors_00b03b60(void*,
    const NativeParticleConeDefinitionAccess*, const void* actual_record,
    float* actual_position, float* actual_velocity);

// Complete B03AC0..B03B38 through canonical captured current virtual0C.
// Native ECX definition, stack(record,position,velocity,matrix), RET10.
// Reloads recordA0/+70 and modelA4 AFTER dispatch; copies actual record60,
// modelB0 or refreshed modelF0 through the existing sequential x87 matrix copy.
void __fastcall generate_native_particle_cone_child_00b03ac0(void*,
    const NativeParticleConeDefinitionAccess*, const void* actual_record,
    float* actual_position, float* actual_velocity, void* actual_matrix);
// Descriptive names are hypotheses. These are new C++ access ABIs; this packet
// establishes no drop-in binary, application binding or gameplay compatibility.
} // namespace bsp
