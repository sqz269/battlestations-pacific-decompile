#include "bsp/keyboard_restore.hpp"

#include <stdexcept>

namespace bsp {
namespace {

void read_int(GuiLuaReader& reader, const char* name, std::int32_t& value) {
    reader.read_00bd6830(gui_lua_key_by_name(name),
        gui_lua_field(GuiLuaFieldType::Int, &value));
}

void read_false_default(GuiLuaReader& reader, const char* name, bool& value) {
    GuiLuaVariant fallback{};
    fallback.tag = static_cast<std::int32_t>(GuiLuaFieldType::Bool);
    reader.read_or_default_00bd68d0(gui_lua_key_by_name(name),
        gui_lua_field(GuiLuaFieldType::Bool, &value), fallback);
}

std::int32_t sensitivity_device_class(std::int32_t category) {
    switch (category) {
    case 0: return 0;
    case 1: case 3: return 1;
    case 2: case 4: return 2;
    default: throw std::invalid_argument("keyboard restore: sensitivity category outside 0..4");
    }
}

KeyboardInputBinding disabled_binding() {
    KeyboardInputBinding result{};
    result.key = -1; // Native archive default differs from loader's key=0.
    return result;
}

// 00699bf0. Unsigned thresholds are significant for negative native keys.
bool suppress_alternate_binding(const KeyboardInputBinding& binding) {
    const auto key = static_cast<std::uint32_t>(binding.key);
    return (binding.device_type == 1 && key >= 8) ||
        (binding.device_type == 2 && key >= 60);
}

} // namespace

void read_keyboard_setup_006aba50(InputSettings& settings, GuiLuaReader& reader,
    KeyboardRuntimeHost& host) {
    settings.runtime_settings_loaded = true; // 006aba5a, native settings+5.
    for (auto& [device_name, device] : settings.devices) {
        const auto device_key = gui_lua_key_by_name(device_name.c_str());
        if (!reader.has_key_00bd5eb0(device_key)) continue;
        reader.enter_00bd8e20(device_key);
        reader.enter_00bd8e20(gui_lua_key_by_name("inputSettings"));
        for (auto& [input_name, bindings] : device.runtime_bindings) {
            const auto input_key = gui_lua_key_by_name(input_name.c_str());
            if (!reader.has_key_00bd5eb0(input_key)) continue;
            reader.enter_00bd8e20(input_key);
            for (std::int32_t slot = 0; slot < 2; ++slot) {
                const auto slot_key = gui_lua_key_by_index(slot);
                const auto index = static_cast<std::size_t>(slot);
                if (!reader.has_key_00bd5eb0(slot_key)) {
                    bindings.at(index) = disabled_binding();
                    continue;
                }
                reader.enter_00bd8e20(slot_key);
                auto& binding = bindings.at(index);
                read_int(reader, "deviceType", binding.device_type);
                read_int(reader, "deviceIdx", binding.device_index);
                read_int(reader, "key", binding.key);
                read_false_default(reader, "slider", binding.slider);
                bool reverse{};
                read_false_default(reader, "reverse", reverse);
                device.runtime_reverse[input_name].at(index) = reverse;
                reader.leave_00bd7a20();
            }
            reader.leave_00bd7a20();
        }
        reader.leave_00bd7a20();
        reader.enter_00bd8e20(gui_lua_key_by_name("sensitivitySettings"));
        for (auto& [name, value] : device.runtime_sensitivities) {
            const auto key = gui_lua_key_by_name(name.c_str());
            if (reader.has_key_00bd5eb0(key)) {
                reader.read_00bd6830(key, gui_lua_field(GuiLuaFieldType::Float, &value));
            }
        }
        reader.leave_00bd7a20();
        reader.leave_00bd7a20();
    }
    apply_keyboard_setup_006aa640(settings, host);
}

void apply_keyboard_setup_006aa640(InputSettings& settings, KeyboardRuntimeHost& host) {
    if (settings.dev_inputs || !settings.runtime_settings_loaded) return;
    if (!host.action_registered_00a92260(0x128)) return;

    for (auto& [device_name, device] : settings.devices) {
        auto scales = device.base_sensitivities; // 0055b400 copies devRec+50h.

        // The existing loader keeps description rows in Lua order. Native
        // devRec+18h and +0 are case-insensitive maps: duplicate rows append
        // codes, and the last sensitivity category wins (006a8381/006a83eb).
        std::map<std::string, SensitivityEntry, CaseInsensitiveLess> sensitivities;
        for (const auto& entry : device.sensitivities) {
            auto& row = sensitivities[entry.name];
            row.value = entry.value;
            row.codes.insert(row.codes.end(), entry.codes.begin(), entry.codes.end());
        }
        for (const auto& [name, row] : sensitivities) {
            const float multiplier = device.runtime_sensitivities[name];
            if (row.codes.empty()) continue;
            const auto device_class = sensitivity_device_class(row.value);
            for (const auto code : row.codes) {
                float& scale = scales[code][device_class];
                if (scale == 0.0f) scale = 1.0f;
                // Native ordered COMISS branches: NaN also takes the lookup.
                if (multiplier >= 1.0f || multiplier <= -1.0f ||
                    device.min1_sens_hacks.find(code) == device.min1_sens_hacks.end()) {
                    // x87 FLD/FMUL/FSTP has an explicit float32 store boundary.
                    scale = static_cast<float>(static_cast<double>(multiplier) * scale);
                }
            }
        }

        std::map<std::string, std::vector<std::int32_t>, CaseInsensitiveLess> inputs;
        for (const auto& entry : device.inputs) {
            if (!entry.table_form) continue;
            auto& codes = inputs[entry.name];
            codes.insert(codes.end(), entry.codes.begin(), entry.codes.end());
        }
        for (const auto& [name, codes] : inputs) {
            const bool alternate = host.use_alternate_axis_slots_006aa090(
                settings, device_name, name);
            for (std::int32_t slot = 0; slot < 2; ++slot) {
                const auto index = static_cast<std::size_t>(slot);
                const auto& binding = device.runtime_bindings[name].at(index);
                for (const auto code : codes) {
                    float scale = scales[code][binding.device_type];
                    if (scale == 0.0f) scale = 1.0f; // Local fallback, not map write.
                    if (device.runtime_reverse[name].at(index)) {
                        scale = static_cast<float>(static_cast<double>(scale) * -1.0);
                    }
                    if (!alternate) {
                        host.bind_action_00a93750(code, slot, binding, scale);
                    } else if (suppress_alternate_binding(binding)) {
                        host.bind_action_00a93750(code, slot + 2, disabled_binding(), 0.0f);
                    } else {
                        host.bind_action_00a93750(code, slot + 2, binding, -0.0f - scale);
                    }
                }
            }
        }
    }
}

} // namespace bsp
