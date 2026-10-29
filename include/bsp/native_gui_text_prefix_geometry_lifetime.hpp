#pragma once
#include "bsp/native_gui_text_primary_prefix_lifetime.hpp"
#include "bsp/native_gui_text_main_geometry_storage.hpp"

namespace bsp {
// Source caller progress, independent of379's own preflight/entered record.
// Arm entered BEFORE invoking379; returned only after normal return, failed on
// escape. No destructor, copied receipt or native cleanup/ownership follows.
struct NativeGuiTextPrefixGeometryProgress final {
    NativeGuiTextPrefixGeometryProgress() = default;
    NativeGuiTextPrefixGeometryProgress(const NativeGuiTextPrefixGeometryProgress&) = delete;
    NativeGuiTextPrefixGeometryProgress& operator=(const NativeGuiTextPrefixGeometryProgress&) = delete;
    bool entered{}, returned{}, failed{};
};

// Install this exact nonmoving graph before Text allocation/raw122. The prefix
// pointer may be a forward ADDRESS during whole-graph construction; inspect no
// incomplete object. Actual validation follows whole-Attempt construction.
// Operand/scratch refs are the actual379 Frame constructor operands, not copies.
struct NativeGuiTextPrefixGeometryLifetimeContext final {
    const NativeGuiTextPrimaryPrefixLifetimeContext* const prefix;
    NativeGuiTextMainGeometryStorageContext& construction;
    const NativeGuiTextMainGeometryStorageFrame& frame;
    const NativeGuiTextMainGeometryStorageAcquired& acquired;
    const NativeGuiTextPrefixGeometryProgress& progress;
    volatile std::uint32_t (&arguments)[4];
    volatile std::uint32_t (&position)[3];
    NativeGuiWidgetTransformScratch& transform_scratch;
    GuiWidgetBounds& bounds_scratch;
};

// Metadata/affinity only. Installed untouched FRESH is legal before raw122 and
// after db55/name-return but BEFORE379. Entered/pending/failed work cannot gain
// an earlier-domain fallback. Completion here is not scalar/liveness authority.
void require_native_gui_text_prefix_geometry_reservation(
    const NativeGuiTextPrimaryPrefixLifetimeContext&);

// Prefix adapter only, after its exact stage/predicate/live-primary checks.
// Requires chosen geometry COMPLETE and SAME live canonical Model+180 Mesh.
// The real late Text+74-zero obligation is external; no outer guard proves it.
void require_native_gui_text_prefix_geometry_primary(void* current_geometry,
    const NativeGuiTextPrimaryPrefixLifetimeContext&);

// Durable Source records only, including after MeshEntry/companion erasure.
// Requires BOTH primary379 and raw85 Shadow mesh zero-terminal/clear plus each
// Model's complete scalar/flags1/pool return. Never dereference Mesh companions.
// This alone is not whole Text/provider/drain settlement or fallback authority.
bool native_gui_text_prefix_geometry_meshes_settled(
    const NativeGuiTextPrimaryPrefixLifetimeContext&) noexcept;
} // namespace bsp
