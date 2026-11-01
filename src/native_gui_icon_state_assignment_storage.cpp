#include "bsp/native_gui_icon_state_assignment_storage.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_crt_memcpy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source assignment supplier requires MSVC Win32 inline assembly.
#endif

namespace bsp {
void* assign_native_gui_icon_state_storage_00ab3c10(
    void* actual_destination, const void* actual_source,
    std::uint32_t& actual_preserve_home, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    const void* captured_source;
    void* captured_destination;
    const void* source_header;
    void* destination_header;
    std::uint32_t name_required;
    void* captured_result;
    __asm {
        mov edi, actual_source
        mov eax, dword ptr [edi]
        mov esi, actual_destination
        lea ebx, [edi + 4]
        lea edx, [esi + 4]
        // Private Source carriers, no extra Native payload reads.
        mov captured_source, edi
        mov captured_destination, esi
        mov source_header, ebx
        mov destination_header, edx
        cmp edx, ebx
        mov dword ptr [esi], eax
        jz assign_self_name
        mov name_required, 1
        jmp assign_name_phase_done
    assign_self_name:
        mov name_required, 0
    assign_name_phase_done:
    }
    if (name_required) {
        std::uint32_t request_word;
        __asm {
            mov ebx, source_header
            mov ecx, dword ptr [ebx]
            mov request_word, ecx
        }
        static_cast<volatile std::uint32_t&>(actual_preserve_home) = 1;
        resize_native_string_header_0041dd40_exact(destination_header, strings,
            request_word, &actual_preserve_home, actual_feature_word_0109eea4);

        std::uint32_t outer_copy_required;
        std::uint32_t copy_count;
        const void* outer_source_data;
        void* outer_destination_data;
        __asm {
            // Reload captured addresses, then the late CURRENT native fields.
            mov ebx, source_header
            mov esi, destination_header
            cmp dword ptr [ebx], 0
            jz assign_no_outer_copy
            mov edx, dword ptr [esi]
            mov eax, dword ptr [ebx + 4]
            mov ecx, dword ptr [esi + 4]
            mov copy_count, edx
            mov outer_source_data, eax
            mov outer_destination_data, ecx
            mov outer_copy_required, 1
            jmp assign_outer_phase_done
        assign_no_outer_copy:
            mov outer_copy_required, 0
        assign_outer_phase_done:
        }
        if (outer_copy_required)
            (void)copy_native_crt_bytes_00bf7680(outer_destination_data,
                outer_source_data, copy_count, actual_feature_word_0109eea4);
    }
    __asm {
        mov edi, captured_source
        mov esi, captured_destination

        mov edx, dword ptr [edi + 0ch]
        mov dword ptr [esi + 0ch], edx
        mov eax, dword ptr [edi + 10h]
        mov dword ptr [esi + 10h], eax
        mov ecx, dword ptr [edi + 14h]
        mov dword ptr [esi + 14h], ecx
        mov edx, dword ptr [edi + 18h]
        mov dword ptr [esi + 18h], edx
        mov eax, dword ptr [edi + 1ch]
        mov dword ptr [esi + 1ch], eax
        mov ecx, dword ptr [edi + 20h]
        mov dword ptr [esi + 20h], ecx
        mov edx, dword ptr [edi + 24h]
        mov dword ptr [esi + 24h], edx
        mov eax, dword ptr [edi + 28h]
        mov dword ptr [esi + 28h], eax
        fld dword ptr [edi + 2ch]
        fstp dword ptr [esi + 2ch]
        mov eax, esi
        mov captured_result, eax
        fld dword ptr [edi + 30h]
        fstp dword ptr [esi + 30h]
        fld dword ptr [edi + 34h]
        fstp dword ptr [esi + 34h]
        fld dword ptr [edi + 38h]
        fstp dword ptr [esi + 38h]
        mov cl, byte ptr [edi + 3ch]
        mov byte ptr [esi + 3ch], cl
        mov dl, byte ptr [edi + 3dh]
        mov byte ptr [esi + 3dh], dl
    }
    return captured_result;
}
} // namespace bsp
