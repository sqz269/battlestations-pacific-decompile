#pragma once

#include "bsp/pose_refresh.hpp"

namespace bsp {

// Native ECX=destination, EDX=source, RET, no meaningful EAX result. Produces
// an affine inverse only for mutually orthogonal nonzero upper basis rows.
// Other inputs retain the native unvalidated row-normalization formula.
// All 16 outputs are written; last column is forced to +0,+0,+0,1.
// Exact destination==source preserves native overwrite/reread behavior; it
// generally is NOT an inverse. Partial overlap is outside this typed contract.
// Native x87 order/spills and ambient FP state are retained without a fallback.
void derive_pose_affine_inverse_00b63d50(CameraMatrix& destination, const CameraMatrix& source);

// No owned flags or matrix cache. The SAME existing pose's derived_valid_10c
// controls its actual output at +110h; all referenced owner fields must outlive
// this view. Matrix storage and pose fields must not overlap in this accessor.
struct PoseDerivedView {
    PoseRefreshView& pose;
    CameraMatrix& derived_110;
};

// Native ECX=owner, EAX=owner+110h, no stack arguments, RET414E40. Any nonzero
// derived byte returns the existing output, even if world_valid_c8 is zero.
// Dirty path refreshes pose, sets derived_valid_10c=1 BEFORE running the math.
const CameraMatrix& get_pose_derived_00414e10(PoseDerivedView&);

} // namespace bsp
