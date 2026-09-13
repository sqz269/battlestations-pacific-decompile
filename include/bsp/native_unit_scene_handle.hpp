#pragma once

#include "bsp/native_unit_observer_endpoint.hpp"

namespace bsp {

// Borrow the existing stable unit alias and that SAME unit's actual +4A4h
// pointer cell. No new scene owner, bool-to-pointer conversion, or semantic
// GameUnitSlot cast. The caller must establish this binding and its lifetime;
// current GameUnitsHost has only has_scene_node=false and cannot supply it.
// Both the unit alias and cell must remain live through the delegated handler.
struct NativeUnitSceneHandleView {
    NativeUnitObserverAlias unit;
    void*& scene_handle_4a4;
};

// Complete 006D1E80[7]: ECX=actual unit, EAX=[ECX+4A4h], bare RET. Primary
// slot +18h of the 21 existing leaf profiles. Returns the current borrowed pointer
// exactly, including null; no refcount, ownership transfer, fallback or caching.
void* native_unit_scene_handle_006d1e80(const NativeUnitSceneHandleView&) noexcept;

// Partial producer fragment 00955448[6], MOV[ESI+4A4h],EAX inside 00955420.
// Caller supplies the actual EAX value loaded from the pointer at unit+360h,
// then that owner's +160h pointer, then its +0Ch cell. This fragment only stores
// the supplied value; it does not run 0087BCC0, resolve resources, refresh pose,
// set the scene matrix, propagate registration or run the remaining initializer.
// No null fallback is inferred for the original producer's load chain.
void publish_native_unit_scene_handle_00955448(NativeUnitSceneHandleView,
    void* actual_eax) noexcept;

struct NativeUnitKilledSceneHandleProvider {
    void* context;
    // Required existing 00779AF0 normal handler on the SAME canonical unit.
    // It queries primary slot +18h, conditionally clears the controlled-node
    // publication, clears world+193Ch, marks positive recon records, then
    // tail-delegates to 00928C80. This packet provides none of that behavior.
    void (*on_killed_00779af0)(void*, void* canonical_unit);
};

// Complete 00951FB0[15]: MOV[ECX+4A4h],0; JMP00779AF0 with unchanged ECX.
// Clear before calling the required provider and perform no work afterwards.
// This is NOT a receiver-adjusting thunk and it does not release the old node.
// Reject a missing provider before changing the actual cell (source-interface
// error only). Native saved table identities never become process pointers.
void clear_unit_scene_handle_on_killed_00951fb0(NativeUnitSceneHandleView,
    const NativeUnitKilledSceneHandleProvider&);

// New source ABI, not a native binary replacement or complete unit constructor.
// No scene runtime binding, lifetime/refcount, native exception/FH3/SEH or
// gameplay equivalence is established by these bounded operations.
} // namespace bsp
