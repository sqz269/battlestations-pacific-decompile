#include "bsp/native_gui_icon_filter_storage.hpp"
#include "bsp/native_gui_icon_state_getter_storage.hpp"
#include "bsp/native_platform_load_messages.hpp"
#include "bsp/gui_icon_runtime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source supplier requires MSVC Win32 inline assembly.
#endif

namespace bsp {
std::uint32_t gui_icon_prefers_bilinear_filter_storage_00ab2600(
    void* actual_138b_icon, NativeGuiIconFilterScratch& scratch,
    const NativePlatformLoadPublication& actual_platform_publication,
    const GuiTextureCallbacks& callbacks,
    const GuiIconStateTextureConstants& extent_constants,
    const volatile float& actual_zero_00d7a218,
    const volatile float& actual_minimum_width_00d5c3b8,
    const volatile double& actual_epsilon_00d7a268) {
    void* const captured_icon = actual_138b_icon;
    volatile std::uint32_t* const index_home_address = &scratch.selected_index_home;
    const volatile float* const zero_address = &actual_zero_00d7a218;
    const volatile float* const one_address = &extent_constants.one_00d7a24c;
    std::uint32_t platform_required;
    __asm {
        mov ecx, captured_icon
        cmp byte ptr [ecx + 100h], 0
        jz icon_filter_early_zero
        cmp dword ptr [ecx + 104h], 0
        jnz icon_filter_early_zero
        mov platform_required, 1
        jmp icon_filter_first_phase_done
    icon_filter_early_zero:
        mov platform_required, 0
    icon_filter_first_phase_done:
    }
    if (!platform_required) return 0;
    void* const captured_platform = actual_platform_publication.load();
    std::uint32_t comparison_required;
    void* captured_vector;
    __asm {
        // Source addresses only; no native payload or constant value sampling.
        mov ecx, captured_icon
        mov eax, captured_platform
        mov ebx, zero_address
        mov edi, one_address
        mov esi, index_home_address

        cmp byte ptr [eax + 14h], 0
        jz icon_filter_true
        movss xmm0, dword ptr [ecx + 48h]
        ucomiss xmm0, dword ptr [ebx]
        lahf
        test ah, 44h
        jp icon_filter_true
        movss xmm1, dword ptr [ecx + 28h]
        movss xmm0, dword ptr [edi]
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        jp icon_filter_true
        movss xmm1, dword ptr [ecx + 2ch]
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        jp icon_filter_true
        movsx edx, word ptr [ecx + 0ech]
        mov dword ptr [esi], edx
        add ecx, 0f0h
        mov captured_vector, ecx
        mov comparison_required, 1
        jmp icon_filter_second_phase_done
    icon_filter_true:
        mov comparison_required, 0
    icon_filter_second_phase_done:
    }
    if (!comparison_required) return 1;
    void* const actual_state = get_native_gui_icon_state_storage_00ab1230(
        captured_vector, scratch.selected_index_home);
    const std::uint32_t compared = gui_icon_state_differs_from_native_size_storage_00ab24b0(
        actual_state, scratch.size_comparison, callbacks, extent_constants,
        actual_minimum_width_00d5c3b8, actual_epsilon_00d7a268);
    std::uint32_t result;
    __asm {
        mov eax, compared
        test al, al
        jz icon_filter_last_zero
        mov eax, 1
        jmp icon_filter_last_done
    icon_filter_last_zero:
        xor eax, eax
    icon_filter_last_done:
        mov result, eax
    }
    return result;
}
} // namespace bsp
