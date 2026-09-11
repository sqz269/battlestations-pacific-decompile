#pragma once
// GUI locale invalidation: 00AA4650, 00AA40A0 and 00ABBE50.
// Names are descriptive hypotheses; native ABI, evidence and constructor scope
// are recorded in docs/GUI_LOCALE_REFRESH.md. This is a C++ state projection,
// not the native 88h-byte cGuiManager or a substitute for its resource loader.

#include <unordered_map>

#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_text.hpp"
#include "bsp/locale_tables.hpp"

namespace bsp {

// Live bytes at 00CE3A70 are 2E 00: the intermediate source is a period.
// It is localized too. Replacing it with an empty string changes the behavior
// for widgets whose cached source is already empty.
inline constexpr char kGuiLocaleRefreshIntermediateSource[] = ".";

// 00ABBE50: __thiscall(ECX = text widget, const char*, localise), RET 8.
// The native C-string overload compares the source, builds a NativeString and
// calls 00ABAED0. A null argument has the same content as an empty string.
// Returns the existing projection's changed flag; the native return is void.
bool set_localised_c_string_00abbe50(GuiTextWidget& widget, GuiTextHost& host,
    const char* source, bool localise);

// Retains the locale-relevant part of the GUI manager: its actual page registry
// (+18h/+1Ch/+20h) and bindings to live text state. Construction establishes the
// empty vector written by 00AA5D70. The constructor's other containers and
// resource fields are analyzed, not represented as fabricated GUI resources.
// A caller may retain this in its application and construct it lazily where
// 004C12B0 gets/creates the native singleton.
class GuiLocaleRefreshManager final : public LocaleGuiRefreshHost {
public:
    GuiLocaleRefreshManager() = default;
    GuiLocaleRefreshManager(const GuiLocaleRefreshManager&) = delete;
    GuiLocaleRefreshManager& operator=(const GuiLocaleRefreshManager&) = delete;

    // Use the existing loader/registration path; this is the same registry the
    // locale walk sees, including its priority order and later additions.
    GuiPageRegistry& page_registry() noexcept { return pages_; }
    const GuiPageRegistry& page_registry() const noexcept { return pages_; }

    // GuiLayoutWidget and GuiTextWidget are existing projections of the same
    // native Text instance. Both widget and host must outlive their binding;
    // unbind before removing either. All Text nodes need a real binding, even
    // invisible nodes or nodes with an empty source. Non-text nodes need none.
    // Wrong widget type is rejected; re-binding replaces the prior references.
    void bind_text(GuiLayoutWidget& node, GuiTextWidget& text, GuiTextHost& host);
    void unbind_text(const GuiLayoutWidget& node) noexcept;

    // 00AA4650: ECX = native manager, no stack args, RET. Walks every page in
    // registry order. Missing text bindings and malformed null graph entries
    // throw std::logic_error rather than silently completing a partial refresh.
    // Host calls may change text state, but must not mutate the graph/bindings
    // during traversal (the native debug iterators reject invalidation too).
    void refresh_locale_00aa4650() override;

    // 00AA40A0: ECX = manager, one widget pointer on stack, RET 4. Text first,
    // then all descendants in child-list order; no visibility/enablement gate.
    // Kept public for callers that already own a widget subtree.
    void refresh_subtree_locale_00aa40a0(GuiLayoutWidget& node);

private:
    struct TextBinding {
        GuiTextWidget* text;
        GuiTextHost* host;
    };

    GuiPageRegistry pages_{};
    std::unordered_map<const GuiLayoutWidget*, TextBinding> text_bindings_{};
};

}  // namespace bsp
