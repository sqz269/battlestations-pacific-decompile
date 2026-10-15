#include "bsp/native_scene_property_string_add.hpp"

#include "bsp/native_string_duplicate.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4 && sizeof(std::uint8_t) == 1);

// Keep the separate physical writes ordered across the genuine raw duplicate.
// These helpers add no phase dispatch, provider callbacks or ownership policy.
__declspec(noinline) void write_word(void* root, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(
        reinterpret_cast<std::uintptr_t>(root) + offset) = value;
}

__declspec(noinline) void write_byte(void* root, Word offset, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(root) + offset) = value;
}
} // namespace

void* add_native_scene_property_string_008f38a0(void* bag, const char* key,
    const char* value, NativeScenePropertyInsertionContext& context) {
    void* const record = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    if (record != nullptr) {
        write_word(record, 0, 0x00ce89d4u);
        write_word(record, 4, 2);
        write_word(record, 0x18, 0);
        write_word(record, 0x1c, 0);
        write_word(record, 0x20, 0);
        write_word(record, 0x24, 0);
        write_word(record, 0x30, 0);
        char* const owned_value = duplicate_native_string_00438e40(value);
        write_word(record, 0x0c, reinterpret_cast<Word>(owned_value));
        write_word(record, 0x34, 0);
        write_byte(record, 0x2c, 1);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

} // namespace bsp
