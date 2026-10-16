#pragma once

#include <cstdint>

namespace bsp {

// Whole 004DC1C0 and 004DCA30: each 61 bytes / 21 instructions. Native ECX
// is the actual outer storage header, EDX is unconsumed, no stacked arguments,
// plain RET. Explicit ignored Source EDX slots retain that physical shape;
// they do not recover an original C++ class or mangled declaration.
void __fastcall tidy_native_dword_vector_outer_storage_004dc1c0(
    void* actual_header, std::uint32_t unused_edx);
void __fastcall tidy_native_dword_vector_outer_storage_004dca30(
    void* actual_header, std::uint32_t unused_edx);

// Capture initial begin at +4. Only when nonzero capture current end at +8,
// destroy that captured 16-byte-header range via actual 004D49B0, then reload
// CURRENT begin at +4 and unconditionally call the actual current-CRT free,
// including for a reloaded null value. Both ignored child metadata words have
// the original receiver bits, proven by entry PUSH ECX and the saved-word load.
// After either returning branch, clear +4, +8, +0Ch in order. No direct opaque
// +0 access, null-header/capacity guard, captured-begin free or rollback.
//
// The outer header must remain valid/writable through child/free/stores; its
// nonempty captured range and backing must satisfy the accepted raw destroy
// and current-CRT domains. Preserve progressive aliases and distinct captures.
// Source void excludes incidental native EAX: initial-null retains incoming
// bits; otherwise the final void-free residue remains. The two whole bodies
// have no local EH. Current C++/CRT and new Source interfaces do not establish
// original CRT, private frames/registers, FH3/SEH/fault behavior, drop-in ABI or
// game/runtime equivalence. Existing projections/library identities remain
// unchanged; descriptive names are hypotheses. Caller unwind scopes stay
// caller-owned; no public native EBP/frame action helper is introduced.

} // namespace bsp
