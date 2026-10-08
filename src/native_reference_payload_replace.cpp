#include "bsp/native_reference_payload_replace.hpp"
#include "bsp/native_string_duplicate.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Native48 SHA-256:
// c4c713091d8c81c8083a9c53eba6d356e55cb459b8c619ac703ead3796b04abf
// Only CALL operands [12,16) and [31,35) bind the actual matching current free
// and admitted physical duplicate57. All remaining 40 bytes stay literal.
// Preserve the genuine post-free ADD/clear and full scalar EAX return.
__declspec(naked) std::uint32_t __fastcall replace_native_reference_payload_008f0310(
    void*, void*, const char*, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        test eax, eax
        jz L_old_empty
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 4], 0
    L_old_empty:
        mov ecx, dword ptr [esp + 8]
        call duplicate_native_string_00438e40
        mov dword ptr [esi + 4], eax
        mov eax, dword ptr [esp + 0Ch]
        mov dword ptr [esi], eax
        pop esi
        ret 8
    }
}

}  // namespace bsp
