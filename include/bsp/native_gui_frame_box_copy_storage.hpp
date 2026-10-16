#pragma once
#include "bsp/native_gui_frame_box_state_storage.hpp"
#include "bsp/native_gui_frame_box_vector_copy.hpp"
#include "bsp/native_gui_widget_copy_storage.hpp"
#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw FrameBox copy storage requires MSVC Win32.
#endif

namespace bsp {
// All references borrow existing genuine providers in the SAME actual domain.
// constructed supplies geometry/owners/data/transform/dispatch/decrement for
// BOTH147 and169. No callback, default, new registry/pool/count or typed Box.
struct NativeGuiFrameBoxCopyStorageServices {
    NativeGuiWidgetCopyContext& base;
    NativeGuiWidgetLifetimeContext& widgets;
    NativeGuiFrameBoxConstructedStorageContext& constructed;
    NativeMaterialDestructionAccess& materials;
    NativeMaterialEffectCacheContext& effects;
    NativeStringRawPoolContext& strings;
    game::GameNativeStringProcess& string_process;
    GuiTextNativeRendererServices& renderer;
    GuiTextNativeLayoutServices& layout;
    NativeLogicalBufferMappingContext& mapping;
    const NativeGuiFrameBoxGeometryContext& geometry_storage;
    NativeGuiWidgetClipBindings& clip;
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
    NativeGuiWidgetPool<GuiFrameBoxPoolTraits>& matching_pool;
    // SAME held actual D5D130 data through+80, never a copied/default table.
    const volatile std::uint32_t* actual_profile_00d5d130;
};
// Retain these exact external frames/acquisitions, their nested metadata and
// whole partial creator/provider/backing graph BEFORE any constructor callback.
// Parent metadata never destroys them, including entered/failed166 clip state.
struct NativeGuiFrameBoxCopyStorageFrames {
    NativeGuiFrameBoxStateStorageFrame& state;
    NativeGuiFrameBoxConstructedStorageFrame& constructed;
    NativeGuiWidgetCopyAcquired& base_acquired;
    NativeGuiFrameBoxStateStorageAcquired& state_acquired;
    NativeGuiFrameBoxConstructedStorageAcquired& constructed_acquired;
};

enum class NativeGuiFrameBoxCopyStoragePhase : std::uint8_t {
    fresh, entered, returned, failed, failed_unwind_returned
};

// New Source scheduling/lifetime metadata ONLY. Construct outside the failing
// scope, with a fresh external138 op for exact destination/source+F0 and a
// DISTINCT stable fresh111 array sufficient for the eventual constructed span.
// Known placement/regions/fresh metadata validate in the first initializer,
// before member stores. No raw payload/header snapshot or construction occurs
// there; registry.prepare_binding reserves the opaque key BEFORE Native entry.
// All metadata/frames/arrays/services and transitive graph stay live/stable and
// disjoint from raw/retired/provider scratch. External callbacks cannot mutate
// scheduling metadata, reset/reenter operations or reuse this actual slot.
//
// After real138 returns, prepare own119 from returned128 committed diagnostics
// (known null/empty for zero count), with no early header/payload read. After
// name/scalars are ready, private163 consumes the SAME registry capability;
// only then does the169 context lifetime begin. Native creator credit/actual
// atomic/slot and full reached graph survive active callbacks; no protective
// retain/count gate or125 deletion during active construction is introduced.
// Added Source preparation/entry/profile errors are not Native EH origins.
//
// Returned destruction requires genuine163 retirement and fresh/returned119.
// Failed_unwind_returned permits metadata-only ending after all applicable
// Native member actions return and private163 unbinds. This is NOT graph
// recovery: caller-held external failed138/111/166/acquisitions/creators,
// orphaned names/material Box credits/live atomic/raw slot and all other
// obligations OUTLIVE this metadata as their real contracts require. In
// particular, no later125/zero dispatch/material release into a destructed Box,
// retry or slot reuse is justified by a retained address. A genuine separately
// proved nonzero credit release is not forbidden by this metadata rule.
// Entered/failed destruction terminates reading metadata only. No reset,
// generic abandonment, factory/pool return or automatic graph cleanup.
class NativeGuiFrameBoxCopyStorageOperation final {
public:
    NativeGuiFrameBoxCopyStorageOperation(void* destination, const void* source,
        NativeGuiFrameBoxCopyStorageServices&, NativeGuiFrameBoxCopyStorageFrames&,
        NativeGuiFrameBoxVectorCopyOperation& fresh_vector,
        NativeGuiFrameBoxRecordReleaseOperation* fresh_destruction_records,
        std::size_t destruction_capacity);
    ~NativeGuiFrameBoxCopyStorageOperation() noexcept;
    NativeGuiFrameBoxCopyStorageOperation(const NativeGuiFrameBoxCopyStorageOperation&) = delete;
    NativeGuiFrameBoxCopyStorageOperation& operator=(const NativeGuiFrameBoxCopyStorageOperation&) = delete;
    NativeGuiFrameBoxCopyStorageOperation(NativeGuiFrameBoxCopyStorageOperation&&) = delete;
    NativeGuiFrameBoxCopyStorageOperation& operator=(NativeGuiFrameBoxCopyStorageOperation&&) = delete;

    NativeGuiFrameBoxCopyStoragePhase phase() const noexcept { return phase_; }
    void* destination_identity() const noexcept { return destination_; }
    const void* source_identity() const noexcept { return source_; }
    std::int32_t cleanup_state() const noexcept { return cleanup_state_; }
    std::uint32_t active_call_site() const noexcept { return active_call_site_; }
    std::uint32_t captured80_target() const noexcept { return target80_; }
    bool unwind_entered() const noexcept { return unwind_entered_; }
    bool unwind_returned() const noexcept { return unwind_returned_; }
    bool construction_binding_published() const noexcept { return reference_constructed_; }
    NativeGuiFrameBoxReference* canonical_reference() noexcept;
    const NativeGuiFrameBoxVectorTidyOperation* tidy_operation() const noexcept;
private:
    friend void* copy_native_gui_frame_box_storage_00ad27a0(void*, const void*,
        NativeGuiFrameBoxCopyStorageOperation&);
    void prepare_tidy();
    void bind_ready();
    void unwind_members() noexcept;
    NativeGuiFrameBoxVectorTidyOperation* tidy() noexcept;
    NativeGuiFrameBoxLifetimeContext* lifetime() noexcept;
    NativeGuiFrameBoxIdentityContext* identity_context() noexcept;
    NativeGuiFrameBoxStateStorageContext* state_context() noexcept;

    void* const destination_;
    const void* const source_;
    NativeGuiFrameBoxCopyStorageServices& services_;
    NativeGuiFrameBoxCopyStorageFrames& frames_;
    NativeGuiFrameBoxVectorCopyOperation& vector_;
    NativeGuiFrameBoxRecordReleaseOperation* const records_;
    const std::size_t capacity_;
    NativeGuiFrameBoxCopyStoragePhase phase_{};
    std::byte phase_padding_[3]; // Explicit Win32 Source padding; no Native ABI.
    volatile std::int32_t cleanup_state_{-1};
    std::uint32_t active_call_site_{}, target80_{};
    bool tidy_constructed_{}, lifetime_constructed_{}, identity_context_constructed_{};
    bool reference_constructed_{}, state_context_constructed_{};
    bool unwind_entered_{}, unwind_returned_{};
    std::byte storage_padding_[1]; // Keep aligned late storage without hidden padding.
    alignas(NativeGuiFrameBoxVectorTidyOperation)
        std::byte tidy_storage_[sizeof(NativeGuiFrameBoxVectorTidyOperation)];
    alignas(NativeGuiFrameBoxLifetimeContext)
        std::byte lifetime_storage_[sizeof(NativeGuiFrameBoxLifetimeContext)];
    alignas(NativeGuiFrameBoxIdentityContext)
        std::byte identity_storage_[sizeof(NativeGuiFrameBoxIdentityContext)];
    alignas(NativeGuiFrameBoxReference)
        std::byte reference_storage_[sizeof(NativeGuiFrameBoxReference)];
    alignas(NativeGuiFrameBoxStateStorageContext)
        std::byte state_storage_[sizeof(NativeGuiFrameBoxStateStorageContext)];
    NativeRenderActualOwnerRegistry::BindingAdmission admission_;
};

// Whole AD27A0[AD27A0,AD2876),214B/53 starts: Native ECX destination,
// stack source, EAX saved destination, RET4. Ordinary Source services/metadata
// ABI, not a drop-in Original private-frame/FH3/SEH/stack/register/IAT entry.
// Borrow nonoverlapping live actual120h FrameBox slots, genuine base/Model/
// string/render/canonical2D/SDK/profile domains and SAME initialized F8C188
// pool authority; preserve all unwritten bytes/proxyF0/trailer11C. Source
// fields remain current through callbacks, never an eager copy projection.
// Base/vector/name/four x87 pairs, state0 then2, late currentEC/vptr80, BYTE74
// clear only after80 returns, and real147 hook retain Native order. Hardware/
// unmasked x87 fault and private cleanup-double-exception behavior unproved.
// Consume name/vector/base actions before real helpers; cleanup escape during
// C++ rethrow terminates. Post-unwind163 retirement reads saved metadata only.
// No automatic factory return, creator/material rollback, recovery/activation.
void* copy_native_gui_frame_box_storage_00ad27a0(void* actual_destination,
    const void* actual_source, NativeGuiFrameBoxCopyStorageOperation& operation);
} // namespace bsp
