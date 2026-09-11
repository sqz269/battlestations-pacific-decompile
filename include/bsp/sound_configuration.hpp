#pragma once

#include "bsp/audio_online_startup.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/sound_manager_levels.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

class SoundClassOwnership;

// Behavioral projections, not native object layouts. Names are hypotheses.
// Evidence, original ABIs and remaining boundaries: SOUND_CONFIGURATION.md.
struct SoundConfigurationScalars {
    // Defaults are the stores in 00A814A4..00A81554, not Lua fallbacks.
    float player_sound_boost_04{1.0f};
    float hrtf_min_angle_08{180.0f};
    float hrtf_max_angle_0c{360.0f};
    float hrtf_frequency_10{5000.0f};
    float doppler_constant_14{2000.0f};
    float doppler_min_18{0.5f};
    float doppler_max_1c{2.0f};
    float sound_distance_1_20{800.0f};
    float sound_distance_2_24{1200.0f};
    float sound_distance_3_28{1800.0f};
    float sound_distance_4_2c{2200.0f};
    float pass_distance_30{200.0f};
    float pass_time_34{3.0f};
};

// The exact 44h byte buffer this executable passes to the FMOD library.
// Other fields (including native pointer words) pass through unmodified.
struct SoundFmodAdvancedSettings {
    std::array<std::uint32_t, 17> words{};
};
static_assert(sizeof(SoundFmodAdvancedSettings) == 0x44);

struct SoundConfiguredChannelGroup {
    std::string name;                 // native NativeString at record +00
    void* channel_group{};            // native record +08
    std::vector<void*> dsps;          // native record +0C/+10/+14
    std::int32_t dsp_capacity{};
};

struct SoundConfiguredType {
    std::string name;                 // native 14h record +00/+04
    std::vector<void*> by_listener;   // native +08/+0C/+10, null-filled holes
};

struct SoundConfigurationState {
    SoundConfigurationScalars scalars;
    std::vector<SoundConfiguredType> types_38;
    std::int32_t type_capacity_40{};
    std::vector<std::string> listeners_ac; // manager+A4, array header +08
    std::int32_t listener_capacity_b4{};
    // The native selected pointer at +104 is projected as a stable ordinal.
    // -1 represents null. The native +108 ordinal survives an unsuccessful select.
    std::int32_t selected_listener_104{-1};
    std::int32_t selected_listener_108{};
    std::int32_t current_listener_10c{-1}; // 00A815EF
    std::int32_t previous_listener_110{}; // caller value until the first change
    std::vector<SoundConfiguredChannelGroup> channel_groups_128;
    std::int32_t channel_group_capacity_130{};
    std::vector<void*> system_dsps_134;
    std::int32_t system_dsp_capacity_13c{};
    void* master_channel_group_140{};
};

// These are genuine external FMOD/CRT calls, with all arguments recovered
// from the instruction listing. No DSP implementation is reconstructed here.
class SoundConfigurationFmodHost : public SoundLevelNameHost {
public:
    virtual FmodResult get_master_channel_group(void* system, void** group) = 0;
    virtual FmodResult get_advanced_settings(void* system,
        SoundFmodAdvancedSettings& settings) = 0;
    virtual FmodResult set_advanced_settings(void* system,
        const SoundFmodAdvancedSettings& settings) = 0;
    virtual FmodResult create_channel_group(void* system, const char* name,
        void** group) = 0;
    virtual FmodResult set_channel_group_volume(void* group, float volume) = 0;
    virtual FmodResult set_channel_group_pitch(void* group, float pitch) = 0;
    virtual FmodResult create_dsp_by_type(void* system, std::int32_t type,
        void** dsp) = 0;
    virtual FmodResult set_dsp_parameter(void* dsp, std::int32_t index,
        float value) = 0;
    virtual FmodResult add_channel_group_dsp(void* group, void* dsp,
        void** connection) = 0;
    virtual FmodResult add_system_dsp(void* system, void* dsp,
        void** connection) = 0;
    // 00C2DDEE: this build pushes exactly two output pointers. Return ignored.
    virtual void memory_get_stats(std::int32_t* current, std::int32_t* maximum) = 0;
};

// Existing Lua owner/VFS boundary: 00B66BD0/00B6A020, 00B66CA0, 00B669A0.
// Open constructs a fresh owner and opens mask 1 (base library only). The
// loader retains its native unprotected error policy; no boolean is checked.
// GuiLua51Host can supply the reader operations without duplicating Lua.
class SoundConfigurationLuaOwner {
public:
    virtual ~SoundConfigurationLuaOwner() = default;
    virtual GuiLuaHost& construct_and_open(std::uint32_t library_mask) = 0;
    virtual void load_and_call_chunk(const char* path, std::int32_t argument) = 0;
    virtual void close() noexcept = 0;
};

inline constexpr char kSoundConfigurationPath[] = "sound/soundsetup.lua";

// 00A7FF80: native __thiscall(ECX=manager), RET, no stack arguments. Runs
// master-group query, Lua loading, scalar/advanced/group/listener/category/type
// configuration and Lua close in that order. Existing state is appended to,
// and absent/non-number scalar values leave existing values unchanged.
void initialize_sound_configuration_00a7ff80(SoundSystemState& system,
    SoundConfigurationState& state, SoundClassOwnership& classes,
    SoundConfigurationFmodHost& fmod, SoundConfigurationLuaOwner& lua_owner);

// 00A800D3..00A813F8: same recovered policy with an already loaded Lua host.
// Does not query the master group or own/close the supplied interpreter.
void apply_sound_configuration_lua_00a7ff80_fragment(SoundSystemState& system,
    SoundConfigurationState& state, SoundClassOwnership& classes,
    SoundConfigurationFmodHost& fmod, GuiLuaHost& lua);

// 00A7C740: native __thiscall(manager, LuaObject*), RET4, DSP* in EAX.
// Only five case-insensitive Type strings are accepted; unknown -> null.
void* create_configured_sound_dsp_00a7c740(void* system,
    const GuiLuaRef& config, SoundConfigurationFmodHost& fmod, GuiLuaHost& lua);

// 00A7AE00: __thiscall(manager+A4, const char*), RET4. First match or 0.
std::int32_t find_sound_listener_00a7ae00(const SoundConfigurationState& state,
    const char* name, SoundLevelNameHost& names);

// 00A7AE80: __thiscall(manager+A4, const char*), RET4. Clears selected pointer
// first; matching name updates pointer and ordinal, miss preserves ordinal.
void select_sound_listener_00a7ae80(SoundConfigurationState& state,
    const char* name, SoundLevelNameHost& names);

// 00A7B120: __thiscall(manager, NativeString*), RET4; length then __stricmp.
void* find_sound_channel_group_00a7b120(const SoundConfigurationState& state,
    const NativeString& name, SoundLevelNameHost& names);

} // namespace bsp
