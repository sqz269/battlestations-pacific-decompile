#pragma once

#include "bsp/frontend_screen_animation.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/wire_goal4_chain407_switches.hpp"

#include <set>

namespace bsp::game {

// Existing bridge parent walk, shared by load-time records and live controls.
// Diagnostic layouts carry GuiLayoutWidget::parent even without transform.parent.
inline GuiWidgetPoint resolved_point(const GuiLayoutWidget& widget) {
    if (widget.parent == nullptr) return widget.transform.position;
    const GuiWidgetPoint parent_point = resolved_point(*widget.parent);
    const GuiWidgetSize offset = pivot_offset(widget.parent->transform);
    GuiWidgetPoint point;
    point.x = (parent_point.x + widget.transform.position.x) - offset.width;
    point.y = (parent_point.y + widget.transform.position.y) - offset.height;
    point.z = parent_point.z + widget.transform.position.z;
    return point;
}

// The bridge's existing diffuse conversion, shared with the focused render-input
// fixture. Moving it here does not change the runtime-clone or Text color rule.
inline D3DCOLOR bridge_color(const float (&color)[4]) noexcept {
    const auto lane = [](float value) -> int {
        const float scaled = value * 255.0f;
        if (!(scaled > 0.0f)) return 0;
        if (scaled >= 255.0f) return 255;
        return static_cast<int>(scaled + 0.5f);
    };
    return D3DCOLOR_ARGB(lane(color[3]), lane(color[0]), lane(color[1]), lane(color[2]));
}

// A narrow bridge opt-in for the SAME retained controls mutated by 006834A0.
// These remain authored widgets: no runtime-clone ordering/rotation semantics.
// Screen pages retain the borrowed widget identities for the frontend lifetime.
class FrontendScrollerBridge {
public:
    void bind(const FrontEndScreenScroller& scroller) {
        if constexpr (kWireGoal4Chain407Bound) {
            for (void* widget : {scroller.thumb_widget, scroller.arrow_start_widget,
                                 scroller.arrow_end_widget}) {
                if (widget) controls_.insert(static_cast<const GuiLayoutWidget*>(widget));
            }
        } else {
            static_cast<void>(scroller);
        }
    }

    bool refresh(GameWidgetRecord& record, const GuiLayoutWidget* widget) const {
        if (!contains(widget)) return false;
        const auto point = resolved_point(*widget);
        record.x = point.x;
        record.y = point.y;
        record.z = point.z;
        record.width = widget->transform.size.width;
        record.height = widget->transform.size.height;
        record.pivot_x = widget->transform.pivot_x;
        record.pivot_y = widget->transform.pivot_y;
        return true;
    }

    D3DCOLOR color(const GameWidgetRecord& record, const GuiLayoutWidget* widget) const {
        if (contains(widget)) return bridge_color(widget->color);
        // Prior bridge behavior for every unregistered authored widget and OFF.
        return (widget != nullptr && record.runtime)
            ? bridge_color(widget->color) : D3DCOLOR_ARGB(255, 255, 255, 255);
    }

private:
    bool contains(const GuiLayoutWidget* widget) const {
        if constexpr (kWireGoal4Chain407Bound) {
            return widget && controls_.count(widget) != 0;
        } else {
            static_cast<void>(widget);
            return false;
        }
    }
    std::set<const GuiLayoutWidget*> controls_;
};

} // namespace bsp::game
