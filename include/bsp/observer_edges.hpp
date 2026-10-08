#pragma once

#include "bsp/observer_lifetime.hpp"
#include <cstdint>

namespace bsp {

// Recovered constructor table; its slot zero is 00693CA0. The immediately
// following table words belong to separate singleton classes, not this edge.
inline constexpr std::uint32_t kObserverEdgeVtable00cf7e64 = 0x00cf7e64;

// Native ECX=first actual endpoint, EDX=actual callback owner; EAX=new edge;
// RET. Allocate16 and initialize {CF7E64, first, owner, 1}; append the same
// pointer first to first.edges_04, then owner.edges_04. Each full array grows
// 2*capacity+2, storing capacity before allocation. No registration rollback
// or edge destruction is inferred on an allocation exception.
NativeObserverEdgeStorage* create_observer_edge_00694850(
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner,
    NativeObserverLifetime&);

// Native ECX=first, EDX=callback owner; RET, no promised EAX result. Capture
// outer shared lock, nested find_pair, then nested create if absent or modulo32
// increment of the existing edge count. Existing registrations keep both arrays.
void register_observer_pair_00694a60(
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner,
    NativeObserverLifetime&);

// Complete normal-return 00694AF0 query. Native ECX=first, EDX=callback owner,
// AL=0/1, RET; upper EAX is not a boolean contract. Capture an outer actual
// section, then use the existing lookup's independent nested lock. Evaluate
// presence before releasing the captured outer section; do not change edges.
// Borrow the existing actual endpoint/array/edge and lifetime admissions.
// A nonnull section must be a real initialized TrackedCriticalSection; the
// native null-section path is retained. This explicit-lifetime C++ interface
// does not reproduce the original entry ABI, private FH3/SEH, asynchronous
// faults, concurrent teardown, enclosing class/world ownership or gameplay.
bool observer_pair_registered_00694af0(
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& callback_owner,
    NativeObserverLifetime&);

// Native scalar-deleting wrapper: ECX=edge, DWORD flags on stack, RET4,
// EAX=original edge address even after free. Rewrite CF7E64; free iff flags&1.
// It does not decrement references or unregister endpoints; callers have
// already performed those operations. Uses the canonical CRT free boundary.
NativeObserverEdgeStorage* delete_observer_edge_00693ca0(
    NativeObserverEdgeStorage*, std::uint32_t flags) noexcept;

} // namespace bsp
