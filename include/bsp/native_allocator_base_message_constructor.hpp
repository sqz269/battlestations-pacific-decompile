#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native allocator base-message construction requires MSVC Win32.
#endif

namespace bsp {

// Complete 00BF638E..00BF63A5 leaf, 24 bytes / 7 instructions. The descriptive
// Source name is provisional; the saved Native library name remains exception.
// Explicit Source fastcall binding: ECX actual receiver, EDX unused raw word,
// stack actual slot address then ignored numeric word, RET8, EAX original receiver.
// EDX is neither read nor written by the body. Other Native incoming registers
// are not additional parameters. This declaration is not an exception-class ABI.
//
// Caller supplies actual live backing for 12 receiver bytes and a readable
// four-byte pointer slot. Capture its ADDRESS before publishing raw 00D69370h
// at receiver+0; read the slot VALUE afterward; AND DWORD receiver+8 with zero
// (a real read-modify-write); finally store the captured word at receiver+4.
// The three DWORD stores cover all 12 bytes. Preserve alias and fault order:
// receiver+8 must be readable as well as writable; no receiver/slot null guard.
// The copied word can be null; its pointee is never read. The second stack value
// is not read. No string copy, allocation, larger owner cast or cleanup is added.
// The profile word remains raw data, not a callable Source vtable. Actual owner,
// slot/pointee lifetime, ownership and Native exception/runtime identity remain
// separate caller contracts; earlier stores can remain if a later access faults.
void* __fastcall construct_native_allocator_base_message_00bf638e(
    void* actual_receiver,
    std::uint32_t unused_edx,
    const void* actual_pointer_slot_address,
    std::uint32_t ignored_numeric);

} // namespace bsp
