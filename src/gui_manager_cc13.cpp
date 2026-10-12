// GUI manager and widget-base helpers, segments 76 and 77. Packet cc13-gui6;
// evidence and uncertainty per routine in include/bsp/gui_manager_cc13.hpp and
// docs/CC13_GUI_MANAGER.md.
#include "bsp/gui_manager_cc13.hpp"

#include <cstring>
#include <string.h>

namespace bsp::gui_manager_cc13 {

namespace {

// _stricmp over the native string's characters, as the image does (CRT
// __stricmp); like the image it stops at the first NUL.
int native_stricmp(const std::string& a, const std::string& b) noexcept {
    return _stricmp(a.c_str(), b.c_str());
}

} // namespace

// 00AA2110
void set_safe_zone_visible_00aa2110(GuiSafeZoneState& state, GuiSafeZoneHost& host, bool show) {
    if (!show) {
        if (state.shown) {
            host.set_frame_visible(GuiSafeZoneFrame::SixteenByNine, false); // +80h
            host.set_frame_visible(GuiSafeZoneFrame::FourByThree, false);   // +7Ch
            state.shown = false;
        }
        return;
    }
    if (!state.shown) {
        host.set_frame_visible(host.platform_widescreen() ? GuiSafeZoneFrame::SixteenByNine
                                                          : GuiSafeZoneFrame::FourByThree,
            true);
        state.shown = true;
    }
}

// 00AA3F70
void update_page_list_00aa3f70(GuiPageListUpdateHost& host, const std::vector<GuiPageHandle>& pages,
    float seconds) {
    // The image reads pages[0] before any test; an empty list reaches the
    // invalid-parameter handler 00BF6713 there. The C++ keeps the precondition
    // visible instead of reading past the end.
    if (pages.empty()) {
        return;
    }
    host.set_pointer_scope_page(pages.front()); // manager +6Ch
    host.run_pointer_pass();                    // 00AA3910
    for (GuiPageHandle page : pages) {
        if (host.page_visible(page)) {         // virtual +38h
            host.page_update(page, seconds);   // virtual +40h
        }
    }
    host.set_pointer_scope_page(nullptr);
}

// 00AA4EF0
void relayout_all_pages_00aa4ef0(GuiWideScreenRelayoutHost& host) {
    const std::vector<GuiPageHandle> pages = host.snapshot_pages();
    for (GuiPageHandle page : pages) {
        host.apply_widescreen_layout(page); // 00AA8710
    }
}

// 00AA6B10
bool contains_point_00aa6b10(const GuiWidgetTransform& widget, float x, float y) noexcept {
    const float width = widget.size.width;
    const float height = widget.size.height;
    const float pivot_x = widget.pivot_x * width;  // FSTP [esp+0Ch]
    const float pivot_y = widget.pivot_y * height; // FSTP [esp+10h]
    const GuiWidgetPoint origin = resolved_position(widget); // 00AA6750
    const float left = origin.x - pivot_x;
    const float top = origin.y - pivot_y;
    // Each test is the negation of FCOMI's JB (below or unordered).
    if (!(x >= left)) {
        return false;
    }
    const float right = left + width;
    if (!(right >= x)) {
        return false;
    }
    if (!(y >= top)) {
        return false;
    }
    const float bottom = top + height;
    return bottom >= y;
}

// 00AA69E0
std::array<float, 2> scale_for_extent_00aa69e0(const GuiWidgetTransform& widget, float width,
    float height) noexcept {
    return {width / widget.size.width, height / widget.size.height};
}

// 00AA8EC0
void set_rect_00aa8ec0(GuiRectNode& widget, const GuiWidgetRect& rect, bool recursive) {
    widget.rect = rect;
    if (!recursive) {
        return;
    }
    for (GuiRectNode* child : widget.children) {
        set_rect_00aa8ec0(*child, rect, recursive);
    }
}

// 00AA8F30
GuiWidgetRect rect_from_widget_00aa8f30(const GuiWidgetTransform& source) noexcept {
    const GuiWidgetPoint origin = resolved_position(source);
    GuiWidgetRect rect;
    rect.left = origin.x;
    rect.right = source.size.width + origin.x;   // FLD [edx+20h]; FADD [eax]
    rect.top = origin.y;
    rect.bottom = origin.y + source.size.height; // FLD [eax+4]; FADD [edx+24h]
    return rect;
}

// 00AA2420 / 00AA7810
bool name_less_00aa2420(const std::string& left, const std::string& right) noexcept {
    if (left.empty()) {
        return !right.empty();
    }
    if (right.empty()) {
        return false;
    }
    return native_stricmp(left, right) < 0;
}

// 00AA9AB0
std::string qualified_name_00aa9ab0(const GuiNamedWidget& widget) {
    if (widget.parent == nullptr) {
        return widget.name;
    }
    return qualified_name_00aa9ab0(*widget.parent) + "." + widget.name;
}

// 00AAA0F0
void visit_child_paths_00aaa0f0(GuiNamedWidget& widget, const std::string& prefix,
    GuiPathVisitHost& host) {
    for (GuiNamedWidget* child : widget.children) {
        host.visit_child(*child, prefix + "." + child->name);
    }
}

// 00AAA2B0
GuiNamedWidget* find_child_by_name_suffix_00aaa2b0(const GuiNamedWidget& widget,
    const std::string& suffix) {
    for (GuiNamedWidget* child : widget.children) {
        const std::string& name = child->node_name; // 00B6D800 on child +4Ch
        if (suffix.size() > name.size()) {
            continue;
        }
        // The tail starts suffix.size() characters before the end and is
        // copied up to its first NUL.
        const std::string tail(name.c_str() + (name.size() - suffix.size()));
        bool equal;
        if (suffix.size() != tail.size()) {
            equal = false;
        } else if (suffix.empty()) {
            equal = tail.empty();
        } else {
            equal = native_stricmp(suffix, tail) == 0;
        }
        if (equal) {
            return child;
        }
    }
    return nullptr;
}

// 00AA9C90
void hoist_flagged_children_00aa9c90(GuiHoistNode& widget, const std::string& path,
    GuiHoistHost& host) {
    // Index walk: an adopted child leaves this list, which the image survives
    // by stepping its iterator first (00AA9CFB).
    std::size_t i = 0;
    while (i < widget.children.size()) {
        GuiHoistNode* child = widget.children[i];
        if (child->hoist) {
            const std::size_t before = widget.children.size();
            host.adopt(path, *child);
            if (widget.children.size() == before) {
                ++i; // the host did not unlink it
            }
            continue;
        }
        if (widget.type_id == 1) {
            hoist_flagged_children_00aa9c90(*child, path, host);
        } else {
            hoist_flagged_children_00aa9c90(*child, path + "." + widget.name, host);
        }
        ++i;
    }
}

// 00AA5390
void show_only_widgets_00aa5390(const std::vector<GuiVisibilityNode*>& pages,
    const std::vector<GuiVisibilityNode*>& shown, GuiVisibilityHost& host) {
    std::vector<int> saved;
    saved.reserve(shown.size());
    for (const GuiVisibilityNode* widget : shown) {
        saved.push_back(static_cast<int>(widget->visibility)); // CVTTSS2SI
    }
    std::vector<GuiVisibilityNode*> work(pages.begin(), pages.end());
    for (std::size_t i = 0; i < work.size(); ++i) {
        GuiVisibilityNode* widget = work[i];
        host.set_visibility(*widget, 0.0f);
        for (GuiVisibilityNode* child : widget->children) {
            work.push_back(child);
        }
    }
    for (std::size_t i = 0; i < shown.size(); ++i) {
        for (GuiVisibilityNode* w = shown[i]; w != nullptr; w = w->parent) {
            host.set_visibility(*w, static_cast<float>(saved[i])); // CVTSI2SS
        }
    }
}

// 00AAB980 / 00AAB9BC: NEG/SBB/AND 3 for the old focus, SETNE + 2 for the new.
int button_label_state(const GuiButton& button, bool focused) noexcept {
    if (focused) {
        return button.disabled ? 3 : 2;
    }
    return button.disabled ? 3 : 0;
}

// 00AABD70
void read_button_label_00aabd70(GuiButton& button, const std::string& label_value,
    GuiButtonLabelHost& host) {
    button.label_name = label_value;
    if (!button.label_name.empty()) {
        button.label_widget = host.find_child_by_name(button, button.label_name);
    }
    if (button.label_widget != nullptr) {
        host.set_label_state(button.label_widget, button_label_state(button, false));
    }
}

// 00AAC270
bool button_label_is_described_00aac270(const GuiButton& button) noexcept {
    return !button.label_name.empty();
}

// 00AAB980
void set_focused_button_00aab980(GuiButtonRegistry& registry, GuiButtonFocusHost& host,
    GuiButton* button) {
    if (GuiButton* old = registry.focused; old != nullptr && old->label_widget != nullptr) {
        host.set_label_state(*old, button_label_state(*old, false));
    }
    registry.focused = button;
    if (button != nullptr && button->label_widget != nullptr) {
        host.set_label_state(*button, button_label_state(*button, true));
    }
    host.selection_sound(true, false); // JMP [00F8BCFC] with cl = 1, dl = 0
}

// 00AAC410
void update_button_hover_00aac410(GuiButtonRegistry& registry, GuiButtonFocusHost& host) {
    GuiButton* hovered = nullptr;
    for (GuiButton* button : registry.buttons) {
        if (button->visible && !button->disabled && button->label_widget != nullptr &&
            button->label_contains_pointer) {
            hovered = button; // the last match wins
        }
    }
    if (hovered != registry.focused) {
        set_focused_button_00aab980(registry, host, hovered);
    }
    if (host.read_pointer_activation() && registry.focused != nullptr) {
        if (registry.focused->listener != nullptr) {
            host.notify_listener(*registry.focused);
        }
        host.selection_sound(false, true); // CALL [00F8BCFC] with cl = 0, dl = 1
    }
}

} // namespace bsp::gui_manager_cc13
