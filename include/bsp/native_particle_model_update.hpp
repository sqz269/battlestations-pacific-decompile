#pragma once
#include "bsp/registered_model_effect.hpp"
#include <cstdint>

namespace bsp {
class RandomThreads;
struct NativeParticleRecordUpdateAccess;
struct NativeParticleEmitterUpdateBindings;

// Required real application services. All pointers designate the existing
// native payloads; these interfaces create no substitute model/particle owner.
class NativeParticleModelUpdateCallees {
public:
    virtual ~NativeParticleModelUpdateCallees() = default;
    // AF6E6C: captured CURRENT definition vtable+08, ECX definition,
    // stack(model,time184), RET8; EAX actual temporary supporting +00..+107 (AFCF50 reads +104).
    // Dispatch the supplied native target through its canonical binding.
    virtual void* definition_virtual08(void* actual_definition,
        std::uint32_t captured_target, NativeNodeStorage& actual_model,
        float time) = 0;

};

struct NativeParticleModelUpdateAccess {
    RegisteredModelEffectCallees* options; // existing actual0051F6B0 binding
    RandomThreads* random; // existing application thread/stream domain
    NativeParticleModelUpdateCallees* services;
    // Actual CRT floor, including its current0109EEA0/MXCSR/x87 dispatch,
    // exceptional-input handlers and control-word effects. Cdecl(double), ST0.
    // Caller float32 quotient is widened to double before this invocation.
    double (__cdecl* floor_00bf85b0)(double);
    // Same real CRT allocation domain as definition virtual08. Cdecl/RET;
    // BF65AC tail-jumps BF9DC8, which RETURNS after actual heap/error handling.
    void (__cdecl* free_00bf65ac)(void*);
    const NativeParticleRecordUpdateAccess* records;
    const NativeParticleEmitterUpdateBindings* emitters;
};

// Complete AF6DD0..AF7391, including previously undisassembled AF704F..AF7071
// initializer loop. ECX actual2DCh model (canonical NativeNodeStorage prefix),
// stack(delta,mode), RET8; EDX adds a required borrowed per-call access pointer.
// Preserves actual model/definition/emitter reloads, x87 lifetimes, float32
// spills, MOVSS copies and native unchecked index/NaN behavior. Native1A5 is
// set before initialization; initialization continues into this call's update.
// No successful constructor, simulation or geometry fallback is supplied.
void __fastcall update_native_particle_model_00af6dd0(NativeNodeStorage*,
    const NativeParticleModelUpdateAccess*, float delta, std::uint8_t mode);

// Complete leaves over original storage; unused EDX preserves native stack.
void* __fastcall copy_native_node_local_position_00b6e0a0(
    const void* actual_node, void* unused_edx, void* destination);
const void* __fastcall get_native_node_local_matrix_00b6db60(const void* actual_node);
void* __fastcall copy_native_particle_definition_bounds_00af3f50(
    const void* actual_definition, void* unused_edx, void* destination);
// Literal native RET; the temporary's destructor has no body effects.
void __fastcall destroy_native_particle_temporary_00afd9f0(void* actual_temporary);

// Exact unbounded segment walks, ECX curve, stack float time, RET4, ST0 float.
// Curve+04 points to actual20/28-byte segment rows respectively. No clamping,
// interpolation repair, finite-value assumption or ownership change is added.
float __fastcall evaluate_native_particle_linear_curve_00affa70(
    const void* actual_curve, void* unused_edx, float time);
float __fastcall evaluate_native_particle_cubic_curve_00affae0(
    const void* actual_curve, void* unused_edx, float time);
} // namespace bsp
