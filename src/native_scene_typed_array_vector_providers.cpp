#include "bsp/native_scene_typed_array_vector_providers.hpp"

#include "bsp/native_scene_property_record_type10_raw_array_storage.hpp"
#include "bsp/native_scene_property_record_type11_raw_array_storage.hpp"
#include "bsp/native_scene_property_record_type7_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

void* add_native_scene_property_type10_array_008f3590(void* bag,
    const char* key, const void* data, std::uint32_t element_count,
    std::uint32_t copy_flag_bits, NativeScenePropertyInsertionContext& context) {
    void* const fresh = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    void* record = nullptr;
    if (fresh != nullptr) {
        record = construct_native_scene_property_record_type10_raw_array_storage_008ef3e0(
            fresh, nullptr, element_count, data, copy_flag_bits);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

void* add_native_scene_property_type11_array_008f3610(void* bag,
    const char* key, const void* data, std::uint32_t element_count,
    std::uint32_t copy_flag_bits, NativeScenePropertyInsertionContext& context) {
    void* const fresh = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    void* record = nullptr;
    if (fresh != nullptr) {
        record = construct_native_scene_property_record_type11_raw_array_storage_008ef460(
            fresh, nullptr, element_count, data, copy_flag_bits);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

void* add_native_scene_property_three_word_008f3690(void* bag,
    const char* key, const void* payload_words,
    NativeScenePropertyInsertionContext& context) {
    void* const fresh = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, 0x38});
    void* record = nullptr;
    if (fresh != nullptr) {
        // Explicit Source reuse, not an Original CALL: this raw producer has
        // the same 45 written/11 preserved bytes and ordered memory effects.
        record = construct_native_scene_property_record_type7_storage_008ef270(
            fresh, nullptr, payload_words);
    }
    return insert_native_scene_property_bag_008f33f0(bag, key, record, context);
}

} // namespace bsp
