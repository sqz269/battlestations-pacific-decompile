#pragma once

#include "bsp/native_land_follow_observer_lifetime.hpp"
#include "bsp/native_land_state_lifetime.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Borrow the SAME actual Follow root and its existing embedded views. The
// callback18 view's FIRST endpoint14 is the actual watched2C cell. No native
// constructor, arena, executable profile, translated pointer or lifetime owner.
struct NativeLandFollowStateCleanupView {
    void* actual_root;
    NativeLandStateCleanupView shared;
    NativeLandFollowObserverCleanupView observer;
};

// PURE address/extent admission and reference composition from existing views:
// no represented field reads, callbacks, native calls, allocation or defaults.
// All supplied fields must alias this SAME live actual root at their evidenced
// offsets. The real receiver has >=98h backing (entry writes94; composite
// Follow108 is followed by state1A0). The30h cleanup prefix is NOT a whole class.
// A mismatched/null/undersized view raises a SOURCE admission error, not a
// recovery path. Direct aggregate callers must satisfy the same contract.
NativeLandFollowStateCleanupView native_land_follow_state_cleanup_view(
    void* actual_root, std::size_t actual_backing_bytes,
    const NativeLandStateCleanupView&, const NativeLandFollowEntryView&);

// COMPLETE ordinary007B6630..007B667C, ECX=Follow root, RET. Invokes genuine
// callback18 destroy006CDD70, THEN shared root destroy007B45F0 on SAME storage.
// FINAL fields/profiles come from those complete providers; do not restore a
// derived profile, clear2C or reset dangling callback/vector headers.
void destroy_native_land_follow_state_007b6630(
    const NativeLandFollowStateCleanupView&, NativeObserverLifetime&);

// COMPLETE scalar009C2A60..009C2AC0, ECX=root, stackedDWORDflags, RET4,
// EAX=original root. Native body inlines the same two cleanups, THEN tests LOW
// bytebit0, optionally frees ROOT through canonicalCRT and returns its identity.
// New C++ SOURCE interface/flag encoding, NOT original ABI/profile replacement.
// Flags1 requires a SEPARATE complete actual same-CRT Follow receiver>=98h,
// NEVER embedded task500, callback18, vector slot or whole task. No field/view
// access after self-free. Member scalar006CDDF0(flags1) is NOT used here.
void* scalar_delete_native_land_follow_state_009c2a60(
    const NativeLandFollowStateCleanupView&, std::uint32_t flags,
    NativeObserverLifetime&);

// All inherited real storage/vector/CF5C94/observer/CRT ordinary-success
// contracts remain required. Borrow actual manager/current lock/dispatch/edge
// services and endpoints; no fake/default observer world. Objects survive both
// cleanup segments, with coherent nonwrapping storage and no structural reentry,
// aliased view mutation, concurrency, fault/privateEH or lifetime guarantee.
// D20AB8 slot0->9C2A60 is raw UNCALLABLE profile evidence. Known7B6630 direct
// callers are private-unwind funclets: reachability only, not an EH binding.
// Full constructors/state entries/approach9B2C80/arena/death/game ABI stay unbound.

} // namespace bsp
