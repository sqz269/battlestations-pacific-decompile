#pragma once
#include "bsp/gui_text_resource_names.hpp"
#include "bsp/gui_text_runtime_submit.hpp"

namespace bsp {
struct GuiTextPropertyConstants {
    const volatile float& one_00d7a24c;
    const volatile float& shadow_offset_00d5c5c0;
    const float (&shadow_color_00e12fd8)[4]; // SAME borrowed native default quartet.
    const volatile float& white_shadow_alpha_00cee07c;
    // SAME native lazy-default storage, shared across all property readers.
    // Caller supplies its established globals; there is no per-Text white cache.
    std::uint32_t& default_color_mask_00f8be48;
    float (&default_color_00f8be38)[4];
};
struct GuiTextPropertyServices {
    GuiTextFontNameServices& font_names;
    GuiTextSubmitServices& submit;
    const bool& crt_sse2_conversion;
    GuiTextPropertyConstants constants;
};

// Complete00AB72D0..73A8, ECX Text/preset DWORD stack/RET4 at73A6,length3.
// Same preset -> no stores. Changed -1 disables raw shadow byte only; every
// other value installs live default shadow fields, with preset1 overriding
// RGB from live one and alpha from CEE07C. No scene/style/content call.
void apply_gui_text_shadow_preset_00ab72d0(GuiTextLifetime&, std::int32_t,
    const GuiTextPropertyConstants&);

// Outer ABB630 string temporaries, not extra widget state. Preserve them until
// nested source submission and its final CURRENT color50 are complete; then
// destroy vertical, align, DefaultText, Font in native order. Destruction must
// never be used to abandon/complete an unfinished content frame.
struct GuiTextPropertiesContinuation {
    std::string font;
    std::string default_text;
    std::string align;
    std::string vertical_align;
    std::unique_ptr<GuiTextSubmitContinuation> submission;
};
enum class GuiTextPropertiesStatus { complete, pending_content };
struct GuiTextPropertiesResult {
    GuiTextPropertiesStatus status;
    std::unique_ptr<GuiTextPropertiesContinuation> pending;
};

// ABB630 native ECX Text/visitor stack/RET4 atABBE42,length3,endABBE44.
// This is ONLY its derived continuation ABB658..ABBE44: existing AAA710 base
// fields AND child traversal must have finished first. Never invoke base again
// from GuiWidgetTypeImplementation::properties_bound. Reads the ordinary
// evaluated Lua table in native order through existing typed-value/default
// readers, then actual source submission if captured DefaultText is nonempty.
// Font lookup is unconditional; ShaderName is a direct field write here, not
// the separately called AB8E70 invalidation. No extra caller-level color50.
// Pending content retains this frame; loaded78/factory completion must wait.
GuiTextPropertiesResult read_gui_text_properties_after_base_00abb630(
    GuiTextLifetime&, const GuiTable&, GuiTextPropertyServices&);

// Call only after the actual missing glyph-child tail has completed, exactly
// as required by the nested submit/content frame. Other pending reasons remain
// explicit errors from that interface. Clears pending only on complete return.
GuiTextPropertiesStatus resume_gui_text_properties_after_child(
    std::unique_ptr<GuiTextPropertiesContinuation>&);

// Supported same-owner domains come from resource names and runtime submission.
// Ordinary evaluated Lua tables use actual Lua conversion/default rules; invalid
// aggregate/subtable shapes fail at their field rather than inventing defaults.
// Native live Lua metamethod callbacks, pool/string/visitor ABI, SEH and failures
// of host allocation are excluded. No GuiTextHost or factory is added.
} // namespace bsp
