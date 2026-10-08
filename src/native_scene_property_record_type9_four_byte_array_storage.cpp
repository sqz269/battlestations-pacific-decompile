#include "bsp/native_scene_property_record_type9_four_byte_array_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#pragma function(memcpy)

namespace bsp {
namespace {

// Genuine current-domain CDECL size bridge; no native reconstruction credit.
__declspec(noinline) void* __cdecl allocate_native_four_byte_array_bytes(
    std::size_t actual_bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, actual_bytes, actual_bytes});
}

} // namespace

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4 &&
    native_scene_property_record_type9_four_byte_array_storage_bytes == 56,
    "The original physical four-byte-array storage ABI requires Win32.");

// Whole native116 SHA256:
// dce8fb8621af17a5791a0e80cca91982def8914d2f3a9e516ee1a2344f05b5e3.
// Only CALL operands [51,55) and [66,70) bind current providers; the other
// 108 bytes preserve both count additions, all store/read order and both RETs.
__declspec(naked) void* __fastcall construct_native_scene_property_record_type9_four_byte_array_storage_008ef360(
    void*, void*, std::uint32_t, const void*, std::uint32_t) {
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0ch]
        mov esi, ecx
        add edi, edi
        mov dword ptr [esi], 00ce89d4h
        mov dword ptr [esi + 4], 9
        add edi, edi
        cmp byte ptr [esp + 14h], 0
        mov dword ptr [esi + 18h], 0
        mov dword ptr [esi + 1ch], 0
        mov dword ptr [esi + 24h], edi
        jz L_retained
        push edi
        call allocate_native_four_byte_array_bytes
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
