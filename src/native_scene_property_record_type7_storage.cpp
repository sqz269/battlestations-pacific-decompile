#include "bsp/native_scene_property_record_type7_storage.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && native_scene_property_record_type7_storage_bytes == 56,
    "The original three-word storage and physical ABI are Win32.");

// Original whole [008EF270,008EF2AF), SHA256
// 9010f6007c93fa311d1e2d7f6c89a06aa802af3575be8e5c22edd26aa36d6872.
// Preserve phase/tag stores before pointed loads, interleaved read/write order,
// literal widths, register/flag effects and RET4. No CALLs or relocations.
__declspec(naked) void* __fastcall construct_native_scene_property_record_type7_storage_008ef270(
    void*, void*, const void*) noexcept {
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [esp + 4]
        mov dword ptr [eax], 00ce89d4h
        mov dword ptr [eax + 4], 7
        mov edx, dword ptr [ecx]
        mov dword ptr [eax + 0ch], edx
        mov edx, dword ptr [ecx + 4]
        mov dword ptr [eax + 10h], edx
        mov ecx, dword ptr [ecx + 8]
        mov dword ptr [eax + 14h], ecx
        xor ecx, ecx
        mov dword ptr [eax + 18h], ecx
        mov dword ptr [eax + 1ch], ecx
        mov dword ptr [eax + 20h], ecx
        mov dword ptr [eax + 24h], ecx
        mov dword ptr [eax + 30h], ecx
        mov dword ptr [eax + 34h], ecx
        mov byte ptr [eax + 2ch], 1
        ret 4
    }
}

} // namespace bsp
