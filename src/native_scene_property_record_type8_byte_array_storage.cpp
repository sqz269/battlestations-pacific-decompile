#include "bsp/native_scene_property_record_type8_byte_array_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#pragma function(memcpy)

namespace bsp {
namespace {

// Genuine current-domain CDECL size bridge; no native reconstruction credit.
__declspec(noinline) void* __cdecl allocate_native_byte_array_bytes(
    std::size_t actual_bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, actual_bytes, actual_bytes});
}

} // namespace

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4 &&
    native_scene_property_record_type8_byte_array_storage_bytes == 56,
    "The original physical byte-array storage ABI requires Win32.");

// Whole native112 SHA256:
// 4c3786af3a642703dc38315ae63d9dee0468552f47a03bc1405fc0e90ab5c928.
// Only actual CALL operands [47,51) and [62,66) bind current providers.
// All other 104 bytes preserve the flag-byte read, store order and both RETs.
__declspec(naked) void* __fastcall construct_native_scene_property_record_type8_byte_array_storage_008ef2f0(
    void*, void*, const void*, std::uint32_t, std::uint32_t) {
    __asm {
        cmp byte ptr [esp + 0ch], 0
        push esi
        mov esi, ecx
        mov dword ptr [esi], 00ce89d4h
        mov dword ptr [esi + 4], 8
        push edi
        mov edi, dword ptr [esp + 10h]
        mov dword ptr [esi + 18h], 0
        mov dword ptr [esi + 1ch], 0
        mov dword ptr [esi + 24h], edi
        jz L_retained
        push edi
        call allocate_native_byte_array_bytes
        mov ecx, dword ptr [esp + 10h]
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
        mov edx, dword ptr [esp + 0ch]
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
