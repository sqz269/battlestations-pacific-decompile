#include "bsp/main_menu_widget_listener.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void require_owner(MainMenuWidgetListenerBindings& b, GuiWidgetOwner& widget) {
    require(&widget.runtime() == &b.owners,
        "main-menu listener requires the same canonical widget owner registry");
}
std::uint8_t text_low_byte(GuiWidgetOwner& widget) {
    auto* text = dynamic_cast<GuiTextRuntimeImplementation*>(&widget.implementation());
    require(text && widget.text_lifetime() == &text->lifetime(),
        "main-menu listener Text profile requires its actual live Text lifetime");
    const auto& value = text->lifetime().text().text;
    return value.empty() ? 0 : static_cast<std::uint8_t>(value.front());
}
__declspec(noinline) float argument_spill(float value) noexcept {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
void alpha(MainMenuWidgetListenerBindings& b, GuiWidgetOwner& widget, bool dim) {
    const float value = dim ? b.dim_alpha_00ce3800 : b.one_00d7a24c;
    widget.implementation().set_alpha4c(widget, argument_spill(value));
}
void select_boolean_state(GuiWidgetOwner& widget, bool selected, bool reload_widget_d4) {
    const auto type = widget.implementation().type5c(widget);
    if (type == 6) {
        auto* icon = dynamic_cast<GuiIconTypeImplementation*>(&widget.implementation());
        require(icon != nullptr, "main-menu listener Icon requires the actual Icon runtime");
        const bool current = reload_widget_d4 ? widget.extra_fields().byte_d4 : selected;
        icon->runtime().select_state_00ab1710(static_cast<std::int16_t>(current), 0, 1.0f);
    } else if (widget.implementation().type5c(widget) == 18) {
        // Preserve the SECOND native current5C call before the FrameBox gate.
        auto* frame = dynamic_cast<GuiFrameBoxTypeImplementation*>(&widget.implementation());
        require(frame != nullptr, "main-menu listener FrameBox requires the actual FrameBox runtime");
        const bool current = reload_widget_d4 ? widget.extra_fields().byte_d4 : selected;
        frame->set_state84_00acf070(widget, static_cast<std::int16_t>(current));
    }
}
}

void main_menu_listener_current0c_00581970(
    MainMenuWidgetListenerBindings& b, GuiWidgetOwner& widget) {
    require_owner(b, widget);
    if (widget.implementation().type5c(widget) == 3) {
        const auto code = text_low_byte(widget);
        if (code != 0xb4 && code != 0xb6) return;
        if (static_cast<std::uint32_t>(b.page_00e08874) - 4u <= 4u) {
            const std::uint32_t zero = 0; // 5819BF XORPS / MOVSS +19C
            std::memcpy(&b.zoom_delta_19c, &zero, sizeof(zero));
        }
        alpha(b, widget, widget.extra_fields().byte_d4);
        return;
    }
    const auto page = b.page_00e08874;
    if (page < 4) return;
    auto& layout = b.layout;
    auto* const current = &widget.layout();
    if (page <= 8) {
        if (current == layout.arrow_top_1bc) {
            if (!b.arrow_top_enabled_1c5) return;
        } else if (current == layout.arrow_bottom_1c0) {
            if (!b.arrow_bottom_enabled_1c4) return;
        } else if (current == layout.scroll_bottom_1cc || current == layout.scroll_top_1c8) {
            set_auto_scroll(layout.mission_scroller_1d4, ScrollDirection::kNone, 0.0f, false);
        } else if (current == layout.scroll_thumb_234) {
            end_thumb_drag(layout.mission_scroller_1d4);
        }
    } else {
        if (page != 9) return;
        if (current == layout.briefing_scroll_bottom_34c || current == layout.briefing_scroll_top_348) {
            set_auto_scroll(layout.briefing_scroller_354, ScrollDirection::kNone, 0.0f, false);
        } else if (current == layout.briefing_scroll_thumb_3b4) {
            end_thumb_drag(layout.briefing_scroller_354);
        }
    }
    select_boolean_state(widget, false, true);
}

void main_menu_listener_current18_00581b20(
    MainMenuWidgetListenerBindings& b, GuiWidgetOwner& widget, bool inside) {
    require_owner(b, widget);
    if (widget.implementation().type5c(widget) == 3) {
        const auto code = text_low_byte(widget);
        if (code != 0xa7 && code != 0xa5 && code != 0xa3 && code != 0xa2 &&
            code != 0xb4 && code != 0xb6) return;
        alpha(b, widget, inside);
        return;
    }
    const auto page = b.page_00e08874;
    if (page < 4) return;
    auto& layout = b.layout;
    auto* const current = &widget.layout();
    if (page <= 8) {
        if (current == layout.arrow_top_1bc) {
            if (!b.arrow_top_enabled_1c5) return;
        } else if (current == layout.arrow_bottom_1c0) {
            if (!b.arrow_bottom_enabled_1c4) return;
        } else if (current == layout.scroll_thumb_234) {
            if (layout.mission_scroller_1d4.dragging) return;
        } else if (current == layout.text_background_568) {
            b.screen.pad_axis_override_off = inside; // native same +56C store
            return;
        }
    } else {
        if (page != 9) return;
        if (current == layout.briefing_scroll_thumb_3b4 && layout.briefing_scroller_354.dragging)
            return;
    }
    select_boolean_state(widget, inside, false);
}

MainMenuWidgetListener::MainMenuWidgetListener(void* identity, MainMenuWidgetListenerBindings bindings)
    : GuiBaseWidgetListener(identity), bindings_(bindings) {}
void MainMenuWidgetListener::call_current0c(GuiWidgetOwner& widget) {
    main_menu_listener_current0c_00581970(bindings_, widget);
}
void MainMenuWidgetListener::call_current18(GuiWidgetOwner& widget, bool inside) {
    main_menu_listener_current18_00581b20(bindings_, widget, inside);
}
} // namespace bsp
