#include "bsp/main_menu_command_listener.hpp"
#include "bsp/gui_listbox_runtime.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
// Parent-owned helpers added with this integration batch.
void focus_first_prompt_navigation_00530670(FrontEndPromptScreen&, FrontEndPromptHost&);
void focus_last_prompt_navigation_00530c20(FrontEndPromptScreen&, FrontEndPromptHost&);
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
std::int32_t signed_word(std::uint32_t word) noexcept {
    std::int32_t value; std::memcpy(&value, &word, 4); return value;
}
__declspec(noinline) void copy_scalar_word(float& destination,
    const volatile float& source) noexcept {
    const volatile float* input = &source;
    float* output = &destination;
    __asm {
        mov ecx, input
        mov eax, [ecx]
        mov edx, output
        mov [edx], eax
    }
}
__declspec(noinline) float argument_spill(float value) noexcept {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
GuiWidgetOwner& bound_owner(MainMenuCommandListenerBindings& b, GuiLayoutWidget* widget) {
    require(widget != nullptr, "main-menu command requires an actual bound widget");
    return b.widget.owners.owner(*widget);
}
GuiListboxRuntime& listbox(MainMenuCommandListenerBindings& b, GuiWidgetOwner& owner) {
    auto& value = b.services.listbox_runtime(owner);
    require(&value.owner() == &owner,
        "main-menu command requires the same bound Listbox companion");
    return value;
}
GuiListboxRuntime& listbox(MainMenuCommandListenerBindings& b) {
    return listbox(b, bound_owner(b, b.widget.layout.main_listbox_1b8));
}
std::uint8_t text_code(GuiWidgetOwner& widget) {
    auto* actual = dynamic_cast<GuiTextRuntimeImplementation*>(&widget.implementation());
    require(actual && widget.text_lifetime() == &actual->lifetime(),
        "main-menu command Text requires the actual lifetime");
    const auto& text = actual->lifetime().text().text;
    return text.empty() ? 0 : static_cast<std::uint8_t>(text.front());
}
bool contains(const MainMenuCommandWidgetListView& list, GuiLayoutWidget& widget) {
    if (!list.count_08) return false;
    auto* head = list.head_04;
    require(head != nullptr, "main-menu command list requires its live sentinel");
    auto* node = head->next;
    for (std::uint32_t index = 0; index < list.count_08; ++index) {
        require(node && node != list.head_04,
            "main-menu command list iterator reached the sentinel before count");
        if (node->widget == &widget) return true;
        node = node->next;
    }
    return false;
}
struct StringTemporary {
    NativeString value;
    NativeStringStorage& strings;
    bool live{};
    explicit StringTemporary(NativeStringStorage& storage) : strings(storage) {}
    ~StringTemporary() { destroy(); }
    void construct(const char* text) {
        value.assign_0041e870(strings, text);
        live = true;
    }
    void destroy() noexcept {
        if (live) {
            live = false;
            destroy_native_string_header_0041dd20(&value, strings);
        }
    }
};
struct CheckpointTemporary {
    MainMenuCommandCheckpointValue value;
    NativeStringStorage& strings;
    bool live{};
    explicit CheckpointTemporary(NativeStringStorage& storage) : strings(storage) {}
    ~CheckpointTemporary() {
        // 436490 only releases current string+0C; words are not destroyed.
        if (live) destroy_native_string_header_0041dd20(&value.text_0c, strings);
    }
};
void raise_checkpoint_prompt(MainMenuCommandListenerBindings& b,
    const MainMenuCommandCheckpointValue& checkpoint, bool available, bool unlocked) {
    const bool offer_checkpoint = available && unlocked;
    StringTemporary title(b.strings), suffix(b.strings), countdown(b.strings);
    title.construct(""); // CE3A0C
    suffix.construct(offer_checkpoint ? "globals.checkpoint_available"
        : "|.\n|globals.continue_without_checkpoint"); // CEFEE4 / CEFEBC
    countdown.construct("");
    StringTemporary prefixed(b.strings), combined(b.strings);
    const NativeString* message = &suffix.value;
    if (!offer_checkpoint) {
        // Full normal 43C130 composition: construct prefix, concat with the
        // checkpoint's CURRENT native text, destroy prefix before outer concat.
        {
            StringTemporary prefix(b.strings);
            prefix.construct(available ? "globals.checkpoint_unit_locked|.\n|."
                : "globals.checkpoint_unit_notavailable|.\n|.");
            concatenate_native_string_headers_004261a0(&prefix.value, &prefixed.value,
                &checkpoint.text_0c, b.strings);
            prefixed.live = true;
        }
        concatenate_native_string_headers_004261a0(&prefixed.value, &combined.value,
            &suffix.value, b.strings);
        combined.live = true;
        message = &combined.value;
    }
    auto& screen = b.services.prompt_00425d10();
    auto& host = b.services.prompt_host(screen);
    countdown.live = false; // 531B00 owns this by-value8h argument.
    raise_prompt_00531b00(screen, host, 5, *message, PromptKind::YesNo,
        offer_checkpoint ? 0x00592ad0u : 0x0058f540u, 0, title.value,
        0.0f, 0, std::move(countdown.value), true);
    combined.destroy();
    prefixed.destroy();
    suffix.destroy();
    title.destroy();
    // The native getter is called again AFTER every local string destruction.
    auto& focus_screen = b.services.prompt_00425d10();
    auto& focus_host = b.services.prompt_host(focus_screen);
    if (offer_checkpoint) focus_first_prompt_navigation_00530670(focus_screen, focus_host);
    else focus_last_prompt_navigation_00530c20(focus_screen, focus_host);
}
void mission_accept(MainMenuCommandListenerBindings& b) {
    const auto selected = listbox(b).selected_data_d8_00a9c990();
    b.selected_00e194dc = selected; // native59946E precedes599473
    b.selected_00e08878 = selected;
    const auto page = b.widget.page_00e08874;
    b.widget.screen.us_campaign = page == 5 || page == 7;
    auto* profile = &b.services.profile_00e188a8_650();
    auto* mission = &b.services.selected_mission_005806a0();
    if (!b.services.call_007f8d60(*profile, *mission)) {
        b.mission_flag_5c = 0;
        b.services.call_0058c010();
        return;
    }
    profile = &b.services.profile_00e188a8_650();
    mission = &b.services.selected_mission_005806a0();
    CheckpointTemporary checkpoint(b.strings);
    b.services.call_007fc490(*profile, checkpoint.value, *mission, b.strings);
    checkpoint.live = true;
    profile = &b.services.profile_00e188a8_650();
    mission = &b.services.selected_mission_005806a0();
    const bool available = b.services.call_007fc370(*profile, *mission);
    bool unlocked = false;
    if (available) {
        const auto vehicle_class = signed_word(checkpoint.value.words[2]);
        const bool first = !b.widget.screen.us_campaign; // reload after profile calls
        unlocked = b.services.call_00584750(first, vehicle_class);
    }
    raise_checkpoint_prompt(b, checkpoint.value, available, unlocked);
}
void pressed_state(GuiWidgetOwner& widget) {
    if (widget.implementation().type5c(widget) == 6) {
        auto* icon = dynamic_cast<GuiIconTypeImplementation*>(&widget.implementation());
        require(icon != nullptr, "main-menu command Icon requires the actual runtime");
        icon->runtime().select_state_00ab1710(2, 0, 1.0f);
    } else if (widget.implementation().type5c(widget) == 18) {
        auto* frame = dynamic_cast<GuiFrameBoxTypeImplementation*>(&widget.implementation());
        require(frame != nullptr, "main-menu command FrameBox requires the actual runtime");
        frame->set_state84_00acf070(widget, 2);
    }
}
void begin_scroll(MainMenuCommandListenerBindings& b, FrontEndScreenScroller& scroller,
    ScrollDirection direction) {
    set_auto_scroll(scroller, direction, argument_spill(b.scroll_step_00cec178), true);
}
} // namespace

void main_menu_listener_current04_005993a0(MainMenuCommandListenerBindings& b,
    GuiWidgetOwner& widget) {
    require(&widget.runtime() == &b.widget.owners,
        "main-menu command requires its same canonical widget owner registry");
    // FLD1 -> FSTP outgoing float at5993CE. Never replace with a mutable global.
    widget.implementation().set_alpha4c(widget, 1.0f);
    if (widget.implementation().type5c(widget) == 3) {
        if (text_code(widget) == 0xa2) {
            switch (b.widget.page_00e08874) {
            case 1: b.services.call_00588a80(); break;
            case 2: case 3: case 11: {
                auto& current = bound_owner(b, b.widget.layout.main_listbox_1b8);
                auto* selected = listbox(b, current).selected_row_00425e50();
                b.services.call_00598b60(selected, current);
                break;
            }
            case 4: case 5: case 6: case 7: mission_accept(b); break;
            case 8: b.services.call_005922f0(); break;
            case 9: b.services.call_00594bf0(); break;
            case 12: b.services.call_005885d0(); break;
            default: break;
            }
            return;
        }
        if (text_code(widget) == 0xa3) {
            switch (b.widget.page_00e08874) {
            case 2: case 3: case 11: b.services.call_00584ae0(false); break;
            case 4: case 5: case 6: case 7: case 8: {
                auto& backdrop = bound_owner(b, b.widget.layout.backdrop_328);
                set_resolved_position(backdrop.layout().transform, b.widget.layout.backdrop_position_124,
                    b.services.transform_host(backdrop));
                b.services.call_00584f50();
                break;
            }
            case 9: b.services.call_00599340(); break;
            case 12: b.services.call_0058c010(); break;
            default: break;
            }
            return;
        }
        if (text_code(widget) == 0xb4) {
            if (static_cast<std::uint32_t>(b.widget.page_00e08874) - 4u <= 4u) {
                copy_scalar_word(b.widget.zoom_delta_19c, b.zoom_out_00ce69cc);
                b.widget.screen.zoom_axis_latched = true;
            }
            return;
        }
        if (text_code(widget) == 0xb6) {
            if (static_cast<std::uint32_t>(b.widget.page_00e08874) - 4u <= 4u) {
                copy_scalar_word(b.widget.zoom_delta_19c, b.zoom_in_00ce54a0);
                b.widget.screen.zoom_axis_latched = true;
            }
            return;
        }
        if (text_code(widget) == 0xa7) {
            if (b.widget.page_00e08874 == 12) {
                const auto& mission = b.services.selected_mission_005806a0();
                b.services.tactical_selection_fields(mission, 2, false);
                b.services.call_005886c0();
            }
            return;
        }
        if (b.widget.page_00e08874 != 12) return;
        if (!contains(b.widgets_280, widget.layout()) &&
            !contains(b.widgets_298, widget.layout()) &&
            !contains(b.widgets_2a4, widget.layout())) return;
        // Capture clickedD8 BEFORE A9C920, then reload clickedD8 for A9C7C0.
        const auto clicked = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(widget.extra_fields().pointer_d8));
        if (listbox(b).selected_index_00a9c920() != signed_word(clicked)) {
            const auto current = static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(widget.extra_fields().pointer_d8));
            listbox(b).select_index_00a9c7c0(signed_word(current));
        }
        return;
    }
    const auto page = b.widget.page_00e08874;
    if (page < 4) return;
    auto& layout = b.widget.layout;
    auto* current = &widget.layout();
    if (page <= 8) {
        const auto selected = listbox(b).selected_data_d8_00a9c990();
        if (current == layout.arrow_top_1bc) {
            if (!b.widget.arrow_top_enabled_1c5) return;
            if (selected != 0) {
                listbox(b).select_index_00a9c7c0(signed_word(selected - 1u));
                listbox(b).refresh80_00a9c220(true); // fresh1B8 AFTER callback
            }
        } else if (current == layout.arrow_bottom_1c0) {
            if (!b.widget.arrow_bottom_enabled_1c4) return;
            auto& captured = listbox(b);
            if (signed_word(selected) < signed_word(captured.row_count_104() - 1u)) {
                const auto next = signed_word(selected + 1u);
                auto* row = captured.row_at_00a9be00(next);
                require(row != nullptr, "main-menu command next row is outside the native list");
                if (!row->scene_flags().hidden) {
                    listbox(b).select_index_00a9c7c0(next);
                    listbox(b).refresh80_00a9c220(true);
                }
            }
        } else if (current == layout.scroll_bottom_1cc) {
            begin_scroll(b, layout.mission_scroller_1d4, ScrollDirection::kTowardEnd);
        } else if (current == layout.scroll_top_1c8) {
            begin_scroll(b, layout.mission_scroller_1d4, ScrollDirection::kTowardStart);
        } else if (current == layout.scroll_thumb_234) {
            begin_thumb_drag(layout.mission_scroller_1d4, b.scroller_host);
        }
    } else {
        if (page != 9) return;
        if (current == layout.briefing_scroll_bottom_34c) {
            begin_scroll(b, layout.briefing_scroller_354, ScrollDirection::kTowardEnd);
        } else if (current == layout.briefing_scroll_top_348) {
            begin_scroll(b, layout.briefing_scroller_354, ScrollDirection::kTowardStart);
        } else if (current == layout.briefing_scroll_thumb_3b4) {
            begin_thumb_drag(layout.briefing_scroller_354, b.scroller_host);
        }
    }
    pressed_state(widget);
}
} // namespace bsp
