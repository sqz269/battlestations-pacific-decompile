#include "bsp/native_scene_property_bool_add.hpp"

#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4 && sizeof(std::uint8_t) == 1);

// Separate physical stores preserve every Original width and intermediate
// write under /O2; no memset, class dispatch or raw constructor is substituted.
__declspec(noinline) void write_word(void* root, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(
        reinterpret_cast<std::uintptr_t>(root) + offset) = value;
}

__declspec(noinline) void write_byte(void* root, Word offset, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(root) + offset) = value;
}
} // namespace

void* add_native_scene_property_bool_byte_008f3940(void* bag,
    const char* key, std::uint8_t value_bits, NativeScenePropertyInsertionContext& context) {
    void* const record = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    if (record != nullptr) {
        write_word(record, 0, 0x00ce89d4u);
        write_word(record, 4, 3);
        write_byte(record, 0x0c, value_bits);
        write_word(record, 0x18, 0);
        write_word(record, 0x1c, 0);
        write_word(record, 0x20, 0);
        write_word(record, 0x24, 0);
        write_word(record, 0x30, 0);
        write_word(record, 0x34, 0);
        write_byte(record, 0x2c, 1);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

} // namespace bsp
