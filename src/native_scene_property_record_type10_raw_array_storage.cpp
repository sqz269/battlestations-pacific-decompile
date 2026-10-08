#include "bsp/native_scene_property_record_type10_raw_array_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#pragma function(memcpy)

namespace bsp {
namespace {

// Genuine current-domain CDECL size adapter; no Native reconstruction credit.
__declspec(noinline) void* __cdecl allocate_native_type10_array_bytes(
    std::size_t actual_bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, actual_bytes, actual_bytes});
}

} // namespace

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4 &&
    native_scene_property_record_type10_raw_array_storage_bytes == 56,
    "The original type-10 raw-array storage ABI requires Win32.");

// Native whole116 SHA256:
// 00c1508701d10a6508b556eff0feb4737f51a4ec289059dce77a796cc571919b.
// The assembly follows that body independently. Native CALL operands [51,55)
// and [66,70) instead bind the genuine current providers below. Emitted bytes,
// complete helper closures and the physical ABI await primary qualification.
__declspec(naked) void* __fastcall construct_native_scene_property_record_type10_raw_array_storage_008ef3e0(
    void*, void*, std::uint32_t, const void*, std::uint32_t) {
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0ch]
        mov esi, ecx
        add edi, edi
        mov dword ptr [esi], 00ce89d4h
        mov dword ptr [esi + 4], 10
        add edi, edi
        cmp byte ptr [esp + 14h], 0
        mov dword ptr [esi + 18h], 0
        mov dword ptr [esi + 1ch], 0
        mov dword ptr [esi + 24h], edi
        jz L_retained
        push edi
        call allocate_native_type10_array_bytes
        mov ecx, dword ptr [esp + 14h]
        push edi
        push ecx
        push eax
        mov dword ptr [esi + 20h], eax
        call memcpy
        add esp, 10h
        pop edi
        mov dword ptr [esi + 34h], 0
        mov byte ptr [esi + 2ch], 1
        mov eax, esi
        pop esi
        ret 0ch
    L_retained:
        mov edx, dword ptr [esp + 10h]
        mov dword ptr [esi + 20h], edx
        pop edi
        mov dword ptr [esi + 34h], 0
        mov byte ptr [esi + 2ch], 1
        mov eax, esi
        pop esi
        ret 0ch
    }
}

} // namespace bsp
