#include "bsp/native_gui_named_value_array_reserve_storage.hpp"
#include "bsp/native_gui_icon_delayed_loads.hpp"
#include "bsp/singleton_lifetime.hpp"

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
Word read_word(const void* base, Word offset) {
    Word value;
    __asm {
        mov eax, base
        mov edx, offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void write_word(void* base, Word offset, Word value) {
    __asm {
        mov eax, base
        mov edx, offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
std::int32_t signed_word(Word value) {
    return static_cast<std::int32_t>(value); // MSVC Win32 DWORD interpretation.
}
} // namespace

void reserve_native_gui_named_value_array_storage_00ab4720(
    void* actual_header12, NativeGuiNamedValueArrayReserveStorageFrame& frame,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    write_word(&frame.native_state_home, 0u, 0xffffffffu); // Native PUSH -1.
    void* const header = actual_header12;
    Word selected_capacity = read_word(&frame.requested_capacity_home, 0u);
    if (signed_word(selected_capacity) < 1) {
        write_word(&frame.requested_capacity_home, 0u, 1u); // AB4745.
        selected_capacity = read_word(&frame.requested_capacity_home, 0u);
    }
    if (signed_word(read_word(header, 8u)) >= signed_word(selected_capacity)) return;

    const Word byte_count = selected_capacity * 12u; // Native wrap32, no cap.
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, byte_count, byte_count}); // BF55BE.
    Word copy_index = 0u; // Native EBP, independent of its saved HOME.
    const Word initial_copy_count = read_word(header, 4u); // AB476E, before homes.
    Word replacement = bits(allocation); // Native EDI, independent of its HOME.
    write_word(&frame.replacement_home, 0u, replacement);
    write_word(&frame.copy_index_home, 0u, copy_index);
    if (signed_word(initial_copy_count) > 0) {
        for (;;) {
            const Word displacement = copy_index * 12u;
            const Word destination = replacement + displacement;
            write_word(&frame.destination_home, 0u, destination);
            const bool has_destination = destination != 0u; // AB4791.
            write_word(&frame.native_state_home, 0u, 0u); // Before null skip.
            if (has_destination) {
                const Word source = read_word(header, 0u) + displacement;
                void* const slot = pointer_from_word(destination);
                // Default-init ONLY the reached individual trivial object.
                // No ()/{}: native-unwritten bytes remain the slot preimage.
                ::new (slot) NativeGuiIconDelayedEntryStorage;
                (void)copy_construct_native_gui_named_value_storage_00ab4690(
                    slot, pointer_from_word(source), frame.copy, strings,
                    actual_feature_word_0109eea4);
                // Native 47CA read8 -> 47CD reload -> 47D1 store8. Whole54
                // already performed read/store; this pure private reload
                // commutes only in the header's separated normal domain.
                replacement = read_word(&frame.replacement_home, 0u);
            }
            ++copy_index; // Includes the real computed-null skip.
            const Word current_copy_count = read_word(header, 4u);
            const bool again = signed_word(copy_index) < signed_word(current_copy_count);
            write_word(&frame.native_state_home, 0u, 0xffffffffu); // AB47DA.
            write_word(&frame.copy_index_home, 0u, copy_index); // AFTER compare.
            if (!again) break;
        }
        selected_capacity = read_word(&frame.requested_capacity_home, 0u); // 47E8 only.
    }

    Word release_index = 0u;
    const Word initial_release_count = read_word(header, 4u); // AB47EE.
    if (signed_word(initial_release_count) > 0) {
        Word displacement = 0u;
        for (;;) {
            const Word entry = read_word(header, 0u) + displacement;
            // Normal AB47F5..4811 matches raw41DD20/AB39F0: DATA first,
            // current LEN+1, fresh getter/return; no header clears/destructor.
            destroy_native_string_header_0041dd20(pointer_from_word(entry), strings);
            ++release_index;
            displacement += 12u;
            if (signed_word(release_index) >= signed_word(read_word(header, 4u))) break;
        }
        selected_capacity = read_word(&frame.requested_capacity_home, 0u); // 4821 only.
    }

    void* const current_old_data = pointer_from_word(read_word(header, 0u)); // 4825.
    singleton_lifetime_free(current_old_data); // Free CURRENT, not captured old data.
    write_word(header, 0u, replacement); // 4830: captured EDI, no fresh HOME.
    write_word(header, 8u, selected_capacity); // 4833: selected captured ESI.
}
} // namespace bsp
