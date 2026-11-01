#include "bsp/native_gui_texture_resolver_storage.hpp"
#include "bsp/native_particle_texture_names_raw.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/gui_texture.hpp"
#include "bsp/gui_icon_runtime.hpp"
#include <atomic>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw resolver requires MSVC Win32 x87/SSE and DWORD storage.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* object, Word byte_offset) noexcept {
    Word result;
    __asm {
        mov edx, object
        add edx, byte_offset
        mov eax, dword ptr [edx]
        mov result, eax
    }
    return result;
}
} // namespace

void* resolve_native_gui_texture_storage_00aa2660(volatile Word (&homes)[4],
    NativeGuiTextureResolverStorageContext& c) {
    void* const name_header = pointer(homes[0]);
    Word filename = read(name_header, 4);
    if (filename == 0) filename = bits(c.empty_name_00f8bc60);
    void* const atlas_manager = c.atlas.actual_atlas_manager_00f8c26c;
    void* const item = find_native_particle_atlas_item_00aefb20(atlas_manager,
        static_cast<const char*>(pointer(filename)), c.atlas.strings, c.atlas.null_pattern_00e17bf0);
    if (item == nullptr) {
        const void* const renderer = c.cache.textures.current_renderer_00f8d394;
        const Word renderer_profile = read(renderer, 0);
        const Word renderer_target = read(pointer(renderer_profile), 0x64);
        // Caller supplies the genuine existing B319B0 Source class domain.
        // These captures are data, not a new class/permission test or code call.
        (void)static_cast<const volatile Word&>(renderer_target);
        return load_native_renderer_texture_00b319b0(pointer(bits(renderer)),
            name_header, 0, c.cache, &c.miss_acquired);
    }
    void* const uv = pointer(homes[1]);
    std::uint32_t compute_dimensions;
    __asm {
        mov edi, item
        mov esi, uv
        fld dword ptr [esi + 8]
        fsub dword ptr [esi]
        fldz
        fcomi st(0), st(1)
        fstp st(1)
        jbe raw_resolve_u_not_negative
        mov cl, 1
        jmp raw_resolve_u_done
    raw_resolve_u_not_negative:
        xor cl, cl
    raw_resolve_u_done:
        fld dword ptr [esi + 0ch]
        fsub dword ptr [esi + 4]
        fxch
        fcomip st(0), st(1)
        fstp st(0)
        jbe raw_resolve_v_not_negative
        mov al, 1
        jmp raw_resolve_v_done
    raw_resolve_v_not_negative:
        xor al, al
    raw_resolve_v_done:
        test cl, cl
        fld dword ptr [edi + 14h]
        fstp dword ptr [esi]
        fld dword ptr [edi + 18h]
        fstp dword ptr [esi + 4]
        fld dword ptr [edi + 1ch]
        fstp dword ptr [esi + 8]
        fld dword ptr [edi + 20h]
        fstp dword ptr [esi + 0ch]
        jz raw_resolve_u_kept
        fld dword ptr [esi + 8]
        movss xmm0, dword ptr [esi]
        fstp dword ptr [esi]
        movss dword ptr [esi + 8], xmm0
    raw_resolve_u_kept:
        test al, al
        jz raw_resolve_v_kept
        fld dword ptr [esi + 0ch]
        movss xmm0, dword ptr [esi + 4]
        fstp dword ptr [esi + 4]
        movss dword ptr [esi + 0ch], xmm0
    raw_resolve_v_kept:
    }
    void* const size_output = pointer(homes[2]);
    __asm {
        mov ebx, size_output
        xorps xmm0, xmm0
        movss xmm1, dword ptr [ebx]
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        jp raw_resolve_no_dimensions
        movss xmm1, dword ptr [ebx + 4]
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        jp raw_resolve_no_dimensions
        mov compute_dimensions, 1
        jmp raw_resolve_dimensions_tested
    raw_resolve_no_dimensions:
        mov compute_dimensions, 0
    raw_resolve_dimensions_tested:
    }
    if (compute_dimensions != 0) {
        void* texture;
        Word captured_profile, captured_target;
        __asm {
            mov edi, item
            mov ecx, dword ptr [edi + 8]
            mov eax, dword ptr [ecx]
            mov edx, dword ptr [eax + 3ch]
            mov texture, ecx
            mov captured_profile, eax
            mov captured_target, edx
        }
        const Word width_result = c.dimensions.dimension_from_captured(
            texture, captured_profile, captured_target);
        volatile Word* const actual_homes = homes;
        const volatile float* const correction = &c.constants.unsigned_correction_00ce3978;
        const volatile double* const width_divisor = &c.constants.width_divisor_00cec380;
        const volatile double* const height_divisor = &c.constants.height_divisor_00cef1b8;
        __asm {
            mov edi, item
            mov ebx, actual_homes
            mov esi, correction
            mov eax, width_result
            test eax, eax
            mov dword ptr [ebx], eax
            fild dword ptr [ebx]
            jge raw_resolve_width_nonnegative
            fadd dword ptr [esi]
        raw_resolve_width_nonnegative:
            mov ecx, dword ptr [edi + 8]
            fstp dword ptr [ebx + 4]
            // AA2660 loads this pair AFTER the width spill, unlike AB17B0.
            mov eax, dword ptr [ecx]
            mov edx, dword ptr [eax + 40h]
            mov texture, ecx
            mov captured_profile, eax
            mov captured_target, edx
        }
        const Word height_result = c.dimensions.dimension_from_captured(
            texture, captured_profile, captured_target);
        __asm {
            mov edi, actual_homes
            mov esi, uv
            mov ebx, size_output
            mov ecx, correction
            mov edx, width_divisor
            mov eax, height_result
            test eax, eax
            mov dword ptr [edi], eax
            fild dword ptr [edi]
            jge raw_resolve_height_nonnegative
            fadd dword ptr [ecx]
        raw_resolve_height_nonnegative:
            fstp dword ptr [edi + 8]
            fld dword ptr [esi + 8]
            fsub dword ptr [esi]
            fstp dword ptr [edi]
            mov eax, dword ptr [edi]
            and eax, 7fffffffh
            mov dword ptr [edi], eax
            fld dword ptr [edi]
            fmul dword ptr [edi + 4]
            fdiv qword ptr [edx]
            fld dword ptr [edi + 0ch]
            fld st(0)
            fmulp st(2), st(0)
            fxch
            fstp dword ptr [ebx]
            fld dword ptr [esi + 0ch]
            fsub dword ptr [esi + 4]
            fstp dword ptr [edi]
            mov ecx, dword ptr [edi]
            and ecx, 7fffffffh
            mov dword ptr [edi], ecx
            fld dword ptr [edi]
            fmul dword ptr [edi + 8]
            mov edx, height_divisor
            fdiv qword ptr [edx]
            fmulp st(1), st(0)
            fstp dword ptr [ebx + 4]
        }
    }
    void* const retained_texture = pointer(read(item, 8));
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        pointer(bits(retained_texture) + 4u)));
    count.fetch_add(1, std::memory_order_seq_cst);
    return pointer(read(item, 8));
}
} // namespace bsp
