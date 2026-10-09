#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tick subnode scalar deletion requires MSVC Win32.
#endif

namespace bsp {

// Complete 0071C4D0..0071C4ED[30] through a new naked Source interface. ECX
// supplies the actual receiver; EDX is unused placement. The three stack words
// are the actual flags DWORD followed by the actual F/M publication addresses.
// Source RET0C consumes all three; Native RET4 consumes only the flags word.
//
// Forward both actual publication references to the admitted raw 00875B30
// Source cleanup. AFTER normal child return, test bit zero of the current low
// byte of the flags word. If set, pass actual current ESI to current CRT free
// and execute ADD ESP,4. Return actual current ESI in EAX on both paths, then
// restore ESI from the actual current saved word. A freed return value is only
// an opaque address. Do not infer a live receiver from the return type.
//
// All raw backing and alias/preservation conditions of that cleanup remain
// required. Borrow the actual stable F/M cells, whose values remain mutable.
// Preserve added publication-address words until their selected loads, plus
// the active return backing and required child saved words. The flags word is
// deliberately read late and may change during cleanup. Extra Source argument
// pushes change stack-alias placement; this is not a Native ABI replacement.
// If free is selected, the current pointer must be eligible for this Source
// CRT's free. Native 00BF65AC implementation/allocator/failure policy is open.
// No owned unwind handling, noexcept, storage, profile binding, or production
// consumer is supplied. The 007F1EE0 wrapper remains separately deferred.
void* __fastcall scalar_delete_native_tick_subnode_0071c4d0(
    void* actual_receiver,
    std::uint32_t unused_edx,
    std::uint32_t actual_flags,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0);

} // namespace bsp
