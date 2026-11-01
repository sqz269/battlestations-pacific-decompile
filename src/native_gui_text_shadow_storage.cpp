#include "bsp/native_gui_text_shadow_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text shadow storage requires MSVC Win32.
#endif

namespace bsp {
void apply_native_gui_text_shadow_storage_00ab72d0(void* actual_text1f4,
    volatile std::uint32_t& actual_preset_home, NativeGuiTextShadowStorageFrame& frame,
    const NativeGuiTextShadowStorageContext& context) {
    static_assert(sizeof(void*) == sizeof(std::uint32_t));
    // These are Source address carriers only; no Native value/HOME is read.
    volatile std::uint32_t* const preset_address = &actual_preset_home;
    const volatile float* const offset_address = &context.offset_00d5c5c0;
    const volatile float* const one_address = &context.one_00d7a24c;
    const volatile float* const alpha_address = &context.white_alpha_00cee07c;
    const volatile float* const color_address = &context.shadow_color_00e12fd8[0];
    std::uint32_t* const spill_address = frame.words_10;
    __asm {
        mov eax, preset_address
        mov eax, dword ptr [eax] // Entry capture, before any Text payload read.
        mov ecx, actual_text1f4
        cmp dword ptr [ecx + 1d0h], eax
        je shadow_done
        cmp eax, -1
        mov dword ptr [ecx + 1d0h], eax
        je shadow_disable
        cmp eax, 1
        mov edx, offset_address
        movss xmm0, dword ptr [edx]
        mov byte ptr [ecx + 15ch], 1
        mov eax, color_address
        mov edx, dword ptr [eax]
        mov dword ptr [ecx + 168h], edx
        mov edx, dword ptr [eax + 4]
        mov dword ptr [ecx + 16ch], edx
        mov edx, dword ptr [eax + 8]
        mov dword ptr [ecx + 170h], edx
        mov edx, dword ptr [eax + 0ch]
        mov dword ptr [ecx + 174h], edx
        mov dword ptr [ecx + 160h], 0
        movss dword ptr [ecx + 164h], xmm0
        jnz shadow_done // CMP preset,1 survives the MOV-only sequence.
        mov eax, one_address
        movss xmm0, dword ptr [eax]
        mov esi, spill_address
        movss dword ptr [esi], xmm0
        mov eax, dword ptr [esi]
        movss dword ptr [esi + 4], xmm0
        mov edx, dword ptr [esi + 4]
        mov dword ptr [ecx + 168h], eax
        movss dword ptr [esi + 8], xmm0
        mov eax, alpha_address
        movss xmm0, dword ptr [eax]
        mov eax, dword ptr [esi + 8]
        mov dword ptr [ecx + 16ch], edx
        movss dword ptr [esi + 0ch], xmm0
        mov edx, dword ptr [esi + 0ch]
        mov dword ptr [ecx + 170h], eax
        mov dword ptr [ecx + 174h], edx
        jmp shadow_done
    shadow_disable:
        mov byte ptr [ecx + 15ch], 0
    shadow_done:
    }
}
} // namespace bsp
