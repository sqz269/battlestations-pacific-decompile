#include "bsp/native_scene_property_record_type4_storage.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4 &&
    native_scene_property_record_type4_storage_bytes == 56,
    "The original two-word storage and physical ABI are Win32.");

// Original whole [008EF230,008EF268), SHA256
// b6b40a5cbb0a523d345de24f97ac879a4e5a56c8d9a1f121f636fd33ce4dfd98.
// Preserve both raw stack loads, every store width/order, literal phase word,
// register/flag effects and RET8. No CALLs, relocations or owning providers.
__declspec(naked) void* __fastcall construct_native_scene_property_record_type4_storage_008ef230(
    void*, void*, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        mov edx, dword ptr [esp + 4]
        mov eax, ecx
        xor ecx, ecx
        mov dword ptr [eax], 00ce89d4h
        mov dword ptr [eax + 4], 4
        mov dword ptr [eax + 18h], ecx
        mov dword ptr [eax + 1ch], ecx
        mov dword ptr [eax + 20h], ecx
        mov dword ptr [eax + 24h], ecx
        mov dword ptr [eax + 28h], edx
        mov edx, dword ptr [esp + 8]
        mov dword ptr [eax + 30h], ecx
        mov dword ptr [eax + 0ch], edx
        mov dword ptr [eax + 34h], ecx
        mov byte ptr [eax + 2ch], 1
        ret 8
    }
}

} // namespace bsp
