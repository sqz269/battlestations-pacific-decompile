#include "bsp/native_scene_property_clone.hpp"

#include "bsp/classes_seg7_cc13.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_scene_property_bag_storage.hpp"
#include "bsp/native_scene_property_record_element_count.hpp"
#include "bsp/native_scene_property_record_type0_storage.hpp"
#include "bsp/native_scene_property_record_type1_float_storage.hpp"
#include "bsp/native_scene_property_record_type2_string_storage.hpp"
#include "bsp/native_scene_property_record_type3_byte_storage.hpp"
#include "bsp/native_scene_property_record_type4_storage.hpp"
#include "bsp/native_scene_property_record_type5_storage.hpp"
#include "bsp/native_scene_property_record_type6_storage.hpp"
#include "bsp/native_scene_property_record_type7_storage.hpp"
#include "bsp/native_scene_property_record_type8_byte_array_storage.hpp"
#include "bsp/native_scene_property_record_type9_four_byte_array_storage.hpp"
#include "bsp/native_scene_property_record_type10_raw_array_storage.hpp"
#include "bsp/native_scene_property_record_type11_raw_array_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "The original property storage is Win32.");

// Physical loads/stores retain the actual record/key reads and their ordering.
// No fabricated C++ record type or Source vtable is introduced.
__forceinline std::uint32_t load_word(const void* root, std::size_t byte_offset) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}

__forceinline std::uint8_t load_byte(const void* root, std::size_t byte_offset) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, root
        mov edx, byte_offset
        movzx eax, byte ptr [eax + edx]
        mov value, eax
    }
    return static_cast<std::uint8_t>(value);
}

__forceinline void store_word(void* root, std::size_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}

void* pointer(std::uint32_t bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

// Actual m32 x87 conversion from Original008F4FD4..008F4FDD. The integer
// result prevents an implicit FP return/reload. Exactly9 bytes/5 instructions:
// D9410C 51 D91C24 58 C3. No CW/status change or extra FP operation is added.
static __declspec(naked) std::uint32_t __fastcall
clone_type1_x87_spill_008f4fd4(const void*) noexcept {
    __asm {
        fld dword ptr [ecx + 0ch]
        push ecx
        fstp dword ptr [esp]
        pop eax
        ret
    }
}
} // namespace

NativeScenePropertyCloneContext::NativeScenePropertyCloneContext(
    game::GameNativeStringProcess& actual_strings,
    game::GameNativePhysicalPoolProcess& actual_physical)
    : insertion_(actual_strings, actual_physical),
      clone_empty_(actual_strings.clone_empty_00f89450()) {}

// Whole normal loop of Original008F41F0,154 bytes/49 instructions. Both
// mutually recursive functions use this SAME concrete borrowed context.
__declspec(noinline) void* clone_native_scene_property_bag_008f41f0(
    const void* actual_source_bag, NativeScenePropertyCloneContext& context) {
    void* const output = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x114, 0x114});
    initialize_native_scene_property_bag_storage_008f41a0(output, 0, 0);

    alignas(std::uint32_t) std::uint8_t iterator[12];
    const auto* const actual_map = static_cast<const std::byte*>(actual_source_bag) + 4;
    store_word(iterator, 0, static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_map)));
    cc13_seg7::property_bag_iterator_begin_00480690(iterator);
    for (;;) {
        void* const node = pointer(load_word(iterator, 4));
        if (node == nullptr) break;
        const auto key_word = load_word(node, 4);
        const char* const captured_key = key_word != 0
            ? static_cast<const char*>(pointer(key_word)) : context.clone_empty();
        const void* const source_record = pointer(load_word(node, 8));
        void* const cloned_record = clone_native_scene_property_record_008f4f60(
            source_record, context);
        insert_native_scene_property_bag_008f33f0(output, captured_key,
            cloned_record, context.insertion());
        cc13_seg7::property_bag_iterator_next_0047e480(iterator);
    }
    return output;
}

// Whole Original008F4F60..008F52B6,854 bytes/274 instructions. Original
// root-null write fault and seven EH cleanup states are outside this successful
// ordinary domain; no replacement failure cleanup or callback factory is added.
__declspec(noinline) void* clone_native_scene_property_record_008f4f60(
    const void* actual_source_record, NativeScenePropertyCloneContext& context) {
    const auto tag = load_word(actual_source_record, 4);
    if (tag > 11) return nullptr;
    void* const root = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    void* result = nullptr;
    switch (tag) {
    case 0: {
        const auto value = load_word(actual_source_record, 0x0c);
        result = initialize_native_scene_property_record_type0_storage_008ef140(root, 0, value);
        break;
    }
    case 1: {
        const auto spilled = clone_type1_x87_spill_008f4fd4(actual_source_record);
        result = construct_native_scene_property_record_type1_float_storage_008ef170(root, nullptr, spilled);
        break;
    }
    case 2: {
        const auto* text = static_cast<const char*>(pointer(load_word(actual_source_record, 0x0c)));
        result = construct_native_scene_property_record_type2_string_storage_008ef1b0(root, nullptr, text);
        break;
    }
    case 3: {
        const auto value = load_byte(actual_source_record, 0x0c);
        result = initialize_native_scene_property_record_type3_byte_storage_008ef1f0(root, 0, value);
        break;
    }
    case 4: {
        const auto value = load_word(actual_source_record, 0x0c);
        const auto identity = load_word(actual_source_record, 0x28);
        result = construct_native_scene_property_record_type4_storage_008ef230(root, nullptr, identity, value);
        break;
    }
    case 5: {
        const auto word08 = load_word(actual_source_record, 0x08);
        const auto word18 = load_word(actual_source_record, 0x18);
        const auto* text = static_cast<const char*>(pointer(load_word(actual_source_record, 0x1c)));
        result = construct_native_scene_property_record_type5_storage_008ef2b0(root, nullptr, text, word18, word08);
        break;
    }
    case 6: {
        const void* const source_child = pointer(load_word(actual_source_record, 0x0c));
        void* const child = clone_native_scene_property_bag_008f41f0(source_child, context);
        result = construct_native_scene_property_record_type6_storage_008ef780(root, 0, child);
        break;
    }
    case 7: {
        const auto* words = static_cast<const std::byte*>(actual_source_record) + 0x0c;
        result = construct_native_scene_property_record_type7_storage_008ef270(root, nullptr, words);
        break;
    }
    case 8: {
        const auto byte_count = load_word(actual_source_record, 0x24);
        const void* const bytes = pointer(load_word(actual_source_record, 0x20));
        result = construct_native_scene_property_record_type8_byte_array_storage_008ef2f0(
            root, nullptr, bytes, byte_count, 1);
        break;
    }
    case 9: {
        const void* const bytes = pointer(load_word(actual_source_record, 0x20));
        const auto count = native_scene_property_record_element_count_008ef7f0(actual_source_record, 0);
        result = construct_native_scene_property_record_type9_four_byte_array_storage_008ef360(
            root, nullptr, count, bytes, 1);
        break;
    }
    case 10: {
        const void* const bytes = pointer(load_word(actual_source_record, 0x20));
        const auto count = native_scene_property_record_element_count_008ef7f0(actual_source_record, 0);
        result = construct_native_scene_property_record_type10_raw_array_storage_008ef3e0(
            root, nullptr, count, bytes, 1);
        break;
    }
    case 11: {
        const void* const bytes = pointer(load_word(actual_source_record, 0x20));
        const auto count = native_scene_property_record_element_count_008ef7f0(actual_source_record, 0);
        result = construct_native_scene_property_record_type11_raw_array_storage_008ef460(
            root, nullptr, count, bytes, 1);
        break;
    }
    }
    const auto ordinal = load_word(actual_source_record, 0x34);
    store_word(result, 0x34, ordinal);
    return result;
}

} // namespace bsp
