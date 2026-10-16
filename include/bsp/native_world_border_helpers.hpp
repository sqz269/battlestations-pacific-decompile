#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native world border helpers require MSVC Win32.
#endif

namespace bsp {

// Complete 004C2310..004C2342 (51 bytes). Original: three actual stacked
// DWORD cells {next, previous, value-cell address}, EAX node, RET 0Ch.
// Allocate 0Ch bytes through the genuine current singleton allocator before
// reading those cells at their respective store sites. The three independent
// guards test node, uint32(node+4), uint32(node+8), respectively; no blanket
// null repair is added. The current allocator/new-handler/throw identity is a
// Source boundary, not Native private-CRT equivalence. Node ownership passes
// to the caller; singleton_lifetime_free releases a successfully returned node.
void* __stdcall allocate_native_world_border_node_004c2310(
    std::uint32_t next, std::uint32_t previous,
    const std::uint32_t* actual_value_cell);

// Complete Source normal/error behavior of 004CF450..004CF4E2 (147 bytes).
// Original: ECX actual 0Ch list header, stacked addition, RET 4; no semantic
// result. Capture header+8, reject iff uint32(3FFFFFFFh-count)<addition, else
// publish uint32(count+addition) only at +8. Error constructs and throws the
// genuine current-SDK std::length_error("list<T> too long"). Original legacy
// string/object allocation, FH3, RTTI/ThrowInfo, and private CRT are not claimed.
// Source-only unused_edx supplies the fastcall stack shape; Original consumes
// no EDX argument. No new header, node, payload, rollback or success stub.
void __fastcall increment_native_world_border_count_004cf450(
    void* actual_list_header, void* unused_edx, std::uint32_t addition);

// Complete 004B6C40..004B6C7A (59 bytes). Original: ECX actual raw game,
// stacked edge DWORD, ST0 float result, RET 4. Edge 0/2 reads +7128 minus
// +711C; EVERY other DWORD reads +7124 minus +7130. Each path uses the native
// FLD/FSUB/FSTP/FLD schedule and overwrites/reloads the actual edge stack cell
// as float. Uses the caller's x87 control word; status follows the native
// operations, with no control/status reset or added floating-point operation.
// Source-only unused_edx is ignored. Existing typed GameMapBox APIs remain
// separate. The borrowed receiver must provide those actual readable cells.
float __fastcall span_native_world_border_004b6c40(
    const void* actual_game, void* unused_edx, std::uint32_t edge);

} // namespace bsp
