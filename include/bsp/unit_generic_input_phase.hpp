#pragma once

#include "bsp/gameplay_settings.hpp"
#include "bsp/native_input_action_owner.hpp"
#include <cstdint>

namespace bsp {

// Borrow live fields of the same unit. This view neither constructs nor owns
// an object. 0095CD9E initializes +63C to 1; 0095CF02 clears +644; 0095CF38
// initializes +708 to -1. The common constructor does NOT initialize +640.
struct UnitGenericInputFields {
    volatile float& value_63c;
    volatile float& timestamp_640;
    volatile std::uint8_t& latch_644;
    const volatile float& field_708;
};

struct UnitGenericInputCalls {
    virtual ~UnitGenericInputCalls() = default;
    // 0095DC6F: actual 24h input owner, whose +4 points at live 30h records.
    // Record 153 must exist; no bounds check occurs in the native consumer.
    virtual const NativeInputActionOwnerStorage& input_owner_004bec00() = 0;
    // Five native sites, including two consecutive calls whose returned
    // objects are both retained. Preserve calls and any publication effects.
    // +38/+3C/+40 must contain native STORED reciprocal rates. A provider
    // supplying raw Lua durations does not satisfy this contract.
    virtual const GameplayTuningSettings& settings_00424c40() = 0;
};

struct UnitGenericInputContext {
    const volatile std::uint32_t& local_player_slot_18ec;
    const volatile std::int32_t& current_role4_1bc;
    const volatile float& mission_clock_00f876a4;
    // Actual global slot: the body reloads this after the input getter, then
    // follows pointer +CCh and reads that object's dword +4Ch. No view owner
    // or substitute target state is created here.
    void* const volatile& global_00e198c4;
    UnitGenericInputCalls& calls;
};

// Complete 0095DC40..0095DDF8 control/arithmetic sequence, native ECX=unit,
// stack float step, RET4. The 00927F30(unit,4) predicate is inlined over the
// actual borrowed slot/role cells, preserving the unsigned 0..7 guard.
// x87 operations, spills and unordered branches are retained on MSVC Win32.
// New C++ ABI; no complete unit/global/singleton owner, SEH or game binding.
void unit_generic_input_phase_0095dc40(UnitGenericInputFields, float step,
    UnitGenericInputContext&);

// Complete false-predicate arm 0095DD71..DD90 -> DDF1..DDF8. Current role4==8
// proves this arm for every possible local-slot dword, so a caller with that
// established role can use it without manufacturing unavailable globals.
// Only +63C changes. Timestamp/latch/+708 and all services remain untouched.
void unit_generic_input_unassigned_0095dd71(volatile float& value_63c) noexcept;

} // namespace bsp
