#pragma once
#include "bsp/gui_text.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/native_node_parenting.hpp"

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
} // namespace bsp
