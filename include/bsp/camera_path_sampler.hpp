#pragma once
#include "bsp/pose_refresh.hpp"

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

} // namespace bsp
