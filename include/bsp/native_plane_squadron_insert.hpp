#pragma once

#include "bsp/native_land_plane_leader.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {

// Borrow genuine live cells on THIS actual squadron (>=3EDh stable backing).
// The five pointer slots are actual+3D0h, not a semantic squadron projection.
struct NativePlaneSquadronInsertView {
    const void* actual_squadron;
    volatile std::int32_t& count_3cc;
    const void* volatile* members_3d0;
    volatile std::uint8_t& dirty_3ec;
};

// Borrow THIS incoming plane's already-live +9D4h/+9D8h cells (>=9DCh).
// The SAME +9D8h cell is observed by the existing genuine leader provider.
struct NativePlaneSquadronInsertPlaneView {
    const void* actual_plane;
    const void* volatile& squadron_9d4;
    volatile std::int32_t& index_9d8;
};

// PURE address/admission checks only. Exact addresses, backing and alignment
// do not establish object lifetime: the caller supplies coherent live cells.
// No represented reads/callbacks/native calls/allocation/default/cache here.
// Invalid Source placement throws logic_error; native invalid paths unbound.
NativePlaneSquadronInsertView native_plane_squadron_insert_view(
    const void* actual_squadron, std::size_t actual_backing_bytes,
    volatile std::int32_t& actual_count_3cc,
    std::array<const void*, 5>& actual_members_3d0,
    volatile std::uint8_t& actual_dirty_3ec);
NativePlaneSquadronInsertPlaneView native_plane_squadron_insert_plane_view(
    const void* actual_plane, std::size_t actual_backing_bytes,
    const void* volatile& actual_squadron_9d4,
    volatile std::int32_t& actual_index_9d8);

class NativePlaneSquadronInsertViews : public NativeLandPlaneLeaderViews {
   public:
    // REQUIRED PURE mapping to the SAME incoming plane's coherent live cells.
    // Use the checked view above; no reads/effects/defaults/copied caches.
    // Inherited plane_leader_view maps each current member to its SAME actual
    // live +9D8h cell, without reading it. The operation then reads it fresh.
    virtual NativePlaneSquadronInsertPlaneView insert_plane_view(const void* actual_plane) = 0;
};

// COMPLETE007ED0D0..007ED151 (129B/42 instructions). Original ECX=squadron,
// stack(plane,signed index), RET8; this is a new Source interface, not image ABI.
// Dirty first INCLUDING null. Nonnull: plane link then index, captured count,
// fresh signed member-index scan (first strictly greater), backward shift,
// member publication, FRESH count increment. No allocation/free/reindex/drain.
// Ordinary nonnull domain: count0..4; live occupied planes; coherent, disjoint
// squadron/plane structural backing; stable storage and nonstructural schedule.
// Null needs only a coherent writable dirty cell and performs no other access.
// No capacity/duplicate guard or ownership policy. Invalid/fault/overflow,
// concurrency/reentry, arena/profile/lifetime/game binding remain unbound.
void native_plane_squadron_insert_sorted_007ed0d0(
    const NativePlaneSquadronInsertView&, const void* incoming_plane,
    std::int32_t requested_index, NativePlaneSquadronInsertViews&);

} // namespace bsp
