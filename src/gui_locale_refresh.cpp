#include "bsp/gui_locale_refresh.hpp"

#include <stdexcept>
#include <string>

namespace bsp {

bool set_localised_c_string_00abbe50(GuiTextWidget& widget, GuiTextHost& host,
    const char* source, bool localise) {
    // 00ABBE70..00ABBEC6 performs the same case-insensitive content gate as
    // 00ABAED0 before constructing the temporary string. The existing setter
    // owns that gate for the std::string projection; no engine call is elided.
    return set_localised_source_00abaed0(
        widget, host, source ? std::string(source) : std::string{}, localise);
}

void GuiLocaleRefreshManager::bind_text(
    GuiLayoutWidget& node, GuiTextWidget& text, GuiTextHost& host) {
    if (node.type != GuiWidgetType::Text) {
        throw std::invalid_argument("GUI locale binding requires a Text node");
    }
    text_bindings_.insert_or_assign(&node, TextBinding{&text, &host});
}

void GuiLocaleRefreshManager::unbind_text(const GuiLayoutWidget& node) noexcept {
    text_bindings_.erase(&node);
}

void GuiLocaleRefreshManager::refresh_locale_00aa4650() {
    // 00AA4659 reads begin; 00AA4666 re-reads end; 00AA468F visits each page.
    for (const auto& page : pages_.pages()) {
        if (!page || !page->root) {
            throw std::logic_error("GUI locale refresh requires a live page root");
        }
        refresh_subtree_locale_00aa40a0(*page->root);
    }
}

void GuiLocaleRefreshManager::refresh_subtree_locale_00aa40a0(
    GuiLayoutWidget& node) {
    // 00AA40C0..00AA40CE: virtual +5Ch type id == 3. Visibility is not tested.
    if (node.type == GuiWidgetType::Text) {
        const auto found = text_bindings_.find(&node);
        if (found == text_bindings_.end()) {
            throw std::logic_error(
                "GUI locale refresh requires GuiTextWidget and GuiTextHost for: " +
                node.key);
        }
        // Copy the binding itself so host code never observes an iterator into
        // this map. Borrowed widget/host lifetime remains the caller's contract.
        const TextBinding binding = found->second;
        const std::string saved_source = binding.text->source;  // +F4h/+F8h
        set_localised_c_string_00abbe50(*binding.text, *binding.host,
            kGuiLocaleRefreshIntermediateSource, true);        // 00AA4121
        set_localised_source_00abaed0(
            *binding.text, *binding.host, saved_source, true);  // 00AA412F
    }
    // 00AA415B..00AA4194: follow widget+68h's intrusive child list, pre-order.
    for (const auto& child : node.children) {
        if (!child) {
            throw std::logic_error("GUI locale refresh requires live child nodes");
        }
        refresh_subtree_locale_00aa40a0(*child);
    }
}

}  // namespace bsp
