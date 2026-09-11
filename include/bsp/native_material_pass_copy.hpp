#pragma once
#include "bsp/native_material_pass_owner.hpp"
#include "bsp/native_material_effect_owner.hpp"

namespace bsp {
// Original ECX destination12-byte header, stack source header; EAX destination,
// RET4. Clear current count, reserve using CURRENT source count, append rows
// with live source-end reloads. Self-assignment clears the array, not a no-op.
// Three state variants first reserve(0) if capacity is negative. Pair variant
// calls actual pair resize(0). Valid readable extents/allocations are required.
NativeMaterialStateArray* copy_native_material_render_rows_00b41c10(
    NativeMaterialStateArray&, const NativeMaterialStateArray&);
NativeMaterialStateArray* copy_native_material_third_rows_00b41ce0(
    NativeMaterialStateArray&, const NativeMaterialStateArray&);
NativeMaterialStateArray* copy_native_material_sampler_rows_00b41dd0(
    NativeMaterialStateArray&, const NativeMaterialStateArray&);
NativeMaterialStateArray* copy_native_material_pass_pairs_00b41e80(
    NativeMaterialStateArray&, const NativeMaterialStateArray&);

// ECX fresh10h binding; stack word, retained owner, actual string header;
// EAX same binding, RET Ch. Construct/copy actual name, then replace retained04
// with publish/retain/release ordering. Member unwind destroys only the name.
NativeMaterialPassBindingStorage* initialize_native_material_pass_binding_00b44690(
    void*, std::uint32_t word, void* retained_owner, const NativeString&,
    NativeMaterialPassDestructionAccess&);

struct NativeMaterialPassCopyAccess {
    NativeMaterialPassDestructionAccess& lifetime;
    void* binding_context;
    // Metadata only, no ownership change: register each newly published state
    // in the SAME canonical owner domain, after copying its actual counter and
    // before row-copy allocation can throw. Must not throw or alter storage.
    void (*bind_state)(void*, NativeMaterialStateOwnerStorage&) noexcept;
};
// ECX actual destination88h pass, stack actual source88h pass; RET4.
// Copy pairs24; replace state owners18/20/1C and copy their CURRENT +04 counts
// into the independent allocations; copy their rows. Then assign54/58/70/74,
// copy indices78/7C, APPEND new bindings at current6C, and copy borrowed14.
// Root words/count, fallback84, byte80, padding and embedded30/3C/48 remain.
// No top-level rollback. A throwing binding constructor frees its raw16 bytes;
// previously published state owners and completed binding entries remain.
void copy_native_material_pass_00b455c0(NativeMaterialPassStorage& destination,
    const NativeMaterialPassStorage& source, NativeMaterialPassCopyAccess&);

struct NativeMaterialSecondaryPassRegistration {
    void* context;
    // Metadata only in the same canonical owner domain; no extra retain.
    // Called after complete construction/publication, before throwing copy.
    void (*bind_pass)(void*, NativeMaterialPassStorage&) noexcept;
};
// ECX actual178h effect, RET. For present primary slotC8, construct one actual
// pooled pass, reread C8, publish secondary100, copy it, set renderF=0,1B=1,
//13=5,14=6 and root08=1. All other thirteen secondary slots are zeroed;
// absent C8 zeros all fourteen. Prior secondary owners are NOT released.
// Constructor failure returns raw slot, but copy failure preserves the already
// published new pass. Construction/copy/registration must share one lifetime.
void build_native_material_secondary_pass_00b45e00(NativeMaterialEffectStorage&,
    NativeMaterialPassConstructionAccess&, NativeMaterialPassCopyAccess&,
    NativeMaterialSecondaryPassRegistration);
} // namespace bsp
