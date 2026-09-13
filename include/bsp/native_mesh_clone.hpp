#pragma once
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_material_owner.hpp"
#include "bsp/native_stream_clone.hpp"

namespace bsp {
class GuiNativeGeometryOwners;
struct NativeMeshSectionStorage;

// Caller-owned creator references, initially all null. Publishing a section
// into mesh or a material into section retains independently; consuming the
// creator clears its cell before release. If copying throws after construction,
// the exact partial mesh and any unconsumed creators remain here. Caller must
// finish/abandon them through the SAME actual owner domain, not retry the copy.
struct NativeMeshCloneAcquired {
    NativeMeshStorage* mesh{};
    NativeMeshSectionStorage* section{};
    NativeMaterialStorage* material{};
    NativeStreamCloneAcquired stream;
};

// Full B72BD0: original ECX=destination+10, stack source+10, RET4. Copy
// each active 10h entry's four raw DWORDs, then the live source count+40.
// Unused destination entries are preserved. Valid counts are 0..4.
void copy_native_mesh_lod_00b72bd0(
    NativeMeshStorage& destination, const NativeMeshStorage& source) noexcept;

// Complete successful B73F50 branch for flags26h, using distinct actual
// constructed meshes and their SAME canonical geometry domain. Original ABI:
// ECX=source, stack destination/flags, RET8. Stream counts and section/name
// counts/slots are reread through loops. Existing destination sections append;
// names are cleared then independently copied with the actual string pool.
// Optional flags/payloads and x87 FLD/FSTP effects are physical, not projections.
// Every new section/material creator is consumed after native publication.
// Allocation/registration exceptions are host diagnostics; partial destination
// effects and unconsumed creators stand in acquired. Null/corrupt storage,
// aliasing and flags8/10 stream copies are
// outside this interface. New MSVC Win32 C++ API, not a native ABI replacement.
void copy_native_mesh_for_text_00b73f50(NativeMeshStorage& destination,
    NativeMeshStorage& source, GuiNativeGeometryOwners&, NativeStringStorage&,
    NativeMaterialDestructionAccess&,
    const volatile std::uint32_t* current_material_vtable_00d5e520,
    NativeMeshCloneAcquired& acquired);

// Exact flags3E variant: same sections/materials/tail, but actual owned index
// and vertex stream copies. No flags26 sharing fallback. Every creator and
// mapping interruption stays in acquired.stream until explicitly resolved.
void copy_native_mesh_for_text_00b73f50_flags3e(NativeMeshStorage& destination,
    NativeMeshStorage& source, GuiNativeGeometryOwners&, NativeStringStorage&,
    NativeMaterialDestructionAccess&,
    const volatile std::uint32_t* current_material_vtable_00d5e520,
    NativeMeshCloneAcquired&, NativeStreamCloneServices&);
} // namespace bsp
