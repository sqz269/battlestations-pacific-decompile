#pragma once
#include "bsp/native_material_pass_base.hpp"

namespace bsp {
// Persistent host metadata outside the actual pass/array/shader storage. A
// borrowed reserve or canonical zero-owner lookup can throw after native writes.
// Keep this frame, pass and canonical domain alive while running/failed.
// Exclude independent retirement/replacement of referenced arrays/owners;
// the operation's own scheduled terminal release may destroy the old owner.
// After that terminal returns, old_identity is identity-only diagnostic data.
// No rollback or replay; metadata does not acquire additional native references.
struct NativeMaterialPassShaderSlotsOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{}, slot{}, index{}, count{}, target_count{}, removed_rows{};
    NativeMaterialPassBaseStorage* pass{};
    NativeMaterialStateOwnerStorage* sampled_sampler{};
    NativeMaterialStateArray* resize_rows{};
    NativeRenderActualOwners* owners{};
    void* old_identity{};
    void* incoming_identity{};
    bool slot_published{}, incoming_incremented{}, old_release_entered{}, old_release_returned{};
    NativeMaterialPassShaderSlotsOperation() = default;
    ~NativeMaterialPassShaderSlotsOperation();
    NativeMaterialPassShaderSlotsOperation(const NativeMaterialPassShaderSlotsOperation&) = delete;
    NativeMaterialPassShaderSlotsOperation& operator=(const NativeMaterialPassShaderSlotsOperation&) = delete;
    // Only after the caller resolves the failed native state/acquisitions.
    // Changes metadata only; does not free, restore, release again or resume.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// B5EFF0 ECX actual pass, stacked DWORD slot, RET4. Repeated first-match scan
// of actual pass20 owner's 12-byte rows; copy last row over match, then reload
// pass20/count, shrink and restart. Removes every matching slot; unstable order.
// Unrelated pass/state bytes, capacity and discarded tail rows remain unchanged
// on ordinary valid extents. Existing B40BE0 extent/failure boundary is reused.
void remove_native_material_pass_sampler_slot_00b5eff0(NativeMaterialPassBaseStorage&,
    std::uint32_t slot, NativeMaterialPassShaderSlotsOperation&);

// B5F080/B5F0C0 ECX actual pass, stacked actual shader-wrapper identity, RET4;
// no semantic EAX result. Equal identities do nothing. Otherwise publish new,
// atomically increment new+04, atomically decrement captured old+04, and invoke
// old's CURRENT virtual0 only when the decrement result is exactly zero.
// Uses the unchanged canonical owner interface; never wraps/replaces the owner.
// Concrete D62A60/D62A70 shader constructor/destructor providers are still a
// separate dependency. The supplied canonical provider must implement actual
// BD30E0 -> current virtual4(flags1), including destruction/free/registration.
// No fake shader or no-op terminal provider is supplied by this packet.
void set_native_material_pass_pixel_shader_00b5f080(NativeMaterialPassBaseStorage&,
    void* actual_wrapper, NativeRenderActualOwners&, NativeMaterialPassShaderSlotsOperation&);
void set_native_material_pass_vertex_shader_00b5f0c0(NativeMaterialPassBaseStorage&,
    void* actual_wrapper, NativeRenderActualOwners&, NativeMaterialPassShaderSlotsOperation&);
} // namespace bsp
