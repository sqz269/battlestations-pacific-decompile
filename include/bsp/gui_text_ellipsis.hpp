#pragma once

#include "bsp/font_data.hpp"
#include "bsp/gui_text.hpp"
#include "bsp/locale_text_lookup.hpp"

#include <optional>

namespace bsp {

// Semantic normal-path00AB8F00: native ECX Text, destination/source UTF16
// wrappers and float width on stack, EAX destination, RET0Ch. Copies and
// applies the existing locale uppercase map when widget.font requests it.
// Measures unsigned advances with x87 truncation and low16 AFTER EVERY step,
// removes a suffix if necessary, then appends three literal dots. A tiny
// target may wrap when reserving dots; it is deliberately not clamped.
//
// Requires the decoded FontData belonging to widget.font, with both identities
// stable for the whole call (including locale callbacks). The caller supplies
// the actual locale tables/runtime. Null-free text,
// signed32 length, finite scalars and signed32 arithmetic conversions form the
// supported domain; rejected inputs throw. Float environment/trap timing,
// native string pools and SEH are excluded. No geometry or owner is created.
// Names are hypotheses; see docs/GUI_TEXT_ELLIPSIS.md.
std::u16string ellipsize_gui_text_00ab8f00(const GuiTextWidget& widget,
    const FontData& font, const LocaleTables& locale,
    LocaleTextRuntimeHost& locale_runtime, std::u16string_view source,
    float normalized_width);

// Partial projection of00ABB000 through its geometry argument preparation:
// cache guard/store -> ordered -1.0f width fallback -> localize/widen ->
//00AB8F00. Same cached source returns nullopt without reconsidering width,
// font or localize. An engaged empty string is a real changed-source update.
// Zero is an explicit zero target; NaN does not select widget width and the
// finite-domain guard rejects it after the native conversion callback.
//
// The caller MUST submit an engaged result to the actual00ABA8D0 owner, then
// invoke Text virtual50 using its CURRENT color, even if geometry is unchanged.
// The entire geometry/shadow/child/resource operation and that final color call
// are excluded (00ABB0CA..00ABB115 and00ABB149..00ABB1C4). No builder-only
// replacement is provided. Source cache remains changed if conversion throws,
// matching native store-before-conversion order. Existing source-cache helper
// retains its documented ASCII case-fold projection for localization keys.
std::optional<std::u16string> prepare_gui_text_ellipsis_00abb000_fragment(
    GuiTextWidget& widget, GuiTextHost& host, const FontData& font,
    const LocaleTables& locale, LocaleTextRuntimeHost& locale_runtime,
    const std::string& source, float normalized_width, bool localize);

} // namespace bsp
