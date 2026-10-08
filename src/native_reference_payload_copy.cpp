#include "bsp/native_reference_payload_copy.hpp"
#include "bsp/native_string_duplicate.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Native55 SHA-256:
// 40611ad298e15a07908bead1b10f73434e3574529a027779139398e0f3edd3c6
// Rebind only CALL operands [23,27) to the matching actual current free and
// [40,44) to the admitted physical duplicate57; all other 47 bytes are literal.
// Saved analysis now includes the genuine ADD ESP,4 and MOV [ESI+4],0 after
// the old free. Snapshot text/scalar before free; never fake self-alias safety.
__declspec(naked) char* __fastcall copy_native_reference_payload_008f0340(
    void*, void*, const void*) {
    __asm {
        mov eax, dword ptr [esp + 4]
        push ebx
        mov ebx, dword ptr [eax + 4]
        push esi
        push edi
        mov edi, dword ptr [eax]
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        test eax, eax
        jz L_old_empty
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 4], 0
    L_old_empty:
        mov ecx, ebx
        call duplicate_native_string_00438e40
        mov dword ptr [esi], edi
        pop edi
        mov dword ptr [esi + 4], eax
        pop esi
        pop ebx
        ret 4
    }
}

}  // namespace bsp
