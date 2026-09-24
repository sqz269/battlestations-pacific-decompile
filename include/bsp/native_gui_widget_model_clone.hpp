#pragma once
#include "bsp/native_gui_text_model_clone.hpp"
#include "bsp/native_instance_geometry.hpp"

namespace bsp {

struct NativeGuiWidgetModelCloneContext {
    NativeInstanceGeometryAccess& access;
    const NativeNodeRawConstants& node_constants;
    // Borrow current actual D62D60; reached geometry10 must be B742A0.
    const volatile std::uint32_t* mesh_vtable_00d62d60;
    // Required only when flags3E reaches nonnull current source180. All
    // services must borrow the same actual geometry/vertex owner domains.
    NativeStreamCloneServices* streams_3e{};
};

// One-shot caller-owned diagnostic/creator publication, initially empty.
// Only construction has native state0 slot-return cleanup. Registration and
// all later exceptions preserve the actual live destination and any acquired
// creators; do not retry or silently roll back. raw_destination/model_owner
// also expose a completed native constructor whose host registration failed.
struct NativeGuiWidgetModelCloneAcquired {
    bool started{};
    bool complete{};
    std::uint32_t active_call_site{};
    std::int32_t native_eh_state{-1};
    void* raw_destination{};
    NativeModelOwner* model_owner{};
    NativeGuiTextModelCloneAcquired creators;
};

// Complete normal B752B0, flags26h or3Eh and parent0 ONLY. Source is a live
// canonical actual Model; its current D62DE8+10 must contain B752B0. Allocate
// the SAME188h pool slot, prepare metadata without native stores, and forward
// the CURRENT source+54 raw header to B75030. Raw model/name-pool mode is
// mandatory; no GuiWidgetOwnerRuntime or semantic constructor is used.
//
// The source and all contexts/actual profile cells remain alive across native
// callbacks. Borrow existing canonical actual owner/scene/mesh bindings; the
// access lifecycle callbacks obey NativeInstanceGeometryAccess's transactional
// host-metadata contracts. Base-copy callbacks precede reloading source180
// and its current geometry10; scalars and pose are read after their callbacks.
// Success returns ONE model creator, also published in acquired.creators.model.
// Transfer or release it exactly once; this frame is not an additional owner.
// Allocation-null diagnostics, host exceptions and canonical callbacks retain
// existing source boundaries; no original FH3/SEH or binary ABI claim.
NativeModelReference& clone_native_gui_widget_model_00b752b0(
    NativeModelOwner& source, std::uint32_t flags,
    NativeGuiWidgetModelCloneContext&, NativeGuiWidgetModelCloneAcquired&);

} // namespace bsp
