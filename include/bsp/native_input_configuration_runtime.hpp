#pragma once
#include "bsp/native_input_configuration_parse.hpp"
#include "bsp/native_input_settings_lifetime.hpp"

namespace bsp {
struct NativeInputConfigurationRuntimeServices {
    NativeInputConfigurationParseServices& parsing;
    NativeInputActionTickContext& tick;
    NativeInputSettingsLifetimeContext& settings;
    NativeInputSettingsDefaultsServices& defaults;
};

// Complete normal00698A10..00699B7D, native ECX embedded524h configuration,
// no stack inputs, RET. Compose the existing actual Lua/modifier/action parser,
// resolve the action owner again and rebind, then resolve it once more for the
// x87 positive-zero update. Read current F88A30 AFTER that update; zero invokes
// the full settings getter and default preservation. Set configuration520 only
// after those calls return, then release value/key/Inputs/globals in that order.
void load_native_input_configuration_00698a10(
    void* actual_configuration, NativeInputConfigurationRuntimeServices&);

// Services must share the application's actual owner/backend/manager cells,
// raw allocations and compatible record services. Timing/device/callback and
// settings-container services remain required; no successful defaults exist.
// Reached device/callback operations execute through the existing source tick.
// Original private stack addresses/FH3, hardware faults, malformed storage,
// asynchronous mutation and gameplay are not covered by this new source ABI.
} // namespace bsp
