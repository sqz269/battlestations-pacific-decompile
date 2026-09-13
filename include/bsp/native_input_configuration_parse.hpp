#pragma once
#include "bsp/native_input_configuration_modifiers.hpp"
#include "bsp/native_input_action_configuration.hpp"

namespace bsp {
// Actual34h binding construction/destruction and the two native append calls.
// Source is an actual14h descriptor; both modifier arguments are actual0Ch
// {base,count,capacity} headers. These retain raw allocations, padding and the
// native copy/cleanup order. No projected InputActionBinding is substituted.
void* construct_native_input_binding_00a93350(void* fresh_binding,
    const void* descriptor, const void* required, const void* forbidden, float scale);
void destroy_native_input_binding_00a93070(void* actual_binding);
void append_native_input_action_descriptor_00a93620(void* actual_action,
    const void* descriptor, const void* required, const void* forbidden, float scale);
void append_native_input_owner_descriptor_00a93830(void* actual_owner,
    std::uint32_t action, const void* descriptor,
    const void* required, const void* forbidden, float scale);

struct NativeInputConfigurationParseServices {
    // Cleanup and activation must use compatible services for the same actual
    // owner/record allocations throughout parsing and its synchronous callbacks.
    NativeInputConfigurationLoadServices& loading;
    NativeInputActionConfigurationContext& actions;
    const bool& crt_sse2_conversion;
    const volatile std::uint8_t& swap_general_00f889c1;
    const volatile std::uint8_t& swap_map_00f889c4;
    const volatile double& negative_one_00d7a250;
};

// Actual Lua objects must retain their addresses throughout parsing and the
// following native tail. This aggregate has no implicit constructor/destructor.
struct NativeInputConfigurationParsedObjects {
    NativeLuaObjectStorage globals, inputs, key, value;
};

// Extends the established script/modifier prefix through699AD7. Gets the
// actual action owner, parses groups/helper/inputs/press/and/not/mul, configures
// and appends actual records, then configures action128h in context1Eh. Inputs
// array entries are probed and then fetched again, stopping at the first nil.
// On success the caller owns all four supplied objects; on failure they unwind
// here. Action activation uses the required real binding/timing services.
// Stops BEFORE699AD8 rebind/update/settings. Does not set configuration520 and
// MUST NOT substitute for the complete698A10 entry.
void load_native_input_configuration_actions_prefix_00698a10(
    void* actual_configuration, NativeInputConfigurationParsedObjects& fresh,
    NativeInputConfigurationParseServices&);
void release_native_input_configuration_parsed_objects(
    NativeInputConfigurationParsedObjects&);

// Original registers/stack ABI and FH3/SEH are documented, not replaced by
// these source APIs. Require valid ranges and nonnegative finite array sizes.
// Uninitialized descriptor/binding padding retains source scratch preimages;
// equal arbitrary native stack preimages, hardware and gameplay are not claimed.
} // namespace bsp
