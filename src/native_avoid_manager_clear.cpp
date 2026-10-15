#include "bsp/native_avoid_manager_clear.hpp"
#include "bsp/native_avoid_cache_cleanup.hpp"

#include <cstddef>

namespace bsp {
static_assert(sizeof(Cc13AvoidZoneManagerNative) == 0x78);
static_assert(offsetof(Cc13AvoidZoneManagerNative, group_slots) == 8);
static_assert(offsetof(Cc13AvoidZoneManagerNative, group_count) == 0x58);

__declspec(naked) void __fastcall clear_native_avoid_manager_0041f600(
    Cc13AvoidZoneManagerNative*, void* const*) {
    __asm {
        push edx // Borrowed actual publication-cell address; not a cached game.
        push ebx
        push esi
        mov ebx, ecx
        push edi
        mov dword ptr [ebx + 58h], 0
        lea esi, [ebx + 8]
        mov edi, 14h
    next_group:
        mov ecx, dword ptr [esi]
        test ecx, ecx
        jz group_cleared
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [eax]
        push 1
        call edx // 0041F620: actual virtual scalar deleting entry.
        mov dword ptr [esi], 0
    group_cleared:
        add esi, 4
        sub edi, 1
        jnz next_group
        pop edi
        pop esi
        mov ecx, ebx
        pop ebx
        pop edx // Restore context without changing the native outgoing flags.
        jmp clear_native_avoid_cache_0041e960 // Native tail site0041F635.
    }
}
} // namespace bsp
