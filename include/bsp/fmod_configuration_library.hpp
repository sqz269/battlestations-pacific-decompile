#pragma once

#include "bsp/sound_configuration.hpp"

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
class FmodConfigurationLibrary final : public SoundConfigurationFmodHost {
public:
    explicit FmodConfigurationLibrary(const std::wstring& dll_path);
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
