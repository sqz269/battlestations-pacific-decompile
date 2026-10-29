#pragma once

#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_ring_prepare {

// Exact native query inputs at 00C4D421. The stack owner is supplied separately.
struct TriangleQuery {
    std::uint32_t walk_start_eax;
    std::uint32_t triangle2_ebx;
    std::uint32_t stop_stack;
    std::uint32_t triangle0_stack;
    std::uint32_t triangle1_stack;
};

struct QueryProvider {
    void* context;
    // Required real 00C410E0 contract. Test the whole uint32 result for zero.
    // Honor/preserve actual ambient x87 controls and MXCSR, including the
    // barycentric dependency's context. No fixed-PC24/PC53 default is supplied.
    std::uint32_t (*triangle_ring_clear_00c410e0)(
        void* context, void* owner, const TriangleQuery& query);
};

// Complete 00C4D370..00C4D594 normal body, 549 bytes. Native ABI: ESI owner,
// no stack args, plain RET; EBX/EBP/EDI saved and ESI unchanged. Parent ignores
// EAX. Explicit C++ arguments do not supply that register/stack ABI.
//
// Raw owner: +04h anchor; +08h points-owner pointer, whose +00h is point data;
// +0Ch next vector data (cached once); +18h previous data; +24h/+28h/+2Ch
// scratch data/count/capacity; +30h/+34h next/previous cursors. Points are two
// raw float dwords. Valid owner storage must include these proven fields.
//
// Search has no added bound. Its exact x87 orientation instructions use
// ambient hardware controls and float spill epochs. After a successful query,
// publish +34h then +30h and append values from +34h, +04h, +30h in that order.
// Each append retains equality-only growth and callback-sensitive reloads.
// All storage shares the supplied native CRT custody; no fallback/query body,
// failure guard, cleanup or runtime activation is supplied by this packet.
void prepare_chain_00c4d370(void* owner,
    const cc15_dyn_live::DynLiveCrt& crt, const QueryProvider& query);

} // namespace bsp::cc16_dyn_ring_prepare
