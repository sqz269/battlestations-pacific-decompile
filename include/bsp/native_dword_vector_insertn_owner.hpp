#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD-vector header insertion requires MSVC Win32.
#endif

namespace bsp {

// Whole ordinary 00698400 (559B/220 starts), lexical catches 0069856E
// (34B/12) and 006985F7 (35B/13), logical action C7EC60 (8B/2).
// Native ECX receiver, EDX unused, four stack words: unused iterator owner,
// position, unsigned count, fill-header pointer. RET10h, no semantic result;
// the actual sole caller discards EAX. This Source retains that physical
// consumed-input shape, not original C++ mangling/private frames/binary ABI.
void __fastcall insert_native_dword_vector_header_count_00698400(
    void* receiver, std::uint32_t unused_edx,
    std::uint32_t unused_iterator_owner_bits, void* position,
    std::uint32_t count, const void* fill_header);

// Construct an owning private fill copy BEFORE receiver reads and count0.
// All subsequent CurrentCpp failures tidy its CURRENT header through actual
// 004CDCE0. Normal exit only frees captured private begin, without clear
// stores. Cleanup stays armed until that free returns.
//
// Growth transfers prefix, constructs inserted copies, then transfers suffix.
// Completion used by the growth catch is POST-FILL, not suffix end. That
// catch remains active through old-range destruction, fresh CURRENT old-begin
// free, cap/end/begin publication AND common normal private-begin free.
// Catch destroys [allocated_base,completion), frees allocated_base, rethrows;
// outer cleanup then tidies the CURRENT private header. No rollback added.
//
// Short spare-tail insertion protects counted fill and end publication with
// its actual CURRENT-end range catch; reset to outer-only scope BEFORE
// assigned fill. Long spare-tail insertion stays in outer-only scope and
// publishes the ACTUAL transfer return before backward swap/assigned fill.
//
// Raw public DWORD reads/stores, repeated CURRENT reloads and wrapping/SAR
// arithmetic preserve the native schedules. No header/count/container bridge,
// validation, null/capacity guard, snapshot, callback or fake EBP helper.
// Unconsumed private tag bits are canonical0; opaque private+0 is uninitialized
// and unowned. Source private argument/frame aliases and asynchronous mutation
// are excluded. Actual current providers retain their noexcept boundaries.
// Original CRT/heap/newhandler/RTTI/FH3/SEH/fault/register/flags/ABI and game
// behavior remain unproved. Existing library names and earlier domains stay.

} // namespace bsp
