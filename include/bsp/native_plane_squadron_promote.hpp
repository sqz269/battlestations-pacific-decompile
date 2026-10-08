#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {

struct NativeSessionMessageBE;

// Borrow THIS actual squadron's live cells; no compact semantic entity/cache.
struct NativePlaneSquadronPromotionView {
    void* actual_squadron;
    volatile std::int32_t& count_3cc;
    const void* volatile* members_3d0;
};

// PURE placement/admission checks, not a lifetime or native-profile proof.
// Actual stable backing >=3E4h, aligned count+3CC and five pointer cells+3D0.
// No represented reads, callbacks, native calls, allocation or defaults on
// valid placement. Invalid Source placement throws; native faults unbound.
NativePlaneSquadronPromotionView native_plane_squadron_promotion_view(
    void* actual_squadron, std::size_t actual_backing_bytes,
    volatile std::int32_t& actual_count_3cc,
    std::array<const void*, 5>& actual_members_3d0);

// COMPLETE007ED610..007ED64F, 63B/20 instructions. Native ECX=squadron,
// stack signed index, RET4; this interface is new Source ABI. Reject <=0 BEFORE
// observing count; reject count<=index; capture, shift backward, publish first,
// then DIRECT whole raw007ED260 on the SAME actual receiver. No capacity,
// duplicate, null-member, allocation, ownership or rotation/reindex callback.
// Ordinary stable count0..5 and old indices0..4; occupied actual planes have
// genuine writable +9D0/+9D8 cells within >=9DCh backing. No structural mutation.
// Count5 additionally admits raw reindex's physical readable caller-stack scan
// through its first nonnegative candidate. New Source caller words/return are
// distinct from original saved-EDI/BE-message frames; no equal frame claim.
// Placement checks do not establish lifetimes. Profiles, private EH/faults,
// concurrency, class ABI, full construction/world/game binding remain unbound.
void native_plane_squadron_promote_007ed610(
    const NativePlaneSquadronPromotionView&, std::int32_t index);

// ONLY already-selected BE arm007F0077..007F008C. Borrow genuine live message
// +1C payload; deferred delivery/selection to SAME squadron already completed.
// Return true even for rejected indices. No full dispatcher/profile/routing,
// sender/queue/retention or synchronous request->promotion binding.
bool native_plane_squadron_receive_leader_promotion_be_007f0077(
    const NativePlaneSquadronPromotionView&, const volatile NativeSessionMessageBE&);

} // namespace bsp
