#include "bsp/gui_resources.hpp"

#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
GuiLayoutWidget& require_page_root(GuiLayoutPage* page, std::string_view name) {
    if (page == nullptr || page->root == nullptr) {
        throw std::runtime_error("00aa5e20 requires GUI page root: " + std::string(name));
    }
    return *page->root;
}
} // namespace

GuiResourceOwner::GuiResourceOwner(GuiPageRegistry& registry, GuiLayoutHost& layout,
    GuiResourceCallbacks callbacks, GuiResourceState initial_state)
    : registry_(registry), layout_(layout), callbacks_(std::move(callbacks)),
      state_(initial_state) {
    if (!callbacks_.resolve_existing_name || !callbacks_.load_texture ||
        !callbacks_.decrement_texture_reference || !callbacks_.destroy_texture ||
        !callbacks_.set_visibility) {
        throw std::invalid_argument("GuiResourceOwner requires all texture and visibility callbacks");
    }
}

void GuiResourceOwner::show_required(
    GuiLayoutWidget* widget, bool visible, std::string_view name) {
    if (widget == nullptr) {
        throw std::runtime_error("00aa5e20 requires direct GUI child: " + std::string(name));
    }
    callbacks_.set_visibility(*widget, visible);
}

void GuiResourceOwner::initialize_00aa5e20() {
    initialized_ = false;
    std::string white_name = "data/interface/textures/whiteGui.tga";
    // 00aa5e88..00aa5ea4: failed resolution leaves the old +28h unchanged.
    if (callbacks_.resolve_existing_name(white_name)) {
        state_.white_gui = callbacks_.load_texture(white_name, 0);
    }

    // 00aa5ef0 precedes 00aa5eff: acquire new, release old, then assign.
    // No identity check: a returned identical pointer still acquired a ref.
    const GuiResourceTexture transparent = callbacks_.load_texture(
        "interface/textures/common/transparent.tga", 0);
    if (state_.transparent) {
        if (callbacks_.decrement_texture_reference(state_.transparent) == 0) {
            callbacks_.destroy_texture(state_.transparent);
        }
        state_.transparent = {};
    }
    state_.transparent = transparent;

    state_.mouse_page = load_gui_page_00aa5840(registry_, layout_, "_Mouse", 1, false);
    GuiLayoutWidget& mouse = require_page_root(state_.mouse_page, "_Mouse");
    callbacks_.set_visibility(mouse, true); // 00aa5fb1
    state_.frontend_cursor = find_child_by_name_00aa7e00(mouse, "MousePtrFE_Icon");
    state_.current_cursor = state_.frontend_cursor;
    state_.gui_cursor = find_child_by_name_00aa7e00(mouse, "MousePtrGUI_Icon");
    show_required(state_.gui_cursor, false, "MousePtrGUI_Icon"); // 00aa6099
    show_required(state_.current_cursor, false, "MousePtrFE_Icon"); // 00aa60a4

    // _Highlight itself stays owned by the registry: there is no manager slot.
    GuiLayoutWidget& highlight = require_page_root(
        load_gui_page_00aa5840(registry_, layout_, "_Highlight", 1, false), "_Highlight");
    callbacks_.set_visibility(highlight, true); // 00aa611b
    state_.highlight_frame = find_child_by_name_00aa7e00(highlight, "hl_FrameBox");
    show_required(state_.highlight_frame, false, "hl_FrameBox");
    state_.highlight_circle = find_child_by_name_00aa7e00(highlight, "hlCircle_FrameBox");
    show_required(state_.highlight_circle, false, "hlCircle_FrameBox");
    state_.safezone_43 = find_child_by_name_00aa7e00(highlight, "safezone_43_FrameBox");
    show_required(state_.safezone_43, false, "safezone_43_FrameBox");
    state_.safezone_169 = find_child_by_name_00aa7e00(highlight, "safezone_169_FrameBox");
    show_required(state_.safezone_169, false, "safezone_169_FrameBox");
    state_.ready_flag = 0; // 00aa6305, after the final visibility dispatch
    initialized_ = true;
}

} // namespace bsp
