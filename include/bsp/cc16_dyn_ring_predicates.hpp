#pragma once

#include "bsp/cc16_dyn_ring_owner.hpp"

#include <cstdint>

namespace bsp::cc16_dyn_ring_predicates {

// The input owner's first dword addresses packed float pairs. Both predicates
// load those two raw words as the six float arguments to 00C377F0.
struct DynRingPoint2 {
    float x_00;
    float y_04;
};
static_assert(sizeof(DynRingPoint2) == 8);

// Exact register/stack inputs to the still-required 00C410E0 body. Its native
// four stack arguments are (owner, stop, triangle0, triangle1), RET 10h; EAX
// supplies the walk start and EBX supplies triangle2. No guessed native fields
// or default body are introduced by this query view.
struct DynRingTriangleQuery {
    std::uint32_t walk_start_eax;
    std::uint32_t triangle2_ebx;
    std::uint32_t stop_stack_08;
    std::uint32_t triangle0_stack_0c;
    std::uint32_t triangle1_stack_10;
};

struct DynRingPredicateCalls {
    virtual ~DynRingPredicateCalls() = default;
    virtual std::uint32_t triangle_ring_clear_00c410e0(
        const cc16_dyn_ring_owner::DynRingOwner& owner,
        const DynRingTriangleQuery& query) = 0;
};

// 00C41190..00C4122A / 00C41230..00C412CD: ECX = owner, no stack
// arguments, plain RET. The original full EAX result is 0 or 1. The source
// returns uint32 explicitly; it does not expose a bool/AL ABI or a binary
// adapter. Neither predicate writes native cursor, link, owner or point data.
// Orientation calls compose the exact ambient-x87 00C377F0 variant with the
// same six float words. No precision/rounding/MXCSR mode is selected here.
std::uint32_t test_next_00c41190(
    const cc16_dyn_ring_owner::DynRingOwner& owner,
    DynRingPredicateCalls& calls);

std::uint32_t test_previous_00c41230(
    const cc16_dyn_ring_owner::DynRingOwner& owner,
    DynRingPredicateCalls& calls);

}  // namespace bsp::cc16_dyn_ring_predicates
