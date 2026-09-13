#pragma once
#include "bsp/native_input_action_owner.hpp"
#include "bsp/native_input_binding_storage.hpp"
#include <cstdint>

namespace bsp {
// Required engine dependency. Dispatch the SAME actual540h settings receiver
// to006AA640. The existing projected InputSettings/keyboard_restore interface
// cannot receive this pointer. No successful fallback implementation is supplied.
struct NativeInputSettingsKeyboardApplication {
    virtual ~NativeInputSettingsKeyboardApplication() = default;
    virtual void apply_keyboard_bindings_006aa640(void* actual_settings) = 0;
};
struct NativeInputSettingsDefaultsServices {
    NativeInputActionOwnerContext& action_owner;
    const NativeInputBindingStorageContext& binding_storage;
    void* volatile& backend_00f8bbf4;
    NativeInputSettingsKeyboardApplication& keyboard;
    // Unspecified high24 bits from native private stack storage; only the low
    // byte is initialized. These are explicit source preimages, not game globals.
    std::uint32_t cleared_descriptor_flag_preimage;
    std::uint32_t reader_missing_flag_preimage;
};

// Shared source library contract for settings+54's signed-int/DWORD tree.
// Reuses the existing18h-node scalar subscript adapter without float conversion.
// Requires a consistent initialized native tree; returns its actual mapped word.
std::int32_t* subscript_native_input_settings_binding_counts(
    void* actual_tree, const std::int32_t* key);

// 006AB820..006ABA42 inclusive, native ECX settings, no stack inputs, RET or
// tail JMP006AA640. Gate on byte+50. Capture actual input owner once, ensure
// map keys4A/4B/46/47/4C/4D/1, then traverse ALL current map nodes in order.
// Record original binding counts and process slots ascending: copy descriptor
// and scale to slot+4, clear the old descriptor/scale. Modifier ownership stays
// with each slot; every installation rebinds its whole action. Finally invoke
// the required keyboard provider with the original settings receiver.
void preserve_native_input_default_bindings_006ab820(
    void* actual_settings, NativeInputSettingsDefaultsServices&);

// Source service ABI, not original ABI/FH3 or complete input startup. Native
// settings construction/loading and concrete raw006AA640 remain dependencies.
// Valid native ranges, existing CRT allocation and actual backend lists required.
} // namespace bsp
