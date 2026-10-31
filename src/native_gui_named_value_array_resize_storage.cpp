#include "bsp/native_gui_named_value_array_resize_storage.hpp"
#include "bsp/native_gui_icon_delayed_loads.hpp"
#include "bsp/native_gui_named_value_array_reserve_storage.hpp"
#include "bsp/native_string.hpp"

#include <new>
#include <type_traits>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeGuiIconDelayedEntryStorage) == 12);
static_assert(std::is_trivial_v<NativeGuiIconDelayedEntryStorage>);

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

void resize_native_gui_named_value_array_storage_00ab4850(
    void* actual_header12, std::uint32_t requested_count,
    NativeGuiNamedValueArrayReserveStorageFrame& reserve_frame,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    const Word requested = requested_count; // Captured EDI at AB4853, once.
    void* const header = actual_header12; // Captured ESI at AB4857.
    if (signed_word(requested) > signed_word(read_word(header, 8u))) {
        write_word(&reserve_frame.requested_capacity_home, 0u, requested); // 485E.
        reserve_native_gui_named_value_array_storage_00ab4720(
            header, reserve_frame, strings, actual_feature_word_0109eea4);
    }
    const Word growth_count = read_word(header, 4u); // CURRENT after reserve/skip.
    if (signed_word(growth_count) < signed_word(requested)) {
        Word cursor = growth_count * 12u;
        Word remaining = requested - growth_count;
        do { // Native enters once, then tests the wrapping countdown for zero.
            const Word current_data = read_word(header, 0u);
            const Word slot = current_data + cursor;
            if (slot != 0u) {
                // Individual no-write default lifetime start, not a T[n] array.
                ::new (pointer_from_word(slot)) NativeGuiIconDelayedEntryStorage;
                write_word(pointer_from_word(slot), 0u, 0u);
                write_word(pointer_from_word(slot), 4u, 0u);
                write_word(pointer_from_word(slot), 8u, 0u);
            }
            cursor += 12u;
            --remaining;
        } while (remaining != 0u);
    }
    // Independent fresh test: growth writes/callbacks can change the descriptor.
    if (signed_word(requested) < signed_word(read_word(header, 4u))) {
        do {
            const Word count_before_decrement = read_word(header, 4u);
            write_word(header, 4u, count_before_decrement - 1u); // AB4893 first.
            const Word updated_count = read_word(header, 4u); // AB4897 reload.
            const Word current_data = read_word(header, 0u); // AB489A after count.
            const Word tail = current_data + updated_count * 12u;
            destroy_native_string_header_0041dd20(pointer_from_word(tail), strings);
        } while (signed_word(requested) < signed_word(read_word(header, 4u)));
    }
    write_word(header, 4u, requested); // AB48C3 on EVERY normal path, original EDI.
}
} // namespace bsp
