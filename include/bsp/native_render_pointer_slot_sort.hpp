#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pointer-slot sorting requires MSVC Win32.
#endif

namespace bsp {

// Native comparator calls use ECX/EDX actual entry addresses and test AL only.
// Upper return bits have no comparison meaning. A supplied comparator may
// propagate failure; no noexcept policy is added to the native callable edge.
using NativeRenderPointerSlotComparator = std::uint32_t (__fastcall*)(
    const void* actual_left_entry, const void* actual_right_entry);

// Complete 00B51B00: unsigned high DWORD at entry+24h, then low DWORD at+20h
// only on a tie. Native EAX is exactly 0 or 1. No semantic entry overlay.
std::uint32_t __fastcall native_render_entry_unsigned_key_less_00b51b00(
    const void* actual_left_entry, const void* actual_right_entry) noexcept;

// Complete 00B1DCE0 and its nine pointer-slot helpers. Original ECX first slot,
// EDX one-past-last; stack signed ideal and comparator; RET8. This sorter has a
// new C++ entry interface; only the comparator retains its register-call shape.
//
// Mutates the caller's actual four-byte pointer cells, without allocation,
// entry ownership changes, copied containers or semantic entry/section access.
// Native signed32 pointer-distance and ideal behavior is retained. No bounds,
// null-entry, finite-value or comparator preflight is added. Native readable/
// writable storage and valid comparator behavior remain caller preconditions.
void sort_native_render_pointer_slots_00b1dce0(void* actual_first_slot,
    void* actual_one_past_last_slot, std::int32_t ideal,
    NativeRenderPointerSlotComparator comparator);

} // namespace bsp
