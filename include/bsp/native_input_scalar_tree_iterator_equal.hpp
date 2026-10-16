#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scalar-tree iterator equality requires MSVC Win32.
#endif

namespace bsp {
// Whole00553070..00553098: 41B/18 instructions. Native ECX left iterator,
// incoming EDX unconsumed, one stacked right pointer, RET4. Both point at
// actual 8B storage (owner0,node4); no new carrier or container is supplied.
// Captured left.owner zero skips right.owner; otherwise compare CURRENT
// right.owner. Actual BF6713 may return; then read CURRENT left.node before
// CURRENT right.node without rechecking owners or repairing either iterator.
//
// Naked Source retains the exact register/stack schedule: AL is equality,
// upper EAX retains CURRENT left.node high24bits, ECX is 0/1, ESI/EDI are
// restored and returned arithmetic flags are the final node CMP flags.
// No owned stores, cleanup, allocator, payload, model, context or guard.
// Child/provider progress and alias effects remain inherited.
//
// The direct published BF6713 Source entry uses CURRENT SDK noinfo policy,
// not Original BF66EF encoded-handler/CRT closure. Valid actual storage,
// caller stack/DF, SDK nonvolatile and failure contracts remain required.
// Original addresses/private frames/fault PCs/FH3/SEH/heap/ABI identity and
// runtime/game behavior remain unproved. Existing Ghidra identity and stored
// prototype are preserved; this explicit API is not applied to Ghidra.
bool __fastcall equal_native_input_scalar_tree_iterators_00553070(
    void* actual_left, void* unused_edx, const void* actual_right);
} // namespace bsp
