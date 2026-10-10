#pragma once
#include "bsp/native_damageable_class_section_iterator_setup_fragment.hpp"
#include "bsp/native_damageable_section_vector.hpp"

namespace bsp {

// Borrow the successful actual state13 iterator owner and its unchanged scratch.
// The preceding row-begin run completed normally and closed its temporary14.
// S is ESP after the parent's E4h locals/four saves. No new Lua owner is made.
struct NativeDamageableClassBackRowSelectionFragmentScratch {
    NativeDamageableClassSectionIteratorSetupFragment& live_iterator_owner;
    NativeDamageableClassSectionIteratorSetupFragmentScratch& actual_iterator_scratch;
    void* actual_header_at_parent_edi; // SAME retained descriptor+18h header
    void* actual_saved_end_at_parent_dc; // actual writable four-byte S+DCh slot
    const NativeDamageableSectionVectorAccess& actual_vector_access;
};

// Only 0087CE88..0087CEBA. Return the raw captured-end-derived row address after
// normal completion; do not dereference it or imply that callbacks repaired it.
// CEBB and all row fields/iteration remain excluded. No state or owner changes.
//
// Header+4/+8 are actual current unsigned32 begin/end words. The aligned header
// and S+DCh slot are stable, disjoint raw extents, also disjoint from Source
// locals, owners and bindings. No C++ pointer arithmetic or typed raw objects are
// required. Keep the SAME genuine returning invalid-parameter service/context
// supplied to the actual preceding vector append. All bindings are stable;
// callback mutation of header contents and S+DCh remains observable. No reentry,
// null service, no-op, replacement policy, callback cast or fabricated binding.
//
// Capture end once. Preserve unsigned wrapping subtraction, current-field read
// order, the saved end comparison BEFORE S+DCh publication and the conditional
// begin read. Derive the returned row only after the second callback returns;
// the third callback runs iff row >= then-current end. No retries or row reload.
// First callback failure precedes the fragment's slot write. Later failure
// preserves publication/provider writes; no cleanup, rollback or second store.
//
// Ordinary MSVC Win32 Source ABI only. Use inside the genuine enclosing ordinary
// C++ reader lifetime; its existing iterator/Damage/Unique owners handle failure.
// Actual application binding, Native register ABI/FH3/SEH/longjmp/fault identity,
// whole-parent composition and runtime evidence remain held.
void* select_native_damageable_class_back_row_0087ce88(
    NativeDamageableClassBackRowSelectionFragmentScratch& actual_scratch);

} // namespace bsp
