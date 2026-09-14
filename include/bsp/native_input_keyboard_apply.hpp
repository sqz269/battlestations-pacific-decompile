#pragma once
#include "bsp/native_input_settings_defaults.hpp"
#include <cstdint>

namespace bsp {
struct NativeKeyboardTreeIterator { void* owner; void* node; };
struct NativeKeyboardBitIterator { void* owner; void* word; std::uint32_t bit; };
static_assert(sizeof(NativeKeyboardTreeIterator) == 8);
static_assert(sizeof(NativeKeyboardBitIterator) == 12);

// Required native library-storage operations, not callbacks that return parsed
// substitutes. All receivers, returned mapped addresses, iterators and pooled
// string keys belong to the actual settings tree/storage. A concrete source
// provider is declared in native_input_keyboard_storage.hpp.
struct NativeInputKeyboardLibrary {
    virtual ~NativeInputKeyboardLibrary() = default;
    virtual void* device_0055c110(void* tree, const void* pooled_name) = 0;
    virtual void* input_codes_006a44b0(void* tree, const void* pooled_name) = 0;
    virtual void* copy_scales_0055b400(void* fresh_tree, const void* source_tree) = 0;
    virtual void destroy_scales_0055b490(void* tree) = 0; // native state0 unwind
    virtual float* multiplier_00444be0(void* tree, const void* pooled_name) = 0;
    virtual void* code_classes_006a5aa0(void* tree, const std::int32_t* code) = 0;
    virtual NativeKeyboardTreeIterator* find_hack_00546840(void* tree,
        NativeKeyboardTreeIterator* output, const std::int32_t* code) = 0;
    virtual void next_sensitivity_00552770(NativeKeyboardTreeIterator*) = 0;
    virtual void* bindings_006a45c0(void* tree, const void* pooled_name) = 0;
    virtual void* reverse_006a4ca0(void* tree, const void* pooled_name) = 0;
    virtual NativeKeyboardBitIterator* advance_bit_0048d3b0(
        NativeKeyboardBitIterator*, std::int32_t distance) = 0;
    virtual void next_input_00552d40(NativeKeyboardTreeIterator*) = 0;
    virtual NativeKeyboardTreeIterator* erase_scales_0055b230(void* tree,
        NativeKeyboardTreeIterator* output, NativeKeyboardTreeIterator first,
        NativeKeyboardTreeIterator last) = 0;
    virtual void next_device_005540c0(NativeKeyboardTreeIterator*) = 0;
};
struct NativeInputKeyboardConstants {
    const volatile float& zero_00d7a218;
    const volatile float& one_00d7a24c;
    const volatile float& negative_one_00d7a260;
    const volatile float& negative_zero_00d7a208;
    const volatile double& reverse_sign_00d7a250;
};
struct NativeInputKeyboardApplyContext {
    NativeInputActionOwnerContext& action_owner;
    const NativeInputBindingStorageContext& binding_storage;
    void* volatile& backend_00f8bbf4;
    NativeInputKeyboardLibrary& library;
    NativeInputKeyboardConstants constants;
    std::uint32_t initial_sensitivity_class_preimage;
    std::uint32_t suppressed_flag_stack_preimage;
    std::uint32_t temporary_map_opaque_preimage;
};

// Source adapter for the existing0069DD80 checked range contract: first begin,
// first end, second owner, second begin are the consumed native stack words.
// The native two trailing stack words are unused. No original STL ABI claimed.
std::uint8_t equal_native_input_code_ranges(const void* first_begin,
    const void* first_end, const void* second_owner, const void* second_begin);
// 69E860: ECX/EDX actual16h checked DWORD-vector headers; EAX0/1, RET.
std::uint32_t equal_native_input_code_vectors_0069e860(const void*, const void*);
// 6AA090: ECX settings; stack two pooled8h string pointers; AL, RET8.
std::uint8_t use_native_input_alternate_axis_slots_006aa090(void* actual_settings,
    const void* device_name, const void* input_name, NativeInputKeyboardLibrary&);
// 699BF0: ECX actual14h descriptor; EAX0/1, RET. Unsigned code thresholds.
std::uint32_t suppress_native_input_alternate_binding_00699bf0(const void*) noexcept;

// 6AA640..6AAC43: complete normal engine schedule, ECX settings, no native
// stack inputs, RET. Actual records and real input-owner/storage/rebind calls.
// Owns one temporary sensitivity tree per device through required library
// operations. No hardware poll. Raw library providers and settings production
// must exist before this is a complete runtime path; no successful fallbacks.
void apply_native_input_keyboard_bindings_006aa640(void* actual_settings,
    NativeInputKeyboardApplyContext&);
class NativeInputKeyboardApplication final : public NativeInputSettingsKeyboardApplication {
public:
    explicit NativeInputKeyboardApplication(NativeInputKeyboardApplyContext& context) noexcept
        : context_(context) {}
    void apply_keyboard_bindings_006aa640(void* actual_settings) override;
private:
    NativeInputKeyboardApplyContext& context_;
};

// Original ABI/FH3/private stack aliases and gameplay are not supplied. Valid
// raw allocations, current native headers and provider lifetimes are required.
// Unknown sensitivity categories retain prior class/private stack preimage.
} // namespace bsp
