#pragma once

#include "bsp/sound_configuration.hpp"
#include "bsp/sound_file_callbacks.hpp"
#include "bsp/sound_resource_asset.hpp"
#include "bsp/sound_resource_cleanup.hpp"
#include "bsp/sound_sample.hpp"
#include "bsp/sound_instance.hpp"
#include "bsp/sound_system_update.hpp"
#include "bsp/sound_spatial_instance.hpp"
#include "bsp/sound_event_instance.hpp"

#include <memory>
#include <string>
#include <vector>

namespace bsp {

struct FmodConfigurationCall {
    const char* function;
    FmodResult result;
};

// Concrete library boundary for the installed Win32 fmodex.dll C exports.
// This loads the supplied DLL; it does not reconstruct FMOD or replace the
// original game's object ABI. See docs/INSTALLED_SOUND_CONFIGURATION.md.
// The caller must serialize access. Release every system before destroying the library;
// opaque FMOD objects must come from this same loaded library instance.
class FmodConfigurationLibrary final : public SoundConfigurationFmodHost,
    public FmodStartupHost, public SoundResourceAssetFmodHost,
    public SoundResourceCleanupFmodHost, public SoundSampleFmodHost,
    public SoundChannelFmodHost, public SoundSystemUpdateFmodHost, public SoundSpatialChannelFmodHost,
    public SoundEventFmodHost {
public:
    explicit FmodConfigurationLibrary(const std::wstring& dll_path);
    FmodConfigurationLibrary(const std::wstring& dll_path,
        const std::wstring& event_dll_path, SoundFileCallbackBundle callbacks = {});
    ~FmodConfigurationLibrary() override;
    FmodConfigurationLibrary(const FmodConfigurationLibrary&) = delete;
    FmodConfigurationLibrary& operator=(const FmodConfigurationLibrary&) = delete;

    // Actual library entry points for a host/probe to own its System. A no-sound
    // setup is selected explicitly with set_output, not imposed by this adapter.
    FmodResult create_system(void** system);
    FmodResult get_system_version(void* system, std::uint32_t* version);
    FmodResult set_output(void* system, FmodOutputType type);
    FmodResult initialize_system(void* system, std::int32_t channels,
        std::uint32_t flags, void* driver_data);
    FmodResult update_system(void* system);
    FmodResult release_system(void* system);
    FmodResult release_event_system(void* event_system);
    FmodResult update_event_system(void* event_system) override;
    FmodResult system_get_channels_playing(void*, std::int32_t*) override;
    FmodResult event_system_set_3d_listener_attributes(void*, std::int32_t,
        const std::array<float, 3>&, const std::array<float, 3>&,
        const std::array<float, 3>&, const std::array<float, 3>&) override;
    FmodResult create_stream(void* system, const char* path, std::uint32_t mode,
        void* extra_info, void** sound);
    FmodResult release_sound(void* sound) override;
    FmodResult release_event_project(void*) override;
    FmodResult create_sound(void*, const char*, std::uint32_t, void*, void**) override;
    FmodResult sound_get_length(void*, std::uint32_t*, std::uint32_t) override;
    FmodResult sound_get_num_subsounds(void*, std::int32_t*) override;
    FmodResult sound_get_subsound(void*, std::int32_t, void**) override;
    FmodResult sound_get_mode(void*, std::uint32_t*) override;
    FmodResult sound_get_defaults(void*, float*, float*, float*, std::int32_t*) override;
    FmodResult sound_set_3d_min_max_distance(void*, float, float) override;
    FmodResult sound_set_variations(void*, float, float, float) override;
    FmodResult sound_set_loop_points(void*, std::uint32_t, std::uint32_t,
        std::uint32_t, std::uint32_t) override;
    FmodResult sound_set_mode(void*, std::uint32_t) override;
    FmodResult event_system_load(void*, const char*, void*, void**) override;
    FmodResult event_project_get_group(void*, const char*, std::int32_t, void**) override;
    FmodResult event_group_get_group(void*, const char*, std::int32_t, void**) override;
    FmodResult event_group_load_event_data(void*, std::uint32_t, std::uint32_t) override;
    FmodResult event_group_free_event_data(void*, void*, std::int32_t) override;
    FmodResult event_system_get_event(void*, const char*, std::uint32_t, void**) override;
    FmodResult event_get_num_parameters(void*, std::int32_t*) override;
    FmodResult event_get_parameter_by_index(void*, std::int32_t, void**) override;
    FmodResult event_parameter_get_range(void*, float*, float*) override;
    FmodResult event_parameter_get_info(void*, std::int32_t*, char**) override;

    FmodResult system_play_sound(void*, std::int32_t, void*, bool, void**) override;
    FmodResult channel_set_loop_count(void*, std::int32_t) override;
    FmodResult channel_set_loop_points(void*, std::uint32_t, std::uint32_t,
        std::uint32_t, std::uint32_t) override;
    FmodResult channel_set_speaker_mix(void*, const std::array<float, 8>&) override;
    FmodResult channel_stop(void*) override;
    FmodResult channel_is_virtual(void*, std::optional<bool>&) override;
    FmodResult channel_is_playing(void*, bool*) override;
    FmodResult channel_set_frequency(void*, float) override;
    FmodResult channel_set_volume(void*, float) override;
    FmodResult channel_set_3d_pan_level(void*, float) override;
    FmodResult channel_set_paused(void*, std::uint8_t) override;
    FmodResult channel_get_audibility(void*, float*) override;
    FmodResult channel_set_group(void*, void*) override;
    FmodResult channel_set_3d_attributes(void*, const std::array<float, 3>&,
        const std::array<float, 3>&) override;
    FmodResult dsp_remove(void*) override;
    FmodResult dsp_release(void*) override;
    FmodResult event_start(void*) override;
    FmodResult event_stop(void*, bool) override;
    FmodResult event_get_state(void*, std::uint32_t*) override;
    FmodResult event_get_parameter(void*, const char*, void**) override;
    FmodResult event_parameter_key_off(void*) override;
    FmodResult event_parameter_set_value(void*, float) override;
    FmodResult event_get_channel_group(void*, void**) override;
    FmodResult channel_group_add_group(void*, void*) override;
    FmodResult event_set_pitch(void*, float, std::uint32_t) override;
    FmodResult event_set_volume(void*, float) override;
    FmodResult event_set_3d_attributes(void*, const std::array<float, 3>&,
        const std::array<float, 3>&, const std::array<float, 3>&) override;
    FmodResult event_set_paused(void*, std::uint8_t) override;

    FmodResult event_system_create(void**) override;
    FmodResult event_system_get_system_object(void*, void**) override;
    FmodResult event_system_init(void*, const FmodEventSystemInitArgs&) override;
    FmodResult system_get_num_drivers(void*, std::int32_t*) override;
    FmodResult system_get_driver_caps(void*, std::int32_t, FmodDriverCaps*) override;
    FmodResult system_set_speaker_mode(void*, FmodSpeakerMode) override;
    FmodResult system_set_output(void*, FmodOutputType) override;
    FmodResult system_set_file_system(void*, const FmodFileSystemHooks&) override;
    FmodResult system_set_3d_settings(void*, const Fmod3DSettings&) override;
    FmodResult system_get_driver(void*, std::int32_t*) override;
    FmodResult system_get_output(void*, FmodOutputType*) override;
    FmodResult system_get_speaker_mode(void*, FmodSpeakerMode*) override;
    void on_out_of_sound_memory(const char*) override;

    FmodResult get_master_channel_group(void*, void**) override;
    FmodResult get_advanced_settings(void*, SoundFmodAdvancedSettings&) override;
    FmodResult set_advanced_settings(void*, const SoundFmodAdvancedSettings&) override;
    FmodResult create_channel_group(void*, const char*, void**) override;
    FmodResult set_channel_group_volume(void*, float) override;
    FmodResult set_channel_group_pitch(void*, float) override;
    FmodResult create_dsp_by_type(void*, std::int32_t, void**) override;
    FmodResult set_dsp_parameter(void*, std::int32_t, float) override;
    FmodResult add_channel_group_dsp(void*, void*, void**) override;
    FmodResult add_system_dsp(void*, void*, void**) override;
    void memory_get_stats(std::int32_t*, std::int32_t*) override;
    int compare_class_name_case_insensitive(const char*, const char*) override;

    // Raw results are preserved, including errors the recovered caller ignores.
    const std::vector<FmodConfigurationCall>& calls() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bsp
