#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native allocator base cleanup requires MSVC Win32.
#endif

namespace bsp {

// Complete physical schedule 00BF6454..00BF6469: 22 Original bytes / 7 ops,
// including the POP ECX omitted by the accepted visible Ghidra listing.
// Naked fastcall binds ECX receiver and an unused incoming EDX word; there
// are no stack arguments and the body uses plain RET. No EAX result is imposed.
// The current Source CRT child may clobber volatile registers and flags.
//
// Caller supplies actual leading 12-byte backing: receiver+8 is readable,
// receiver+0 is writable, and receiver+4 is readable on nonzero control.
// Compare the full DWORD at+8 BEFORE publishing raw 00D69370h at+0. If nonzero,
// load the CURRENT pointer at+4, call current Source CRT free with that value,
// then POP the argument into ECX. No receiver/message null guard or field reset.
// Zero control returns with ECX still the receiver; normal child return leaves
// ECX equal to the pushed message. No local message/control stores are added.
//
// This raw boundary has no noexcept promise. Actual backing/pointee ownership,
// lifetime and CRT failure policy remain caller/provider contracts. Raw profile
// data creates no callable vtable, exception type, RTTI, static owner or slot.
// Original placement, child ABI/identity, flags/faults, exception/runtime and
// gameplay equivalence remain unproved; this is a qualified Source interface.
void __fastcall cleanup_native_allocator_base_00bf6454(
    void* actual_receiver,
    std::uint32_t unused_edx);

} // namespace bsp
