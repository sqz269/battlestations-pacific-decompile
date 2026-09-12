#pragma once
#include "bsp/native_particle_definition.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstdint>

namespace bsp {
// Producer-established actual 10h temporary owner. Each key occupies 2Ch:
// x,y,incoming.xy,outgoing.xy,endpoint-kind,a,b,c,d. Coefficients and unused
// constructor scratch are indeterminate until their producing operation.
struct NativeParticleParameterBuilderStorage {
    void* records_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
    std::int32_t kind_0c;
};
// All pointers borrow CURRENT application data. owners supplies the SAME
// F8D344 pool used by AFFDF0/B00090; this API never initializes another pool.
struct NativeParticleParameterLoadingBindings {
    NativeParticleDefinitionBindings& owners;
    const CameraAxesCrtAccess& crt;
    void* (__cdecl* allocate_array_00bf55be)(std::size_t);
    const volatile double* backward_limit_00ce3928;
    const volatile float* backward_clamp_00ce3cb4;
    const volatile double* forward_limit_00d7a3a0;
    const volatile float* forward_clamp_00d7a2f0;
    const volatile float* endpoint_time_00ce3d08;
    const volatile float* endpoint_backward_00ce65d8;
    const volatile double* tangent_scale_00cf1450;
    const volatile float* default_slope_00d5dca0;
    const volatile double* integral_half_00d7a280;
    const volatile double* integral_quarter_00d7a348;
    const volatile float* first_time_00d7a218;
    const volatile double* last_time_00d7a220;
    const volatile float* linear_incoming_00d7a260;
    const volatile float* linear_outgoing_00d7a24c;
};

// Native ECX owner, RET/EAX owner. Clears only +0/+4/+8, grows to32 keys;
// kind+C is untouched. New C++ interface and host unwind, not native FH3 ABI.
void* construct_native_particle_parameter_builder_00afbed0(void*, NativeParticleParameterLoadingBindings&);
// ECX owner, stack(first_value,last_value), RET8. Inserts endpoints into the
// current array without clearing existing keys or writing owner kind+C.
void initialize_native_particle_parameter_endpoints_00afc360(void*, float, float, NativeParticleParameterLoadingBindings&);
// ECX owner, stack actual4h line header, RET4/AL. Hermite/Linear require >=2
// keys, first x==current0, last x==current100. Failed parsing keeps mutations.
bool parse_native_particle_parameter_00afc470(void*, const void*, NativeParticleParameterLoadingBindings&);
// ECX owner, RET/EAX pooled actual0Ch payload. +0 multiplier is untouched;
// count/capacity are BYTES at+8/+9, type WORD at+A; slab index+C untouched.
void* convert_native_particle_parameter_00afbf60(void*, NativeParticleParameterLoadingBindings&);
// Native ECX owner, RET/ST0; payload is already a float.
float first_native_particle_parameter_value_00afc1b0(const void*);
// Native ECX owner, RET. Free captured records then clear CURRENT+0/+4/+8.
// Leaves kind+C unchanged; uses owners.free_array_00bf6989.
void destroy_native_particle_parameter_builder_00af4110(void*, NativeParticleParameterLoadingBindings&) noexcept;
} // namespace bsp
