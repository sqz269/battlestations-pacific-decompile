#include "bsp/input_binding_text.hpp"

#include <cstring>

namespace bsp {

const std::array<KeyNameOverride, 20> kKeyNameOverrides{{
    {0xC8, 0x01480001}, {0xD0, 0x01500001}, {0xCB, 0x014B0001}, {0xCD, 0x014D0001},
    {0xB7, 0x01370001}, {0xC5, 0x00450001}, {0xDB, 0x015B0001}, {0xDC, 0x015C0001},
    {0xB8, 0x21380001}, {0x9D, 0x011D0001}, {0xD2, 0x01520001}, {0xC7, 0x01470001},
    {0xC9, 0x01490001}, {0xD3, 0x01530001}, {0xCF, 0x014F0001}, {0xD1, 0x01510001},
    {0x45, 0x01450001}, {0xB5, 0x01350001}, {0x9C, 0x011C0001}, {0xDD, 0x015D0001},
}};

namespace {

bool equals_ascii_ci(const std::string& a, const char* b) {
    const std::size_t n = std::strlen(b);
    if (a.size() != n) return false;
    for (std::size_t i = 0; i < n; ++i) {
        auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? char(c - 'A' + 'a') : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}

} // namespace

std::string keyboard_key_display_name_0069e300(std::uint32_t key_code,
                                               const GetKeyNameTextFn& get_key_name_text) {
    // 0069E323..0069E37B: Backspace takes the Italian label when Return reads INVIO.
    if (key_code == 0x0E &&
        equals_ascii_ci(keyboard_key_display_name_0069e300(key_code + 0x0E, get_key_name_text),
                        "^.INVIO")) // 00CF8000
        return "^.INDIETRO";        // 00CF7FF4

    // 0069E380..0069E3A3: default lParam, then the first matching override.
    auto lparam = static_cast<std::int32_t>((key_code << 16) + 1);
    for (const auto& entry : kKeyNameOverrides) {
        if (entry.key_code == key_code) {
            lparam = entry.lparam;
            break;
        }
    }

    char buffer[0x100]{};
    get_key_name_text(lparam, buffer, static_cast<int>(sizeof buffer)); // 0069E3B5
    if (std::strlen(buffer) == 0) return std::string();                 // 0069E3CD -> 0069E4B5
    return std::string("^.") + buffer;                                  // 00CF7FF0, 0069E467
}

void append_binding_display_text_0069e9e0(const InputSettings& settings,
                                          const KeyboardInputBinding& binding,
                                          std::string& out, const BindingTextHooks& hooks) {
    // 0069EA02..0069EA8A: "." (00CE3A70) + (index + 1) + "#|" (00CF801C).
    if (static_cast<std::uint32_t>(binding.device_index) != 0)
        out += "." + std::to_string(binding.device_index + 1) + "#|";

    // 0069EB2A..0069EB47: class 2 with a present device.
    if (binding.device_type == 2 && hooks.device_present(binding.device_index)) {
        // 0069EB59..0069EB62: the second lookup; null is the cs_unknown branch.
        if (!hooks.device_present(binding.device_index)) {
            out += "FE_pc.cs_unknown"; // 00CF8008, 0069ECCC
            return;
        }
        // 0069EB68..0069EC54: ControllerInputNames[device name][key] when both exist.
        const auto device = settings.controller_input_names.find(hooks.device_name(binding.device_index));
        if (device != settings.controller_input_names.end()) {
            const auto label = device->second.find(binding.key);
            if (label != device->second.end()) {
                out += label->second;
                return;
            }
        }
        // 0069EC6E..0069EC8B: the device's own label, virtual +3Ch.
        out += hooks.device_key_label(binding.device_index, binding.key);
        return;
    }

    // 0069ED09..0069ED4E: keyboard name for class 0.
    if (binding.device_type == 0)
        out += keyboard_key_display_name_0069e300(static_cast<std::uint32_t>(binding.key),
                                                  hooks.get_key_name_text);

    // 0069ED75..0069EE16: every InputNames entry bound to this (class, key).
    for (const auto& [name, record] : settings.input_names)
        if (record.primary == binding.device_type && record.secondary == binding.key) out += name;
}

std::string binding_slot_text_006a9850(const InputSettings& settings, const std::string& device,
                                       const std::string& input, std::int32_t slot,
                                       const BindingTextHooks& hooks) {
    // 006A9873..006A9888: the global DEVINPUTS flag (settings+50h).
    if (settings.dev_inputs) return "(DEVINPUTS)"; // 00CF81C0

    // 006A988D..006A98D8: devices[device].runtime_bindings[input][slot].
    const DeviceSettings& record = settings.devices.at(device);
    const auto& bindings = record.runtime_bindings.at(input);
    const KeyboardInputBinding& binding = bindings.at(static_cast<std::size_t>(slot));
    if (binding.device_type == -1) return "FE_pc.cs_na"; // 00CF81B4

    std::string text;
    append_binding_display_text_0069e9e0(settings, binding, text, hooks);

    // 006A991A..006A9938: axes are class 1 from key 8 and class 2 from key 3Ch.
    const bool axis = (binding.device_type == 1 && static_cast<std::uint32_t>(binding.key) >= 8) ||
                      (binding.device_type == 2 && static_cast<std::uint32_t>(binding.key) >= 0x3C);
    if (axis) {
        const bool vertical = binding.device_type == 1 && binding.key == 0x0A; // 006A993E
        // 006A994E..006A9973: runtime_reverse[input][slot] on the device record.
        const bool reversed = record.runtime_reverse.at(input).at(static_cast<std::size_t>(slot));
        if (reversed) text += vertical ? "|. |^globals.up" : "|.-";   // 00CF803C / 00CF8038
        else text += vertical ? "|. |^globals.down" : "|.+";         // 00CF8024 / 00CF8020
    }
    // 006A99D8..006A99E8.
    if (text.empty()) text = "FE_pc.cs_unknown";
    return text;
}

} // namespace bsp
