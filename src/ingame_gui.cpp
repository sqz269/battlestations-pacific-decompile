// In-game GUI pieces of segments 26 and 27. Evidence: docs/CC13_INGAME_GUI.md.

#include "bsp/ingame_gui.hpp"

namespace bsp {

const SupportManagerSound* support_manager_sound_for_player(std::uint32_t player) noexcept {
    for (const SupportManagerSound& sound : kSupportManagerSounds) {
        if (sound.player == player) return &sound;
    }
    return nullptr;
}

void play_support_manager_sound(const SupportManagerSound& sound, SupportManagerSoundHost& host) {
    // e.g. 0065F18D..0065F1DD: the function-static class index.
    if (!host.class_cached(sound.guard_global)) {
        host.cache_class(sound.guard_global, sound.class_global,
            host.find_sound_class_00a7acf0(kSupportManagerSoundClass));
    }
    const std::int64_t type = host.find_sound_type_00a7b0a0(kSupportManagerSoundType);
    void* handle = host.create_tracked_sound_00a7e490(sound.sample_field,
        host.cached_class(sound.class_global), type, true);
    if (handle != nullptr) host.release_handle(handle);
}

std::string format_marker_distance_0063e5c0(float distance, bool nautical_miles) {
    // 0063E69A..0063E6AF: UCOMISS with 0.0f; equality skips the text.
    if (distance == 0.0f) return {};
    const float tenths = static_cast<float>(static_cast<double>(distance) /
        (nautical_miles ? kDistanceTenthNauticalMile : kDistanceTenthKilometre));
    // 0063E6E7..0063E72A: CVTTSS2SI, signed /10 and %10, x87 remainder test.
    const std::int32_t whole = static_cast<std::int32_t>(tenths);
    std::int32_t tens = whole / 10;
    std::int32_t digit = whole % 10;
    if (static_cast<double>(tenths) - whole > 0.5) ++digit;
    if (digit == 10) {
        ++tens;
        digit = 0;
    }
    std::string text = std::to_string(tens);
    text += '.';
    text += std::to_string(digit);
    text += " |";
    text += nautical_miles ? kDistanceMileKey : kDistanceKilometreKey;
    return text;
}

std::int32_t device_class_icon_index_0065ea30(std::string_view name) noexcept {
    const auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c + 32) : c; };
    for (const DeviceClassIcon& entry : kDeviceClassIcons) {
        if (entry.name.size() != name.size()) continue;
        bool equal = true;
        for (std::size_t i = 0; i < name.size() && equal; ++i) equal = lower(entry.name[i]) == lower(name[i]);
        if (equal) return entry.index;
    }
    return kDeviceClassIconUnknown;  // 0065EE53..0065EE76, after the "%s" diagnostic
}

} // namespace bsp