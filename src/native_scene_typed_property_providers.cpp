#include "bsp/native_scene_typed_property_providers.hpp"

#include "bsp/native_enum_dictionary_word_getters.hpp"
#include "bsp/native_scene_property_record_type8_byte_array_storage.hpp"
#include "bsp/native_scene_property_record_type9_four_byte_array_storage.hpp"
#include "bsp/native_string_duplicate.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

// Physical accesses remain separate from external calls under /O2. These
// helpers add no phase dispatch, provider indirection or ownership policy.
__declspec(noinline) Word read_word(const void* root, Word offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        reinterpret_cast<std::uintptr_t>(root) + offset);
}
__declspec(noinline) void write_word(void* root, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(
        reinterpret_cast<std::uintptr_t>(root) + offset) = value;
}
} // namespace

void* assign_native_scene_property_from_token_008f3370(void* record,
    const char* token, game::GameNativeStringProcess& strings) {
    if (read_word(record, 4) == 2) {
        void* const old = reinterpret_cast<void*>(read_word(record, 0x0c));
        if (old != nullptr) {
            singleton_lifetime_free(old);
            write_word(record, 0x0c, 0);
        }
        char* const replacement = duplicate_native_string_00438e40(token);
        write_word(record, 0x0c, reinterpret_cast<Word>(replacement));
    } else {
        const void* const declaration = reinterpret_cast<const void*>(
            read_word(record, 0x28));
        const Word value = read_native_enum_symbol_word_0048e840(declaration,
            token, strings.raw_context(), strings.enum_empty_00e186ed(),
            strings.query_empty_00e17654());
        write_word(record, 0x0c, value);
    }
    return record;
}

void* add_native_scene_property_byte_array_008f3490(void* bag,
    const char* key, const void* data, Word byte_count, Word copy_flag_bits,
    NativeScenePropertyInsertionContext& context) {
    void* const fresh = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    void* record = nullptr;
    if (fresh != nullptr) {
        record = construct_native_scene_property_record_type8_byte_array_storage_008ef2f0(
            fresh, nullptr, data, byte_count, copy_flag_bits);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

void* add_native_scene_property_four_byte_array_008f3510(void* bag,
    const char* key, const void* data, Word element_count, Word copy_flag_bits,
    NativeScenePropertyInsertionContext& context) {
    void* const fresh = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    void* record = nullptr;
    if (fresh != nullptr) {
        record = construct_native_scene_property_record_type9_four_byte_array_storage_008ef360(
            fresh, nullptr, element_count, data, copy_flag_bits);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

} // namespace bsp
