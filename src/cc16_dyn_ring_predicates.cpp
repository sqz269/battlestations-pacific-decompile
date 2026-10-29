#include "bsp/cc16_dyn_ring_predicates.hpp"

#include "bsp/cc13_dyn1.hpp"

namespace bsp::cc16_dyn_ring_predicates {
namespace {

template <class T>
const T* at_index(const T* base, std::uint32_t index) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(base)
        + index * static_cast<std::uint32_t>(sizeof(T));
    return reinterpret_cast<const T*>(address);
}

const DynRingPoint2* points(const cc16_dyn_ring_owner::DynRingOwner& owner) noexcept {
    // MOV EAX,[ECX+8]; MOV EAX,[EAX]. The reused leaf view retains that
    // first owner dword as opaque_00; its use as this address is proven here.
    return reinterpret_cast<const DynRingPoint2*>(
        static_cast<std::uintptr_t>(owner.ring_00.owner_08->opaque_00));
}

bool counterclockwise(const DynRingPoint2* vertices, std::uint32_t first,
                      std::uint32_t second, std::uint32_t third) noexcept {
    const DynRingPoint2* const a = at_index(vertices, first);
    const DynRingPoint2* const b = at_index(vertices, second);
    const DynRingPoint2* const c = at_index(vertices, third);
    return cc13_dyn1::ccw_2d_00c377f0(
        a->x_00, a->y_04, b->x_00, b->y_04, c->x_00, c->y_04);
}

}  // namespace

std::uint32_t test_next_00c41190(
    const cc16_dyn_ring_owner::DynRingOwner& owner,
    DynRingPredicateCalls& calls) {
    const std::uint32_t first_cursor = owner.next_cursor_30;  // EDX retained across first CCW
    const DynRingPoint2* vertices = points(owner);
    const std::uint32_t next = *at_index(owner.ring_00.next_0c, first_cursor);
    const std::uint32_t previous = *at_index(owner.ring_00.previous_18, first_cursor);
    if (!counterclockwise(vertices, previous, first_cursor, next)) {  // 00C411C1
        return 0u;
    }

    const std::uint32_t previous_cursor = owner.previous_cursor_34;
    const std::uint32_t* const next_links = owner.ring_00.next_0c;  // EDI retained
    vertices = points(owner);
    const std::uint32_t next_of_first = *at_index(next_links, first_cursor);
    const std::uint32_t next_of_previous = *at_index(next_links, previous_cursor);
    if (!counterclockwise(vertices, next_of_first, previous_cursor,
                          next_of_previous)) {  // 00C411FB
        return 0u;
    }

    // 00C41209..00C4121B reload both cursors, retaining the cached next array.
    const std::uint32_t current_next = owner.next_cursor_30;
    const std::uint32_t current_previous = owner.previous_cursor_34;
    const std::uint32_t triangle2 = *at_index(next_links, current_next);
    const std::uint32_t walk_start = *at_index(next_links, triangle2);
    const DynRingTriangleQuery query{
        walk_start, triangle2, current_previous, current_previous, current_next};
    const std::uint32_t result = calls.triangle_ring_clear_00c410e0(owner, query);
    // NEG EAX; SBB EAX,EAX; NEG EAX consumes every bit, not just AL.
    return result != 0u ? 1u : 0u;
}

std::uint32_t test_previous_00c41230(
    const cc16_dyn_ring_owner::DynRingOwner& owner,
    DynRingPredicateCalls& calls) {
    const std::uint32_t first_cursor = owner.previous_cursor_34;  // EDX retained
    const DynRingPoint2* vertices = points(owner);
    const std::uint32_t* const next_links = owner.ring_00.next_0c;  // EBP retained throughout
    const std::uint32_t next = *at_index(next_links, first_cursor);
    const std::uint32_t previous = *at_index(owner.ring_00.previous_18, first_cursor);
    if (!counterclockwise(vertices, previous, first_cursor, next)) {  // 00C41263
        return 0u;
    }

    const std::uint32_t* const previous_links = owner.ring_00.previous_18;  // EDI retained
    const std::uint32_t previous_of_first = *at_index(previous_links, first_cursor);
    vertices = points(owner);
    const std::uint32_t next_cursor = owner.next_cursor_30;
    const std::uint32_t previous_of_next = *at_index(previous_links, next_cursor);
    if (!counterclockwise(vertices, previous_of_next, next_cursor,
                          previous_of_first)) {  // 00C4129D
        return 0u;
    }

    // 00C412AB..00C412BE uses both cached link arrays and fresh cursor words.
    const std::uint32_t current_previous = owner.previous_cursor_34;
    const std::uint32_t current_next = owner.next_cursor_30;
    const std::uint32_t stop = *at_index(previous_links, current_previous);
    const std::uint32_t walk_start = *at_index(next_links, current_next);
    const DynRingTriangleQuery query{
        walk_start, current_next, stop, stop, current_previous};
    const std::uint32_t result = calls.triangle_ring_clear_00c410e0(owner, query);
    return result != 0u ? 1u : 0u;
}

}  // namespace bsp::cc16_dyn_ring_predicates
