#pragma once
#include "bsp/native_input_settings_startup.hpp"
#include "bsp/native_input_keyboard_apply.hpp"
#include <array>

namespace bsp {
// Concrete table storage operates on the actual native headers and allocations.
// The separate keyboard-library operations share the same device/tree storage.
struct NativeInputSettingsTableServices {
    NativeInputSettingsScriptServices& scripts;
    NativeInputKeyboardLibrary& trees;
    const bool& crt_sse2_conversion;
    const volatile float& one_00d7a24c;
    const volatile float& base_zero_replacement_00cf7fe8;
    std::uint32_t descriptor_flag_stack_preimage;
    std::uint32_t vector_opaque_stack_preimage;
    std::uint32_t sensitivity_default_stack_preimage;
};

// Complete normal006A7BE0 schedule: existing raw script prefix, native settings
// containers, real tracked Lua objects, KeyboardSetup/InputNames/Conflicts,
// DEVINPUTS and same-temporary-state ControllerInputNames, then reverse cleanup.
// Original ECX settings, no stack arguments, RET. This is a new explicit-service
// C++ ABI. Required library providers are not supplied or replaced by defaults.
void load_native_input_settings_tables_006a7be0(void* actual_settings,
    NativeInputSettingsTableServices&);

// Actual settings must already contain constructed native headers and owner78.
// The byte4 guard survives failures; byte5 is not changed. Constructor/singleton/
// destructor integration is supplied by native_input_settings_lifetime.hpp.
// Malformed native storage, original FH3/private-stack aliases, hardware-fault
// behavior and gameplay remain outside this API.
} // namespace bsp
