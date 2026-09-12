#pragma once
#include "bsp/native_particle_unit_random.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct NativeNodeStorage;

struct NativeParticleSphereDefinitionAccess {
    const NativeParticleUnitRandomAccess* unit_random;
    const CameraAxesCrtAccess* crt;
    const volatile double* random_offset_00d7a210;
    const volatile double* two_pi_00ce3828;
    const volatile double* pi_00ce3d28;
    const volatile double* percent_00d7a220;
    const volatile float* percent_limit_00ce3d08;
    const volatile std::uint32_t* one_00d7a24c;
    // Actual allocation domain paired with the model-update free service.
    void* (__cdecl* allocate_00bf681b)(std::size_t);
    // B02F45 captures CURRENT vtable+0C before the call. Dispatch this exact
    // target on the same actual objects; EDX carries that captured address.
    void (__fastcall* definition_virtual0c)(void* actual_definition,
        std::uint32_t captured_target, const void* parent108h,
        void* position, void* velocity);
};

// Complete normal body B02BC0..B02C30 through real allocator and AFE0A0.
// Native ECX definition, stack(model,time), RET8, EAX actual108h allocation.
// Added EDX access; native compiler EH registration is not reproduced.
void* __fastcall create_native_particle_sphere_record_00b02bc0(void*,
    const NativeParticleSphereDefinitionAccess*, NativeNodeStorage*, float time);

// Complete B02CE0..B02F26: same actual8Ch definition and parent108h record.
// Native ECX definition, stack(parent,position,velocity), RET0C. Added EDX
// access. Original x87/SSE spills, NaN branches and aliased stores retained.
void __fastcall generate_native_particle_sphere_00b02ce0(void*,
    const NativeParticleSphereDefinitionAccess*, const void* parent108h,
    void* position, void* velocity);

// Complete B02F30..B02FA8 through the captured current virtual0C target.
// Native ECX definition, stack(parent,position,velocity,matrix), RET10.
// Re-reads parent definition/model after the callback, then copies parent60,
// modelB0 or refreshed modelF0 in the original forward x87 order.
void __fastcall generate_native_particle_sphere_matrix_00b02f30(void*,
    const NativeParticleSphereDefinitionAccess*, const void* parent108h,
    void* position, void* velocity, void* matrix);
} // namespace bsp
