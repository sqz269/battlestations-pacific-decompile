#include "bsp/frontend_prompt_layout.hpp"

#include <cstring>

namespace bsp {
namespace {
constexpr PromptWidget order[]{PromptWidget::Yes, PromptWidget::Accept,
    PromptWidget::Restart, PromptWidget::No};
bool enabled(const FrontEndPromptScreen& screen, PromptWidget button) {
    switch (button) {
    case PromptWidget::Yes: return screen.buttons[0];
    case PromptWidget::No: return screen.buttons[1];
    case PromptWidget::Accept: return screen.buttons[2];
    case PromptWidget::Restart: return screen.buttons[3];
    default: return false; // internal callers pass only the four above
    }
}
float normalized_width(FrontEndPromptLayoutHost& host, PromptWidget button) {
    // Native FDIV double(960), then FSTP float before the later additions.
    return static_cast<float>(static_cast<double>(host.measured_button_width_114(button)) / 960.0);
}
template<class T> T read_field(const void* object, std::size_t offset) {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(object) + offset, sizeof result);
    return result;
}
template<class T> void write_field(void* object, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
struct TemporaryString {
    NativeString value;
    NativeStringStorage& storage;
    TemporaryString(NativeStringStorage& allocator, const char* text) : storage(allocator) {
        value.assign_0041e870(storage, text);
    }
    ~TemporaryString() { value.release_to(storage); }
};
}

void layout_prompt_buttons_00530a60(const FrontEndPromptScreen& screen,
    FrontEndPromptLayoutHost& host) {
    float total = 0.0f;
    int spaces = 1;
    for (const auto button : order) {
        if (enabled(screen, button)) {
            const float width = normalized_width(host, button);
            total = static_cast<float>(static_cast<double>(width) + total);
            ++spaces;
        }
    }
    const float available = host.group_width_00aa6740();
    const float gap = static_cast<float>((static_cast<double>(available) - total) / spaces);
    float x = gap;
    for (const auto button : order) {
        if (!enabled(screen, button)) continue;
        host.set_button_local_x_00aa78d0(button, x);
        if (button != PromptWidget::No) {
            // Width is read AFTER SetLocalX, even when no later button is on.
            const float width = normalized_width(host, button);
            x = static_cast<float>((static_cast<double>(width) + gap) + x);
        }
    }
}

void* FrontEndPromptWidgets::button(PromptWidget which) const noexcept {
    switch (which) {
    case PromptWidget::Yes: return yes_28;
    case PromptWidget::No: return no_2c;
    case PromptWidget::Accept: return accept_30;
    case PromptWidget::Restart: return restart_34;
    default: return nullptr; // caller precondition: a button enumerator
    }
}
float ActualFrontEndPromptLayoutHost::measured_button_width_114(PromptWidget button) {
    return read_field<float>(widgets_.button(button), 0x114);
}
float ActualFrontEndPromptLayoutHost::group_width_00aa6740() {
    return calls_.widget_width_00aa6740(widgets_.group_38);
}
void ActualFrontEndPromptLayoutHost::set_button_local_x_00aa78d0(PromptWidget button, float x) {
    calls_.widget_set_local_x_00aa78d0(widgets_.button(button), x);
}

void prepare_prompt_button_00532110(std::vector<NativeString>& scratch,
    PromptWidget button, FrontEndPromptButtonHost& host) {
    if (scratch.empty()) return;
    host.set_localized_button_00abaed0(button, scratch.back(), true);
    if (!scratch.empty()) {
        destroy_native_string_header_0041dd20(&scratch.back(), host.strings());
        scratch.pop_back();
    }
}

void enter_prompt_screen_00531380(FrontEndPromptScreen& screen,
    FrontEndPromptWidgets& widgets, FrontEndPromptEnterHost& host) {
    if (screen.background_visible) host.widget_flag(widgets.background_18, 0x34, true);
    auto& storage = host.strings();
    auto find = [&](const char* name, void*& destination) {
        TemporaryString key(storage, name);
        destination = host.find_child_00aa7e00(widgets.root_14, key.value, true);
    };
    find("Gamertag_Text", widgets.title_1c);
    find("Message_Text", widgets.message_20);
    find("Options_Listbox", widgets.navigation_24);
    find("Yes_Text", widgets.yes_28);
    find("No_Text", widgets.no_2c);
    find("Accept_Text", widgets.accept_30);
    find("Checkpoint_Text", widgets.restart_34);
    {
        TemporaryString key(storage, "background_Icon");
        void* icon = host.find_child_00aa7e00(widgets.root_14, key.value, true);
        host.widget_flag(icon, 0x34, screen.field_288);
    }
    screen.field_288 = false;
    host.widget_flag(widgets.navigation_24, 0x34, true);
    host.widget_flag(widgets.group_38, 0x84, host.mission_present_00e198c4());
    if (auto* previous = host.previous_screen_00e1930c()) {
        if (previous->active) host.previous_screen_exit_v1c(*previous);
        previous->wanted = false;
        previous->active = false;
        host.previous_screen_commit_004f83b0(*previous);
    }
    if (auto* overlay = host.overlay_screen_00e198b4_54()) {
        if (overlay->active) host.overlay_input_v0c(*overlay);
    }
    screen.input_mode = 1;
    if (!host.alternate_input_00f88a30()) screen.input_mode = 0;
    else host.gui_reset_screens_00aa0f70();

    // 00531030(-59h),(-5Dh),(-5Eh), concatenation then004C5E60 produces
    // A2 A3 A7 as zero-extended UTF-16 code units, not a code-page conversion.
    constexpr std::u16string_view callback_text = u"\u00a2\u00a3\u00a7";
    constexpr PromptWidget entry_order[]{PromptWidget::Yes, PromptWidget::No,
        PromptWidget::Accept, PromptWidget::Restart};
    for (const auto button : entry_order) {
        host.button_callback_00531130(widgets.button(button), callback_text,
            widgets.listener_10, widgets.button(button));
        // 00AA6BC0(0,0) is exactly these two stores. These and the following
        // reads deliberately re-fetch the widget binding after the callback.
        void* actual = widgets.button(button);
        write_field(actual, 0xdc, std::uint32_t{0});
        write_field(actual, 0x79, std::uint8_t{0});
        actual = widgets.button(button);
        write_field(actual, 0x1b4, std::uint8_t{1});
        write_field(actual, 0x1b8, 0.0f);
        write_field(actual, 0x1bc, std::uint32_t{0x3c360b61});
        const float white[]{1.0f, 1.0f, 1.0f, 1.0f};
        host.widget_color_v50(widgets.button(button), white);
    }
}
}
