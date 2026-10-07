#pragma once

#include "bsp/native_land_state_entries.hpp"
#include "bsp/plane_squadron_host.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Borrow ONE genuine live int32 cell at this SAME actual plane+9D8h. This
// describes no enclosing plane class, constructor, profile, arena or lifetime.
// Signedness is immaterial to the native DWORD==0 test; every nonzero bit
// pattern (including -1) is false. No semantic index/membership translation.
struct NativeLandPlaneLeaderView {
    const void* actual_plane;
    const volatile std::int32_t& field_9d8;
};

// PURE address/admission checks only: nonnull, aligned actual plane, >=9DCh
// stable backing and exact already-live int32 cell at+9D8. No value reads,
// callbacks, native calls, allocation, defaults, caches or object construction.
// Invalid Source placement reports logic_error; native fault paths unbound.
NativeLandPlaneLeaderView native_land_plane_leader_view(const void* actual_plane,
    std::size_t actual_backing_bytes, const volatile std::int32_t& actual_field_9d8);

// COMPLETE007B8AD0..007B8ADC: ECX actual plane, EAX exactly0/1, plainRET.
// ONE actual volatile DWORD-zero read. New Source bool ABI, not an image entry.
// Admit coherent view and stable live reached storage; concurrent mutation,
// faults/structural reentry and observer/death-time ownership remain unbound.
bool native_land_plane_is_leader_007b8ad0(const NativeLandPlaneLeaderView&) noexcept;

class NativeLandPlaneLeaderViews {
   public:
    virtual ~NativeLandPlaneLeaderViews() = default;
    // REQUIRED PURE mapping of THIS supplied current plane identity to its
    // actual live field. No reads/callbacks/nativecalls/allocation/default/cache
    // or synchronized copied cells. Use the checked address-only view above.
    virtual NativeLandPlaneLeaderView plane_leader_view(const void* actual_plane) = 0;
};

// Connected opt-in adoption of the existing outer constructor/actual state
// entry/copy caller. Only its leader provider is closed here; every other
// complete base/composite/control/tuning/entry/observer service stays abstract.
// The caller reloads task3FC AFTER its embedded72C predicate before this call.
class NativeLandTaskPlaneLeaderConstructorCalls
    : public NativeLandTaskParameterCopyConstructorCalls, public NativeLandPlaneLeaderViews {
   public:
    using NativeLandTaskParameterCopyConstructorCalls::NativeLandTaskParameterCopyConstructorCalls;
    bool plane_is_leader_007b8ad0(const void* actual_plane) final;
};

// Connected adoption at the existing cruise caller's actual plane3FC probe.
// PURE canonical404 mapping and COMPLETE actual tuning singleton remain
// required. No tuning/default/controller snapshot or larger native ABI claim.
class NativeLandPlaneLeaderCruiseHost
    : public LandTaskCruiseProfileHost, public NativeLandPlaneLeaderViews {
   public:
    bool plane_is_leader_007b8ad0(const void* actual_plane) final;
};

} // namespace bsp
