#include "bsp/native_scene_property_enum_add.hpp"

#include "bsp/native_enum_dictionary_word_getters.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4 && sizeof(std::uint8_t) == 1);

// Separate physical stores preserve each Original width and intermediate
// write under /O2. No raw constructor, bulk initialization or patch follows.
__declspec(noinline) void write_word(void* root, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(
        reinterpret_cast<std::uintptr_t>(root) + offset) = value;
}

__declspec(noinline) void write_byte(void* root, Word offset, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(root) + offset) = value;
}
} // namespace

void* add_native_scene_property_enum_word_008f39b0(void* bag, const char* key,
    Word declaration_bits, Word value_bits, NativeScenePropertyInsertionContext& context) {
    void* const record = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    if (record != nullptr) {
        write_word(record, 0, 0x00ce89d4u);
        write_word(record, 4, 4);
        write_word(record, 0x18, 0);
        write_word(record, 0x1c, 0);
        write_word(record, 0x20, 0);
        write_word(record, 0x24, 0);
        write_word(record, 0x28, declaration_bits);
        write_word(record, 0x30, 0);
        write_word(record, 0x34, 0);
        write_word(record, 0x0c, value_bits);
        write_byte(record, 0x2c, 1);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

void* add_native_scene_property_enum_symbol_008f3a20(void* bag, const char* key,
    const void* actual_enum, const char* symbol, game::GameNativeStringProcess& strings,
    NativeScenePropertyInsertionContext& context) {
    void* const record = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    if (record != nullptr) {
        write_word(record, 0, 0x00ce89d4u);
        write_word(record, 4, 4);
        write_word(record, 0x18, 0);
        write_word(record, 0x1c, 0);
        write_word(record, 0x20, 0);
        write_word(record, 0x24, 0);
        write_word(record, 0x30, 0);
        write_word(record, 0x28, reinterpret_cast<Word>(actual_enum));
        const Word value_bits = read_native_enum_symbol_word_0048e840(actual_enum,
            symbol, strings.raw_context(), strings.enum_empty_00e186ed(),
            strings.query_empty_00e17654());
        write_word(record, 0x0c, value_bits);
        write_word(record, 0x34, 0);
        write_byte(record, 0x2c, 1);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

} // namespace bsp
