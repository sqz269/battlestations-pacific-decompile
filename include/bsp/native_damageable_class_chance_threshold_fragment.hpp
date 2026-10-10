#pragma once
#include "bsp/native_damageable_class_section_msh_field_fragment.hpp"

namespace bsp {

// Same actual enclosing 0087CA80 invocation; S is ESP after E4h locals/four saves.
struct NativeDamageableClassChanceThresholdFragmentScratch {
    NativeDamageableClassSectionMshFieldFragment& live_msh_owner;
    NativeDamageableClassSectionMshFieldFragmentScratch& same_msh_scratch;
    void* fresh_field_at_parent_6c; // aligned14h bytes, previous field already dead
    const char* actual_failure_chance_key_00d0ac88;
    const char* actual_failure_damage_threshold_key_00d0ac70;
};

// Only D0FB..D180: two adjacent state17->17 fields, not an original entrypoint.
// Borrow the actual captured ESI raw30h row, successful live Msh owner and SAME
// scratch/iterator value S2C. Earlier Index/Fire/fallback owners have normally
// restored17. Msh S18 and its saved captures remain live; do not close/reopen it.
// No new Lua owner, iterator accessor, row selection, effect or continuation API.
//
// The caller keeps the same row alive and writable through both fields; the
// earlier returning invalid-parameter callbacks do not establish row validity.
// Alignment is at least4. Row, S6C, outer Lua/string storage, actual key storage,
// bindings and private Source frames/captures must be disjoint and stable.
// Borrow the genuine live key spans with their retained identities; this adapter
// neither supplies replacement strings nor proves the original Native bytes.
// Existing Lua owner/index/tracking/capacity and error-handler-below-removed-slots
// contracts apply. Cleanup may shift indices; use the same actual S2C object.
// No callback reentry/private-frame alias or binding mutation. Preserve valid
// row/header callback changes and completed writes; do not repair or roll back.
//
// Stage retained defaults before23/24. Real exact-number-or yields the ordinary
// C++ float value (explicit ABI spills remain qualified). Chance FDIV uses a
// binary64 divisor and keeps its quotient in ST0 while lowering23->17 BEFORE
// raw FSTP+28. Threshold raw FSTP+2C runs under24, THEN lowers17. Destroy actual
// S6C after each lowering, without retry. The private inner guard never owns Msh.
// No typed float row subobject, C++/SSE division, reciprocal or quotient spill.
//
// Ordinary MSVC Win32 Source with the current x87 environment, no FP mode change.
// Native register/FH3/SEH/longjmp, precision/status/trap/fault/NaN identity, actual
// production binding, full parent and runtime proof remain held. Guard unwind
// uses noexcept destruction; secondary cleanup failure follows C++ termination.
void read_native_damageable_class_chance_threshold_fragment_0087d0fb(
    void* actual_selected_row,
    NativeDamageableClassChanceThresholdFragmentScratch& actual_scratch);

} // namespace bsp
