#include "bsp/input_action_classifier.hpp"
#include "bsp/input_tick.hpp"

#include <cstddef>

namespace bsp {

void update_input_action_listener_00a91a50(InputActionListener& listener,
    float seconds, bool down_previous, bool down_current,
    const InputActionTimingThresholds& thresholds) noexcept {
    listener.pressed = false;
    listener.press_aux = false;
    listener.fast_release = false;
    listener.release_aux = false;
    listener.hold_fired = false;
    listener.held = false;
    listener.hold_press = false;

    // Each sum is explicitly spilled to binary32 before native comparisons
    // (00a91a6e/84, 00a91ac8/da), despite the surrounding x87 stack juggling.
    listener.since_press = listener.since_press + seconds;
    listener.since_release = listener.since_release + seconds;
    const float since_last_release = listener.since_press; // native +14h
    const float since_last_press = listener.since_release; // native +18h
    if (since_last_release > thresholds.quick_edge_and_hold ||
        since_last_press > thresholds.quick_edge_and_hold) {
        listener.state_a = false;
        listener.state_b = false;
    }

    if (down_current) {
        listener.held_time = listener.held_time + seconds;
        listener.held_time_biased = listener.held_time_biased + seconds;
        if (!down_previous) {
            listener.state_c = true;
            listener.press_aux = listener.state_a;
            if (since_last_release < thresholds.quick_edge_and_hold) {
                listener.pressed = true;
                listener.state_a = true;
            }
            listener.since_release = 0.0f;
        } else if (listener.held_time_biased > thresholds.repeat_delay) {
            listener.hold_fired = true;
            listener.held_time_biased =
                listener.held_time_biased - thresholds.repeat_step;
        }
        // JC at 00a91b40 rejects both less-than and unordered; equality fires.
        if (listener.held_time >= thresholds.quick_edge_and_hold) {
            listener.held = true;
            if (listener.state_c) {
                listener.hold_press = true;
                listener.state_c = false;
            }
        }
    } else {
        listener.held_time = 0.0f;
        listener.held_time_biased = 0.0f;
        listener.state_c = false;
        if (down_previous) {
            listener.release_aux = listener.state_b;
            if (since_last_press < thresholds.quick_edge_and_hold) {
                listener.fast_release = true;
                listener.state_b = true;
            }
            listener.since_press = 0.0f;
        }
    }

    // 00a91b94..b97 / 00a91ba7..baa use JNZ: auxiliary flags EXCLUDE these.
    listener.press_confirmed = listener.pressed && !listener.press_aux;
    listener.release_confirmed = listener.fast_release && !listener.release_aux;
}

namespace {

InputDevice* resolve_binding_device(const InputBindingDeviceGroups& groups,
    std::int32_t device_class, std::uint32_t device_index) noexcept {
    const auto& devices = groups[static_cast<std::size_t>(device_class)];
    return device_index < devices.size() ? devices[device_index] : nullptr;
}

bool resolve_modifier_devices(std::vector<InputActionModifierBinding>& modifiers,
    const InputBindingDeviceGroups& groups) noexcept {
    for (auto& modifier : modifiers) {
        modifier.cached_device = resolve_binding_device(
            groups, modifier.device_class, modifier.device_index);
        if (modifier.cached_device == nullptr) return false;
    }
    return true;
}

} // namespace

void rebind_input_action_00a91e80(std::vector<InputActionBinding>& bindings,
    const InputBindingDeviceGroups& groups) noexcept {
    for (auto& binding : bindings) {
        binding.resolved = false;                       // 00a91ead
        if (binding.device_class == -1) continue;      // 00a91eaa..eb0
        binding.cached_device = resolve_binding_device(
            groups, binding.device_class, binding.device_index);
        if (binding.cached_device == nullptr) continue; // 00a91efb..f00
        if (!resolve_modifier_devices(binding.required_modifiers, groups)) continue;
        if (!resolve_modifier_devices(binding.forbidden_modifiers, groups)) continue;
        binding.resolved = true;                       // 00a92026
    }
}

} // namespace bsp
