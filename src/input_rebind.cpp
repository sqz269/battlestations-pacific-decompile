#include "bsp/input_rebind.hpp"

#include <cstddef>
#include <vector>

namespace bsp {

namespace {

// Native compares lengths, then _stricmp (006A9B8x, 006AAD8x, 006A9FBx).
bool equals_ci(const std::string& a, const std::string& b) {
    const CaseInsensitiveLess less;
    return !less(a, b) && !less(b, a);
}

// 00699BF0: class 1 from key 8, class 2 from 3Ch, unsigned compares.
bool is_axis_binding(const KeyboardInputBinding& binding) {
    const auto key = static_cast<std::uint32_t>(binding.key);
    return (binding.device_type == 1 && key >= 8) || (binding.device_type == 2 && key >= 0x3C);
}

std::vector<bool>::reference reverse_bit(DeviceSettings& record, const std::string& input,
                                         std::int32_t slot) {
    return record.runtime_reverse.at(input).at(static_cast<std::size_t>(slot));
}

} // namespace

void set_input_binding_slot_006a9a40(InputSettings& settings, const std::string& device,
                                     const std::string& input, std::int32_t slot,
                                     const KeyboardInputBinding& binding, float scale) {
    if (settings.dev_inputs) return;  // 006A9A58
    auto& record = settings.devices.at(device);                                  // 0055C110
    const auto index = static_cast<std::size_t>(slot);
    record.runtime_bindings.at(input).at(index) = binding;                       // 006A9AB1..AE8
    reverse_bit(record, input, slot) = 0.0f > scale;                             // 006A9AC1..B2A

    for (const auto& pair : record.axis_pairs) {                                 // 006A9B30
        const std::string* partner = nullptr;
        if (equals_ci(pair.first, input)) partner = &pair.second;
        else if (equals_ci(pair.second, input)) partner = &pair.first;
        if (!partner) continue;
        const std::string other = *partner;  // 006A9C24 copy
        auto& other_binding = record.runtime_bindings.at(other).at(index);
        if (is_axis_binding(binding)) {
            // 006A9C73..006A9D18: the partner shares the axis, opposite sense.
            other_binding = binding;
            reverse_bit(record, other, slot) = !(scale < 0.0f);                  // [00D7A218] = 0.0f
        } else if (is_axis_binding(other_binding)) {
            // 006A9D1D..006A9E0F: a button replaces the shared axis.
            other_binding = KeyboardInputBinding{};
            other_binding.key = -1;
            reverse_bit(record, other, slot) = false;
        }
    }
}

bool input_in_axis_pair_006a9f70(const InputSettings& settings, const std::string& device,
                                 const std::string& input) {
    for (const auto& pair : settings.devices.at(device).axis_pairs) {
        if (equals_ci(input, pair.first) || equals_ci(input, pair.second)) return true;
    }
    return false;
}

bool find_input_binding_conflict_006aac50(const InputSettings& settings,
                                          const std::string& device, const std::string& input,
                                          std::int32_t slot, std::string* out) {
    if (settings.dev_inputs) return true;  // 006AAC68..006AAC82
    const auto& record = settings.devices.at(device);
    const auto index = static_cast<std::size_t>(slot);
    const auto& binding = record.runtime_bindings.at(input).at(index);
    if (binding.device_type == -1) return false;
    const bool reversed = record.runtime_reverse.at(input).at(index);  // 006AACF9..006AAD0A

    // 006AAD30..006AAE90: the first group that names (device, input).
    std::size_t group = settings.conflict_groups.size();
    for (std::size_t g = 0; g < settings.conflict_groups.size(); ++g) {
        bool found = false;
        for (const auto& entry : settings.conflict_groups[g]) {
            if (equals_ci(entry.first, device) && equals_ci(entry.second, input)) {
                found = true;
                break;
            }
        }
        if (found) {
            group = g;
            break;
        }
    }
    if (group == settings.conflict_groups.size()) return false;

    // 006AAEA0..006AAF73: g, then every group a Conflicts.Pairs row links to it.
    std::vector<std::size_t> related{group};
    for (const auto& pair : settings.conflict_pairs) {
        if (static_cast<std::size_t>(pair.first) == group)
            related.push_back(static_cast<std::size_t>(pair.second));
        else if (static_cast<std::size_t>(pair.second) == group)
            related.push_back(static_cast<std::size_t>(pair.first));
    }

    const bool paired = input_in_axis_pair_006a9f70(settings, device, input);  // 006AAF8C
    for (const auto r : related) {
        for (const auto& entry : settings.conflict_groups.at(r)) {
            const bool same_device = equals_ci(device, entry.first);
            const bool same_input = equals_ci(input, entry.second);
            // 006AB0F0..006AB2BE: entry.second shares an AxisPair with input.
            bool shares_axis = false;
            if (paired && same_device) {
                for (const auto& pair : record.axis_pairs) {
                    if ((equals_ci(pair.first, input) || equals_ci(pair.second, input)) &&
                        (equals_ci(pair.first, entry.second) ||
                         equals_ci(pair.second, entry.second))) {
                        shares_axis = true;
                        break;
                    }
                }
            }
            const auto& other_record = settings.devices.at(entry.first);
            for (std::size_t s = 0; s < 2; ++s) {  // 006AB2C3..006AB477, stride 14h, < 28h
                const auto& other = other_record.runtime_bindings.at(entry.second).at(s);
                if (other.device_type == -1) continue;
                if (s == index && ((same_device && same_input) ||
                                   (shares_axis && is_axis_binding(other))))
                    continue;
                if (other.device_type != binding.device_type ||
                    other.device_index != binding.device_index || other.key != binding.key)
                    continue;
                if (other_record.runtime_reverse.at(entry.second).at(s) != reversed) continue;
                if (out) *out = entry.second + "|. (|" + entry.first + "|.)|";  // 00CF81D0/D8
                return true;
            }
        }
    }
    return false;
}

} // namespace bsp
