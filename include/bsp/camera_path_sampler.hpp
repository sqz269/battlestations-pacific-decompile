#pragma once
#include "bsp/pose_refresh.hpp"
#include <array>
#include <vector>

namespace bsp {

// Borrowed actual fields, not a replacement path owner or recovered class ABI.
struct CameraPathView {
    void**& knots_begin_08;
    void**& knots_end_0c;
    void*& parent_14;
    std::uint8_t& wrap_byte_24;
};

class CameraPathHost : public PoseRefreshResolver {
public:
    // Pure lookups of existing actual storage: no copies, allocation, retention,
    // side effects or default objects. Owners/storage live through the call.
    virtual CameraPathView& resolve_path(void* actual_path) = 0;
    // Eight contiguous float words: XYZ +00, direction +0C, start +18,
    // duration +1C. Return the actual record prefix, never a snapshot.
    virtual float* resolve_path_knot_words(void* actual_knot) = 0;
    // Genuine CRT invalid-parameter operation. Its installed callback can return;
    // native then continues its unchecked access, without recovery or retry.
    virtual void range_error_00bf6713() = 0;
};

// Native ECX destination; stack by-value from XYZ, to XYZ, t; RET1C, EAX output.
// Subtraction and product each spill to binary32 before the final addition.
std::array<float, 3>* lerp_camera_path_vector_007ae200(std::array<float, 3>&,
    std::array<float, 3> from, std::array<float, 3> to, float parameter);

// Native ECX path; stack float t, mandatory position, optional direction; RET0C.
// First record with unordered-or-t<=start+duration wins. No time clamp. Wraps
// its next record to zero; copies current stored direction after position writes.
void sample_camera_path_linear_007afac0(CameraPathView&, float parameter,
    std::array<float, 3>& position, std::array<float, 3>* direction, CameraPathHost&);

// Native ECX path; stack float t, position, optional direction, flags; RET10.
// <3 records delegates to the linear body. Only flags' low byte is consulted.
// Float outputs may alias each other or actual knot float storage: derivative
// positions are re-read after position writes, as in the native numerical body.
// Output overlapping the path's pointer slots or record pointer table is outside
// this borrowed API's contract. Invalid pointer ranges remain native caller errors.
void sample_camera_path_local_007afe80(CameraPathView&, float parameter,
    std::array<float, 3>& position, std::array<float, 3>* direction,
    std::uint32_t flags, CameraPathHost&);

// Same native argument layout/RET10 as local sampler. Re-reads parent +14 for
// optional direction after position's homogeneous transform/divide. Refreshes
// each captured actual pose only if its +C8 byte is zero. No normalization.
void sample_camera_path_world_007b04c0(CameraPathView&, float parameter,
    std::array<float, 3>& position, std::array<float, 3>* direction,
    std::uint32_t flags, CameraPathHost&);

// 007AF150, the Path loader tail (reached from BSP_ScenePath_LoadSourceHolder
// 007B34F0 at 007B3708 when the built byte +18h is clear). __fastcall(path),
// plain RET 007AF731. Packet cc9_land_convoy_movement, docs/LAND_AND_STRUCTURES.md
// "The convoy formation, bound". The knot table is +8h/+0Ch (pointer vector);
// each knot record holds XYZ +00, direction +0C, start +18, duration +1C, the
// words resolve_path_knot_words() returns. What it derives:
//  - fewer than two knots: +28h = +1Ch = 0; a single knot gets start 0,
//    duration 0 and the direction triple 00F8758C..00F87594;
//  - otherwise the closed byte +24h = |first - last|^2 < 1.0, then per knot i:
//    start = running +1Ch; next = i + 1, or for the last knot (closed ? 1 : 0);
//    d = next - this (binary32 lanes); duration = sqrt(|d|^2) when the binary32
//    |d|^2 exceeds the double 1e-10 at 00CE3820, else 0; direction = d / duration
//    (no guard: a zero duration divides by zero, as natively); running += duration
//    for every knot but the last;
//  - length +28h = last.start + (closed ? 0 : last.duration);
//  - built byte +18h = 1.
// The helpers 007AE330 (lane-wise min) and 007AE3E0 (lane-wise max) run per knot
// into stack-only locals that nothing reads (the bounding box is dead code); they
// have no effect and are not reproduced. |d|^2 is summed as (dy^2 + dx^2) + dz^2
// (007AF589..007AF5A3) and stored as binary32 before the compare (007AF5A5).
struct CameraPathKnotTotals {
    bool closed_24 = false;
    float running_1c = 0.0f;
    float length_28 = 0.0f;
    bool built_18 = false;
};
using CameraPathKnotWords = std::array<float, 8>;
CameraPathKnotTotals derive_camera_path_knots_007af150(
    std::vector<CameraPathKnotWords>& knots, const std::array<float, 3>& up_00f8758c);

} // namespace bsp
