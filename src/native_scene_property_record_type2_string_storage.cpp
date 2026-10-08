#include "bsp/native_scene_property_record_type2_string_storage.hpp"
#include "bsp/native_string_duplicate.hpp"

namespace bsp {

// Complete native60 SHA-256:
// be944adca54cbd0702bc89a12e88ab0c8ac84f306bb8c14188266b8f70ae5a04
// Only the genuine CALL operand [39,43) binds the admitted physical ECX/RET0
// duplicate. All other 56 bytes, including phase identity, remain literal.
__declspec(naked) void* __fastcall
construct_native_scene_property_record_type2_string_storage_008ef1b0(
    void*, void*, const char*) {
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 8]
        mov dword ptr [esi], 0CE89D4h
        mov dword ptr [esi + 4], 2
        push edi
        xor edi, edi
        mov dword ptr [esi + 18h], edi
        mov dword ptr [esi + 1Ch], edi
        mov dword ptr [esi + 20h], edi
        mov dword ptr [esi + 24h], edi
        mov dword ptr [esi + 30h], edi
        call duplicate_native_string_00438e40
        mov dword ptr [esi + 34h], edi
        mov dword ptr [esi + 0Ch], eax
        pop edi
        mov byte ptr [esi + 2Ch], 1
        mov eax, esi
        pop esi
        ret 4
    }
}

}  // namespace bsp
