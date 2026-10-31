#include "bsp/native_gui_icon_state_getter_storage.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source getter requires MSVC Win32 inline assembly.
#endif

namespace bsp {
void* get_native_gui_icon_state_storage_00ab1230(void* actual_vector_f0,
    const volatile std::uint32_t& actual_index_home) {
    void* const captured_vector = actual_vector_f0;
    const volatile std::uint32_t* const index_address = &actual_index_home;
    std::uint32_t captured_index;
    std::uint32_t validation_required;
    void* result;
    __asm {
        // Source ABI address bindings only, before Native operand loads.
        mov ecx, captured_vector
        mov edx, index_address

        mov esi, ecx
        mov ecx, dword ptr [esi + 4]
        test ecx, ecx
        mov edi, dword ptr [edx]
        mov captured_index, edi
        jz state_get_validate
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
        sar eax, 6
        cmp edi, eax
        jb state_get_valid
    state_get_validate:
        mov validation_required, 1
        jmp state_get_first_phase_done
    state_get_valid:
        mov validation_required, 0
    state_get_first_phase_done:
    }
    if (validation_required)
        invoke_native_invalid_parameter_00bf6713();
    __asm {
        // Restore captured Source carriers; never reread the index home.
        mov esi, captured_vector
        mov edi, captured_index

        mov eax, edi
        shl eax, 6
        add eax, dword ptr [esi + 4]
        mov result, eax
    }
    return result;
}
} // namespace bsp
