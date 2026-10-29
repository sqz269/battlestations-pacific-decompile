#pragma once

#include <cstdint>

namespace bsp {
struct NativeGameStorage;
struct NativeGameLifetimeCalls;

// Required WHOLE raw 004D95F0 binding. This is not GamePauseState projection
// storage or a reset-interface-stack approximation. The Game reference is the
// original hidden incoming ECX, and value is the original stacked DWORD zero.
struct NativeFrontEndManagerDestructionCalls {
    virtual ~NativeFrontEndManagerDestructionCalls() = default;
    virtual void call_004d95f0(NativeGameStorage& actual_game_ecx,
        std::uint32_t value) = 0;
};

// Borrow these SAME actual publications and the Game lifetime's SAME real
// scalar-dispatch service. No cell, table, allocator or successful default is
// created here. Keep all cells, providers and valid actual Game backing alive
// through the whole call; deletion tables/heaps must match captured receivers.
struct NativeFrontEndManagerDestructionContext {
    void* volatile& actual_main_menu_00e198ac;
    void* volatile& actual_multi_menu_00e198b4;
    void* volatile& actual_options_00e198b8;
    void* volatile& actual_in_mission_00e198c4;
    NativeGameLifetimeCalls& lifetime_calls;
    NativeFrontEndManagerDestructionCalls& required_calls;
};

// Complete normal 004DA650..004DA6BA: 107 bytes/38 instructions. Original
// incoming ECX=actual 71A0h Game, no stack arguments, plain RET; the no-input
// annotation misses ECX forwarded unchanged to the first 004D95F0 call.
// New void C++ interface only, not an Original register/stack or FH/SEH ABI.
// First invoke whole pause helper(Game,0), then process CURRENT AC/B4/B8/C4
// cells in order: capture once, if nonnull real current-table scalar byte0,
// flags1, then clear CURRENT cell after normal return, even if replaced.
// Serialized ownership is required; callback publication changes are allowed
// at these epochs, not arbitrary concurrent/reentrant parent invocation.
// C++ failure naturally preserves reached effects and skips the post-call
// clear/later rows. No catch, rollback, recovery or replay policy is added.
void destroy_native_front_end_managers_004da650(NativeGameStorage&,
    NativeFrontEndManagerDestructionContext&);
} // namespace bsp
