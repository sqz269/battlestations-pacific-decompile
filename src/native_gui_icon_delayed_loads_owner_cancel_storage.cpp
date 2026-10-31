#include "bsp/native_gui_icon_delayed_loads_owner_cancel_storage.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

void* pointer_from_word(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word read_word(const void* base, Word byte_offset) {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void write_word(void* base, Word byte_offset, Word value) {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
std::int32_t signed_word(Word value) {
    return static_cast<std::int32_t>(value); // MSVC Win32 DWORD interpretation.
}
} // namespace

void cancel_native_gui_icon_delayed_loads_owner_storage_00ab50e0(
    void* actual_delayed_owner16, NativeGuiIconDelayedLoadsOwnerCancelStorageFrame& frame,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    void* const owner = actual_delayed_owner16; // Captured EBX, no owner cast.
    Word index = 0u; // Native EAX register role, separate from saved HOME.
    const Word initial_count = read_word(owner, 8u); // AB50E6 BEFORE HOME0.
    write_word(&frame.saved_index_home, 0u, 0u); // AB50E9, even empty return.
    if (signed_word(initial_count) <= 0) return;
    Word displacement = 0u; // Native EBP, never recomputed from reloaded index.
    for (;;) {
        const Word iteration_data = read_word(owner, 4u); // AB5100 FIRST.
        const Word owner_argument = read_word(&frame.owner_argument_home, 0u); // 5103.
        const Word entry_owner = read_word(pointer_from_word(iteration_data + displacement), 8u);
        if (entry_owner != owner_argument) {
            ++index; // AB5192 wrap32, only the unmatched path advances.
            write_word(&frame.saved_index_home, 0u, index);
            displacement += 12u;
        } else if (index != 0xffffffffu) { // Actual matched sentinel branch 5111.
            const Word matched_count = read_word(owner, 8u); // AB511A.
            if (index != matched_count - 1u) {
                const Word assignment_data = read_word(owner, 4u); // ONE base, 5124.
                const Word source = assignment_data + matched_count * 12u - 12u;
                const Word destination = assignment_data + displacement;
                if (destination != source) { // Assignment, never constructor.
                    const Word request = read_word(pointer_from_word(source), 0u);
                    write_word(&frame.preserve_home, 0u, 1u); // Full PUSH1 HOME.
                    resize_native_string_header_0041dd40_exact(pointer_from_word(destination),
                        strings, request, &frame.preserve_home, actual_feature_word_0109eea4);
                    if (read_word(pointer_from_word(source), 0u) != 0u) { // CURRENT 5141.
                        const Word count = read_word(pointer_from_word(destination), 0u);
                        const Word source_data = read_word(pointer_from_word(source), 4u);
                        const Word destination_data = read_word(pointer_from_word(destination), 4u);
                        // Native BF call remains reached even when count==0.
                        (void)copy_native_crt_bytes_00bf7680(pointer_from_word(destination_data),
                            pointer_from_word(source_data), count, actual_feature_word_0109eea4);
                    }
                }
                const Word value = read_word(pointer_from_word(source), 8u); // Late 5159.
                write_word(pointer_from_word(destination), 8u, value); // Even self.
            }
            const Word tail_count = read_word(owner, 8u); // 515F FIRST, after callbacks.
            const Word tail_data = read_word(owner, 4u); // 5162 SECOND.
            const Word tail = tail_data + tail_count * 12u - 12u;
            destroy_native_string_header_0041dd20(pointer_from_word(tail), strings);
            const Word current_count = read_word(owner, 8u); // AFTER release, 5188.
            write_word(owner, 8u, current_count - 1u);
            index = read_word(&frame.saved_index_home, 0u); // 518C, not displacement.
        }
        // Also reached for a matched FFFFFFFF register index, without removal.
        if (signed_word(index) >= signed_word(read_word(owner, 8u))) break; // 519C.
    }
}
} // namespace bsp
