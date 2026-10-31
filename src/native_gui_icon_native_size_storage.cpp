#include "bsp/native_gui_icon_native_size_storage.hpp"
#include "bsp/gui_texture.hpp"
#include "bsp/gui_icon_runtime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source supplier requires MSVC Win32 inline assembly.
#endif

namespace bsp {
void* write_native_gui_icon_native_size_storage_00ab17b0(
    void* actual_output_float2, void* actual_texture,
    volatile std::uint32_t& actual_uv_pointer_home,
    NativeGuiIconNativeSizeScratch& scratch,
    const GuiTextureCallbacks& callbacks,
    const GuiIconStateTextureConstants& constants) {
    void* const captured_output = actual_output_float2;
    void* const captured_texture = actual_texture;
    std::uint32_t* const scratch_homes = &scratch.width_scale_home;
    volatile std::uint32_t* const uv_pointer_home = &actual_uv_pointer_home;
    const volatile float* const correction_address = &constants.unsigned_correction_00ce3978;
    const volatile double* const width_divisor_address = &constants.width_divisor_00cec380;
    const volatile double* const height_divisor_address = &constants.height_divisor_00cef1b8;
    std::uint32_t captured_profile;
    std::uint32_t captured_target;
    __asm {
        mov esi, captured_texture
        mov eax, dword ptr [esi]
        mov edx, dword ptr [eax + 48h]
        // Only captured registers become Source carriers; relative FP depth0.
        mov captured_profile, eax
        mov captured_target, edx
    }
    const std::uint32_t width_result = callbacks.dimension_from_captured(
        captured_texture, captured_profile, captured_target);
    __asm {
        // Source address bindings precede FILD; no value is sampled here.
        mov esi, captured_texture
        mov edi, scratch_homes
        mov ebx, correction_address
        mov ecx, width_divisor_address
        mov eax, width_result

        test eax, eax
        mov dword ptr [edi + 4], eax
        fild dword ptr [edi + 4]
        jge native_size_width_nonnegative
        fadd dword ptr [ebx]
    native_size_width_nonnegative:
        fdiv qword ptr [ecx]
        mov eax, dword ptr [esi]
        mov edx, dword ptr [eax + 4ch]
        mov ecx, esi
        fstp dword ptr [edi]
        // The actual pair was captured while ST0 was live. Store metadata only
        // after its spill, then transport THAT pair without a table re-read.
        mov captured_profile, eax
        mov captured_target, edx
    }
    const std::uint32_t height_result = callbacks.dimension_from_captured(
        captured_texture, captured_profile, captured_target);
    __asm {
        // Bind all Source addresses before this region's first FILD.
        mov edi, captured_output
        mov esi, scratch_homes
        mov ebx, correction_address
        mov ecx, height_divisor_address
        mov edx, uv_pointer_home
        mov eax, height_result

        test eax, eax
        mov dword ptr [esi + 4], eax
        fild dword ptr [esi + 4]
        jge native_size_height_nonnegative
        fadd dword ptr [ebx]
    native_size_height_nonnegative:
        fdiv qword ptr [ecx]
        mov eax, dword ptr [edx]
        fstp dword ptr [esi + 4]
        fld dword ptr [eax + 8]
        fsub dword ptr [eax]
        fstp dword ptr [edx]
        mov ecx, dword ptr [edx]
        and ecx, 7fffffffh
        mov dword ptr [edx], ecx
        fld dword ptr [edx]
        fmul dword ptr [esi]
        fstp dword ptr [edi]
        fld dword ptr [eax + 0ch]
        fsub dword ptr [eax + 4]
        mov eax, edi
        fstp dword ptr [edx]
        mov ecx, dword ptr [edx]
        and ecx, 7fffffffh
        mov dword ptr [edx], ecx
        fld dword ptr [edx]
        fmul dword ptr [esi + 4]
        fstp dword ptr [edi + 4]
    }
    return captured_output;
}
} // namespace bsp
