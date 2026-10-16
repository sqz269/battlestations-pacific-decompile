#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native traffic DWORD insertion requires MSVC Win32.
#endif

namespace bsp {

// Whole 00493120..004932D3,436B/181 instructions. Original physical inputs:
// ECX raw16B borrowed header; actual stack+4 opaque preimage,+8 position,
// +0C count,+10 value-cell address. The first word is not directly read.
// No Original EDX input. Source-only unused EDX reserves both fastcall register
// slots so all FOUR actual stacked DWORDs and RET10 are represented faithfully.
//
// Before checking count, dereference the value-cell and replace actual stack
// +10 with its DWORD contents. In-place paths replace actual stack+0C with
// wrapped count*4. All fills borrow the actual overwritten stack+10 cell;
// aliases can observe both writes. No local snapshot/frame narrowing/RAII.
// raw_header+4/+8/+0C are actual begin/end/capacity words; word0 is opaque.
//
// This physical Source interface exposes observed EAX, not a recovered
// Original semantic return type/class: count0 leaves the original value-cell
// address; allocation returns published new end; in-place paths return their
// last fill endpoint. All paths use RET10. Child/current SDK allocator, copy,
// free and owning exception boundaries remain qualified, with no Native
// private CRT/FH3/RTTI, full exception-frame/fault/flags ABI or game claim.
void* __fastcall insert_native_traffic_dwords_00493120(
    void* raw_header, std::uint32_t source_unused_edx,
    std::uint32_t opaque_first_word, void* position,
    std::uint32_t count, const void* value_cell);

} // namespace bsp
