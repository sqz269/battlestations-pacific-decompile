#pragma once
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/gui_native_clip_parameters.hpp"
#include <optional>

namespace bsp {
struct GuiTextClipRefreshServices {
    GuiTextBufferServices& buffers;
    NativeMaterialParameterAccess& parameters;
    const float& aspect_ratio_00e12fc0; // SAME borrowed live global, not a copy.
};

// Suspended immediately BEFORE native00AB7AF8 current child virtual70.
// This is a borrowed traversal frame, not child ownership or a list snapshot.
// Membership/order of the parent's transform.children must remain stable from
// the first suspension through completion; native intrusive-node mutation ABI
// is not represented by the existing vector. Duplicate payloads stay distinct
// by their positions. Parent, child and runtime must survive the continuation.
struct GuiTextClipRefreshContinuation {
    GuiWidgetOwner& parent;
    GuiWidgetOwnerRuntime& widgets;
    std::size_t child_index;
    GuiWidgetTransform* pending_child;
    // Resolve the SAME current child owner at the call boundary. The caller
    // must invoke its actual current70, including all nested continuations.
    // This function does not substitute another slot or claim completion.
    GuiWidgetOwner& child_owner() const;
};

//00AB7A40 ECX Text/no stack/RET at00AB7B0B. Implements actual main material
// registration, then LIVE shadow registration, then selects the first child.
// Null node/mesh or zero section count skips only that drawable. Uses actual
// owners and AA9F10 names/pool/borrowed sources; does not allocate new state.
// nullopt means the native return was reached (empty child list). A frame
// means child70 is still REQUIRED; current generic type interface has no70.
// Full body read; general child dispatch remains an explicit partial boundary.
std::optional<GuiTextClipRefreshContinuation>
begin_gui_text_clip_refresh_00ab7a40(GuiTextLifetime&, GuiTextClipRefreshServices&);

// Run ONLY AFTER the pending child's real current70 has fully completed.
// Implements00AB7AFA..7B0B: check current entry, consume this frame, advance,
// reload live list/end/next child, and pause before its CURRENT virtual70.
// No material registration is repeated. Input frame is consumed even when
// another frame is returned; nullopt alone denotes ordinary completion.
std::optional<GuiTextClipRefreshContinuation>
resume_gui_text_clip_refresh_after_child70_00ab7a40(GuiTextClipRefreshContinuation&);
} // namespace bsp
