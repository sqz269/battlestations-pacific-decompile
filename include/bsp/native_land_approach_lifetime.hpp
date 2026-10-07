#pragma once

#include "bsp/native_land_follow_observer_lifetime.hpp"
#include "bsp/native_land_state_lifetime.hpp"
#include "bsp/native_land_state_registry_lifetime.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {

// Borrow ONE actual approach with >=26Ch contiguous live backing. Every view
// aliases its indicated embedded member; no sidecars, copied identities,
// default state/profile, construction, ownership or lifetime extension.
struct NativeLandApproachCleanupView {
    void* actual_root;
    volatile std::uint32_t& profile_00;
    NativeBotStateRegistryStorage& registry_b8;
    NativeLandMoveToCleanupView moveto_cc;
    NativeLandStateCleanupView follow_108;
    NativeLandFollowObserverCleanupView follow_observer_120;
    // EXACT native cleanup order: +254, +228, +200, +1E0, +1C0, +1A0.
    std::array<NativeLandStateCleanupView, 6> trailing_states;
};

// PURE extent/address validation, without represented field reads, callbacks,
// native calls, allocation or translated caches. Mismatch is a SOURCE admission
// error, not a native fallback. These are actual shared-state/vector/prefix
// fields, never the unrelated 90h observer-node projection.
NativeLandApproachCleanupView native_land_approach_cleanup_view(
    void* actual_root, std::size_t actual_backing_bytes,
    volatile std::uint32_t& profile_00, NativeBotStateRegistryStorage& registry_b8,
    const NativeLandMoveToCleanupView& moveto_cc,
    const NativeLandStateCleanupView& follow_108,
    const NativeLandFollowObserverCleanupView& follow_observer_120,
    const std::array<NativeLandStateCleanupView, 6>& trailing_states);

// COMPLETE ordinary009B2C80..009B2D62: 226B/52 instructions, original ECX=root,
// RET. New SOURCE interface, not an original ABI bridge. Six shared cleanups,
// Follow observer then shared cleanup, MoveTo cleanup, registry cleanup, LAST
// raw D1FDB8 stamp. All ten natural calls use existing concrete complete bodies.
// Provider-final profiles/fields, dangling shared-vector headers and borrowed
// registry proxy/name/state pointees are retained. No receiver self-free.
//
// Require stable coherent actual nonnull approach/members, valid known CF5C94
// 24B vector elements/ranges and FIRST endpoint identities, genuine required
// NativeObserverLifetime manager/publications/recursive-lock/deletion context,
// and ordinary successful same-CRT allocations/free. Remaining state
// constructors are NOT invoked or supplied by this facade. No structural
// reentry, alias invalidation, concurrency, failure/overflow, null placement,
// fault/private EH, arena, dequeue/task death/base cleanup, observer lifetime,
// image class ABI or gameplay admission. Raw image profiles remain UNCALLABLE.
void destroy_native_land_approach_009b2c80(
    const NativeLandApproachCleanupView&, NativeObserverLifetime&);

} // namespace bsp
