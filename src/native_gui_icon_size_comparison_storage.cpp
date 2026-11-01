#include "bsp/native_gui_icon_size_comparison_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source supplier requires MSVC Win32 inline assembly.
#endif

namespace bsp {
std::uint32_t gui_icon_state_differs_from_native_size_storage_00ab24b0(
    void* actual_40b_state, NativeGuiIconSizeComparisonScratch& scratch,
    const GuiTextureCallbacks& callbacks,
    const GuiIconStateTextureConstants& extent_constants,
    const volatile float& actual_minimum_width_00d5c3b8,
    const volatile double& actual_epsilon_00d7a268) {
    void* const captured_state = actual_40b_state;
    void* captured_texture;
    volatile std::uint32_t* const uv_pointer_home_address = &scratch.uv_pointer_home;
    std::uint32_t* const comparison_homes = &scratch.difference_home;
    const volatile float* const minimum_width_address = &actual_minimum_width_00d5c3b8;
    const volatile double* const epsilon_address = &actual_epsilon_00d7a268;
    __asm {
        mov esi, captured_state
        mov edx, dword ptr [esi]
        lea eax, [esi + 0ch]
        mov ecx, uv_pointer_home_address
        mov dword ptr [ecx], eax
        mov captured_texture, edx
    }
    (void)write_native_gui_icon_native_size_storage_00ab17b0(
        &scratch.native_width_home, captured_texture, scratch.uv_pointer_home,
        scratch.extent_scratch, callbacks, extent_constants);
    std::uint32_t comparison_result;
    __asm {
        // Pure Source address bindings, before relative x87 ST0 becomes live.
        mov esi, captured_state
        mov edi, comparison_homes
        mov ecx, minimum_width_address
        mov edx, epsilon_address

        movss xmm0, dword ptr [ecx]
        comiss xmm0, dword ptr [edi + 4]
        ja size_compare_yes
        fld dword ptr [esi + 34h]
        fsub dword ptr [edi + 4]
        fstp dword ptr [edi]
        mov ecx, dword ptr [edi]
        fld qword ptr [edx]
        and ecx, 7fffffffh
        mov dword ptr [edi], ecx
        fld dword ptr [edi]
        fcomip st(0), st(1)
        ja size_compare_width_yes
        fld dword ptr [esi + 38h]
        fsub dword ptr [edi + 8]
        fstp dword ptr [edi]
        mov ecx, dword ptr [edi]
        and ecx, 7fffffffh
        mov dword ptr [edi], ecx
        fld dword ptr [edi]
        fcomip st(0), st(1)
        fstp st(0)
        ja size_compare_yes
        xor eax, eax
        jmp size_compare_done
    size_compare_width_yes:
        fstp st(0)
    size_compare_yes:
        mov eax, 1
    size_compare_done:
        mov comparison_result, eax
    }
    return comparison_result;
}
} // namespace bsp
