#pragma once

#include <cstdint>

namespace bsp {

// 006980E0 / STL_UninitCopy_WordVector16_006980e0. Actual ownership transfer:
// construct each output from a private empty header, then swap only +4/+8/+C.
// Original ECX=first, EDX=last, four stack words=output/owner/output-bits/tag,
// RET10h; EAX=advanced output. Owner is read only for a nonempty catch prefix
// and is unconsumed by actual 004D29D0. Slots 5/6 are never read by the core.
// Raw 32-bit addresses advance by wrapped 10h until equality; no validation.
// Source uses a new private frame/current C++ EH and accepted current CRT.
// Native frame aliases/FH3/SEH, CRT identity, binary ABI and game behavior are
// not established. Public +0 is not directly accessed; backing aliases retain
// the actual ordered field operations, not a blanket +0 immutability promise.
void* __fastcall transfer_native_dword_vector_range_006980e0(
    void* first, void* last, void* output, std::uint32_t unused_owner_bits,
    std::uint32_t unused_output_bits, std::uint32_t unused_tag);

// 00698350 / STL_TagDispatch_00698350: three stacked first/last/output inputs,
// RET0Ch. Returns the actual core EAX. Native incoming ECX supplies only dead
// owner/tag bits; Source uses canonical private zero bits, not an owner/frame
// identity. Duplicate output bits occupy the core's other dead stack slot.
void* __stdcall transfer_native_dword_vector_range_dispatch_00698350(
    void* first, void* last, void* output);

} // namespace bsp
