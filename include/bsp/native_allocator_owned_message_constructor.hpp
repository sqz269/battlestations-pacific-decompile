#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native allocator owned-message construction requires MSVC Win32.
#endif

namespace bsp {

// Complete 00BF6340..00BF638D: 78 Original bytes / 33 operations. This Source
// name is descriptive; the saved Native library name remains exception.
// Naked fastcall binds ECX receiver, unused incoming EDX, then the actual slot
// address as one stack word. Return the receiver in EAX and finish with RET4.
// Current CRT children may clobber volatile registers and flags.
//
// Caller supplies actual leading 12-byte backing and a readable DWORD slot.
// Capture the slot ADDRESS before publishing raw 00D69370h at receiver+0;
// read its VALUE afterward. For a nonnull value, retain the strlen argument,
// INC its 32-bit length, call nullable malloc, and publish the returned pointer
// at +4 before branching. On success, reread the SAME slot after publication
// and call current Source strcpy_s with the allocated pointer, size and late
// slot value; ignore its integer result. Initial null uses real DWORD AND at
// +4 (read and write). Every ordinary path finally writes full DWORD 1 at +8.
// +0/+8 require writes; +4 requires a write and a read on the initial-null path.
// No prior free, null/overflow/alias guard, remeasurement or atomicity promise.
//
// This raw boundary has no noexcept promise. Slot/pointee/backing lifetime,
// allocator compatibility and current CRT failure policy remain caller/provider
// contracts. The raw profile creates no callable Native vtable, exception type,
// RTTI, throw binding, static owner or slot producer. Source placement, modern
// CRT calls and this explicit API do not establish Original ABI/runtime/gameplay.
void* __fastcall construct_native_allocator_owned_message_00bf6340(
    void* actual_receiver,
    std::uint32_t unused_EDX,
    const void* actual_pointer_slot_address);

} // namespace bsp
