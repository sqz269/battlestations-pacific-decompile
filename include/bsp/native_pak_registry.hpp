#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual PAK registry storage requires MSVC Win32.
#endif

namespace bsp {
// Borrow the two actual publication cells. Their addresses stay fixed while
// their values may change. No private manager, pool, or registry is created.
struct NativePakRegistryContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_registry_publication_010904d8;
};

// 736C30[199]: native cdecl(), EAX publication, RET. Added context changes ABI.
// Slow path captures the first manager's section, rechecks publication, then
// publishes raw1Ch and captures its +8 before the second manager getter.
// Registration failure retains the allocation/publication and releases the
// captured section. Fast return is captured; slow return reloads after leave.
void* get_native_pak_registry_00736c30(NativePakRegistryContext&);

// BB4FB0[54]: ECX raw1Ch, EAX same owner, RET. Primary+0, refcount+4,
// secondary+8, ordinal+Ch, non-owning DWORD vector base/count/capacity+10/14/18.
void* construct_native_pak_registry_00bb4fb0(void* actual_owner);

// BB46F0[95]/BB47A0[80]: ECX raw0Ch vector, signed capacity/count on stack,
// RET4, no semantic EAX result. Reserve clamps capacity to at least one;
// resize zeroes added DWORD slots and never destroys the non-owning values.
void reserve_native_pak_registry_slots_00bb46f0(void* actual_vector,
    std::int32_t capacity);
void resize_native_pak_registry_slots_00bb47a0(void* actual_vector,
    std::int32_t count);

// BB5000[128]: ECX raw1Ch, RET. Resize current vector to zero, free its
// current base, clear publication unconditionally, stamp secondary CE3818,
// then primary D64178 -> existing BD30F0/CEB130. No unregister or vector reset.
// Native EH state1 -> secondary base, state0 -> reference base; it does not
// retry vector cleanup. Explicit source context changes the native ABI.
void destroy_native_pak_registry_00bb5000(void*, NativePakRegistryContext&);

// Native ECX owner, full DWORD flags on stack, RET4, EAX captured owner.
// BB5410[30] frees only after successful destruction and only if flags&1.
// BB4FF0[8] subtracts eight from actual secondary this, then tail-jumps BB5410.
void* delete_native_pak_registry_00bb5410(void*, std::uint32_t flags,
    NativePakRegistryContext&);
void* delete_native_pak_registry_secondary_00bb4ff0(void*, std::uint32_t flags,
    NativePakRegistryContext&);

// BB49C0[41]: base-only scalar deletion, no this adjustment; clears the
// current publication, stamps this CE3818, and frees this iff flags&1.
void* delete_native_pak_registry_base_00bb49c0(void*, std::uint32_t flags,
    NativePakRegistryContext&) noexcept;

// Complete native unwind targets BB4610[17] and BB3F50[11], ECX this, RET.
// Secondary base clears publication/stamps CE3818. Reference base stamps
// D64178 and tail-calls actual BD30F0. Neither frees nor unregisters.
void destroy_native_pak_registry_lifetime_base_00bb4610(void*,
    NativePakRegistryContext&) noexcept;
void destroy_native_pak_registry_reference_base_00bb3f50(void*) noexcept;

// Raw identity tokens are not executable source vtables. Original FH3/SEH,
// source CRT/Win32 service ABI identity, mutable EH spills and game reachability
// are not claimed. BB5770/BB5910 lookup and integrated canonical secondary
// destruction dispatch are separate contracts.
} // namespace bsp
