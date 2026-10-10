#include "bsp/native_damageable_class_back_row_selection_fragment.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

void* select_native_damageable_class_back_row_0087ce88(
    NativeDamageableClassBackRowSelectionFragmentScratch& actual_scratch) {
    void* const header = actual_scratch.actual_header_at_parent_edi;
    void* const saved_end_slot = actual_scratch.actual_saved_end_at_parent_dc;
    const auto& invalid = actual_scratch.actual_vector_access.invalid_parameters;
    std::uint32_t captured_end;
    std::uint32_t current_begin;
    __asm {
        mov edx, header
        mov eax, dword ptr [edx+8]
        mov captured_end, eax
        mov eax, dword ptr [edx+4]
        mov current_begin, eax
    }
    if (current_begin > captured_end)
        invalid.invalid_parameter(invalid.context);

    std::uint32_t candidate;
    unsigned char above_current_end;
    __asm {
        mov eax, captured_end
        sub eax, 30h
        mov candidate, eax
        mov edx, header
        cmp eax, dword ptr [edx+8]
        seta above_current_end
        mov eax, captured_end
        mov edx, saved_end_slot
        mov dword ptr [edx], eax
    }
    if (above_current_end) {
        invalid.invalid_parameter(invalid.context);
    } else {
        __asm {
            mov edx, header
            mov eax, dword ptr [edx+4]
            mov current_begin, eax
        }
        if (candidate < current_begin)
            invalid.invalid_parameter(invalid.context);
    }

    std::uint32_t row;
    std::uint32_t current_end;
    __asm {
        mov eax, captured_end
        sub eax, 30h
        mov row, eax
        mov edx, header
        mov eax, dword ptr [edx+8]
        mov current_end, eax
    }
    if (row >= current_end)
        invalid.invalid_parameter(invalid.context);
    return reinterpret_cast<void*>(row);
}

} // namespace bsp
