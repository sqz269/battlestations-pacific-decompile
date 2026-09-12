#pragma once
#include <cstdint>

namespace bsp {
struct CameraAxesCrtAccess;
struct NativeParticleParameterLoadingBindings;
struct FrameClock;
class SystemTimeTimerVirtuals;

// Borrow the current application CRT, curve and clock domains. Scalar pointers
// are read where the native instructions read their image globals. The clock
// is the SAME canonical FrameClock used by system-time and frame services;
// its typed projection is never cast to the original singleton/vtable ABI.
struct NativeTracelineUpdateAccess {
    const CameraAxesCrtAccess* crt;
    const volatile std::uint32_t* truncate_mode_0109eea4;
    const volatile float* negative_zero_00d7a208;
    const volatile double* percent_00d7a220;
    const volatile float* direction_00e13028; // three consecutive float words
    const NativeParticleParameterLoadingBindings* parameters;
    FrameClock* const volatile* clock_01090ab0;
    SystemTimeTimerVirtuals* clock_virtuals;
};

// Complete AF22B0..AF2620 over the actual derived1BCh node, payload80h at184,
// and producer-allocated 14h ring rows at188. No allocation, copied owner,
// finite-input guard or capacity repair. Native ECX node, EDX point, stack
// interval, RET4. Source adds access as the second stack word and returns8.
// The x87 length/normalization, CRT conversion, native wrap and all spill
// points are retained. The endpoint's stored time is negative_zero - interval.
void __fastcall append_native_traceline_point_00af22b0(void* actual_node,
    const float* actual_point, float interval, const NativeTracelineUpdateAccess*);

// Complete AF2630..AF2643. Native ECX node, stack(point,interval), RET8.
// Source adds borrowed EDX access; the forwarded interval still passes through
// the original FLD/FSTP conversion before the point-insertion call.
void __fastcall set_native_traceline_point_00af2630(void* actual_node,
    const NativeTracelineUpdateAccess*, const float* actual_point, float interval);

// Complete B0A110..B0A3EB through the existing current-clock virtual1C service.
// Native ECX Tracer definition, five stack words(state,age,word,matrix,delta),
// RET14/EAX1; word and delta are unread. Source adds only EDX access. The state
// +30 gate is a DWORD pointer comparison, not a floating comparison. All x87
// operations, float32 spills and the signed-int64 clock ratio are preserved.
std::uint32_t __fastcall update_native_particle_tracer_00b0a110(void* actual_definition,
    const NativeTracelineUpdateAccess*, void* actual_state, float age,
    std::uint32_t word, const void* actual_matrix, float delta);

// AF26A0..AF3168 is a separate mesh-fill/render-submission body (native RET10).
// It is analyzed only in this packet; no source implementation is claimed.
} // namespace bsp
