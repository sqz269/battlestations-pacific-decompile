#include "bsp/native_scene_property_record_type5_storage.hpp"
#include "bsp/native_string_duplicate.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4 &&
    native_scene_property_record_type5_storage_bytes == 56,
    "The original three-stack-word storage and physical ABI are Win32.");

// Whole native63 SHA256:
// a3f703ea58a3c6f7fab8ce6d36764b8b1445cd101e5a0236a493d45718138e15.
// Rebind only the real CALL operand [35,39) to the admitted current-domain
// duplicate entry. All other59 bytes retain stack timing, stores and RET0C.
__declspec(naked) void* __fastcall construct_native_scene_property_record_type5_storage_008ef2b0(
    void*, void*, const char*, std::uint32_t, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 0ch]
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 0ch]
        mov dword ptr [esi], 00ce89d4h
        mov dword ptr [esi + 4], 5
        mov dword ptr [esi + 8], eax
        mov dword ptr [esi + 18h], ecx
        mov ecx, dword ptr [esp + 8]
        call duplicate_native_string_00438e40
        mov dword ptr [esi + 1ch], eax
        xor eax, eax
        mov dword ptr [esi + 20h], eax
        mov dword ptr [esi + 24h], eax
        mov dword ptr [esi + 34h], eax
        mov byte ptr [esi + 2ch], 1
        mov eax, esi
        pop esi
        ret 0ch
    }
}

} // namespace bsp
