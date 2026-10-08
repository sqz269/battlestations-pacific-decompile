#include "bsp/native_scene_property_record_type1_float_storage.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4 &&
    native_scene_property_record_type1_float_storage_bytes == 56,
    "The original partial float storage and physical ABI are Win32.");

// Original whole [008EF170,008EF1A5), SHA256
// 684d66087700bdea183c40bc4344d965ad5ccf6f8aab80f0f7b9dbff9fe3c51f.
// Preserve legacy MOVSS, the initial argument load, literal phase word, every
// store width/order, register/flag effect and RET4. Zero CALLs/relocations.
// No Source phase global, numeric conversion, allocator or lifetime provider.
__declspec(naked) void* __fastcall construct_native_scene_property_record_type1_float_storage_008ef170(
    void*, void*, std::uint32_t) noexcept {
    __asm {
        movss xmm0, dword ptr [esp + 4]
        mov eax, ecx
        xor ecx, ecx
        mov dword ptr [eax], 00ce89d4h
        mov edx, 1
        mov dword ptr [eax + 4], edx
        movss dword ptr [eax + 0ch], xmm0
        mov dword ptr [eax + 18h], ecx
        mov dword ptr [eax + 1ch], ecx
        mov dword ptr [eax + 20h], ecx
        mov dword ptr [eax + 24h], ecx
        mov dword ptr [eax + 30h], ecx
        mov dword ptr [eax + 34h], ecx
        mov byte ptr [eax + 2ch], dl
        ret 4
    }
}

} // namespace bsp
