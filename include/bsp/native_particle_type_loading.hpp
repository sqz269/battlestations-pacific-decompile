#pragma once
#include <cstdint>

namespace bsp {
struct NativeParticleParameterLoadingBindings;
struct NativeParticleTypePropertyBindings;
struct NativeParticleParameterRuntimeRawContext;
struct CameraAxesCrtAccess;

// Same runtime pool as conversion/return, and the actual numeric/CRT domain.
// Numeric pointer members remain live cells; the kernels reload each member
// at its original use point. Property text is a borrowed actual4h header;
// this closure neither allocates nor returns strings, so needs no string pool.
struct NativeParticleTypeParameterRawContext {
    NativeParticleParameterRuntimeRawContext& parameters;
    const volatile double* percentage_scale_00d7a358;
    const CameraAxesCrtAccess& crt;
    const volatile double* last_time_00d7a220;
    const volatile double* bound_base_00d7a210;
    const volatile double* derivative_scale_00d7a2b0;
    const volatile double* discriminant_scale_00d7a328;
};

// Borrow current native text/parameter/particle-property domains. The property
// services must use the same owners as parameters. No temporary owner graph.
struct NativeParticleTypeLoadingBindings {
    NativeParticleParameterLoadingBindings& parameters;
    NativeParticleTypePropertyBindings& properties;
    char* text_scratch_00f8c2c8;
    const volatile double* percentage_scale_00d7a358;
    const volatile double* angle_scale_00d5daf8;
    const volatile double* axis_base_00ce3830;
    const volatile float* one_00d7a24c;
    const volatile float* negative_zero_00d7a208;
    const volatile double* bound_base_00d7a210;
    const volatile double* derivative_scale_00d7a2b0;
    const volatile double* discriminant_scale_00d7a328;
    // The native parser reuses one builder stack slot whose initial kind is
    // not initialized by AFBED0. Valid curve syntax overwrites this word.
    // Preserve it across lines; malformed ignored parse failures may read it.
    std::int32_t initial_builder_kind_0c;
};

// Common actual particle-definition parameter property. Original ECX owner,
// stack(pooled property,builder,float percentage), RET0C; AL handled flag.
bool load_native_particle_type_parameter_00b00980(void* actual_definition,
    const void* actual_property, void* actual_builder, float percentage,
    NativeParticleTypeLoadingBindings&);
bool load_native_particle_type_parameter_00b00980(void* actual_definition,
    const void* actual_property, void* actual_builder, float percentage,
    NativeParticleTypeParameterRawContext&);

// Original ECX builder; no stack inputs, RET, EAX converted first float value.
std::int32_t first_native_particle_parameter_integer_00afc1c0(const void*);
// Original ECX runtime parameter; no stack inputs, RET. Scans curve maxima
// and scales by (multiplier/current last-time)+current base, with native spills.
float bound_native_runtime_particle_value_00b001a0(const void*,
    NativeParticleTypeLoadingBindings&);
// Original ECX actual1Ch segment; RET/ST0, current derivative scale and CRT.
float bound_native_particle_cubic_segment_00affe90(const void*,
    NativeParticleTypeLoadingBindings&);
// Original fastcall ECX/EDX root pointers, stack(a,b,c), RET0C/ST0 float
// discriminant. Negative discriminants retain both caller output words.
float solve_native_particle_quadratic_00affe20(float*, float*, float, float, float,
    NativeParticleTypeLoadingBindings&);
// Original ECX Sprite definition; stack parameter, RET4; publishes +88/+8C.
void set_native_sprite_particle_size_00b08870(void*, void*,
    NativeParticleTypeLoadingBindings&);

// Raw entry binding adds integer instructions only: no extra x87 return spill.
// ECX is the native object, EDX the new borrowed context; quadratic retains
// native ECX/EDX root pointers and adds its context after the three stack floats.
float __fastcall bound_native_runtime_particle_value_00b001a0(const void*,
    const NativeParticleTypeParameterRawContext*);
float __fastcall bound_native_particle_cubic_segment_00affe90(const void*,
    const NativeParticleTypeParameterRawContext*);
float __fastcall solve_native_particle_quadratic_00affe20(float*, float*, float,
    float, float, const NativeParticleTypeParameterRawContext*);
// ECX definition, EDX new context, stack parameter; retains the native result
// spill before publishing +8C, and stores +88 before entering the bound helper.
void __fastcall set_native_sprite_particle_size_00b08870(void*,
    const NativeParticleTypeParameterRawContext*, void*);

// Original ECX actual definition; stack TextBuffer, RET4/AL success.
bool load_native_sprite_particle_definition_00b08ac0(void*, void*,
    NativeParticleTypeLoadingBindings&);
bool load_native_floating_particle_definition_00b07d60(void*, void*,
    NativeParticleTypeLoadingBindings&);
} // namespace bsp
