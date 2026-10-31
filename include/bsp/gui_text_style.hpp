#pragma once
#include "bsp/gui_text.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/native_node_parenting.hpp"
#include "bsp/native_node_parenting_storage.hpp"

namespace bsp {
// Borrowed associations for the SAME Text widget; this is not a new Text
// owner, native overlay, state copy or interface of forwarding callbacks.
// The caller supplies its canonical Text projection and live +188h pointer
// slot. All referenced storage and actual service domains must outlive calls.
// Text factory/glyph/shadow allocation and destruction remain external.
struct GuiTextStyleBinding {
    GuiWidgetOwner& widget;
    GuiTextWidget& text;
    NativeNodeBinding*& shadow_188;
    const GuiMaterialBindingServices& materials;
    NativeNodeParentingRuntime& parenting;
};

// Complete supported Text current+50,00AB6B50: ECX Text, RGBA pointer stack,
// RET4. Existing base color writes, then only shadow material diffuse alpha.
// Incoming RGBA stays borrowed through the base call. The existing text.color
// projection mirrors the canonical layout color stores; no shadow RGB write.
void set_gui_text_color50_00ab6b50(GuiTextStyleBinding&, const float (&rgba)[4]);

// Complete00AB6C30: ECX Text; enabled low byte, position DWORD, offset float,
// color pointer stack; RET10h. Raw field stores precede actual node parenting;
// rereads enabled and shadow pointer before optional root-registration clear.
void configure_gui_text_shadow_00ab6c30(GuiTextStyleBinding&, std::uint8_t enabled,
    GuiTextShadowPos position, float offset, const GuiTextColor& color);

// Partial input-domain projection of raw00AB7200 current+80: ECX Text, state
// DWORD stack, RET4. Complete body
// for hidden widgets (any state), or non-hidden state0..3. Other non-hidden
// indices select storage outside the four colors natively; explicitly rejected
// here, so that unchecked-address input domain is NOT reconstructed.
// Hidden uses the existing owner+77h flag, independent of node visibility.
// State1 brightens RGB even when hidden; no has_state_colors gate or clamp.
void set_gui_text_state80_00ab7200(GuiTextStyleBinding&, std::int32_t state);

// Ordinary caller-retained Source homes for raw00AB6C30. Offset is the exact
// MOVSS DWORD payload; color_argument is an actual 32-bit DATA pointer. All
// four homes and parenting preimages are prepared before entry, without new
// defaults. Supply a distinct persistent frame/fresh acquisition for reentry.
struct NativeGuiTextShadowStorageFrame {
    volatile std::uint32_t enabled_argument;
    volatile std::uint32_t position_argument;
    volatile std::uint32_t offset_argument;
    volatile std::uint32_t color_argument;
    NativeNodeParentingFrame parenting;
};

// Complete raw primitive: CURRENT Text+4C/+188 and the genuine raw node graph,
// with no typed widget/Model association. Actual Text, DWORD/color backing,
// frames, acquisitions and the SAME context/recursive providers remain live
// through callbacks and failure. Incoming homes/frame/acquisition metadata
// are disjoint from Text, context and reached node backing; the four
// sequential color reads may overlap Text.
// Parenting may replace +15C/+188; the optional root call reloads both. Keep
// every reached generation/partial callee record on escape; no cleanup/retry.
// Existing callee admission/valid-backing contracts apply. No Native ABI,
// private-stack alias, FH3/SEH/fault, NoThrow or application/origin claim.
void configure_gui_text_shadow_00ab6c30(void* actual_text,
    NativeGuiTextShadowStorageFrame&, NativeNodeParentingContext&,
    NativeNodeSetParentAcquired&);
} // namespace bsp
