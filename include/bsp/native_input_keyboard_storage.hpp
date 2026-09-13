#pragma once
#include "bsp/native_input_keyboard_apply.hpp"
#include "bsp/native_string.hpp"

namespace bsp {
// Concrete source implementation of all fourteen native-header keyboard
// storage operations. The existing interface remains a consumer/fixture seam;
// this provider invokes reconstructed source, with no original code callbacks.
// Borrowed strings service must outlive this object and its consuming contexts.
class NativeInputKeyboardStorage final : public NativeInputKeyboardLibrary {
public:
    explicit NativeInputKeyboardStorage(NativeStringStorage& strings) noexcept : strings_(strings) {}
    void* device_0055c110(void*, const void*) override;
    void* input_codes_006a44b0(void*, const void*) override;
    void* copy_scales_0055b400(void*, const void*) override;
    void destroy_scales_0055b490(void*) override;
    float* multiplier_00444be0(void*, const void*) override;
    void* code_classes_006a5aa0(void*, const std::int32_t*) override;
    NativeKeyboardTreeIterator* find_hack_00546840(void*, NativeKeyboardTreeIterator*, const std::int32_t*) override;
    void next_sensitivity_00552770(NativeKeyboardTreeIterator*) override;
    void* bindings_006a45c0(void*, const void*) override;
    void* reverse_006a4ca0(void*, const void*) override;
    NativeKeyboardBitIterator* advance_bit_0048d3b0(NativeKeyboardBitIterator*, std::int32_t) override;
    void next_input_00552d40(NativeKeyboardTreeIterator*) override;
    NativeKeyboardTreeIterator* erase_scales_0055b230(void*, NativeKeyboardTreeIterator*,
        NativeKeyboardTreeIterator, NativeKeyboardTreeIterator) override;
    void next_device_005540c0(NativeKeyboardTreeIterator*) override;
private:
    NativeStringStorage& strings_;
};
} // namespace bsp
