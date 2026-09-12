#pragma once
#include "bsp/native_particle_model_construction.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstdint>

namespace bsp {
struct NativeParticleEmissionSpawnAccess;
struct NativeParticleRecordChildrenAccess;
// Borrowed CURRENT application memory and real downstream implementations.
// No substitute model, record, container, globals, or successful emission is
// supplied. The actual18h owner and its arrays retain their canonical layout.
struct NativeParticleRecordUpdateAccess {
    const CameraAxesCrtAccess* crt;
    const volatile float* percent_limit_00ce3d08;
    const volatile double* percent_00d7a220;
    const volatile double* transverse_00d7a258;
    const volatile float* direction_00e13028; // three separately reloaded words
    const volatile double* length_threshold_00d7a268;
    const volatile std::uint32_t* one_00d7a24c;
    // Concrete B04C80/AFDAF0 and AFD440/AFE1A0/AFDBF0 kernels share
    // these actual application services. They preserve the original five/six
    // stack words; EDX supplies the corresponding borrowed context.
    const NativeParticleEmissionSpawnAccess* emission;
    const NativeParticleRecordChildrenAccess* children;
};

// Complete AFCF50..AFD0E6, ECX destination108h, stack source108h, RET4,
// EAX destination. Forward x87 stores (including matrix), integer pointer
// words, original overlap ordering; this is not memcpy or a retaining copy.
void* __fastcall copy_native_particle_record_00afcf50(void* destination,
    void* unused_edx, const void* source);
// Complete AFD410..AFD43C, ECX actual18h, stack source, RET4. Signed current
// count14/capacity08 gate; byte index04 and108h stride; reload count after copy.
void __fastcall append_native_particle_record_00afd410(NativeParticleModelArraysStorage*,
    void* unused_edx, const void* source);
// Complete AFE030..AFE07A, ECX SAME actual model, stack destination, RET4.
// Local E0 if model1B0; otherwise refresh native world and copy120 forward.
void* __fastcall copy_native_particle_model_position_00afe030(NativeNodeStorage*,
    void* unused_edx, void* destination);
// Complete AFE290..AFEAD6 through the concrete emission kernels and their real services.
// ECX actual108h record; stack(time,step,low mode), RET0C; AL alive only (upper
// EAX unspecified). EDX adds this invocation's access. Exact x87/SSE schedule,
// NaN branches, loop restart, curve dispatch, live globals and alias reloads.
std::uint8_t __fastcall update_native_particle_record_00afe290(void* actual_record,
    const NativeParticleRecordUpdateAccess*, float time, float step, std::uint8_t mode);
// Complete AFD7A0..AFD81D. ECX SAME owner, stack(time,low mode), RET8, EAX
// signed current count14. Reload model+1A0 each record; byte-index swap removal
// revisits the moved record and observes count/base changes by emission.
std::int32_t __fastcall update_native_particle_records_00afd7a0(
    NativeParticleModelArraysStorage*, const NativeParticleRecordUpdateAccess*,
    float time, std::uint8_t mode);
} // namespace bsp
