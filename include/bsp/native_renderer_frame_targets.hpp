#pragma once

#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

struct NativeRendererFrameTargetsContext {
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    NativeFrameTargetOwnerContext& actual_group_owner;
    // Borrow the original two-DWORD D5E600 profile: BD30E0, B1FCF0.
    // Tokens resolve only to established source operations, never callbacks.
    const volatile std::uint32_t* actual_group_profile_00d5e600;
    const volatile std::uint8_t& actual_srgb_enabled_00f8d398;
};

// Full B24E70..B24FAE: native ECX renderer, stack group, RET4. The new
// interface adds explicit actual application context. Actual guard entry
// precedes the renderer+1908 identity read, which precedes cleanup arming.
// On change reread old, publish/retain incoming, then release captured old
// through its current concrete profile. Null incoming changes no GPU state.
// Nonnull incoming optionally sets render state C2 from its exact +3C byte,
// unbinds colors1..3, binds colors0..3, then depth, through full providers.
// Incoming remains the original argument across callbacks; each child getter
// reloads its current field. No rollback, object substitution or early clear.
void bind_native_renderer_frame_targets_00b24e70(void* actual_renderer,
    NativeFrameTargetOwnerStorage* incoming, NativeRendererFrameTargetsContext&);

// The current concrete D5E600 owner and its established surface/vector/CRT
// lifetime domain must remain valid. Foreign profiles are outside this
// contract. A skipped guard remains uninitialized; enabling its cleanup is
// outside the native valid domain. No original caller ABI/gameplay claim.

} // namespace bsp
