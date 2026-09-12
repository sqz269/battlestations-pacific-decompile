#pragma once

#include <cstdint>

namespace bsp {

// Complete BCF910[19]. ECX is the raw owner, incoming EDX is unused;
// EAX returns zero for null begin(+4), otherwise (end(+8)-begin) SAR2.
// The returned uint32_t preserves the signed result's raw bit pattern.
std::uint32_t __fastcall count_native_singleton_slots_00bcf910(
    const void* owner, void* unused_edx) noexcept;

// Complete BD0160[30]. cdecl(first,last,value_slot), plain RET. Store the
// current DWORD at value_slot at each successive four-byte address until
// equality with last. No order/alignment/bounds checks; value aliases remain
// observable because each store gets a fresh DWORD load.
void __cdecl assign_native_singleton_slots_00bd0160(
    void* first, const void* last, const void* value_slot) noexcept;

// Complete BD0180[47]. cdecl(first,last,destination_end), plain RET; EAX is
// destination_end - ((last-first) SAR2)*4 modulo32, also for nonpositive
// distance. Calls actual memmove_s only for signed-positive distance, with
// destination-size == count == distance*4. Its status is ignored.
void* __cdecl copy_backward_native_singleton_slots_00bd0180(
    const void* first, const void* last, void* destination_end);

// Complete BD0220[42]. ECX is raw owner, incoming EDX is unused, plain RET.
// Capture current +4 and free the nonnull capture through the existing
// malloc-paired singleton_lifetime_free service, then zero +4/+8/+0C in
// that order. Preserve +0/+10. The returning-free ADD ESP,4 is included.
void __fastcall clear_native_singleton_storage_00bd0220(
    void* owner, void* unused_edx) noexcept;

// Complete BD0500[48]. stdcall(first,last,destination), RET0C; incoming ECX
// is not an input. Raw32 SAR2 distance is tested for nonzero, including
// negative results. Calls actual memmove_s with equal size/count distance*4,
// ignores status and returns the pre-call destination+distance*4 in EAX.
void* __stdcall copy_native_singleton_slots_00bd0500(
    const void* first, const void* last, void* destination);

// Complete BD0560[46]. stdcall(destination,count,value_slot), RET0C;
// incoming ECX is unused. Full unsigned count loop, fresh DWORD value load
// for each store, EAX initial destination+count*4 modulo32. Count zero makes
// no value-slot load. No count-one or nonaliasing restriction is imposed.
void* __stdcall fill_native_singleton_slots_00bd0560(
    void* destination, std::uint32_t count, const void* value_slot) noexcept;

// MSVC Win32 naked entries retain the original raw register/stack arithmetic
// and DWORD accesses without C++ unrelated-pointer or typed-alias arithmetic.
// Source CRT boundaries are fixed actual memmove_s and the existing free
// service, not injected callbacks. Source allocations must belong to that
// owning CRT. Malformed memmove calls are retained; current source errno and
// invalid-parameter handler ownership are not original BFFB8B/BF66EF/109DD64.
// Original static-CRT internals, fault-address/SEH identity, native EH frames,
// service-internal register/flag effects and game integration are not claimed.
} // namespace bsp
