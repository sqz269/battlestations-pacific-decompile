#include "bsp/native_gui_named_value_array_append_storage.hpp"
#include "bsp/native_gui_icon_delayed_loads.hpp"

#include <new>
#include <type_traits>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeGuiIconDelayedEntryStorage) == 12);
static_assert(std::is_trivial_v<NativeGuiIconDelayedEntryStorage>);

Word bits(const void* pointer) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}
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

void append_native_gui_named_value_array_storage_00ab4990(
    void* actual_header12, volatile std::uint32_t& actual_source_argument_home,
    NativeGuiNamedValueArrayAppendStorageFrame& frame, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    write_word(&frame.native_state_home, 0u, 0xffffffffu); // Native PUSH -1.
    void* const header = actual_header12; // Captured ESI, not saved HOME authority.
    const Word capacity = read_word(header, 8u); // AB49AB FIRST.
    const Word initial_count = read_word(header, 4u); // AB49AE SECOND.
    const bool full = initial_count == capacity;
    write_word(&frame.saved_header_home, 0u, bits(header)); // AB49B1 before branch.
    if (full) {
        const Word doubled = capacity + capacity; // AB49B7 wrap32.
        const Word request = signed_word(doubled) > 1 ? doubled : 1u;
        write_word(&frame.reserve.requested_capacity_home, 0u, request); // AB49C3.
        reserve_native_gui_named_value_array_storage_00ab4720(
            header, frame.reserve, strings, actual_feature_word_0109eea4);
    }

    const Word count_for_slot = read_word(header, 4u); // AB49C9 after reserve/skip.
    const Word data_for_slot = read_word(header, 0u); // AB49CC.
    const Word captured_slot = data_for_slot + count_for_slot * 12u;
    write_word(&frame.saved_slot_home, 0u, captured_slot); // AB49D4.
    const bool nonnull_slot = captured_slot != 0u; // AB49D8.
    write_word(&frame.native_state_home, 0u, 0u); // State0 even for null skip.
    if (nonnull_slot) {
        void* const slot = pointer_from_word(captured_slot);
        // Individual trivial default-init, NO ()/{} and no T[n] family claim.
        ::new (slot) NativeGuiIconDelayedEntryStorage;
        const Word source = actual_source_argument_home; // CURRENT HOME, AB49E4.
        (void)copy_construct_native_gui_named_value_storage_00ab4690(
            slot, pointer_from_word(source), frame.copy, strings,
            actual_feature_word_0109eea4);
    }
    const Word current_count = read_word(header, 4u); // AB49EE, after copy/null skip.
    write_word(header, 4u, current_count + 1u); // Native wrapping ADD, no old count.
}
} // namespace bsp
