#include "bsp/native_post_effect_raw_access.hpp"

namespace bsp {

__declspec(naked) std::uint32_t __fastcall
native_post_effect_pass_raw_08_00b4cb90(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 8h]
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall
native_post_effect_pass_raw_18_00b4ec50(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 18h]
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall
native_post_effect_owner_raw_23c_00b4ffe0(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 23ch]
        ret
    }
}

} // namespace bsp
