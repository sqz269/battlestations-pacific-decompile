#include "bsp/native_scene_string_list_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#pragma function(memcpy)

namespace bsp {
namespace {

// Genuine private CDECL bridge to the established current allocation service.
// Native and host requests are the same exact raw byte count; no Native credit.
__declspec(noinline) void* __cdecl allocate_native_scene_string_list_bytes(
    std::size_t actual_bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, actual_bytes, actual_bytes});
}

} // namespace

static_assert(sizeof(void*) == 4 && sizeof(std::size_t) == 4);

__declspec(naked) void* __fastcall construct_native_scene_string_list_storage_0094efb0(void*) {
    __asm {
        push esi
        mov esi, ecx
        xor ecx, ecx
        mov eax, 40h
        mov dword ptr [esi + 8], eax
        mov edx, 4
        mul edx
        seto cl
        neg ecx
        or ecx, eax
        push ecx
        call allocate_native_scene_string_list_bytes
        mov dword ptr [esi], eax
        add esp, 4
        mov dword ptr [esi + 4], 0
        mov eax, esi
        pop esi
        ret
    }
}

__declspec(naked) void __fastcall grow_native_scene_string_list_storage_0094f010(void*) {
    __asm {
        push esi
        mov esi, ecx
        add dword ptr [esi + 8], 40h
        mov eax, dword ptr [esi + 8]
        xor ecx, ecx
        mov edx, 4
        mul edx
        seto cl
        push edi
        neg ecx
        or ecx, eax
        push ecx
        call allocate_native_scene_string_list_bytes
        mov ecx, dword ptr [esi]
        mov edi, eax
        mov eax, dword ptr [esi + 4]
        add eax, eax
        add eax, eax
        push eax
        push ecx
        push edi
        call memcpy
        mov eax, dword ptr [esi]
        add esp, 10h
        test eax, eax
        jz L_growth_publish
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi], 0
    L_growth_publish:
        mov dword ptr [esi], edi
        pop edi
        pop esi
        ret
    }
}

} // namespace bsp
