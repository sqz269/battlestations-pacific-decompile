#include "bsp/frontend_prompts.hpp"

#include <limits>
#include <utility>

namespace bsp {
namespace {
constexpr std::array<PromptWidget, 4> buttons{
    PromptWidget::Yes, PromptWidget::No, PromptWidget::Accept, PromptWidget::Restart};
constexpr std::array<int, 4> navigation_order{0, 2, 3, 1};
constexpr const char* label_keys[2][7] = {
    {"globals.yes", "globals.no", "globals.accept", "globals.restartmission",
     "globals.exittomenu", "globals.exitgame", "globals.back"},
    {"globals.dialog_yes", "globals.dialog_no", "globals.dialog_accept",
     "globals.dialog_restartmission", "globals.dialog_exittomenu",
     "globals.dialog_exitgame", "globals.dialog_back"},
};
void sync_gate_kinds(FrontEndPromptScreen& screen) {
    screen.menu.pending_command_a = static_cast<int>(screen.records[4].kind);
    screen.menu.pending_command_b = static_cast<int>(screen.records[6].kind);
}
void copy_record(FrontEndPromptRecord& to, const FrontEndPromptRecord& from,
                 NativeStringStorage& storage, int slot) {
    to.slot = slot;
    to.message.copy_from_00be0a30_fragment(storage, from.message);
    to.kind = from.kind;
    to.callback = from.callback;
    to.title.copy_from_00be0a30_fragment(storage, from.title);
    to.timed = from.timed;
    to.started = from.started;
    to.timeout = from.timeout;
    to.timeout_result = from.timeout_result;
    to.countdown_key.copy_from_00be0a30_fragment(storage, from.countdown_key);
    to.dismissible = from.dismissible;
}
struct OwnedArgument {
    NativeString& value;
    NativeStringStorage& storage;
    ~OwnedArgument() { value.release_to(storage); }
};
// CVTTSS2SI returns the integer-indefinite pattern for NaN/out-of-range.
int truncate_native_float(float value) {
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<int>(value);
}
}

void construct_prompt_screen_00533120(FrontEndPromptScreen& screen, NativeStringStorage& storage) {
    screen.menu.base.wanted = false;
    screen.menu.base.active = false;
    screen.background_visible = true;
    for (int table = 0; table != 2; ++table) {
        for (int key = 0; key != 7; ++key) {
            NativeString temporary;
            temporary.assign_0041e870(storage, label_keys[table][key]);
            OwnedArgument cleanup{temporary, storage};
            screen.labels[table][key].copy_from_00be0a30_fragment(storage, temporary);
        }
    }
    screen.requested_focus = -1;
    screen.field_288 = false;
    // Records/list are fresh C++ members: record fields mirror 00530f40,
    // while std::list owns the native STL node/iterator allocation boundary.
}

void destroy_prompt_record_00530f90(FrontEndPromptRecord& record, NativeStringStorage& storage) noexcept {
    destroy_native_string_header_0041dd20(&record.countdown_key, storage);
    destroy_native_string_header_0041dd20(&record.title, storage);
    destroy_native_string_header_0041dd20(&record.message, storage);
}

void release_prompt_screen_strings(FrontEndPromptScreen& screen, NativeStringStorage& storage) noexcept {
    for (auto& record : screen.records) {
        record.countdown_key.release_to(storage);
        record.title.release_to(storage);
        record.message.release_to(storage);
    }
    for (auto& record : screen.pending) destroy_prompt_record_00530f90(record, storage);
    screen.pending.clear();
    for (auto& table : screen.labels) for (auto& text : table) text.release_to(storage);
}

void raise_prompt_00531b00(FrontEndPromptScreen& screen, FrontEndPromptHost& host,
    int slot, const NativeString& message, PromptKind kind, std::uint32_t callback,
    std::uint32_t unused_flag, const NativeString& title, float timeout,
    int timeout_result, NativeString countdown_key, bool dismissible) {
    (void)unused_flag; // stack dword +14h from entry ESP is never read
    auto& storage = host.strings();
    OwnedArgument cleanup{countdown_key, storage};
    dismiss_all_prompts_00530650(screen, host);
    auto& record = screen.records[slot];
    record.slot = slot;
    record.message.copy_from_00be0a30_fragment(storage, message);
    record.kind = kind;
    record.callback = callback;
    sync_gate_kinds(screen);
    record.title.copy_from_00be0a30_fragment(storage, title);
    record.timeout = timeout;
    record.timed = timeout != 0.0f; // unordered NaN enables timing too
    record.timeout_result = timeout_result;
    record.countdown_key.copy_from_00be0a30_fragment(storage, countdown_key);
    record.dismissible = dismissible;
    refresh_prompt_screen_00532360(screen, host);
}

void dismiss_prompt_00532a20(FrontEndPromptScreen& screen, FrontEndPromptHost& host, int slot) {
    auto& record = screen.records[slot];
    if (record.kind == PromptKind::Empty) return;
    auto callback = record.callback;
    if (record.kind == PromptKind::Busy || !record.dismissible) callback = 0;
    if (!record.dismissible) {
        // Native inserts at sentinel.prev (back), then copies into that node.
        screen.pending.emplace_back();
        copy_record(screen.pending.back(), record, host.strings(), slot);
    }
    record.kind = PromptKind::Empty;
    record.callback = 0;
    record.timed = false;
    record.timeout = 0.0f;
    record.dismissible = true;
    record.slot = 7;
    sync_gate_kinds(screen);
    if (callback) host.invoke_callback(callback, 2);
    refresh_prompt_screen_00532360(screen, host);
}

void dismiss_all_prompts_00530650(FrontEndPromptScreen& screen, FrontEndPromptHost& host) {
    for (int slot = 0; slot != 7; ++slot) dismiss_prompt_00532a20(screen, host, slot);
}

void refresh_prompt_screen_00532360(FrontEndPromptScreen& screen, FrontEndPromptHost& host) {
    if (screen.menu.base.active) {
        host.navigation_owner(false);
        host.set_widget_flag(PromptWidget::Navigation, 0x60, false);
    }
    if (!screen.menu.modal_dialog_active) screen.saved_cinematic = host.cinematic_mode();
    screen.current = 6;
    while (screen.current >= 0 && screen.records[screen.current].kind == PromptKind::Empty)
        --screen.current;
    screen.menu.modal_dialog_active = false;
    for (int slot = 0; slot <= screen.current; ++slot) {
        const auto kind = screen.records[slot].kind;
        screen.menu.modal_dialog_active |= kind == PromptKind::YesNo ||
            kind == PromptKind::Accept || kind == PromptKind::YesNoRestart;
    }
    if (host.local_player_count() == 0 && host.mission_present()) {
        if (screen.menu.modal_dialog_active) {
            const auto saved = screen.saved_cinematic;
            screen.menu.modal_dialog_active = false;
            host.set_cinematic_mode(true, false, true);
            screen.menu.modal_dialog_active = true;
            screen.saved_cinematic = saved;
        } else if (screen.saved_cinematic != host.cinematic_mode()) {
            host.set_cinematic_mode(screen.saved_cinematic, false, true);
        }
    }
    if (screen.current == -1) {
        if (!screen.pending.empty()) {
            auto& oldest = screen.pending.front();
            NativeString countdown;
            countdown.copy_from_00be0a30_fragment(host.strings(), oldest.countdown_key);
            raise_prompt_00531b00(screen, host, oldest.slot, oldest.message, oldest.kind,
                oldest.callback, 0, oldest.title, oldest.timeout, oldest.timeout_result,
                std::move(countdown), oldest.dismissible);
            // Native deliberately re-reads the current first node after raise.
            destroy_prompt_record_00530f90(screen.pending.front(), host.strings());
            screen.pending.pop_front();
        } else {
            host.set_input_mapping(0x1c, 0);
            close_menu_command_screen(screen.menu, host.screens());
        }
        return;
    }
    host.set_input_mapping(0x1c, 5);
    screen.menu.base.wanted = true;
    screen.menu.base.active = true;
    host.screens().menu_command_commit();
    host.enter_screen();
    auto& record = screen.records[screen.current];
    record.started = host.sample_clock();
    host.set_localized_text(PromptWidget::Title, record.title, true);
    host.set_localized_text(PromptWidget::Message, record.message, true);
    if (!host.mission_present()) host.set_widget_flag(PromptWidget::Group, 0x84, false);
    switch (record.kind) {
    case PromptKind::YesNo: screen.buttons = {true, true, false, false}; break;
    case PromptKind::Accept: screen.buttons = {false, false, true, false}; break;
    case PromptKind::Busy: screen.buttons = {false, false, false, false}; break;
    case PromptKind::YesNoRestart: screen.buttons = {true, true, false, true}; break;
    default: break; // Unknown kinds retain all four native button bytes.
    }
    for (auto widget : buttons) {
        host.clear_glyphs(widget);
        host.set_literal_text(widget, "", true);
    }
    // Read input_mode again for each lookup, as the native call sites do.
    host.set_localized_text(PromptWidget::Yes,
        screen.labels[screen.input_mode][screen.current == 6 ? 5 : 0], true);
    host.set_localized_text(PromptWidget::No,
        screen.labels[screen.input_mode][screen.current == 6 ? 6 :
            (record.kind == PromptKind::YesNoRestart ? 4 : 1)], true);
    host.set_localized_text(PromptWidget::Accept, screen.labels[screen.input_mode][2], true);
    host.set_localized_text(PromptWidget::Restart, screen.labels[screen.input_mode][3], true);
    for (int index = 0; index != 4; ++index)
        host.set_widget_flag(buttons[index], 0x34, screen.buttons[index]);
    if (screen.input_mode == 0) {
        host.navigation_owner(false);
        host.clear_navigation();
        for (auto index : navigation_order) if (screen.buttons[index]) {
            host.prepare_button(buttons[index]);
            host.add_navigation_button(buttons[index], 0, 0);
        }
        host.erase_navigation_scratch();
        if (screen.requested_focus == -1) host.set_navigation_focus(host.navigation_count() - 1);
        else {
            host.set_navigation_focus(screen.requested_focus);
            screen.requested_focus = -1;
        }
        host.set_navigation_callback(0x006964b0);
        host.navigation_owner(true);
        host.set_widget_flag(PromptWidget::Navigation, 0x60, true);
    }
    host.layout_buttons_00530a60();
}

void complete_prompt_00532c50(FrontEndPromptScreen& screen, FrontEndPromptHost& host, int result) {
    auto& record = screen.records[screen.current]; // Native requires an occupied current slot.
    const auto callback = record.callback;
    record.callback = 0;
    record.dismissible = true;
    dismiss_prompt_00532a20(screen, host, screen.current);
    if (callback) host.invoke_callback(callback, result);
    host.input_update(0.0f);
    host.set_consumed_button(host.any_dynamic_device_button());
}

void prompt_widget_event_00532cb0(FrontEndPromptScreen& screen, FrontEndPromptHost& host,
                                  std::uintptr_t event) {
    if (host.event_type(event) != 3) return;
    host.event_strength(event, 1.0f);
    if (host.event_code(event) == 0xa7) {
        if (screen.buttons[3]) complete_prompt_00532c50(screen, host, 3);
    } else if (host.event_code(event) == 0xa2) {
        if (screen.buttons[2]) complete_prompt_00532c50(screen, host, 2);
        else if (screen.buttons[0]) complete_prompt_00532c50(screen, host, 1);
    } else if (host.event_code(event) == 0xa3) complete_prompt_00532c50(screen, host, 0);
}

void update_prompt_screen_00532dc0(FrontEndPromptScreen& screen, FrontEndPromptHost& host,
                                  float seconds) {
    const bool wanted_background = screen.background_visible;
    if (wanted_background != host.widget_visible(PromptWidget::Background))
        host.set_widget_flag(PromptWidget::Background, 0x34, wanted_background);
    auto& record = screen.records[screen.current];
    if (record.timed) {
        ClockTimestamp elapsed;
        const auto now = host.sample_clock();
        subtract_timestamp_00530890(elapsed, now, record.started);
        const float elapsed_seconds = timestamp_seconds_x87(elapsed);
        if (elapsed_seconds > record.timeout) { // strict; equality/NaN do not expire
            complete_prompt_00532c50(screen, host, record.timeout_result);
            return;
        }
        const auto scope = host.begin_localization_scope();
        const float remaining = record.timeout - elapsed_seconds;
        host.set_localization_integer(scope, record.countdown_key,
            truncate_native_float(host.countdown_round_00bf85b0(remaining)));
        host.end_localization_scope(scope);
        host.set_literal_text(PromptWidget::Message, "", true);
        host.set_localized_text(PromptWidget::Message, record.message, true);
    }
    if (screen.buttons[1] && (host.input_action(0xf7) || host.input_action(0x4b)))
        complete_prompt_00532c50(screen, host, 0);
    else if (screen.buttons[2] && (host.input_action(0xf8) || host.input_action(0x4a) || host.input_action(0x4b)))
        complete_prompt_00532c50(screen, host, 2);
    else if (screen.buttons[3] && host.input_action(0xf5))
        complete_prompt_00532c50(screen, host, 3);
    else if (screen.input_mode == 1 && screen.buttons[0] &&
        (host.input_action(0xf6) || host.input_action(0x4a)))
        complete_prompt_00532c50(screen, host, 1);
    // Completion can re-enter the screen: each visibility test below is fresh.
    for (int index = 0; index != 4; ++index)
        if (screen.buttons[index]) host.update_widget(buttons[index], seconds);
    if (host.game_state() == 2 && host.title_primary_device_button_zero()) {
        for (int index = 0; index != 4; ++index)
            if (screen.buttons[index] && host.widget_pressed_field(buttons[index]) != 0)
                host.dispatch_widget(buttons[index]);
    }
    host.finish_input_dispatch(2);
}
}
