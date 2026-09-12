#include "bsp/gui_listbox_row_control.hpp"
#include "bsp/gui_listbox_runtime.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include "bsp/native_string_compare.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace bsp {
void apply_gui_listbox_widget_state_00a9b340(GuiWidgetOwner& row,
    std::int32_t state, const volatile float* one) {
    if (row.implementation().type5c(row) == 3) { // A9B34F
        auto* const text = dynamic_cast<GuiTextRuntimeImplementation*>(&row.implementation());
        if (!text || &text->lifetime() != row.text_lifetime())
            throw std::logic_error("Listbox Text row requires its canonical Text implementation");
        auto& actual = text->lifetime().text();
        if (actual.text.size() == 1) { // +EC is NativeWideString LENGTH.
            struct Header { std::uint32_t length; const char* data; };
            static_assert(sizeof(Header) == 8, "BSP NativeString transport is Win32");
            const Header source{static_cast<std::uint32_t>(actual.source.size()),
                actual.source.c_str()};
            if (equal_native_string_header_00425850(&source, "globals.live")) {
                if (!one)
                    throw std::logic_error("Listbox globals.live color requires live D7A24C");
                const float lane = *one; // A9B38B: one MOVSS, four copies.
                const float color[4]{lane, lane, lane, lane};
                text->set_color50_00ab6b50(color); // A9B3B7
                return;
            }
        }
        text->set_state80_00ab7200(state); // A9B382
        return;
    }
    // A9B3C7 is a second actual type query, not reuse of the first result.
    if (row.implementation().type5c(row) == 6) {
        auto* const icon = dynamic_cast<GuiIconTypeImplementation*>(&row.implementation());
        if (!icon)
            throw std::logic_error("Listbox Icon row requires its canonical Icon implementation");
        const auto bits = static_cast<std::uint16_t>(static_cast<std::uint32_t>(state));
        std::int16_t index;
        std::memcpy(&index, &bits, sizeof(index));
        icon->runtime().select_state_00ab1710(index, 0, 1.0f); // A9B3E5, FLD1.
    }
}

void apply_gui_listbox_row_state_00a9ba90(GuiWidgetOwner& row,
    std::int32_t state, const volatile float* one) {
    if (row.implementation().type5c(row) != 2) { // A9BABF
        apply_gui_listbox_widget_state_00a9b340(row, state, one); // A9BB11
        return;
    }
    auto& children = row.layout().children;
    std::size_t ordinal = 0;
    while (ordinal < children.size()) {
        auto* const child = children[ordinal].get();
        apply_gui_listbox_widget_state_00a9b340(
            row.runtime().owner(*child), state, one); // A9BAF6
        // Canonical children use a vector owning actual layouts. Re-find the
        // same native current-node identity AFTER callbacks so removal of an
        // earlier child or vector reallocation cannot skip the next live row.
        const auto current = std::find_if(children.begin(), children.end(),
            [child](const auto& owned) { return owned.get() == child; });
        if (current == children.end())
            throw std::logic_error("Listbox row control requires its current Group child to remain attached");
        ordinal = static_cast<std::size_t>(current - children.begin()) + 1;
    }
}

bool has_selectable_gui_listbox_row_00a9ba40(const GuiListboxRuntime& listbox) noexcept {
    for (auto* const row : listbox.rows_)
        if (!row->scene_flags().hidden) return true;
    return false;
}
} // namespace bsp
