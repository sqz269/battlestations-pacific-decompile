#pragma once
#include "bsp/native_gui_frame_box_identity.hpp"
#include "bsp/native_gui_frame_box_geometry.hpp"
#include "bsp/native_gui_frame_box_constructed_storage.hpp"
#include "bsp/native_gui_widget_clip_storage.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/gui_text_native_layout.hpp"
#include <cstddef>

namespace bsp {
namespace game { class GameNativeStringProcess; }

// Every service already exists in the SAME actual domain. This aggregate owns
// no renderer/pool/widget/ancestor, count or callback implementation. The163
// companion is admitted after genuine complete FrameBox construction, or only
// through184's parent-private field-ready prepared admission before current80.
struct NativeGuiFrameBoxStateStorageContext {
    GuiNativeGeometryOwners& geometry;
    NativeMaterialDestructionAccess& materials;
    NativeMaterialEffectCacheContext& effects;
    NativeStringRawPoolContext& strings;
    game::GameNativeStringProcess& string_process;
    NativeRenderActualOwnerRegistry& owners;
    NativeGuiFrameBoxReference& box_reference;
    const game::GameNativeReadOnlyData& data;
    GuiTextNativeRendererServices& renderer;
    GuiTextNativeLayoutServices& layout;
    NativeLogicalBufferMappingContext& mapping;
    const NativeGuiFrameBoxGeometryContext& geometry_storage;
    NativeGuiWidgetTransformBindings& transform;
    NativeGuiFrameBoxConstructedTransformDispatch& transform_dispatch;
    NativeGuiWidgetClipBindings& clip;
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
};

// Backing exists independently BEFORE any169 callback. Its166 object lifetime
// begins only after the real material has been selected. No automatic166
// destructor/rollback occurs when outer169 fails. A completed returned operation
// may be explicitly destroyed only when its borrowed obligations are resolved.
struct alignas(NativeGuiWidgetClipOperation) NativeGuiFrameBoxStateClipStorage {
    NativeGuiFrameBoxStateClipStorage() noexcept {}
    std::byte bytes[sizeof(NativeGuiWidgetClipOperation)];
};
struct NativeGuiFrameBoxStateStorageFrame {
    // Caller-prepared, mutually disjoint live preimages, not blanket zeros.
    // Original F=entryESP-40h: locals[0..8] are F10..F30. F18 changes from
    // declaration-name length to section identity; F10 becomes white RGBA.
    volatile std::uint32_t (&locals)[9];
    // Original incoming DWORD at F44. Entry reads ONLY LOWWORD. Later native
    // stores/read use ONLY LOWBYTE as new-section flag; other bytes survive.
    volatile std::uint32_t& argument_slot;
    NativeGuiFrameBoxGeometryFrame& geometry;
    NativeGuiWidgetTransformScratch& transform;
    NativeGuiFrameBoxStateClipStorage& clip_storage;
};
struct NativeGuiFrameBoxStateStorageRelease {
    void* identity{}; // may have ended; diagnostic only after zero dispatch
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    long result{};
    bool entered{}, decremented{}, zero_entered{}, zero_returned{}, complete{};
};
struct NativeGuiFrameBoxStateStorageAcquired {
    // These are Source boundaries, never proof of Native exception origin.
    // Public diagnostics do not retain graph/creators or authorize reset/retry.
    enum class Boundary { none, provider_call, host_metadata };
    bool started{}, complete{}, failed{}, exception_cleanup_started{};
    bool stream_published{}, map_entered{}, map_returned{}, unlock_entered{}, unlocked{};
    bool section_retained{}, material_retained{}, owner_published{}, material_published{};
    bool geometry_entered{}, geometry_returned{}, transform_entered{}, transform_returned{};
    bool section_appended{}, color_entered{}, color_returned{};
    bool clip_preparation_entered{}, clip_constructed{}, clip_entered{}, clip_returned{};
    bool first_record_selected{}, second_record_selected{};
    volatile std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{}, completed_name_returns{};
    Boundary boundary{Boundary::none};
    void* captured_model{};
    NativeMeshStorage* captured_mesh{}; // borrowed, never another creator
    void* captured_stream{};
    void* captured_first_record{};
    void* captured_second_record{};
    void* declaration_renderer{};
    void* vertex_renderer{};
    std::uint32_t declaration_target{}, vertex_target{}, map_target{}, unlock_target{};
    std::uint32_t geometry_target{}, color_target{};
    NativeTextureSurfaceReferenceIncrement existing_pair_increment{}, final_pair_decrement{};
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired vertex;
    GuiNativeSectionAcquired section;
    NativeMaterialFactoryAcquired material;
    NativeMeshSectionStorage* section_creator{}; // one newly created/retained ref
    NativeMaterialStorage* material_creator{}; // one newly created/retained ref
    NativeGuiWidgetClipOperation* clip_operation{}; // independently held lifetime
    NativeGuiFrameBoxStateStorageRelease vertex_release, declaration_release;
    NativeGuiFrameBoxStateStorageRelease material_release, section_release;
};

// Whole AD0D80 [AD0D80,AD1581),2049B/630starts. Native ECX actual FrameBox,
// incoming stack LOWWORD index, RET4. This is a new ordinary Source ABI.
// The supplied completed actual120h slot/163 companion share the SAME actual
// +04 and registry; no count gate/decrement/protective retain or typed widget
// cast exists. The sole progress exception is184's private admission AFTER real
// base/vector/name/scalars are ready and119/111 are prepared. It proves neither
// whole-constructor completion nor125 admission during the active constructor.
// Its creator credit/slot/full graph survive callbacks; failure leaves acquired
// material Box credits/creators independently held through the parent's genuine
// member unwind and metadata-only retirement. No arbitrary unfinished copy.
//
// Before entry retain the WHOLE independent actual Box/ancestor/Model/mesh/
// material/effect/name/stream/map/device/layout/renderer/pool/provider graph,
// creator companions, contexts, imports/constants and distinct recursive
// frames/backing. Native fields/counts/163 metadata cannot discover or retain
// that graph or protect ancestor/direct scalar/reentry/replacement lifetimes.
// External serialization excludes reentry, slot reuse and reset/assignment of
// reached frames/diagnostics. Callbacks may alter only valid actual fields;
// all captured/current reads and public alias feedback remain scheduled.
//
// Known-region checks do not prove transitive backing/provenance/legal object
// placement/lifetimes. Writable metadata/locals/argument/transform/geometry/
// clip backing must be mutually separate and disjoint from the WHOLE provider
// graph and all reached raw/mapped storage. Native name headers may change
// through their genuine services only. Current literal-length copies require
// valid readable literal and writable destination extents at that reached time.
// BF7680 uses the existing qualified current-SDK memmove/zero-count-omission
// Source boundary, admitting overlap without new Native CRT body/ABI credit.
// Actual resource pointers/subobjects are genuine, live and nonnull when Native
// dereferences them; a returning BF6713 still requires its eventual raw record.
//
// Geometry uses SAME162/157 data/profile domains and live E1301C; AA7220 uses
// SAME147/132 dispatch/data aliases, D7A24C/D7A208 and LIVE E12FC4. Clip uses
// SAME166 name/parameter/aspect domains. Private service identities lacking
// getters remain explicit caller proof. No callback/default/semantic owner is
// substituted, and no new Native credit belongs to composition glue.
//
// Seven Native string-only unwind actions consume state before41DD20. Source
// preentry/metadata errors preserve reached obligations; provider_call is not
// proof of Native origin. No implicit unmap/resource/creator/widget rollback,
// success retirement, pool return, retry or dead-field rediscovery occurs.
// Keep all acquired creators/publications, failed166 lifetime and independent
// graph alive after failure. completed_name_returns counts logical completed
// cleanups even when DATA is null. Nested cleanup escape terminates by Source
// policy; canonical zero terminals are noexcept. No Native FH3/SEH/private
// stack/IAT/register ABI, hardware/FPU fault, production activation or gameplay
// equivalence is claimed. Final canonical registration is a separate check.
void rebuild_native_gui_frame_box_state_storage_00ad0d80(void* actual_box,
    NativeGuiFrameBoxStateStorageFrame&, NativeGuiFrameBoxStateStorageContext&,
    NativeGuiFrameBoxStateStorageAcquired&);
} // namespace bsp
