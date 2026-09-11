#include "bsp/input_focus_reset.hpp"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <algorithm>
#include <stdexcept>

namespace bsp {
namespace {
std::size_t checked_class(const InputFocusBackendState& state, std::int32_t value) {
    if (value < 0 || value >= input_device_class_count ||
        static_cast<std::size_t>(value) >= state.groups.size())
        throw std::out_of_range("input class outside the canonical device groups");
    return static_cast<std::size_t>(value);
}

InputFocusBackendState& current_backend(InputFocusResetHost& host) {
    auto* backend = host.current_backend_00f8bbf4();
    if (!backend) throw std::logic_error("native focus reset requires a live input backend");
    return *backend;
}

InputDevice* first_mouse(InputFocusBackendState& backend) {
    const auto& devices = backend.groups[checked_class(backend, 1)];
    return devices.empty() ? nullptr : devices.front();
}

InputFocusDeviceHost& other_host(InputFocusDeviceHost* host) {
    if (!host) throw std::invalid_argument("input operation requires its actual device host");
    return *host;
}

float poll_delta(float value) noexcept {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
} // namespace

HWND get_platform_window_00bec230(const Win32PlatformState& platform) noexcept {
    return platform.window;
}

void set_mouse_cooperative_level_00a9a140(MouseInputDevice& mouse, HWND window,
    std::uint32_t flags) {
    if (!mouse.direct_input)
        throw std::invalid_argument("mouse cooperative level requires an actual DirectInput device");
    mouse.direct_input->SetCooperativeLevel(window, flags);
    mouse.cooperative_configured = true;
}

void delete_mouse_input_device_00a9a390(MouseInputDevice& mouse, std::uint32_t flags) {
    // Native only changes D5B638 -> CEB130 and conditionally frees storage.
    // Its borrowed DirectInput pointer is not released by this destructor.
    if (flags & 1u) delete &mouse;
    else mouse.~MouseInputDevice();
}

std::int32_t input_device_identifier_zero_00a93eb0() noexcept { return 0; }

void remove_active_input_device_00a90ee0(InputFocusBackendState& backend,
    InputDevice& device) {
    const auto device_class = device.device_class();
    auto& group = backend.groups[checked_class(backend, device_class)];
    const auto found = std::find(group.begin(), group.end(), &device);
    if (found == group.end()) return;
    backend.bindings_dirty_d4 = true;
    group.erase(found); // native memmove_s closes the gap, preserving order
    if (auto callback = backend.devices_changed_d8) callback(device_class, -1);
}

void delete_input_device_class_00bebf30(InputFocusBackendState& backend,
    std::int32_t device_class, InputFocusDeviceHost& devices) {
    checked_class(backend, device_class);
    for (int index = 0; index != input_device_slot_count; ++index) {
        auto& slot = backend.slots.slot_reference(device_class, index);
        if (auto* device = slot) {
            devices.delete_device_vslot04(*device, 1);
            slot = nullptr; // clear after callback, even if it changed the slot
        }
    }
}

void enumerate_input_devices_00a983c0(InputFocusBackendState& backend) {
    backend.xbox_360_present_f4 = false;
    backend.direct_input.enum_devices(0, 1);
}

void activate_input_device_slot_00a91620(InputFocusBackendState& backend,
    std::int32_t device_class, std::int32_t slot, InputFocusDeviceHost& devices) {
    const auto group_index = checked_class(backend, device_class);
    auto* const candidate = backend.slots.slot_reference(device_class, slot);
    auto& accepted_ids = backend.accepted_device_ids[group_index];
    bool accepted = false;
    // Native scans every ID even after a match, so later virtual calls remain.
    for (std::size_t i = 0; i < accepted_ids.size(); ++i) {
        if (accepted_ids[i] == -1) accepted = true;
        else {
            if (!candidate)
                throw std::logic_error("native device identifier query requires a live slot");
            const auto identifier = devices.query_identifier_vslot34(*candidate);
            if (accepted_ids[i] == identifier) accepted = true;
        }
    }
    if (!accepted) return;
    auto& group = backend.groups[group_index];
    if (std::find(group.begin(), group.end(), candidate) != group.end()) return;
    backend.bindings_dirty_d4 = true;
    const auto previous_count = static_cast<std::int32_t>(group.size());
    group.push_back(candidate); // A91430/A91260 are checked STL pointer insertion
    if (auto callback = backend.devices_changed_d8) callback(device_class, previous_count);
}

void retarget_mouse_input_bindings_00a92840(InputTickState& actions,
    InputDevice* replacement) noexcept {
    for (auto& record : actions.records) {
        for (auto& binding : record.bindings) {
            if (binding.device_class == 1) binding.cached_device = replacement;
            for (auto& modifier : binding.required_modifiers)
                if (modifier.device_class == 1) modifier.cached_device = replacement;
            for (auto& modifier : binding.forbidden_modifiers)
                if (modifier.device_class == 1) modifier.cached_device = replacement;
        }
    }
}

void reset_focus_input_00beca40(InputFocusResetHost& host,
    InputFocusDeviceHost& devices) {
    auto* initial = host.current_backend_00f8bbf4();
    if (!initial) return;
    auto* mouse = first_mouse(*initial);
    if (!mouse) throw std::logic_error("native focus reset requires its first active mouse");
    remove_active_input_device_00a90ee0(*initial, *mouse);
    delete_input_device_class_00bebf30(current_backend(host), 1, devices);
    enumerate_input_devices_00a983c0(current_backend(host));
    activate_input_device_slot_00a91620(current_backend(host), 1, 0, devices);
    auto* replacement = first_mouse(current_backend(host));
    // PUSH replacement precedes lazy 004BEC00; preserve it across getter work.
    auto& actions = host.input_manager_004bec00();
    retarget_mouse_input_bindings_00a92840(actions, replacement);
}

bool input_class_at_requested_count_00a90490(const InputFocusBackendState& backend,
    std::int32_t device_class) {
    const auto index = checked_class(backend, device_class);
    return static_cast<std::uint32_t>(backend.requested_active_counts[index]) ==
        static_cast<std::uint32_t>(backend.groups[index].size());
}

void input_backend_pre_tick_00a97390() noexcept {} // installed byte C3

void update_input_backend_00a918a0(InputFocusBackendState& backend,
    float seconds, InputFocusDeviceHost& devices) {
    input_backend_pre_tick_00a97390();
    for (int device_class = 0; device_class != input_device_class_count; ++device_class) {
        for (int index = 0; index != input_device_slot_count; ++index) {
            auto* const device = backend.slots.slot(device_class, index);
            if (!device) continue;
            devices.poll_device_vslot10(*device, poll_delta(seconds));
            if (input_class_at_requested_count_00a90490(backend, device_class)) continue;
            if (!backend.accept_inactive_gamepads_64 && device_class == 2 &&
                !devices.activity_vslot28(*device)) continue;
            activate_input_device_slot_00a91620(backend, device_class, index, devices);
        }
    }
}

KeyboardMouseFocusDeviceHost::KeyboardMouseFocusDeviceHost(Win32PlatformState& platform,
    InputFocusDeviceHost* other_devices) noexcept
    : platform_(platform), other_devices_(other_devices) {}

std::int32_t KeyboardMouseFocusDeviceHost::query_identifier_vslot34(InputDevice& device) {
    if (dynamic_cast<MouseInputDevice*>(&device) || dynamic_cast<KeyboardInputDevice*>(&device))
        return input_device_identifier_zero_00a93eb0();
    return other_host(other_devices_).query_identifier_vslot34(device);
}

void KeyboardMouseFocusDeviceHost::delete_device_vslot04(InputDevice& device,
    std::uint32_t flags) {
    if (auto* mouse = dynamic_cast<MouseInputDevice*>(&device))
        delete_mouse_input_device_00a9a390(*mouse, flags);
    else other_host(other_devices_).delete_device_vslot04(device, flags);
}

void KeyboardMouseFocusDeviceHost::poll_device_vslot10(InputDevice& device, float seconds) {
    if (auto* keyboard = dynamic_cast<KeyboardInputDevice*>(&device))
        (void)keyboard->poll_00a9a4a0(platform_.byte_170);
    else if (auto* mouse = dynamic_cast<MouseInputDevice*>(&device))
        (void)mouse->poll_00a9a180({reinterpret_cast<std::uintptr_t>(
            get_platform_window_00bec230(platform_)), platform_.settings_changed_2c});
    else other_host(other_devices_).poll_device_vslot10(device, seconds);
}

bool KeyboardMouseFocusDeviceHost::activity_vslot28(InputDevice& device) {
    // A918A0 asks this only for class2; do not fabricate joystick activity.
    return other_host(other_devices_).activity_vslot28(device);
}

} // namespace bsp
