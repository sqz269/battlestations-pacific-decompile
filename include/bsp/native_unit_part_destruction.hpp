#pragma once
#include "bsp/native_spatial_lifecycle.hpp"
#include "bsp/native_unit_part_entries.hpp"
#include "bsp/native_unit_lifecycle_services.hpp"
#include "bsp/point_effect_controls.hpp"

namespace bsp {
// Required actual owners and complete service bindings. Point pointers in an
// entry's +4 array name existing PointEffectInstanceStorage objects. The selected
// set callback dispatches its current native slot 0; it must not invent a target.
// All access records, referenced cells and borrowed views survive the operation.
struct NativeUnitPartDestructionAccess {
    const NativeSpatialLifecycleAccess& spatial;
    NativeUnitPartSelectedSetCallbacks selected_set;
    NativeControlledListenerPublication listener;
    NativeControlledListenerRenderer& listener_renderer;
    PointEffectChildEvents& effects;
    EffectManager* volatile& effect_manager_00f87650;
    EffectManagerLifetimeAccess& effect_lifetime;
};

// Complete physical body 7112E0..71136B, 140 bytes. ECX=10h entry, RET; source
// EDX adds access. Stop every current nonnull effect, set captured effect+9=1,
// then clear the current array slot. Counts and backing reload at native points.
// Finally resize0/free its +4 pointer array, retaining dangling data/capacity.
// Does not free the entry or the pointed effects. On a source exception during
// the stop loop, the sole native member unwind destroys the pointer array.
void __fastcall destroy_native_unit_part_entry_007112e0(void* actual_entry,
    const NativeUnitPartDestructionAccess*);

// Complete physical body 712C80..712F18, 665 bytes. ECX=1ACh part, RET; source
// EDX adds access. Detach only if owner+164 and flag+184 are both nonzero;
// clear F8, clear matching controlled listener, release selected set, destroy
// entries, lists, groups, any republished selected set, then the collision base.
// Does not free the part allocation. Source exceptions use the seven native
// member-unwind states; a throwing cleanup during unwinding terminates.
void __fastcall destroy_native_unit_part_00712c80(void* actual_part,
    const NativeUnitPartDestructionAccess*);

// Complete 712FD0..712FED, 30 bytes. ECX=part, stacked flags, EAX=original part,
// RET4; source EDX adds access. Run the complete destructor before testing bit0
// and optionally calling the actual CRT free. An escaping destructor exception
// leaves the allocation to the caller, as in the original wrapper.
void* __fastcall delete_native_unit_part_00712fd0(void* actual_part,
    const NativeUnitPartDestructionAccess*, std::uint32_t flags);

// Reuse the canonical B1C500/B1C770/B1D1D0 pointer-array bodies for the physically
// equivalent 7102A0/710730/711120 specialization. Their valid-storage and signed
// representable-allocation contract applies. These interfaces do not establish
// native FH3/fault delivery, private native stack aliases or executable admission.
} // namespace bsp
