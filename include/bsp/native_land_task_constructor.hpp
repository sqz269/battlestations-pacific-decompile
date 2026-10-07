#pragma once
#include "bsp/native_pilot_bot_task_owner.hpp"

namespace bsp {

// Borrow ONE task's actual live cells, including the canonical404 identity
// used by the existing land hook/cruise/validity views. No owned storage,
// initial values, layout overlay, translated pointer cache or lifetime claim.
struct NativeLandTaskRetainedFields {
    const void* volatile& plane_3fc;
    const void* volatile& squadron_404;
    const void* volatile& block_424;
    const void* volatile& target_428;
    const void* volatile& queue_record_42c;
};

// REQUIRED actual NONNULL executable profile-table identities corresponding
// to these three native tables. No fabricated address constants/default tables
// are supplied. These identities are invariant through the caller and must
// support the actual receiver/profile/entry domain, not descriptive metadata.
struct NativeLandTaskExecutableProfiles {
    const void* task_00d1ffa0;
    const void* approach_00d1ff94;
    const void* registry_00d1ff90;
};

// All identities/cells belong to SAME caller-owned stable nonnull task.
// Subobject identities must denote the actual indicated task members. This
// SOURCE view is not the native670h layout or an arena/factory adapter. A task
// handle cannot be reused while any active/retired reference can reach it.
struct NativeLandTaskConstructorView {
    NativePilotBotTaskHandle task;
    const void* approach_3f8;
    const void* registry_4b0;
    const void* command_004;
    const void* gun_314;
    const void* control_38c;
    const void* moveto_4c4;
    const void* follow_500;
    const void* park_620;
    NativeLandTaskRetainedFields retained;
    const void* volatile& task_profile_000;
    const void* volatile& approach_profile_3f8;
    const void* volatile& registry_profile_4b0;
    const void* volatile& current_state_310;
    const void* volatile& command_alias_410;
    const void* volatile& gun_alias_414;
    const void* volatile& control_alias_418;
};

class NativeLandTaskConstructorCalls {
   public:
    virtual ~NativeLandTaskConstructorCalls() = default;
    // REQUIRED COMPLETE actual base on this SAME task, owner and kind3.
    // Command/gun/control construction, random/descriptor fields and normal
    // side effects remain required; a sparse/zeroed BotTaskRecord is excluded.
    virtual void construct_base_0099c6f0(const NativeLandTaskConstructorView&,
        const void* owner, std::uint32_t kind) = 0;
    // Faithful actual raw owner+50h read at this point AFTER the base call.
    // No invented callback, default plane or current-membership substitution.
    virtual const void* owner_plane_50(const void* owner) = 0;
    // REQUIRED COMPLETE009B2E50 on SAME task.approach_3f8, captured plane and
    // original block. Includes full009AFE70/009F9CE0, COMPLETE006C0B50 record
    // production, registry, actual states/profiles/observer/name work. Publish
    // the retained fields through their SAME actual cells; no approximation.
    virtual void construct_composite_009b2e50(const NativeLandTaskConstructorView&,
        const void* plane, const void* original_block) = 0;
    // Execute actual virtual+38h with ECX=the captured PLANE's EMBEDDED+72Ch
    // object; AL nonzero is true. This is not a loaded controller-pointer field.
    virtual bool plane_embedded_72c_predicate_38(const void* plane) = 0;
    // REQUIRED actual007B8AD0, observing this fresh plane's+9D8h==0 in AL.
    virtual bool plane_is_leader_007b8ad0(const void* plane) = 0;
    // Execute actual selected-state profile+4 on this live state. Follow/park
    // entries have real work; only proved empty MoveTo entry is empty. No
    // default/no-op dispatch. Alias binding has not yet run in the caller.
    virtual void enter_state_04(const void* state) = 0;
};

// Complete ordinary009B3240 caller: nativeECXtask, stackowner/block, EAXtask,
// RET8. New SOURCE API; required base/composite/profile/entry services remain
// unbound. Capture control-plane before ordered profile stores; refresh3FC for
// leader selection; publish310 before entry; bind aliases AFTER entry.
// Admit valid nonnull task/owner/plane/block/subobjects/profiles, live inputs and
// coherent cells through normal returns. Providers may mutate field values,
// but cannot invalidate identities/cells, structurally reenter this caller or
// synthesize raw-read/profile effects. No allocation/privateEH/fault/concurrent
// mutation/observer-death lifetime/originalABI/gameplay guarantee is supplied.
NativePilotBotTaskHandle construct_native_land_task_009b3240(
    const NativeLandTaskConstructorView&, const void* owner,
    const void* original_block, NativeLandTaskExecutableProfiles,
    NativeLandTaskConstructorCalls&);

// Complete NONNULL task domain of009F9980: nativeECXapproach, stacktask,RET4.
// Bind actual command4/gun314/control38C identities into approach18/1C/20
// (task410/414/418), in that order. No ownership or initial values. Native
// NULL helper behavior (0,314h,38Ch address words) is outside this API domain.
void bind_native_land_approach_to_nonnull_task_009f9980(
    const NativeLandTaskConstructorView&) noexcept;

} // namespace bsp
