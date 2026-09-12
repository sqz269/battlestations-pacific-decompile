#pragma once
#include "bsp/native_particle_unit_random.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct NativeNodeStorage;

// Borrow actual current application state. No copied model, record, curve,
// random stream, globals or successful downstream implementation is supplied.
struct NativeParticleSmartareaDefinitionAccess {
    const NativeParticleUnitRandomAccess* unit_random;
    const CameraAxesCrtAccess* crt;
    const volatile float* percent_limit_00ce3d08;
    const volatile double* percent_00d7a220;
    const volatile double* turn_00ce3828;
    const volatile double* degrees_00cf1448;
    const volatile double* transverse_00d7a258;
    void* (__cdecl* allocate_00bf681b)(std::size_t);
    void (__cdecl* free_00bf65ac)(void*);
    const volatile std::uint32_t* one_00d7a24c;
    // ECX actual definition; EDX ALREADY captured current vtable+0C target;
    // stack(parent108h,position,velocity), RET0C. Dispatch exactly that target
    // on these actual pointers, including callback mutations before reloads.
    void (__fastcall* definition_virtual0c)(void* actual_definition,
        std::uint32_t captured_target, const void* actual_parent_record,
        void* position, void* velocity);
};

// Complete B01CE0..B01D50 through the real allocation domain and shared AFE0A0.
// Original ECX definition; stack(model,time), RET8; EAX actual108h or null.
// New EDX access and C++ unwind ABI; native SEH replacement is not claimed.
void* __fastcall create_native_particle_smartarea_record_00b01ce0(
    void* actual_definition, const NativeParticleSmartareaDefinitionAccess*,
    NativeNodeStorage* actual_model, float time);

// Complete B01EC0..B0220E, actual vtable+0C. Original ECX definition;
// stack(parent108h,position,velocity), RET0C; EDX adds borrowed access.
// Preserves x87 spills/transcendentals, two canonical primary random draws,
// live curves80/84/88/8C and current parent/model matrix choice. Matrix rotates
// velocity only; position offset remains in the native horizontal frame.
void __fastcall generate_native_particle_smartarea_00b01ec0(
    void* actual_definition, const NativeParticleSmartareaDefinitionAccess*,
    const void* actual_parent_record, void* position, void* velocity);

// Complete B01E20..B01E98, actual vtable+10. Original ECX definition;
// stack(parent108h,position,velocity,matrix), RET10; EDX adds borrowed access.
// Captures current vtable+0C before calling it, then reloads parent definition,
// mode and model to copy parent60/localB0/worldF0 with sixteen x87 store pairs.
void __fastcall generate_native_particle_smartarea_with_matrix_00b01e20(
    void* actual_definition, const NativeParticleSmartareaDefinitionAccess*,
    const void* actual_parent_record, void* position, void* velocity, void* matrix);
} // namespace bsp
