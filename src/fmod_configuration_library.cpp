#include "bsp/fmod_configuration_library.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstring>
#include <filesystem>
#include <stdexcept>

namespace bsp {

struct FmodConfigurationLibrary::Impl {
    HMODULE module{};
    HMODULE event_module{};
    std::wstring event_path;
    SoundFileCallbackBundle file_callbacks;
    std::vector<FmodConfigurationCall> results;

    Impl(const std::wstring& path, const std::wstring& event_dll,
        SoundFileCallbackBundle callbacks) : event_path(event_dll), file_callbacks(callbacks) {
        static_assert(sizeof(void*) == 4, "The installed FMOD ABI is Win32.");
        module = LoadLibraryExW(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!module) throw std::runtime_error("Cannot load FMOD DLL: Win32 error " +
            std::to_string(GetLastError()));
    }
    ~Impl() {
        if (event_module) FreeLibrary(event_module);
        if (module) FreeLibrary(module);
    }

    HMODULE event_library() {
        if (!event_module) {
            event_module = LoadLibraryExW(event_path.c_str(), nullptr,
                LOAD_WITH_ALTERED_SEARCH_PATH);
            if (!event_module) throw std::runtime_error("Cannot load FMOD Event DLL: Win32 error " +
                std::to_string(GetLastError()));
        }
        return event_module;
    }

    template<class... Args>
    FmodResult call(const char* name, Args... args) {
        return call_from(module, name, args...);
    }
    template<class... Args>
    FmodResult call_from(HMODULE library, const char* name, Args... args) {
        // The installed DLL exports undecorated aliases for its stdcall C API.
        // Original game call-site stack cleanup establishes this ABI;
        // no native image addresses are called here.
        using Function = std::uint32_t (__stdcall*)(Args...);
        const FARPROC address = GetProcAddress(library, name);
        if (!address) throw std::runtime_error(std::string("Missing FMOD export: ") + name);
        Function function;
        static_assert(sizeof(function) == sizeof(address));
        std::memcpy(&function, &address, sizeof(function));
        const auto result = static_cast<FmodResult>(function(args...));
        results.push_back({name, result});
        return result;
    }
};

FmodConfigurationLibrary::FmodConfigurationLibrary(const std::wstring& path)
    : FmodConfigurationLibrary(path,
        (std::filesystem::path(path).parent_path() / L"fmod_event.dll").wstring()) {}
FmodConfigurationLibrary::FmodConfigurationLibrary(const std::wstring& path,
    const std::wstring& event_path, SoundFileCallbackBundle callbacks)
    : impl_(std::make_unique<Impl>(path, event_path, callbacks)) {}
FmodConfigurationLibrary::~FmodConfigurationLibrary() = default;

FmodResult FmodConfigurationLibrary::create_system(void** system) {
    return impl_->call("FMOD_System_Create", system);
}
FmodResult FmodConfigurationLibrary::get_system_version(void* system, std::uint32_t* version) {
    return impl_->call("FMOD_System_GetVersion", system, version);
}
FmodResult FmodConfigurationLibrary::set_output(void* system, FmodOutputType type) {
    return impl_->call("FMOD_System_SetOutput", system, static_cast<std::int32_t>(type));
}
FmodResult FmodConfigurationLibrary::initialize_system(void* system,
    std::int32_t channels, std::uint32_t flags, void* data) {
    return impl_->call("FMOD_System_Init", system, channels, flags, data);
}
FmodResult FmodConfigurationLibrary::update_system(void* system) {
    return impl_->call("FMOD_System_Update", system);
}
FmodResult FmodConfigurationLibrary::release_system(void* system) {
    return impl_->call("FMOD_System_Release", system);
}
FmodResult FmodConfigurationLibrary::event_system_create(void** event) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventSystem_Create@4", event);
}
FmodResult FmodConfigurationLibrary::event_system_get_system_object(void* event, void** system) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventSystem_GetSystemObject@8", event, system);
}
FmodResult FmodConfigurationLibrary::event_system_init(void* event,
    const FmodEventSystemInitArgs& args) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventSystem_Init@20", event,
        args.max_channels, args.init_flags, args.extra_driver_data, args.event_init_flags);
}
FmodResult FmodConfigurationLibrary::release_event_system(void* event) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventSystem_Release@4", event);
}
FmodResult FmodConfigurationLibrary::update_event_system(void* event) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventSystem_Update@4", event);
}
FmodResult FmodConfigurationLibrary::create_stream(void* system, const char* path,
    std::uint32_t mode, void* extra_info, void** sound) {
    return impl_->call("FMOD_System_CreateStream", system, path, mode, extra_info, sound);
}
FmodResult FmodConfigurationLibrary::release_sound(void* sound) {
    return impl_->call("FMOD_Sound_Release", sound);
}
FmodResult FmodConfigurationLibrary::release_event_project(void* project) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventProject_Release@4", project);
}
FmodResult FmodConfigurationLibrary::create_sound(void* system, const char* data,
    std::uint32_t mode, void* extra, void** sound) {
    return impl_->call("FMOD_System_CreateSound", system, data, mode, extra, sound);
}
FmodResult FmodConfigurationLibrary::sound_get_length(void* sound,
    std::uint32_t* length, std::uint32_t unit) {
    return impl_->call("FMOD_Sound_GetLength", sound, length, unit);
}
FmodResult FmodConfigurationLibrary::system_play_sound(void* system, std::int32_t index,
    void* sound, bool paused, void** channel) {
    return impl_->call("FMOD_System_PlaySound", system, index, sound, static_cast<std::int32_t>(paused), channel);
}
FmodResult FmodConfigurationLibrary::channel_set_loop_count(void* channel, std::int32_t count) {
    return impl_->call("FMOD_Channel_SetLoopCount", channel, count);
}
FmodResult FmodConfigurationLibrary::channel_set_loop_points(void* channel, std::uint32_t start,
    std::uint32_t start_unit, std::uint32_t end, std::uint32_t end_unit) {
    return impl_->call("FMOD_Channel_SetLoopPoints", channel, start, start_unit, end, end_unit);
}
FmodResult FmodConfigurationLibrary::channel_set_speaker_mix(void* channel, const std::array<float, 8>& mix) {
    return impl_->call("FMOD_Channel_SetSpeakerMix", channel, mix[0], mix[1], mix[2], mix[3], mix[4], mix[5], mix[6], mix[7]);
}
FmodResult FmodConfigurationLibrary::channel_stop(void* channel) {
    return impl_->call("FMOD_Channel_Stop", channel);
}
FmodResult FmodConfigurationLibrary::channel_is_virtual(void* channel, std::optional<bool>& value) {
    // The C++ imports use bool*. The installed C API uses a four-byte FMOD_BOOL.
    // Preserve an unwritten native output instead of inventing false on error.
    std::int32_t native = value ? static_cast<std::int32_t>(*value) : -1;
    const auto result = impl_->call("FMOD_Channel_IsVirtual", channel, &native);
    if (native != -1) value = native != 0;
    return result;
}
FmodResult FmodConfigurationLibrary::channel_is_playing(void* channel, bool* value) {
    std::int32_t native = static_cast<std::int32_t>(*value);
    const auto result = impl_->call("FMOD_Channel_IsPlaying", channel, &native);
    *value = native != 0;
    return result;
}
FmodResult FmodConfigurationLibrary::channel_set_frequency(void* channel, float value) {
    return impl_->call("FMOD_Channel_SetFrequency", channel, value);
}
FmodResult FmodConfigurationLibrary::channel_set_volume(void* channel, float value) {
    return impl_->call("FMOD_Channel_SetVolume", channel, value);
}
FmodResult FmodConfigurationLibrary::channel_set_3d_pan_level(void* channel, float value) {
    return impl_->call("FMOD_Channel_Set3DPanLevel", channel, value);
}
FmodResult FmodConfigurationLibrary::channel_set_paused(void* channel, std::uint8_t value) {
    return impl_->call("FMOD_Channel_SetPaused", channel, static_cast<std::int32_t>(value));
}
FmodResult FmodConfigurationLibrary::channel_get_audibility(void* channel, float* value) {
    return impl_->call("FMOD_Channel_GetAudibility", channel, value);
}
FmodResult FmodConfigurationLibrary::sound_get_num_subsounds(void* sound,
    std::int32_t* count) {
    return impl_->call("FMOD_Sound_GetNumSubSounds", sound, count);
}
FmodResult FmodConfigurationLibrary::sound_get_subsound(void* sound,
    std::int32_t index, void** subsound) {
    return impl_->call("FMOD_Sound_GetSubSound", sound, index, subsound);
}
FmodResult FmodConfigurationLibrary::sound_get_mode(void* sound, std::uint32_t* mode) {
    return impl_->call("FMOD_Sound_GetMode", sound, mode);
}
FmodResult FmodConfigurationLibrary::sound_get_defaults(void* sound,
    float* frequency, float* volume, float* pan, std::int32_t* priority) {
    return impl_->call("FMOD_Sound_GetDefaults", sound, frequency, volume, pan, priority);
}
FmodResult FmodConfigurationLibrary::sound_set_3d_min_max_distance(void* sound,
    float minimum, float maximum) {
    return impl_->call("FMOD_Sound_Set3DMinMaxDistance", sound, minimum, maximum);
}
FmodResult FmodConfigurationLibrary::sound_set_variations(void* sound,
    float frequency, float volume, float pan) {
    return impl_->call("FMOD_Sound_SetVariations", sound, frequency, volume, pan);
}
FmodResult FmodConfigurationLibrary::sound_set_loop_points(void* sound,
    std::uint32_t start, std::uint32_t start_unit, std::uint32_t end,
    std::uint32_t end_unit) {
    return impl_->call("FMOD_Sound_SetLoopPoints", sound, start, start_unit, end, end_unit);
}
FmodResult FmodConfigurationLibrary::sound_set_mode(void* sound, std::uint32_t mode) {
    return impl_->call("FMOD_Sound_SetMode", sound, mode);
}
FmodResult FmodConfigurationLibrary::event_system_load(void* system, const char* path,
    void* load_info, void** project) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventSystem_Load@16", system,
        path, load_info, project);
}
FmodResult FmodConfigurationLibrary::event_project_get_group(void* project, const char* name,
    std::int32_t cache_events, void** group) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventProject_GetGroup@16", project, name, cache_events, group);
}
FmodResult FmodConfigurationLibrary::event_group_get_group(void* parent, const char* name,
    std::int32_t cache_events, void** group) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventGroup_GetGroup@16", parent, name, cache_events, group);
}
FmodResult FmodConfigurationLibrary::event_group_load_event_data(void* group,
    std::uint32_t resources, std::uint32_t mode) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventGroup_LoadEventData@12", group, resources, mode);
}
FmodResult FmodConfigurationLibrary::event_group_free_event_data(void* group, void* event, std::int32_t wait) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventGroup_FreeEventData@12", group, event, wait);
}
FmodResult FmodConfigurationLibrary::event_system_get_event(void* system, const char* name,
    std::uint32_t mode, void** event) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventSystem_GetEvent@16", system, name, mode, event);
}
FmodResult FmodConfigurationLibrary::event_get_num_parameters(void* event, std::int32_t* count) {
    return impl_->call_from(impl_->event_library(), "_FMOD_Event_GetNumParameters@8", event, count);
}
FmodResult FmodConfigurationLibrary::event_get_parameter_by_index(void* event, std::int32_t index, void** parameter) {
    return impl_->call_from(impl_->event_library(), "_FMOD_Event_GetParameterByIndex@12", event, index, parameter);
}
FmodResult FmodConfigurationLibrary::event_parameter_get_range(void* parameter, float* minimum, float* maximum) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventParameter_GetRange@12", parameter, minimum, maximum);
}
FmodResult FmodConfigurationLibrary::event_parameter_get_info(void* parameter, std::int32_t* index, char** name) {
    return impl_->call_from(impl_->event_library(), "_FMOD_EventParameter_GetInfo@12", parameter, index, name);
}
FmodResult FmodConfigurationLibrary::system_get_num_drivers(void* system,
    std::int32_t* count) {
    return impl_->call("FMOD_System_GetNumDrivers", system, count);
}
FmodResult FmodConfigurationLibrary::system_get_driver_caps(void* system,
    std::int32_t index, FmodDriverCaps* caps) {
    return impl_->call("FMOD_System_GetDriverCaps", system, index, &caps->caps,
        &caps->min_frequency, &caps->max_frequency, &caps->control_panel_speaker_mode);
}
FmodResult FmodConfigurationLibrary::system_set_speaker_mode(void* system,
    FmodSpeakerMode mode) {
    return impl_->call("FMOD_System_SetSpeakerMode", system, static_cast<std::uint32_t>(mode));
}
FmodResult FmodConfigurationLibrary::system_set_output(void* system, FmodOutputType type) {
    return set_output(system, type);
}
FmodResult FmodConfigurationLibrary::system_set_file_system(void* system,
    const FmodFileSystemHooks& hooks) {
    const auto native = sound_system_file_system_hooks();
    if (hooks.user_open != native.user_open || hooks.user_close != native.user_close ||
        hooks.user_read != native.user_read || hooks.user_seek != native.user_seek)
        throw std::invalid_argument("Unsupported FMOD callback provenance");
    // Integer image addresses identify the recovered callback set only. The DLL
    // receives callable reconstructed entry points with their original stack ABI.
    const auto& callbacks = impl_->file_callbacks;
    return impl_->call("FMOD_System_SetFileSystem", system, callbacks.open,
        callbacks.close, callbacks.read, callbacks.seek, hooks.block_align);
}
FmodResult FmodConfigurationLibrary::system_set_3d_settings(void* system,
    const Fmod3DSettings& settings) {
    return impl_->call("FMOD_System_Set3DSettings", system, settings.doppler_scale,
        settings.distance_factor, settings.rolloff_scale);
}
FmodResult FmodConfigurationLibrary::system_get_driver(void* system, std::int32_t* driver) {
    return impl_->call("FMOD_System_GetDriver", system, driver);
}
FmodResult FmodConfigurationLibrary::system_get_output(void* system, FmodOutputType* output) {
    return impl_->call("FMOD_System_GetOutput", system, output);
}
FmodResult FmodConfigurationLibrary::system_get_speaker_mode(void* system,
    FmodSpeakerMode* mode) {
    return impl_->call("FMOD_System_GetSpeakerMode", system, mode);
}
void FmodConfigurationLibrary::on_out_of_sound_memory(const char*) {
    std::int32_t current{}, maximum{};
    memory_get_stats(&current, &maximum);
}
FmodResult FmodConfigurationLibrary::get_master_channel_group(void* system, void** group) {
    return impl_->call("FMOD_System_GetMasterChannelGroup", system, group);
}
FmodResult FmodConfigurationLibrary::get_advanced_settings(void* system,
    SoundFmodAdvancedSettings& settings) {
    return impl_->call("FMOD_System_GetAdvancedSettings", system, &settings);
}
FmodResult FmodConfigurationLibrary::set_advanced_settings(void* system,
    const SoundFmodAdvancedSettings& settings) {
    return impl_->call("FMOD_System_SetAdvancedSettings", system, &settings);
}
FmodResult FmodConfigurationLibrary::create_channel_group(void* system,
    const char* name, void** group) {
    return impl_->call("FMOD_System_CreateChannelGroup", system, name, group);
}
FmodResult FmodConfigurationLibrary::set_channel_group_volume(void* group, float value) {
    return impl_->call("FMOD_ChannelGroup_SetVolume", group, value);
}
FmodResult FmodConfigurationLibrary::set_channel_group_pitch(void* group, float value) {
    return impl_->call("FMOD_ChannelGroup_SetPitch", group, value);
}
FmodResult FmodConfigurationLibrary::create_dsp_by_type(void* system,
    std::int32_t type, void** dsp) {
    return impl_->call("FMOD_System_CreateDSPByType", system, type, dsp);
}
FmodResult FmodConfigurationLibrary::set_dsp_parameter(void* dsp,
    std::int32_t index, float value) {
    return impl_->call("FMOD_DSP_SetParameter", dsp, index, value);
}
FmodResult FmodConfigurationLibrary::add_channel_group_dsp(void* group,
    void* dsp, void** connection) {
    return impl_->call("FMOD_ChannelGroup_AddDSP", group, dsp, connection);
}
FmodResult FmodConfigurationLibrary::add_system_dsp(void* system,
    void* dsp, void** connection) {
    return impl_->call("FMOD_System_AddDSP", system, dsp, connection);
}
void FmodConfigurationLibrary::memory_get_stats(std::int32_t* current,
    std::int32_t* maximum) {
    impl_->call("FMOD_Memory_GetStats", current, maximum);
}
int FmodConfigurationLibrary::compare_class_name_case_insensitive(
    const char* left, const char* right) {
    return _stricmp(left, right);
}
const std::vector<FmodConfigurationCall>& FmodConfigurationLibrary::calls() const noexcept {
    return impl_->results;
}

} // namespace bsp
