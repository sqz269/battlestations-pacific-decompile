#include "bsp/native_shadow_update_leaves.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shadow-update leaves require MSVC Win32.
#endif

namespace bsp {

std::uint32_t get_native_override_context_word_00b7aae0(
    const volatile std::uint32_t& actual_0109019c) noexcept {
    return actual_0109019c;
}

__declspec(naked) void __fastcall set_native_material_effect_byte_00b400c0(
    void*, void*, std::uint32_t) {
    __asm {
        mov al, byte ptr [esp + 4]
        mov byte ptr [ecx + 13Ch], al
        ret 4
    }
}

__declspec(naked) void __fastcall set_native_material_effect_float_bits_00b40820(
    void*, void*, std::uint32_t, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        movss xmm0, dword ptr [esp + 8]
        movss dword ptr [ecx + eax * 4 + 140h], xmm0
        ret 8
    }
}

} // namespace bsp
