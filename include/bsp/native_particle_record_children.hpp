#pragma once
#include "bsp/native_particle_model_construction.hpp"
#include "bsp/random_threads.hpp"
#include <cstdint>

namespace bsp {
// Same canonical owner/model/record and primary RandomThreads domain. Every
// pointer is required when reached; no substitute definition dispatch or CRT
// conversion is provided. Borrow current scalar storage, not cached values.
struct NativeParticleRecordChildrenAccess {
    RandomThreads* random;
    double (__cdecl* floor_00bf85b0)(double);
    // Register-input CRT: consumes ST0, EAX signed result, retaining native
    // dispatch/floating-environment/exception effects. X86 assembly calls only.
    void (__cdecl* truncate_st0_00bf7420)();
    const volatile float* zero_00d7a218;
    const volatile std::uint32_t* one_00d7a24c;
    const volatile double* signed_random_scale_00d5da30;
    const volatile double* random_offset_00d7a210;
    const volatile double* percent_00d7a220;
    // Actual definition current vtable+10, captured at AFDC0C. Original ECX
    // definition; four stack words(parent108h,position,velocity,matrix), RET10.
    // EDX carries the ALREADY captured target. Dispatch that exact target on
    // the same actual objects; do not reread table/slot or fake generated data.
    void (__fastcall* definition_virtual10)(void* actual_definition,
        std::uint32_t captured_target, const void* actual_parent_record,
        void* position, void* velocity, void* matrix);
};

// Complete AFE1A0..AFE28D. Original ECX actual108h record;
// stack(model,definition,time,velocity), RET10. Added EDX borrows current one.
// Sparse native writes; forward x87 velocity copy preserves input aliases.
void __fastcall initialize_native_particle_record_00afe1a0(void* actual_record,
    const volatile std::uint32_t* one_00d7a24c, NativeNodeStorage* actual_model,
    void* actual_definition, float time, const void* velocity);
// Complete AFDBF0..AFDCEB THROUGH required captured virtual10. Original ECX
// parent108h; stack(position,velocity,matrix,ordinal,fraction), RET14.
// Ordinal is unread; matrix's incoming stack slot becomes float scratch after
// dispatch. Preserve post-callback parent reloads and native x87/store order.
void __fastcall interpolate_native_particle_record_00afdbf0(
    const void* actual_parent_record, const NativeParticleRecordChildrenAccess*,
    void* position, void* velocity, void* matrix, std::int32_t ordinal, float fraction);
// Complete AFD440..AFD792 THROUGH real CRT and captured generation dispatch.
// Original ECX actual18h owner; stack(same model,definition,parent108h,
// requested,time,elapsed), RET18, EAX actual appended count. Elapsed is unread.
// Added EDX borrows access. Floor->float32->register CRT conversion, signed
// capacity/current count checks, original record publication and random draws.
std::int32_t __fastcall append_native_particle_record_children_00afd440(
    NativeParticleModelArraysStorage*, const NativeParticleRecordChildrenAccess*,
    NativeNodeStorage* actual_model, void* actual_definition,
    const void* actual_parent_record, float requested, float time, float elapsed);
} // namespace bsp
