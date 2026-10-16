#include "bsp/native_avoid_manager_group_lookup.hpp"

#include <cstddef>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(Cc13AvoidZoneManagerNative) == 0x78);
static_assert(offsetof(Cc13AvoidZoneManagerNative, group_slots) == 0x08);
static_assert(offsetof(Cc13AvoidZoneManagerNative, group_count) == 0x58);
static_assert(sizeof(Cc13AvoidZoneNativeGroup) == 0x14);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, layer_key) == 0x10);

__declspec(naked) Cc13AvoidZoneNativeGroup* __fastcall
lookup_native_avoid_manager_group_004120d0(
    Cc13AvoidZoneManagerNative*, void*, std::int32_t) {
    __asm {
        push ebx // 004120D0
        push ebp
        push esi
        push edi
        mov edi,dword ptr [ecx+58h] // Capture count ONCE, with signed tests.
        xor ebp,ebp
        xor eax,eax
        test edi,edi
        jle fallback_slot
        mov ebx,dword ptr [esp+14h] // Actual public layer cell, positive count only.
        lea esi,[ecx+8]
    scan_slot:
        mov edx,dword ptr [esi]
        mov edx,dword ptr [edx+10h] // No null check before the key read.
        cmp edx,ebx
        je exact_slot
        cmovl ebp,eax // Signed comparison; retain the last PHYSICAL qualifying index.
        add eax,1
        add esi,4
        cmp eax,edi
        jl scan_slot
    fallback_slot:
        mov eax,dword ptr [ecx+ebp*4+8] // Fresh chosen-slot load, even count <= 0.
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 4
    exact_slot:
        mov eax,dword ptr [ecx+eax*4+8] // Fresh matching-slot load.
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 4
    }
}

} // namespace bsp
