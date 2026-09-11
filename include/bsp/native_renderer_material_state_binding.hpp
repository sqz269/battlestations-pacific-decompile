#pragma once

#include "bsp/native_material_pass_states.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Fixed borrowed context; no owner, reference companion, callback or table of
// host callables. Only a reached identity needs its two immutable original
// DWORDs: D61A2C={BD30E0,B422F0}, D61A34={BD30E0,B42310}. A final-zero release
// reads current slot0, then the full BD30E0 schedule rereads the owner's CURRENT
// profile and slot4 before flags1 deletion. Unsupported profiles are outside
// the source domain; do not use a noexcept NativeMaterialStateReference bridge.
struct NativeRendererMaterialStateBindingContext {
    NativeRendererSynchronizationGlobals* const actual_synchronization_0108d6dc;
    const volatile std::uint32_t* const actual_render_profile_00d61a2c;
    const volatile std::uint32_t* const actual_sampler_profile_00d61a34;
};

// Full B27A80[122], B27B90[130]. Original ECX renderer, stack incoming actual
// owner, RET4; no semantic result. New fixed context in EDX, saved in one added
// stack cell. An identical incoming owner returns before any context read.
// At reached accesses, actual 14h NativeMaterialStateOwnerStorage lifetime is
// established; count+04 and raw row header+08 belong to that same allocation.
// Only reached final-zero terminal profiles require the views above; incoming
// profile words are not read or validated by these binders. Owning heap and
// valid extent/exception boundaries are those of native_material_pass_states.
// Render inputs use actual 8-byte rows; sampler inputs use 12-byte rows.
void __fastcall bind_native_renderer_material_render_states_00b27a80(
    void* actual_renderer, const NativeRendererMaterialStateBindingContext*,
    NativeMaterialStateOwnerStorage* actual_incoming);
void __fastcall bind_native_renderer_material_sampler_states_00b27b90(
    void* actual_renderer, const NativeRendererMaterialStateBindingContext*,
    NativeMaterialStateOwnerStorage* actual_incoming);

// Outer identity skip; recapture old, publish incoming before real atomic
// retain/release; call full raw deleting owner providers on old final zero.
// Retain original incoming identity through callbacks. Read current row base
// and signed count at each original access; row loads are value/state and
// value/state/slot, respectively. Invoke full actual B24460/B24610; increment
// current 1B94/1B9C only after returning, including a changed null binding.
// No outer guard/EH, rollback, extra null-success return or cached row snapshot.
// All reached raw renderer/owner/row/profile storage stays valid across calls;
// fixed context is unchanged/alive. Null incoming skips row/context access
// unless releasing a nonnull old owner. Real device tables remain actual COM.
// New Win32 C++ context interface, not original stack/SEH/caller ABI or gameplay.
// B27B00, general B241C0 cache clear and B262C0 resource release stay separate.

} // namespace bsp
