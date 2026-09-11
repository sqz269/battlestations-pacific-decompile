#include "bsp/keyboard_axis_pairs.hpp"

#include <algorithm>

namespace bsp {
namespace {

bool same_map_name(const std::string& first, const std::string& second) {
    const CaseInsensitiveLess less;
    return !less(first, second) && !less(second, first);
}

std::vector<std::int32_t> input_codes_or_insert(DeviceSettings& device,
    const std::string& name) {
    // devRec+0 is the description map, distinct from bindings at +0x0c.
    // 006a44b0 inserts an empty vector on a miss. The loader's row model
    // retains duplicate Lua rows; native 006a8117 appends their code lists.
    std::vector<std::int32_t> result;
    bool found = false;
    for (const auto& entry : device.inputs) {
        if (!entry.table_form || !same_map_name(entry.name, name)) continue;
        found = true;
        result.insert(result.end(), entry.codes.begin(), entry.codes.end());
    }
    if (!found) device.inputs.push_back(InputEntry{name, {}, true});
    return result;
}

} // namespace

bool keyboard_input_codes_equal_0069e860(const std::vector<std::int32_t>& first,
    const std::vector<std::int32_t>& second) noexcept {
    // 0069e890 length compare; 0069ddc2..0069ddce integer CMP and +4 walk.
    return first.size() == second.size() &&
        std::equal(first.begin(), first.end(), second.begin());
}

bool use_alternate_axis_slots_006aa090(InputSettings& settings,
    const std::string& device_name, const std::string& input_name) {
    // A host input-name reference can live in the row vector that insertion
    // grows. Native pooled strings/map nodes remain stable during insertion.
    const std::string query = input_name;
    auto& device = settings.devices[device_name]; // 0055c110(settings+8).
    for (const auto& pair : device.axis_pairs) {
        if (query.size() != pair.second.size() ||
            !same_map_name(query, pair.second)) continue;
        // Preserve the native lookup order (006aa171 then 006aa17d). Copies
        // preserve the equal-name case even if appending a missing row moves
        // host storage; the native maps return the same vector in that case.
        const auto second = input_codes_or_insert(device, pair.second);
        const auto first = input_codes_or_insert(device, pair.first);
        if (keyboard_input_codes_equal_0069e860(first, second)) return true;
    }
    return false;
}

} // namespace bsp
