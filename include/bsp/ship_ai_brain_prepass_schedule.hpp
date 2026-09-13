#pragma once
#include "bsp/ship_ai_neighbour_candidates.hpp"
#include "bsp/ship_ai_avoidance_request.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/robot_config.hpp"
#include "bsp/pose_refresh.hpp"
#include <array>

namespace bsp {
struct ShipAiBrainPrepassView {
    float& torpedo_period_b44;
    float& torpedo_countdown_b48;
    float& ship_period_b4c;
    float& ship_countdown_b50;
    TrackedCriticalSection* const& lock_04;
    ShipAiAvoidanceRequestBlock& avoidance;
};

// Inherits N's live unit/list/settings/admission boundaries. All field/identity
// lookups are pure, borrowed, and may not manufacture missing runtime owners.
class ShipAiBrainPrepassHost : public ShipAiNeighbourCandidateHost {
public:
    virtual std::int32_t unit_level_390(const void*) const = 0;
    virtual const NavigatorBotParameters& navigator_parameters(std::int32_t level) const = 0;
    virtual const void* world_list45_head_220() const = 0;
    virtual std::int32_t world_list45_count_21c() const = 0;
    virtual std::uint8_t unit_byte_5d(const void*) const = 0;
    virtual const void* unit_pointer_4f8(const void*) const = 0;

    // Native receiver is candidate+310. Verified MTorpedo bodies: slot38
    // 008561F0 reads shot+148; slot2C 00855F00 performs the actual pose/water
    // predicate and returns0/1/2. Preserve actual dispatch for other identities.
    virtual std::uint8_t shot_vtable_38(const void* candidate) = 0;
    virtual std::int32_t shot_vtable_2c(const void* candidate) = 0;
    // Primary slot34, RET4, EAX returned vector identity. Do not return a
    // copied value: the captured self vector is read AFTER the candidate call.
    // Ship00812090 uses body-axis speed; projectile006E2860 copies318/31C/320.
    virtual const float* velocity_vtable_34(const void*, std::array<float,3>& output) = 0;
    // Actual nav=brain+8 receiver, unchanged candidate stack pointer, RET4.
    // Complete callee owns128 capacity, duplicate lifetime and68h allocation.
    virtual void admit_torpedo_009f0ad0(const void* candidate) = 0;

    virtual void* special_head_f89ad8() const = 0;
    virtual PoseRefreshView& special_pose(void*) = 0;
    virtual void* special_next_340(void*) const = 0;
    virtual const void* helper_ac4() const = 0;
    // The producer-owned +4 byte is outside GameplayTuningSettings's projected
    // named fields. Resolve it from this captured settings owner; no default.
    virtual std::uint8_t settings_byte_04(const GameplayTuningSettings&) const = 0;
    // Real4-byte owner-pointer helper. Reads submarine mode/depth/role/control
    // owners and writes owner+122C; stack seconds is consumed but never read.
    virtual void update_helper_009db8f0(const void*, float seconds) = 0;
};

struct ShipAiBrainPrepassLiterals {
    const volatile double& hull_scale_00ceff98; // actual widened0.6f
    const volatile double& horizon_add_00d7a2b0; // actual3.0
};

// PARTIAL009F1420: full normal post-goal schedule009F158A..009F1B9F;
// excludes interleaved earlier-goal branch15B1..15BD and native SEH bookkeeping.
// Entry follows the existing goal refresh and its actual pre-stores. Updates
// both timers; captures/acquires actual optional lock only if either is due;
// retains it through torpedo/list6/special scans; releases captured lock before
// AC4 helper and avoidance publication. Pure lookups may not mutate storage.
// Original nodes/units/returned vectors/settings must outlive their native
// uses. No list snapshots, fallback candidates, seeds or default owners.
// C++ scan exceptions release the captured section like the verified unwind
// guard. This does not recreate original FH3/hardware-fault behavior or ABI.
void ship_ai_brain_prepass_after_goal_009f158a(ShipAiBrainPrepassView,
    float seconds, ShipAiBrainPrepassHost&, const ShipAiBrainPrepassLiterals&);
} // namespace bsp
