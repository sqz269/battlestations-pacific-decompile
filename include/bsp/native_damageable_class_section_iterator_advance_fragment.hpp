#pragma once
#include "bsp/native_damageable_class_section_iterator_setup_fragment.hpp"

namespace bsp {

// Only parent 0087CA80's 0087D1A0..0087D1CE tail: advance the actual Sections
// iterator and report whether its key is still bound. S is ESP after the
// parent's E4h locals/four saves. Sections S+80h, key S+58h and value S+2Ch
// are the SAME objects retained by this successful setup owner and scratch.
// The owner remains live in state13; all inner field/string/handle owners have
// already closed normally. The owner reference expresses this lifetime
// precondition; it does not inspect private state or establish scratch identity.
//
// Borrow stable, disjoint actual storage and unchanged scratch bindings. The
// existing key/value object lifetimes were begun by setup; never copy/reopen
// them or their tracked indices. Keep the pair exclusively owned, with no
// surviving reference to its detached working key. All existing Lua stack,
// tracking capacity, inherited error-handler and non-reentry contracts apply.
// In particular, the handler must remain below every removed/consumed slot.
//
// True corresponds to the excluded CE00 back edge. False leaves the existing
// pair owner live for its already established D1CF cleanup; this tail never
// closes it. Failure keeps completed releases/index shifts and discards the
// detached work slot through the genuine protected provider. It propagates to
// the existing ordinary enclosing owners; do not retry or restore the old pair.
//
// Ordinary MSVC Win32 Source only. No row/vector/cursor or effect/string work,
// new owner/guard, native loop/entrypoint or application binding is supplied.
// Native ABI/FH3/SEH/longjmp/fault identity, whole-parent and runtime proof remain
// held. Use inside the genuine enclosing ordinary C++ reader lifetime.
bool advance_native_damageable_class_section_iterator_0087d1a0(
    NativeDamageableClassSectionIteratorSetupFragment& live_iterator_owner,
    NativeDamageableClassSectionIteratorSetupFragmentScratch& actual_scratch);

} // namespace bsp
