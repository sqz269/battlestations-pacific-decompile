#pragma once

#include "bsp/cc16_dyn_ring_predicates.hpp"

#include <cstdint>

namespace bsp::cc16_dyn_ring_query {

struct DynRingBarycentricWeights {
    float u;
    float v;
    float w;
};

// Required body/FP-context contract for 00C37860. The original takes EAX=u_out,
// ECX=v_out, EDX=w_out and eight stack floats, then RET 20h. All three outputs
// must have the original store epochs and arithmetic under the actual ambient
// x87 control (precision, rounding and exception masks). Preserve that control
// and MXCSR; leave the x87 stack balanced. No PC24 or PC53 default is supplied.
// The existing fixed-precision source alone does not establish this contract.
struct DynRingQueryCalls {
    virtual ~DynRingQueryCalls() = default;
    virtual DynRingBarycentricWeights barycentric_00c37860(
        cc16_dyn_ring_predicates::DynRingPoint2 triangle0,
        cc16_dyn_ring_predicates::DynRingPoint2 triangle1,
        cc16_dyn_ring_predicates::DynRingPoint2 triangle2,
        cc16_dyn_ring_predicates::DynRingPoint2 point) = 0;
};

// 00C410E0..00C41187. EAX=walk start, EBX=triangle2; stack arguments are
// (owner, stop, triangle0, triangle1), RET 10h. Whole EAX is 1 when the walk
// reaches stop without an included point, otherwise 0. Original EBX is read
// unchanged; EBP/ESI/EDI are saved/restored. This is a new C++ interface, not
// that register/stack ABI. It writes no native owner/cursor/link/point data.
// Inline Win32 SSE/x87 comparisons use the actual ambient controls; the
// required barycentric implementation must honor the same context.
std::uint32_t triangle_ring_clear_00c410e0(
    const cc16_dyn_ring_owner::DynRingOwner& owner,
    const cc16_dyn_ring_predicates::DynRingTriangleQuery& query,
    DynRingQueryCalls& calls);

}  // namespace bsp::cc16_dyn_ring_query
