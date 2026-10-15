#include "bsp/native_scene_property_reference_set.hpp"

#include "bsp/native_string_duplicate.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Whole Original58B SHA256:
// 8dac017d064d307eb6bd6f93de1476f5882325716cb81fd81a6b65ec1c707328
// Rebind only CALL operands [12,16) and [31,35); retain the other 50 bytes.
// 00BF6989 -> 00BF65AC binds current canonical singleton_lifetime_free.
// Physical 00438E40 binds the accepted ECX/RET0 duplicate, including null.
__declspec(naked) void* __fastcall set_native_scene_property_reference_008f0420(
    void*, void*, const char*, std::uint32_t, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 1ch]
        test eax, eax
        jz L_duplicate
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 1ch], 0
    L_duplicate:
        mov ecx, dword ptr [esp + 8]
        call duplicate_native_string_00438e40
        mov ecx, dword ptr [esp + 10h]
        mov dword ptr [esi + 1ch], eax
        mov eax, dword ptr [esp + 0ch]
        mov dword ptr [esi + 18h], eax
        mov dword ptr [esi + 8], ecx
        mov eax, esi
        pop esi
        ret 0ch
    }
}

} // namespace bsp
