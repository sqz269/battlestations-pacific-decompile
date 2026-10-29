#include "bsp/cc16_dyn_ring_query.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error The Dyn ring query preserves ambient SSE/x87 instructions on MSVC Win32.
#endif

namespace bsp::cc16_dyn_ring_query {
namespace {

template <class T>
const T* at_index(const T* base, std::uint32_t index) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(base)
        + index * static_cast<std::uint32_t>(sizeof(T));
    return reinterpret_cast<const T*>(address);
}

// 00C41139..00C4115D. COMISS carry includes unordered; the x87 addition
// remains in a register for FCOMIP. Hardware uses the actual x87 CW/MXCSR,
// including precision, rounding, masks and denormal handling. No inferred
// fixed-PC arithmetic model or intermediate float store is introduced.
bool included_under_ambient_controls(float u, float v) noexcept {
    std::uint32_t included;
    __asm {
        xorps xmm0, xmm0
        movss xmm1, u
        comiss xmm1, xmm0
        jc point_outside
        movss xmm1, v
        comiss xmm1, xmm0
        jc point_outside
        fld u
        fadd v
        fld1
        fcomip st(0), st(1)
        fstp st(0)
        jnc point_inside
    point_outside:
        mov included, 0
        jmp comparison_done
    point_inside:
        mov included, 1
    comparison_done:
    }
    return included != 0u;
}

}  // namespace

std::uint32_t triangle_ring_clear_00c410e0(
    const cc16_dyn_ring_owner::DynRingOwner& owner,
    const cc16_dyn_ring_predicates::DynRingTriangleQuery& query,
    DynRingQueryCalls& calls) {
    // EBP captures triangle1 before the by-value stack argument is reused as
    // u_out by the original helper. Nothing writes the caller's query view.
    const std::uint32_t triangle1 = query.triangle1_stack_10;
    std::uint32_t cursor = query.walk_start_eax;
    if (cursor == query.stop_stack_08) {
        return 1u;  // No point/owner/link dereference or FP arithmetic.
    }

    // The native ESI point base is loaded once for the nonempty walk.
    const auto* const vertices = reinterpret_cast<
        const cc16_dyn_ring_predicates::DynRingPoint2*>(
            static_cast<std::uintptr_t>(owner.ring_00.owner_08->opaque_00));
    do {
        const auto point = *at_index(vertices, cursor);
        const auto third = *at_index(vertices, query.triangle2_ebx);
        const auto second = *at_index(vertices, triangle1);
        const auto first = *at_index(vertices, query.triangle0_stack_0c);
        const DynRingBarycentricWeights weights =
            calls.barycentric_00c37860(first, second, third, point);  // 00C41134
        if (included_under_ambient_controls(weights.u, weights.v)) {
            return 0u;
        }
        // 00C4115F..00C4116D reloads the next-array pointer on each advance.
        cursor = *at_index(owner.ring_00.next_0c, cursor);
    } while (cursor != query.stop_stack_08);
    return 1u;
}

}  // namespace bsp::cc16_dyn_ring_query
