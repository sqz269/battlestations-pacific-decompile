#pragma once
#include "bsp/native_gui_text_storage_construct.hpp"
#include "bsp/native_gui_text_reference.hpp"
#include "bsp/native_node_raw_transform.hpp"
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/gui_text_native_layout.hpp"

namespace bsp {
struct NativeGuiTextCursorStorageContext {
    NativeGuiTextSectionStorageContext& shared; // SAME existing85/93 domains
    NativeGuiTextReference& text_reference; // SAME actual122, no new count
    NativeRenderActualOwnerRegistry& owners;
    game::GameNativeReadOnlyData& data; // already held real aliases/literals
    const NativeNodeMappedRawTransformProfiles& transforms;
    GuiTextNativeRendererServices& renderer;
    GuiTextNativeLayoutServices& layout; // genuine SAME constructor domains
};
struct NativeGuiTextCursorStorageFrame {
    // Caller-established, aligned, mutually separate preimages. Only mask is
    // initialized on entry. The SAME raw8B header at locals[2] survives every
    // name call and failure; no placement NativeString/blanket initialization.
    volatile std::uint32_t (&locals)[4]; // mask/rawslot/name length/name data
    volatile std::uint32_t (&matrix)[16];
    float visibility, scalar_178, scalar_17c;
    NativeNodeParentingFrame& parenting; // distinct prepared recursive frames
};
struct NativeGuiTextCursorStorageRelease {
    void* identity{}; // diagnostic only after terminal; never rediscover fields
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    long result{};
    bool entered{}, decremented{}, zero_entered{}, zero_returned{}, complete{};
};
struct NativeGuiTextCursorStorageAcquired {
    // provider_call is a SOURCE provider boundary, not Native exception origin.
    // native_site/phase refine known contracts only; neither proves origin.
    enum class Failure { none, provider_call, host_metadata };
    bool started{}, complete{}, skipped{}, exception_cleanup_started{};
    bool model_constructed{}, model_published{}, model_registered{};
    bool conditional_name_consumed{}, raw_model_return_entered{}, raw_model_returned{};
    bool delegated_mesh_action3{}, delegated_mesh_action3_consumed{};
    bool raw_mesh_return_entered{}, stream_published{}, model_geometry_published{};
    bool material_owner_published{}, material_published{}, section_appended{};
    std::uint32_t active_call_site{}, completed_name_returns{};
    volatile std::int32_t native_eh_state{-1};
    Failure failure{Failure::none};
    NativeModelOwner* model_owner{};
    NativeModelReference* model_creator{}; // SAME creator, no extra retain
    GuiNativeMeshAcquired mesh;
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired vertex;
    GuiNativeSectionAcquired section;
    NativeMaterialFactoryAcquired material;
    NativeMeshStorage* mesh_creator{};
    NativeMeshSectionStorage* section_creator{};
    NativeMaterialStorage* material_creator{};
    void* matrix_receiver{};
    void* visibility_receiver{};
    void* geometry_receiver{};
    void* declaration_renderer{};
    void* vertex_renderer{};
    std::uint32_t matrix_target{}, declaration_target{}, vertex_target{};
    NativeTextureSurfaceReferenceIncrement final_decrement{};
    NativeGuiTextCursorStorageRelease declaration_release, vertex_release;
    NativeGuiTextCursorStorageRelease material_release, section_release, mesh_release;
    NativeNodeSetParentAcquired parenting;
};

// Complete AB8910[789B/236 starts], Native ECX actual Text/no stack args/RET.
// SAME genuine93 destination and canonical122 binding; preflight checks the
// bound identity/registry without reading/gating/decrementing its actual count.
// Nonnull current184 returns after mask0. Cold construction uses real supplied
// model/node/mesh/material/renderer/layout/string/readonly domains. No typed
// Text/GuiWidgetOwner overlay, fake getter/factory or new owner/count exists.
//
// Prehold the WHOLE independent graph and opaque backing BEFORE callbacks.
// Known disjoint-region checks cannot prove transitive ownership/provenance.
// Context, frame, local/matrix backing, acquisitions, recursive frames, all
// creators and provider/constant/import storage remain alive and separated
// through failure, even if an outer caller ends Text. Required canonical
// aliases may share identities; caller-mutable regions never alias them.
// The concrete layout service and mapped context must borrow these SAME
// actual providers/data; their private identities remain caller proof.
//
// Entered calls are one-shot, including failure. Retain diagnostics/creators,
// publications, raw slots and consumed actions independently; never reset,
// retry, rediscover dead fields or automatically clean an interrupted graph.
// The genuine combined mesh provider owns action3 during its constructor;
// cursor never repeats its consumed B72F70. Added host preparation/admission
// failures preserve reached obligations; completed creators are not rollback.
// Acquired metadata destruction is not native retirement: do not destroy it
// while any failed/published graph or provider operation remains outstanding.
// No Text success destructor, factory pool return, production activation,
// raw rebuild/copy/font/glyph/index, Native private-frame/FH3/SEH/ABI claim.
void ensure_native_gui_text_cursor_storage_00ab8910(void* actual_text,
    NativeGuiTextCursorStorageFrame&, NativeGuiTextCursorStorageContext&,
    NativeGuiTextCursorStorageAcquired&);
} // namespace bsp
