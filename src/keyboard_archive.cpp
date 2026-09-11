#include "bsp/keyboard_archive.hpp"
#include "bsp/gui_lua_reader.hpp"

#include <stdexcept>

namespace bsp {

void write_keyboard_setup_006a51c0(const InputSettings& settings,
                                 SettingsWriter& writer) {
    // Device traversal is settings+8, not the ordered-name vector at +14h.
    for (const auto& [device_name, device] : settings.devices) {
        writer.begin_section(device_name.c_str());
        writer.begin_section("inputSettings");
        for (const auto& [input_name, bindings] : device.runtime_bindings) {
            writer.begin_section(input_name.c_str());
            for (std::int32_t slot = 0; slot < 2; ++slot) {
                const auto index = static_cast<std::size_t>(slot);
                const auto& binding = bindings.at(index);
                if (binding.device_type == -1) continue;

                writer.begin_section(gui_lua_key_by_index(slot));
                writer.write_field("deviceType", SettingsValue::from_int(binding.device_type));
                writer.write_field("deviceIdx", SettingsValue::from_int(binding.device_index));
                writer.write_field("key", SettingsValue::from_int(binding.key));
                if (binding.slider) {
                    writer.write_field("slider", SettingsValue::from_bool(true));
                }

                // Native 006a540e indexes the independent map at devRec+6Ch
                // by input name, then advances its vector<bool> iterator by
                // slot. It is not the binding's unknown +8 dword or bit zero
                // for both slots, as uncorrected pseudocode can suggest.
                const auto found = device.runtime_reverse.find(input_name);
                if (found == device.runtime_reverse.end()) {
                    throw std::out_of_range("keyboard archive: missing reverse flags for " + input_name);
                }
                if (found->second.at(index)) {
                    writer.write_field("reverse", SettingsValue::from_bool(true));
                }
                writer.end_section();
            }
            writer.end_section();
        }
        writer.end_section();

        writer.begin_section("sensitivitySettings");
        for (const auto& [name, value] : device.runtime_sensitivities) {
            writer.write_field(name.c_str(), SettingsValue::from_float(value));
        }
        writer.end_section();
        writer.end_section();
    }
}

} // namespace bsp
