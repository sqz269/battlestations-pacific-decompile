#include "bsp/native_scene_registry_mutation_adapters.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene registry mutation adapters require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& cell(void* p, Word offset) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<unsigned char*>(p) + offset);
}
Word address(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
Word sar3(Word value) noexcept {
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}
void invalid(NativeSceneRegistryVectorMutationBindings& bindings) {
    const auto service = bindings.actual_00bf6713;
    if (!service) throw std::logic_error("missing genuine returning BF6713 binding");
    service();
}
struct CompletedMessage {
    NativeLegacySboStringStorage& message;
    ~CompletedMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(message); }
};
}

void resize_native_scene_registry_boundaries_00b83490(void* vector,
    NativeSceneRegistryResizeFrame& frame, NativeSceneRegistryVectorMutationBindings& bindings) {
    const Word initial_begin = cell(vector, 4);
    const Word initial_size = initial_begin ? sar3(cell(vector, 8) - initial_begin) : 0u;
    const Word requested = frame.requested_count;
    if (initial_size < requested) {
        const Word repeated_size = initial_begin ? sar3(cell(vector, 8) - initial_begin) : 0u;
        const Word end = cell(vector, 8);
        if (initial_begin > end) invalid(bindings); // B834C8
        frame.insert.pair_or_work_18 = address(frame.pair_argument);
        frame.insert.count_14 = requested - repeated_size;
        frame.insert.position_10 = end;
        frame.insert.iterator_owner_0c = address(vector);
        insert_native_scene_registry_pairs_00b82fd0(vector, frame.insert, bindings.storage); // B834D9
        return;
    }
    if (initial_begin == 0) return;
    const Word end = cell(vector, 8);
    if (requested >= sar3(end - initial_begin)) return;
    if (initial_begin > end) invalid(bindings); // B834FB
    const Word begin = cell(vector, 4);
    if (begin > cell(vector, 8)) invalid(bindings); // B83508
    frame.pair_argument[1] = begin; // B8350D: original incoming pair word1
    const Word first = begin + requested * 8u;
    if (first > cell(vector, 8) || first < cell(vector, 4)) invalid(bindings); // B8351E
    frame.erase.last_position_10 = end;
    frame.erase.last_owner_0c = address(vector);
    frame.erase.first_position_08 = first;
    frame.erase.first_owner_04 = address(vector);
    frame.erase.result_00 = address(frame.pair_argument);
    erase_native_scene_registry_pairs_00b82bf0(vector, frame.erase, bindings); // B8352E
}

void assign_native_scene_registry_boundaries_00b83560(void* vector,
    NativeSceneRegistryAssignFrame& frame, NativeSceneRegistryVectorMutationBindings& bindings) {
    const Word incoming = frame.pair_source_argument;
    const Word pair1 = *reinterpret_cast<volatile Word*>(incoming + 4u);
    const Word pair0 = *reinterpret_cast<volatile Word*>(incoming);
    const Word end = cell(vector, 8);
    const bool initial_invalid = cell(vector, 4) > end;
    frame.captured_pair[0] = pair0;
    frame.captured_pair[1] = pair1;
    if (initial_invalid) invalid(bindings); // B83580, AFTER local pair stores
    const Word begin = cell(vector, 4);
    if (begin > cell(vector, 8)) invalid(bindings); // B8358E
    frame.erase.last_position_10 = end;
    frame.erase.last_owner_0c = address(vector);
    frame.erase.first_position_08 = begin;
    frame.erase.first_owner_04 = address(vector);
    frame.erase.result_00 = address(frame.erase_result);
    erase_native_scene_registry_pairs_00b82bf0(vector, frame.erase, bindings); // B8359E
    const Word current_begin = cell(vector, 4);
    if (current_begin > cell(vector, 8)) invalid(bindings); // B835AC
    const Word current_count = frame.count_argument; // B835B1 AFTER callbacks
    frame.insert.pair_or_work_18 = address(frame.captured_pair);
    frame.insert.count_14 = current_count;
    frame.insert.position_10 = current_begin;
    frame.insert.iterator_owner_0c = address(vector);
    insert_native_scene_registry_pairs_00b82fd0(vector, frame.insert, bindings.storage); // B835BF
}

void grow_native_scene_registry_list_size_00b82d30(void* list, Word increment,
    NativeSceneRegistryListGrowthScratch& scratch, const char* message) {
    const Word count = cell(list, 8);
    if (0x3fffffffu - count < increment) {
        scratch.message.capacity_18 = 15;
        scratch.message.length_14 = 0;
        scratch.message.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_counted_00408720(scratch.message, message, 16); // B82D7C
        const CompletedMessage completed{scratch.message}; // state0 at B82D89
        throw NativeHardwareLayoutTreeLengthError{scratch.message}; // 411700 then D69260/throw transport
    }
    cell(list, 8) = count + increment; // B82DAD/B82DAF; no callback/count reload
}
} // namespace bsp
