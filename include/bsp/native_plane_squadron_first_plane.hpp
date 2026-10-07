#pragma once

#include <cstddef>

namespace bsp {

// Borrow ONE genuine live nullable pointer cell at this SAME squadron+3D0h.
// Never retain a copied pointer in the view or substitute a semantic member
// list/count. This establishes no enclosing class, profile, arena or lifetime.
struct NativePlaneSquadronFirstPlaneView {
    const void* actual_squadron;
    const void* const volatile& first_plane_3d0;
};

// PURE placement/admission checks only: nonnull aligned root, >=3D4h stable
// backing and exact already-live pointer cell. No represented reads/effects,
// native calls/callbacks/allocation/default/profile or translated caches.
// Caller supplies coherent LIVE storage; address checks cannot prove lifetime.
// Invalid Source placement throws logic_error; native fault paths unbound.
NativePlaneSquadronFirstPlaneView native_plane_squadron_first_plane_view(
    const void* actual_squadron, std::size_t actual_backing_bytes,
    const void* const volatile& actual_first_plane_3d0);

// COMPLETE007ED010..007ED017: ECX=squadron; EAX=[ECX+3D0h]; plain RET.
// ONE actual volatile pointer read, including null or a retained pointer when
// count is zero. No count guard, projection, callback or profile dispatch.
// New Source interface, not the original register/class ABI. Stable coherent
// live cell required; returned identity's lifetime is caller-owned separately.
// Faults/concurrency/structural reentry/arena/world/game bindings unbound.
const void* native_plane_squadron_first_plane_007ed010(
    const NativePlaneSquadronFirstPlaneView&) noexcept;

} // namespace bsp
