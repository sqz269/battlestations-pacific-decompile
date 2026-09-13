#pragma once
#include "bsp/native_input_configuration_cleanup.hpp"
#include "bsp/native_lua_bootstrap.hpp"
#include "bsp/native_lua_file_loading.hpp"

namespace bsp {

// Complete6974F0 over the actual embedded configuration and actual14h output.
// Native ECX configuration; stack output, Lua object, low-byte swap flag;
// EAX original output, RET0Ch. Read Lua indices1/2/3 through the real tracked
// object API. A nonnil third value is fetched again. Only after destruction of
// all Lua temporaries, read F88A30 && (configuration4C9 || configuration4CC)
// && swap; the first matching actual checked-vector row exchanges its two codes.
// Writes output+C,+4,+0,+8 and byte+10 in that order; padding11..13 is retained.
// Source-only extra arguments borrow the live publication and CRT conversion
// mode. No new Lua owner, registry object, table copy or device lookup is used.
void* decode_native_input_descriptor_006974f0(void* actual_configuration,
    void* actual_output, NativeLuaObjectStorage& table, std::uint8_t swap,
    const volatile std::uint8_t& x360comp_00f88a30,
    const bool& crt_sse2_conversion);

struct NativeInputConfigurationLoadServices {
    NativeInputConfigurationCleanupContext& cleanup;
    NativeStringStorage& strings;
    const NativeLuaBootstrapInputs& bootstrap;
    const NativeLuaFileServices& files;
    const volatile std::uint8_t& x360comp_00f88a30;
};

// Exact normal-flow prefix698A10..698B5E, stopping before globals acquisition
// at698B5F. Always clear the actual configuration/actions; open its same-base
// Lua owner with mask1 only when byte4C8 is zero. Always execute X360COMP and
// Scripts\datatables\Inputs.lua, destroy each string before the next stage,
// then set byte4C8. Borrow the application's real Lua/VFS/string services.
// This fragment does not implement the remaining InputModifiers/Inputs parser
// and MUST NOT replace a call to the full698A10 routine.
void load_native_input_configuration_script_prefix_00698a10(
    void* actual_configuration, NativeInputConfigurationLoadServices&);

// New source ABI over valid native storage and bounded string allocations.
// Returning CRT checks and synchronous Lua/allocation callbacks are retained.
// Native FH3/SEH, hardware faults, arbitrary stack aliases, malformed ranges,
// complete application integration and gameplay are not claimed.
} // namespace bsp
