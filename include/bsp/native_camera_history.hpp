#pragma once
#include <cstdint>

namespace bsp {
struct CameraAxesCrtAccess;

// Borrow the SAME actual CRT access and actual immutable native constants.
// The owner is actual service+314, not a new history or projected camera.
struct NativeCameraHistoryContext {
    const CameraAxesCrtAccess* crt;
    const volatile std::uint32_t* reset_distance_00ce3930;
    const volatile std::uint32_t* one_00d7a24c;
};

// Full 005803E0..00580490: ECX outputXYZ, EDX oldXYZ, stack target/fraction,
// RET8, EAX output. Retains all binary32 x87 spills and live source reads.
// The native first argument slot is scratch after its pointer is captured.
void* __fastcall lerp_native_float3_005803e0(void* output_xyz,
    const void* old_xyz, const void* target_xyz, const void* fraction_word);

// Full B0DB00..B0E924. Original ECX history, stack time/camera, RET8; EDX
// adds context. Time is the original float DWORD without C++ FP conversion.
// Actual history: delay+00, count+04, eight48h samples at+08 (world64/fov/time),
// result matrix+248, previous world position+288; complete span is294h bytes.
// Count must be in the native-valid0..8 domain; no clamp or allocation added.
//
// Retains original restart/slide, current world copies, time search including
// unordered comparisons, interpolation, all rounded normalization/cross steps,
// projection/inverse/product and final result/position stores. Zero-duration
// and unordered paths are not replaced with a first-sample shortcut.
// The actual camera must have the canonical raw node/cache/scalar layout.
// Required CRT and raw callees must return; C++ exceptions across this naked
// frame, native SEH/stack aliases and game equivalence are not established.
void __fastcall update_native_camera_history_00b0db00(void* actual_history,
    const NativeCameraHistoryContext*, std::uint32_t actual_time_bits,
    void* actual_camera);
} // namespace bsp
