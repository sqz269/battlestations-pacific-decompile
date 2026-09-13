#include "bsp/native_input_keyboard_storage.hpp"
#include "bsp/native_input_device_index.hpp"
#include "bsp/native_input_vector_map_index.hpp"
#include "bsp/native_input_scale_maps.hpp"
#include "bsp/native_input_keyboard_iterators.hpp"
#include "bsp/native_input_settings_tree_cleanup.hpp"

namespace bsp {
void* NativeInputKeyboardStorage::device_0055c110(void* tree, const void* key) {
    return index_native_input_device_tree_0055c110(tree, static_cast<const NativeString*>(key), strings_);
}
void* NativeInputKeyboardStorage::input_codes_006a44b0(void* tree, const void* key) {
    return index_native_input_word_vector_006a44b0(tree, static_cast<const NativeString*>(key), strings_);
}
void* NativeInputKeyboardStorage::copy_scales_0055b400(void* tree, const void* source) {
    return copy_native_input_scale_map_0055b400(tree, source);
}
void NativeInputKeyboardStorage::destroy_scales_0055b490(void* tree) {
    destroy_native_input_scale_map_0055b490(tree);
}
float* NativeInputKeyboardStorage::multiplier_00444be0(void* tree, const void* key) {
    return lookup_or_insert_native_input_float_00444be0(tree, key, strings_);
}
void* NativeInputKeyboardStorage::code_classes_006a5aa0(void* tree, const std::int32_t* key) {
    return lookup_or_insert_native_input_scalar_tree_006a5aa0(tree, key);
}
NativeKeyboardTreeIterator* NativeInputKeyboardStorage::find_hack_00546840(
    void* tree, NativeKeyboardTreeIterator* output, const std::int32_t* key) {
    return find_native_input_hack_00546840(tree, output, key);
}
void NativeInputKeyboardStorage::next_sensitivity_00552770(NativeKeyboardTreeIterator* iterator) {
    advance_native_input_sensitivity_00552770(iterator);
}
void* NativeInputKeyboardStorage::bindings_006a45c0(void* tree, const void* key) {
    return index_native_input_descriptor_vector_006a45c0(tree, static_cast<const NativeString*>(key), strings_);
}
void* NativeInputKeyboardStorage::reverse_006a4ca0(void* tree, const void* key) {
    return index_native_input_bit_vector_006a4ca0(tree, static_cast<const NativeString*>(key), strings_);
}
NativeKeyboardBitIterator* NativeInputKeyboardStorage::advance_bit_0048d3b0(
    NativeKeyboardBitIterator* iterator, std::int32_t distance) {
    return advance_native_input_bit_0048d3b0(iterator, distance);
}
void NativeInputKeyboardStorage::next_input_00552d40(NativeKeyboardTreeIterator* iterator) {
    advance_native_input_codes_00552d40(iterator);
}
NativeKeyboardTreeIterator* NativeInputKeyboardStorage::erase_scales_0055b230(void* tree,
    NativeKeyboardTreeIterator* output, NativeKeyboardTreeIterator first, NativeKeyboardTreeIterator last) {
    return static_cast<NativeKeyboardTreeIterator*>(erase_native_input_scale_tree_range_0055b230(tree, output, first, last, strings_));
}
void NativeInputKeyboardStorage::next_device_005540c0(NativeKeyboardTreeIterator* iterator) {
    advance_native_input_device_005540c0(iterator);
}
} // namespace bsp
