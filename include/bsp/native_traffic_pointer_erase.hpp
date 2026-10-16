#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native traffic pointer erase requires MSVC Win32.
#endif

namespace bsp {

// Complete 00488FF0..00489049 (90 bytes). Original: ECX actual vector;
// five actual stacked DWORD cells {out, first-owner, first, last-owner, last};
// EAX actual out pointer, RET 14h. Source-only unused_edx binds that shape;
// Original consumes no incoming EDX. Owner words are opaque DWORDs, not a
// recovered class, container identity check or guessed payload pointer type.
//
// Capture first-owner and receiver before the diagnostic. A zero first-owner
// or unequal owner words calls the genuine qualified current invalid-parameter
// entry; it may return. Read actual first/last argument cells AFTER that call.
// For unequal endpoints capture current vector+8 minus last modulo32, SAR2,
// then LEA the wrapped byte count/new end. Copy iff signed count>0 using actual
// current SDK memmove_s(first, bytes, last, bytes); publish captured new end
// even when signed count<=0. Equal endpoints skip the vector+8 read/store.
//
// Read the CURRENT out argument cell only after copy/end publication, then
// store out+4=first BEFORE out+0=captured first-owner. No snapshot, rollback,
// null repair, errno/exception conversion or extra validation is added.
// Actual reached cells must be backed, including aliased stack/output cells.
// Current SDK invalid-parameter/memmove_s/errno/handler identities are explicit
// Source boundaries; Native private CRT, complete binary ABI and game behavior
// are not claimed. No noexcept/noreturn promise. See the packet documentation.
void* __fastcall erase_native_traffic_pointer_range_00488ff0(
    void* actual_vector, void* unused_edx, void* actual_out,
    std::uint32_t first_owner, void* first,
    std::uint32_t last_owner, const void* last);

} // namespace bsp
