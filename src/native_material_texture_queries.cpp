#include "bsp/native_material_texture_queries.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material texture queries require MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(offsetof(NativeMaterialStorage, word_104) == 0x104);
static_assert(offsetof(NativeMaterialEffectBaseStorage, textures_0c) == 0x0c);
static_assert(offsetof(NativeMaterialEffectBaseStorage, texture_count_38) == 0x38);
static_assert(offsetof(NativeMaterialEffectBaseStorage, fallback_98) == 0x98);
static_assert(sizeof(NativePostEffectFrameAtomic) == 4);

__declspec(naked) std::uint32_t __fastcall get_native_material_word_104_00b17320(
    const NativeMaterialStorage*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 104h]
        ret
    }
}

__declspec(naked) void* __fastcall select_native_material_effect_texture_00b17d90(
    const NativeMaterialEffectBaseStorage*,
    NativePostEffectFrameAtomic const volatile&,
    std::int32_t) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        push esi
        mov esi, ecx
        movsx ecx, word ptr [esi + 38h]
        cmp eax, ecx
        jge fallback
        mov eax, dword ptr [esi + eax*4 + 0Ch]
        test eax, eax
        jnz done
    fallback:
        mov ecx, dword ptr [esi + 98h]
        add ecx, 4
        push ecx
        call dword ptr [edx]
        mov eax, dword ptr [esi + 98h]
    done:
        pop esi
        ret 4
    }
}
} // namespace bsp
