#pragma once
// Packet cc13-big47a (cc13_big47a marker): large segment 47 (flight) bodies the
// cc13_seg47 / cc13_seg47b passes deferred. docs/CC13_BIG47A.md lists every
// function with its outcome, original ABI, thread and precision assumption and
// its uncertainty.
//
// Same shape as cc13_seg47 / cc13_seg47b, whose Raw and hosts this reuses:
// objects are reached through byte offsets into the Win32 layout and every call
// the image makes goes through a host method named for its target, invoked at
// the binary's point with the binary's arguments. Descriptive names are
// hypotheses, not recovered symbols. Nothing here is wired; nothing is
// ABI-compatible.

#include "bsp/cc13_seg47b.hpp"

#include <cstdint>

namespace bsp::cc13_big47a {

using cc13_seg47::Raw;
using cc13_seg47::raw;

// ---------------------------------------------------------------------------
// Image constants (address, width).
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGameGlobalsPointer = 0x00E188A8;  // pointer global; +19ECh = effect group
inline constexpr std::uint32_t kTipEffectAltitudeBase = 0x00E0B044;  // .data float, 598.0f (read from memory)
inline constexpr std::uint32_t kTipEffectAltitudeSpan = 0x00E0B048;  // .data float, 300.0f (read from memory)
inline constexpr float kOne = 1.0f;                            // 00D7A24C, float
inline constexpr double kEngineRunMinSpeed = 55.55555725097656;  // 00D05B38, double (bits 404BC71C80000000)
inline constexpr float kNegativeZero = -0.0f;                  // 00D7A208, float (bits 80000000)
inline constexpr double kEngineRunMinThrottle = 0.8999999761581421;  // 00D7A390, double (0.9f widened)
inline constexpr double kEngineRunLevel = 0.5;                 // 00D7A280, double
inline constexpr float kEngineRunTarget = 0.5f;                // 00CE3800, float
inline constexpr double kEngineFadeLevel = 0.0;                // 00D7A258, double
inline constexpr float kEngineFadeStop = 0.0f;                 // 00D7A218, float
inline constexpr float kMinusOne = -1.0f;                      // 00D7A260, float (time reset and level floor)
inline constexpr float kPointEventA0 = 1.5f;                   // 00CE380C, float
inline constexpr float kPointEventA4 = 3.0f;                   // 00CE3854, float
inline constexpr double kPointEventLimitScale = 3.0;           // 00D7A2B0, double
inline constexpr double kPointEventSpeedFloorTest = 0.10000000149011612;  // 00D7A3A0, double (0.1f widened)
inline constexpr float kPointEventSpeedFloor = 0.1f;           // 00D7A2F0, float (bits 3DCCCCCD)
inline constexpr std::uint32_t kPointEventResetOrigin = 0x00F87574;  // BSS float[3], read from memory

struct Big47aHost : cc13_seg47b::Seg47bHost {
    // 00866E60 BSP_PointEffect_FindChildByType, __thiscall(ECX = effect; void** out, type), RET 8.
    // Always writes *out (retained child or 0); it does not release a prior *out.
    virtual void point_effect_find_child_by_type_00866e60(Raw effect, void** out, std::int32_t type) = 0;
    // 00868420 BSP_PointEffect_CreateFromMatrix, __fastcall(ECX = out handle, EDX = group;
    // definition handle by value, const float* matrix, 0, 0), RET 10h, EAX = out handle. The
    // caller copy-constructs the by-value handle (retain); the callee consumes it.
    virtual void** point_effect_create_from_matrix_00868420(void** out, std::uint32_t group, void* definition,
                                                           const float* matrix, std::uint32_t a,
                                                           std::uint32_t b) = 0;
    // Point-effect child (type 3) vtable +14h, __thiscall(ECX = child; float), RET 4: set the
    // child's level. The child's current level is read as [child+1Ch]+19Ch.
    virtual void effect_child_set_level_vcall_14(Raw child, float level) = 0;
    // 0042B2F0 BSP_Vector3_LengthFloatThreshold, __thiscall(ECX = float[3]), RET 0, ST0 (float-rounded).
    virtual float vector3_length_threshold_0042b2f0(const float* v) = 0;
    // 0085DC80 BSP_Matrix_OrthonormalizeBasisRows, __thiscall(ECX = matrix), RET 0.
    virtual void matrix_orthonormalize_basis_rows_0085dc80(Raw matrix) = 0;
};

// 007C4DD0 BSP_Plane_UpdateEngineEffects, __thiscall(ECX = plane; float dt), RET 4.
void plane_update_engine_effects_007c4dd0(Big47aHost& host, Raw plane, float dt);

// 007BEEE0 BSP_PlaneTickElement_CommitStepPose, __thiscall(ECX = tick element = plane+310h), RET 0.
// `element` is the ECX value; plane fields are reached at element-310h.
void plane_tick_element_commit_step_pose_007beee0(Big47aHost& host, Raw element);

// 007BE9B0 BSP_Plane_ApplyPointEventLocal, __thiscall(ECX = target = plane+84Ch; float a[3],
// float b[3], float c[3], float time, all by value), RET 28h.
void plane_apply_point_event_local_007be9b0(Big47aHost& host, Raw target, const float (&a)[3],
                                            const float (&b)[3], const float (&c)[3], float time);

}  // namespace bsp::cc13_big47a
