#pragma once

#include "bsp/native_game_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Game current virtual calls require MSVC Win32.
#endif

namespace bsp {

// New Source glue, with no corresponding Original function or binary-ABI
// replacement. The service owns no receiver, table, allocation or context.
// Every receiver is a live actual object with a readable current primary table
// containing genuinely callable entries of the required Win32 call shape.
// Tables and code remain live across their calls. Numeric Original-PE profile
// identities or copied Original entry values are not rebuilt callable tables.
// The selected method's owner and destruction/free domain must already be
// qualified for the actual construction, and the record pointer (when present)
// must satisfy that real method's access and lifetime contract.
//
// This service must outlive every context and retained operation borrowing it.
// It supplies no table production, child lifetime, allocator repair or full Game
// composition. Calls are not noexcept: target exceptions propagate unchanged.
// Pointer byte copies supply no synchronization or native invalid-storage fault
// equivalence; concurrent table mutation is outside this contract.
struct NativeGameCurrentVirtualCalls final : NativeGameLifetimeCalls {
    // Current table at exact byte_slot; forward the complete flags word through
    // the existing concrete physics dispatcher. The return value is ignored.
    void virtual_scalar(void* captured, std::uint32_t byte_slot,
                        std::uint32_t flags) override;

    // Caller has already decremented the reference count. Invoke current slot0
    // with ECX=captured and no stacked argument; no extra decrement/count check.
    void virtual_terminal(void* captured) override;

    // ECX=the captured current peer+4 receiver; current table byte slot4 gets
    // the unchanged captured record pointer as one callee-popped stack word.
    // No peer/list/lock action or record ownership is introduced here.
    void virtual_04(void* receiver, void* record) override;
};

} // namespace bsp
