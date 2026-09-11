#include "bsp/fmod_configuration_library.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstring>
#include <stdexcept>

namespace bsp {

struct FmodConfigurationLibrary::Impl {
    HMODULE module{};
    std::vector<FmodConfigurationCall> results;

    explicit Impl(const std::wstring& path) {
        static_assert(sizeof(void*) == 4, "The installed FMOD ABI is Win32.");
        module = LoadLibraryExW(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!module) throw std::runtime_error("Cannot load FMOD DLL: Win32 error " +
            std::to_string(GetLastError()));
    }
    ~Impl() { if (module) FreeLibrary(module); }

    template<class... Args>
    FmodResult call(const char* name, Args... args) {
        // The installed DLL exports undecorated aliases for its stdcall C API.
        // Original game call-site stack cleanup establishes this ABI;
        // no native image addresses are called here.
        using Function = std::uint32_t (__stdcall*)(Args...);
        const FARPROC address = GetProcAddress(module, name);
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
    : impl_(std::make_unique<Impl>(path)) {}
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
