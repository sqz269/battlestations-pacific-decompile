#include "bsp/sound_manager_levels.hpp"

#include "bsp/game_settings.hpp"

#include <cstddef>

namespace bsp {

void dirty_sound_classes_00a7a3f0(SoundManagerLevels& manager,
    std::uint32_t mask) noexcept
{
    for (SoundLevelEntry* entry : manager.entries_8c) {
        if ((mask & sound_class_bit(entry->class_44->class_index_08)) != 0u) {
            entry->dirty_14 = 1u;
        }
    }
}

void apply_sound_global_level_00a7a440(SoundManagerLevels& manager,
    float volume) noexcept
{
    manager.global_4c = volume;
    dirty_sound_classes_00a7a3f0(manager, kAllSoundGroups);
}

void set_sound_class_levels_00a7abf0(SoundManagerLevels& manager,
    std::uint32_t mask, float volume) noexcept
{
    std::uint32_t index = 0u;
    for (SoundClassLevel* sound_class : manager.classes_98) {
        const std::uint32_t bit = sound_class_bit(index++);
        if ((mask & bit) != 0u) {
            if (sound_class->volume_10 == volume) {
                mask ^= bit;
            } else {
                sound_class->volume_10 = volume;
            }
        }
    }
    if (mask != 0u) {
        dirty_sound_classes_00a7a3f0(manager, mask);
    }
}

std::int32_t find_sound_class_00a7acf0(const SoundManagerLevels& manager,
    const NativeString& name, SoundLevelNameHost& host)
{
    for (std::size_t index = 0; index < manager.classes_98.size(); ++index) {
        const NativeString& candidate = *manager.classes_98[index]->name_18;
        if (candidate.length() == name.length()) {
            if (candidate.length() == 0u ||
                host.compare_class_name_case_insensitive(candidate.data(), name.data()) == 0) {
                return static_cast<std::int32_t>(index);
            }
        }
    }
    return -1;
}

void set_sound_class_level_00a7f8e0(SoundManagerLevels& manager,
    std::int32_t index, float volume, SoundLevelTableHost& host)
{
    if (index >= static_cast<std::int32_t>(manager.classes_98.size())) {
        host.grow_classes_before_read(manager, index + 1);
    }
    const auto ordinal = static_cast<std::size_t>(index);
    if (manager.classes_98[ordinal]->volume_10 != volume) {
        if (index >= static_cast<std::int32_t>(manager.classes_98.size())) {
            host.grow_classes_before_write(manager, index + 1);
        }
        manager.classes_98[ordinal]->volume_10 = volume;
        dirty_sound_classes_00a7a3f0(manager,
            sound_class_bit(static_cast<std::uint32_t>(index)));
    }
}

} // namespace bsp
