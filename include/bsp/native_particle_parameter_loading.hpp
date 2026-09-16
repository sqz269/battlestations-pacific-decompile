#pragma once
#include "bsp/native_particle_definition.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
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

// Raw builder services over the application's actual string publication cells,
// CRT state and native numeric cells. No parameter pool is needed until runtime
// conversion, which remains a separate interface. Pointer members stay live:
// callbacks may rebind them; numeric kernels reload through their addresses.
// This does not change the existing 68-byte legacy bindings above.
struct NativeParticleParameterBuilderRawContext {
    NativeStringRawPoolContext& strings;
    const CameraAxesCrtAccess& crt;
    const volatile double* backward_limit_00ce3928;
    const volatile float* backward_clamp_00ce3cb4;
    const volatile double* forward_limit_00d7a3a0;
    const volatile float* forward_clamp_00d7a2f0;
    const volatile float* endpoint_time_00ce3d08;
    const volatile float* endpoint_backward_00ce65d8;
    const volatile float* first_time_00d7a218;
    const volatile double* last_time_00d7a220;
    const volatile float* linear_incoming_00d7a260;
    const volatile float* linear_outgoing_00d7a24c;
};

// Genuine raw-pool overloads of the same complete builder bodies. Preserve
// actual 10h owner/2Ch rows, partial parse mutations and sparse scratch. Fixed
// CRT arrays, current raw pool getter, no NativeStringStorage adapter. Normal
// getter failures propagate; exact native unwind states use noexcept guards,
// so secondary exceptions terminate. Constructor kind+C remains untouched;
// endpoints append without clearing; parse AL is not a transaction result.
void* construct_native_particle_parameter_builder_00afbed0(void*, NativeParticleParameterBuilderRawContext&);
void initialize_native_particle_parameter_endpoints_00afc360(void*, float, float, NativeParticleParameterBuilderRawContext&);
bool parse_native_particle_parameter_00afc470(void*, const void*, NativeParticleParameterBuilderRawContext&);
void destroy_native_particle_parameter_builder_00af4110(void*, NativeParticleParameterBuilderRawContext&) noexcept;
// Actual AFBED0 state0 action: AF4060[26], ECX owner, RET. Free captured
// records, then clear current +0/+4/+8; kind+C remains untouched.
void destroy_native_particle_parameter_key_vector_00af4060(void*) noexcept;

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
// Existing complete AFFCB0/AFFD20 bodies, exposed for Tracer updates. ECX
// actual parameter, EDX current bindings, stack time, RET4/ST0. Tail entries
// preserve any caller-owned x87 stack values; no float-return wrapper spill.
float __fastcall integrate_native_particle_parameter_hermite_00affcb0(
    const void*, const NativeParticleParameterLoadingBindings*, float);
float __fastcall integrate_native_particle_parameter_linear_00affd20(
    const void*, const NativeParticleParameterLoadingBindings*, float);
} // namespace bsp
