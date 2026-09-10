#include "bsp/input_tick.hpp"

#include <cstring>

namespace bsp {
namespace {
// 00d7a218 is eight zero bytes, so every `comiss xmm0, [00d7a218]` in this
// packet is a comparison against 0.0f.
constexpr float kZero = 0.0f;

// The CRT __stricmp at 00bf7fbf, restricted to the equality test 00a926f0 makes.
// Written out rather than called so the reconstruction keeps the same ASCII-only
// folding src/input_settings.cpp already uses for BSP_NativeString comparisons.
bool equal_case_insensitive(const char* left, const char* right) noexcept {
    while (*left != '\0' && *right != '\0') {
        unsigned int a = static_cast<unsigned char>(*left++);
        unsigned int b = static_cast<unsigned char>(*right++);
        if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
        if (b >= 'A' && b <= 'Z') b += 'a' - 'A';
        if (a != b) return false;
    }
    return *left == *right;
}

// The listener state 00a91e20 clears at 00a91e46..00a91e79.
void clear_listener_state(InputActionListener& listener) noexcept {
    listener.pressed = false;
    listener.press_confirmed = false;
    listener.press_aux = false;
    listener.fast_release = false;
    listener.release_confirmed = false;
    listener.release_aux = false;
    listener.hold_fired = false;
    listener.held = false;
    listener.hold_press = false;
    listener.state_a = false;
    listener.state_b = false;
    listener.state_c = false;
    listener.since_press = kZero;
    listener.since_release = kZero;
    listener.held_time = kZero;
    listener.held_time_biased = kZero;
}

InputActionListener* listener_of(InputTickState& state, InputActionRecord& record) noexcept {
    if (!record.has_listener || record.listener >= state.listeners.size()) return nullptr;
    return &state.listeners[record.listener];
}
}  // namespace

void begin_action_frame_00a92370(InputActionRecord& record) noexcept {
    record.previous_hold = record.current_hold;  // 00a9237a/00a92381
    record.previous_down = record.current_down;  // 00a9237d/00a92384
    record.current_hold = kZero;                 // 00a92387
    record.current_down = false;                 // 00a9238c
}

bool action_pressed_this_frame_004c43c0(const InputActionRecord& record) noexcept {
    if (!record.current_down) return false;
    if (!(record.current_hold > kZero)) return false;
    return !record.previous_down || !(record.previous_hold > kZero);
}

bool action_down_previous(const InputActionRecord& record) noexcept {
    return record.previous_down && record.previous_hold > kZero;  // 00a92cc3..00a92cdc
}

bool action_down_current(const InputActionRecord& record) noexcept {
    return record.current_down && record.current_hold > kZero;    // 00a92ca6..00a92cba
}

void reset_action_00a91e20(InputActionRecord& record, InputActionListener* listener) noexcept {
    record.previous_hold = kZero;
    record.previous_down = false;
    record.current_hold = kZero;
    record.current_down = false;
    if (listener != nullptr) clear_listener_state(*listener);
}

void continue_action_00a919f0(InputActionRecord& record, float amount) noexcept {
    record.previous_down = true;    // 00a919f8
    record.current_down = true;     // 00a919fb
    record.previous_hold = amount;  // 00a919fe
    record.current_hold = amount;   // 00a91a03
}

void apply_effect_param_00a926f0(const InputEffectParam& param, InputActionListener& listener) noexcept {
    // 00a926f6: when the name pointer is non-null and matches "fastRelease"
    // case-insensitively the listener flag at +0Bh is raised and nothing else runs.
    if (param.name != nullptr && equal_case_insensitive(param.name, "fastRelease")) {
        listener.fast_release = true;
        return;
    }
    // 00a9271f: otherwise the block is asked about "holdPress" through the method
    // at 00425850. That method is outside this packet, so the reconstruction can
    // only decide it from the name the block carries.
    if (param.name != nullptr && equal_case_insensitive(param.name, "holdPress")) {
        listener.hold_press = true;
    }
}

void start_action_00a92aa0(InputActionRecord& record, float amount,
    const InputEffectParam& param, InputActionListener* listener) noexcept {
    record.previous_hold = kZero;   // 00a92aa7
    record.previous_down = false;   // 00a92ab2
    record.current_down = true;     // 00a92ab6
    record.current_hold = amount;   // 00a92aba
    if (param.object != nullptr && listener != nullptr) {  // 00a92abf/00a92ac4
        apply_effect_param_00a926f0(param, *listener);
    }
}

bool timed_entry_expired(const TimedInputEntry& entry, float expiry_reference) noexcept {
    return !(expiry_reference < entry.expiry);  // 004e4fc8/004e4fcc
}

void update_input_manager_00a92c40(InputTickState& state, float seconds, InputTickHost& host) {
    host.backend_update(seconds);                       // 00a92c57
    if (host.take_backend_bindings_dirty()) {           // 00a92c5e..00a92c6d
        host.rebind_all_actions(state);                 // 00a92c71
    }
    // The loop reloads base and count from the singleton on every step
    // (00a92cef..00a92cfe), so a rebind that resizes the table during the walk is
    // observed by the next comparison. The reconstruction re-reads size() for the
    // same reason.
    for (std::size_t i = 0; i < state.records.size(); ++i) {
        InputActionRecord& record = state.records[i];
        if (!record.enabled) continue;                  // 00a92c88
        begin_action_frame_00a92370(record);
        host.poll_action_bindings(record, i);           // rest of 00a92370
        InputActionListener* listener = listener_of(state, record);
        if (listener == nullptr) continue;              // 00a92c95..00a92ca4
        host.listener_classify(*listener, seconds,
            action_down_previous(record), action_down_current(record));  // 00a92cea
    }
    host.post_update_hook();                            // 00a92d02..00a92d0d
}

int run_input_effect_lists_004e4e6c(InputTickState& state, float expiry_reference) {
    // Suppression set at game+5B0h. Each node holds one action index at node+0Ch
    // and the walk resets that record every frame the entry is present.
    for (int action : state.suppressed_actions) {
        if (action < 0 || static_cast<std::size_t>(action) >= state.records.size()) continue;
        InputActionRecord& record = state.records[static_cast<std::size_t>(action)];
        reset_action_00a91e20(record, listener_of(state, record));  // 004e4ec2
    }
    // 004e4eda: when the timed set is not empty, index 1 is reset as well. The
    // literal 30h at 004e4eeb makes this a fixed index, not a set member.
    if (!state.timed_actions.empty()
        && static_cast<std::size_t>(kTimedListGuardAction) < state.records.size()) {
        InputActionRecord& record = state.records[static_cast<std::size_t>(kTimedListGuardAction)];
        reset_action_00a91e20(record, listener_of(state, record));   // 004e4eee
    }
    // Timed set at game+5BCh. The native loop advances the iterator at 004e4f4b
    // before it touches the node, so the erase at 004e4fdf cannot invalidate the
    // iterator it holds; the index walk below reproduces that ordering.
    int erased = 0;
    std::size_t i = 0;
    while (i < state.timed_actions.size()) {
        TimedInputEntry& entry = state.timed_actions[i];
        const bool valid_index = entry.action_index >= 0
            && static_cast<std::size_t>(entry.action_index) < state.records.size();
        if (valid_index) {
            InputActionRecord& record = state.records[static_cast<std::size_t>(entry.action_index)];
            InputActionListener* listener = listener_of(state, record);
            if (!entry.started) {
                start_action_00a92aa0(record, entry.amount, entry.param, listener);  // 004e4f98
                entry.started = true;                                                // 004e4f9d
            } else {
                continue_action_00a919f0(record, entry.amount);                      // 004e4fba
            }
        }
        if (timed_entry_expired(entry, expiry_reference)) {
            state.timed_actions.erase(state.timed_actions.begin()
                + static_cast<std::ptrdiff_t>(i));                                   // 004d11d0
            ++erased;
            continue;
        }
        ++i;
    }
    return erased;
}

InputTickResult run_game_input_tick(InputTickState& state, float raw_delta,
    float expiry_reference, bool run_effect_lists, InputTickHost& host) {
    InputTickResult result{};
    update_input_manager_00a92c40(state, raw_delta, host);  // OnMove 004e4a63..004e4a77
    if (run_effect_lists) {
        result.entries_erased = run_input_effect_lists_004e4e6c(state, expiry_reference);
        result.effect_lists_ran = true;
    }
    return result;
}

}  // namespace bsp
