#include "bsp/native_scene_property_reference_subbag_add.hpp"

#include "bsp/native_string_duplicate.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4 && sizeof(std::uint8_t) == 1);

// Independent physical stores preserve every width/order under /O2. The
// ordinary helper ABI/stack timing is not the Original instruction schedule.
__declspec(noinline) void write_word(void* root, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(
        reinterpret_cast<std::uintptr_t>(root) + offset) = value;
}

__declspec(noinline) void write_byte(void* root, Word offset, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(root) + offset) = value;
}

Word pointer_word(const void* actual_pointer) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(actual_pointer));
}
} // namespace

// Whole Original150B SHA256:
// f7a7ff20da135588626d27c811df9b0480f15ab1af0b756e19922622e1a07909.
void* add_native_scene_property_reference_008f3ac0(void* bag, const char* key,
    const char* nullable_target, Word word_18_bits, Word word_08_bits,
    NativeScenePropertyInsertionContext& context) {
    void* const record = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    if (record != nullptr) {
        write_word(record, 0, 0x00ce89d4u);
        write_word(record, 4, 5);
        write_word(record, 8, word_08_bits);
        write_word(record, 0x18, word_18_bits);
        char* const duplicate = duplicate_native_string_00438e40(nullable_target);
        write_word(record, 0x1c, pointer_word(duplicate));
        write_word(record, 0x20, 0);
        write_word(record, 0x24, 0);
        write_word(record, 0x34, 0);
        write_byte(record, 0x2c, 1);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

// Whole Original106B SHA256:
// 80a0988b31919688b721d0d728275fb68843e438b1d0524a5b1ae8e0ef20e634.
void* add_native_scene_property_subbag_008f3b60(void* bag, const char* key,
    void* actual_owned_child, NativeScenePropertyInsertionContext& context) {
    void* const record = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    if (record != nullptr) {
        write_word(record, 0, 0x00ce89d4u);
        write_word(record, 4, 6);
        write_word(record, 0x0c, pointer_word(actual_owned_child));
        write_word(record, 0x18, 0);
        write_word(record, 0x1c, 0);
        write_word(record, 0x20, 0);
        write_word(record, 0x24, 0);
        write_word(record, 0x30, 0);
        write_word(actual_owned_child, 0x110, pointer_word(record));
        write_word(record, 0x34, 0);
        write_word(record, 8, 0);
        write_byte(record, 0x2c, 1);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

} // namespace bsp
