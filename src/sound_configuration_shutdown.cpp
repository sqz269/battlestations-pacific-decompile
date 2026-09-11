#include "bsp/sound_configuration_shutdown.hpp"

#include <cassert>
#include <utility>

namespace bsp {
namespace {

void inspect_memory_error(FmodResult result,
    SoundConfigurationShutdownFmodHost& fmod) {
    if (result == FmodResult::err_memory) {
        std::int32_t current{};
        std::int32_t maximum{};
        fmod.memory_get_stats(&current, &maximum);
    }
}

void* native_handle(std::uint32_t word) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(word));
}

} // namespace

void release_sound_configured_group_effects_00a7aaf0(
    SoundConfiguredChannelGroup& group,
    SoundConfigurationShutdownFmodHost& fmod) {
    while (!group.dsps.empty()) {
        inspect_memory_error(fmod.dsp_remove(group.dsps.back()), fmod);
        // 00A7AB28 reloads count and backing after remove and diagnostics.
        inspect_memory_error(fmod.dsp_release(group.dsps.back()), fmod);
        if (!group.dsps.empty()) {
            group.dsps.pop_back();
        }
    }
    inspect_memory_error(fmod.channel_group_release(group.channel_group), fmod);
}

void destroy_sound_configured_group_storage_00a7bfe0(
    SoundConfiguredChannelGroup& group) {
    std::vector<void*>{}.swap(group.dsps);
    std::string{}.swap(group.name);
}

void destroy_sound_configured_type_storage_00a7c6c0(SoundConfiguredType& type) {
    std::vector<void*>{}.swap(type.by_listener);
    std::string{}.swap(type.name);
}

void shutdown_sound_configuration_effects_00a7f560(SoundConfigurationState& state,
    std::array<std::uint32_t, 5>& words_144,
    SoundConfigurationShutdownFmodHost& fmod) {
    while (!state.channel_groups_128.empty()) {
        release_sound_configured_group_effects_00a7aaf0(
            state.channel_groups_128.back(), fmod);
        // 00A7F588 reloads the outer count and selects the current last group.
        if (!state.channel_groups_128.empty()) {
            destroy_sound_configured_group_storage_00a7bfe0(
                state.channel_groups_128.back());
            state.channel_groups_128.pop_back();
        }
    }
    if (words_144[2] != 0) {
        fmod.dsp_remove(native_handle(words_144[2]));
        fmod.dsp_release(native_handle(words_144[2]));
    }
    if (words_144[0] != 0) {
        fmod.channel_group_release(native_handle(words_144[0]));
        words_144[0] = 0;
    }
    if (words_144[1] != 0) {
        fmod.channel_group_release(native_handle(words_144[1]));
        words_144[1] = 0;
    }
    if (words_144[3] != 0) {
        fmod.dsp_remove(native_handle(words_144[3]));
        fmod.dsp_release(native_handle(words_144[3]));
    }
    if (words_144[4] != 0) {
        fmod.dsp_remove(native_handle(words_144[4]));
        fmod.dsp_release(native_handle(words_144[4]));
    }
}

void shrink_sound_system_dsps_00a7a980_fragment(SoundConfigurationState& state,
    std::size_t requested_count) {
    assert(requested_count <= state.system_dsps_134.size());
    while (state.system_dsps_134.size() > requested_count) {
        state.system_dsps_134.pop_back();
    }
}

void shrink_sound_configured_groups_00a7f240_fragment(SoundConfigurationState& state,
    std::size_t requested_count) {
    assert(requested_count <= state.channel_groups_128.size());
    while (state.channel_groups_128.size() > requested_count) {
        // Transfer storage so the logical count drops before native dtor work.
        auto removed = std::move(state.channel_groups_128.back());
        state.channel_groups_128.pop_back();
        destroy_sound_configured_group_storage_00a7bfe0(removed);
    }
}

void shrink_sound_configured_types_00a7fcc0_fragment(SoundConfigurationState& state,
    std::size_t requested_count) {
    assert(requested_count <= state.types_38.size());
    while (state.types_38.size() > requested_count) {
        auto removed = std::move(state.types_38.back());
        state.types_38.pop_back();
        destroy_sound_configured_type_storage_00a7c6c0(removed);
    }
}

} // namespace bsp
