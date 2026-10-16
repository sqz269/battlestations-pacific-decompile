#include "bsp/native_avoid_zone_group_remove_owner.hpp"
#include "bsp/cc13_avoid1.hpp"
#include "bsp/native_avoid_clip_cleanup.hpp"
#include "bsp/native_avoid_group_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(Cc13AvoidZoneNativeGroup) == 0x14);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, zones) == 4);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, count) == 8);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, capacity) == 0xc);
static_assert(offsetof(AvoidZoneNativeStorage, associated_entity) == 0x10);

__declspec(naked) void __fastcall remove_native_avoid_zone_group_owner_0041d150(
    void*, void*, void*) {
    __asm {
        push ebp
        mov ebp, dword ptr [esp + 8]
        test ebp, ebp
        push edi
        mov edi, ecx
        jz clear_all
        mov eax, dword ptr [edi + 8]
        mov ecx, dword ptr [edi + 4]
        push ebx
        mov ebx, dword ptr [edi + 4]
        lea edx, [ecx + eax*4]
        cmp ebx, edx
        jz pop_ebx
        push esi
        // Preserve Native 0041D16E's two-byte MOV EDI,EDI identity encoding.
        _emit 08bh
        _emit 0ffh
    same_slot:
        mov esi, dword ptr [ebx]
        // Native deliberately reads +10 before its later null test.
        cmp dword ptr [esi + 10h], ebp
        jne advance
        test esi, esi
        jz replace_last
        mov ecx, esi
        call clear_native_avoid_corner_records_004167c0
        mov eax, dword ptr [esi]
        test eax, eax
        jz free_zone
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi], 0
    free_zone:
        push esi
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [ebx], 0
    replace_last:
        mov eax, dword ptr [edi + 8]
        mov ecx, dword ptr [edi + 4]
        mov edx, dword ptr [ecx + eax*4 - 4]
        mov dword ptr [ebx], edx
        add dword ptr [edi + 8], -1
        jmp compare_end
    advance:
        add ebx, 4
    compare_end:
        mov eax, dword ptr [edi + 8]
        mov ecx, dword ptr [edi + 4]
        lea edx, [ecx + eax*4]
        cmp ebx, edx
        jne same_slot
        pop esi
    pop_ebx:
        pop ebx
        pop edi
        pop ebp
        ret 4
    clear_all:
        // Original ECX still holds group: no stacked argument or tail-jump.
        call clear_native_avoid_group_0041d0c0
        pop edi
        pop ebp
        ret 4
    }
}

} // namespace bsp
