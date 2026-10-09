#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native allocator base default construction requires MSVC Win32.
#endif

namespace bsp {

// Complete 00BF632F..00BF633F schedule: 17 Original bytes / five operations.
// Naked fastcall binds the actual receiver to ECX and an unused incoming DWORD
// to EDX. EAX returns that receiver; ECX and EDX remain unchanged. Plain RET
// consumes only the return address; there are no stack arguments or children.
//
// Caller supplies actual leading 12-byte backing: +4 and +8 are readable and
// writable DWORDs, and +0 is writable. Clear +4, then +8, with actual DWORD
// read-modify-write AND operations before publishing raw 00D69370h at +0.
// No prior profile read, message-pointee access, prior free or null guard.
// On normal return the second AND leaves CF/OF/SF=0, ZF/PF=1, AF undefined;
// the following MOV and RET preserve those flags. No LOCK/atomicity promise.
//
// This raw boundary has no noexcept promise. Backing validity and lifetime
// remain caller contracts. The profile is opaque numerical data, creating no
// callable vtable, Native type, RTTI, static owner or slot. Original placement,
// caller ABI, fault/exception/runtime and gameplay parity remain unproved.
void* __fastcall construct_native_allocator_base_default_00bf632f(
    void* actual_receiver,
    std::uint32_t unused_edx);

} // namespace bsp
