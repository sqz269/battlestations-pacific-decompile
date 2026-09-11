#include "bsp/sound_configuration.hpp"
#include "bsp/sound_class_ownership.hpp"

#include <algorithm>
#include <cstring>
#include <memory>
#include <utility>

namespace bsp {
namespace {

struct LuaRefOwner {
    GuiLuaHost& lua;
    GuiLuaRef ref;
    ~LuaRefOwner() { lua.release(ref); }
};

template<class Action>
void each_lua_entry(GuiLuaHost& lua, const GuiLuaRef& table, Action action)
{
    GuiLuaRef key, value;
    bool restart = true;
    while (lua.next(table, key, value, restart)) {
        LuaRefOwner key_owner{lua, key};
        LuaRefOwner value_owner{lua, value};
        restart = false;
        action(key, value);
    }
}

float number_or_default(GuiLuaHost& lua, const GuiLuaRef& config,
    const char* key, float fallback)
{
    LuaRefOwner value{lua, lua.get_by_name(config, key)};
    // 00B66330 requires LUA_TNUMBER; numeric strings do not override defaults.
    return lua.type_of(value.ref) == GuiLuaType::Number
        ? static_cast<float>(lua.to_number(value.ref)) : fallback;
}

void check_memory(FmodResult result, SoundConfigurationFmodHost& fmod)
{
    if (result == FmodResult::err_memory) {
        std::int32_t current{}, maximum{};
        fmod.memory_get_stats(&current, &maximum);
    }
}

template<class Call>
void call_with_number(GuiLuaHost& lua, const GuiLuaRef& config,
    const char* key, float fallback, SoundConfigurationFmodHost& fmod, Call call)
{
    FmodResult result;
    {
        LuaRefOwner value{lua, lua.get_by_name(config, key)};
        const float number = lua.type_of(value.ref) == GuiLuaType::Number
            ? static_cast<float>(lua.to_number(value.ref)) : fallback;
        result = call(number);
    } // native releases the looked-up LuaObject before checking FMOD_RESULT
    check_memory(result, fmod);
}

bool listener_matches(const std::string& stored, const char* name,
    SoundLevelNameHost& names)
{
    if (stored.empty()) return !name || !*name;
    if (!name) return false;
    return names.compare_class_name_case_insensitive(stored.c_str(), name) == 0;
}

struct NativeName {
    NativeString value;
    explicit NativeName(const char* text)
    {
        value.assign_0041e870(crt_string_storage(), text);
    }
    ~NativeName() { value.release_to(crt_string_storage()); }
};

void append_sound_listener_00a7f9f0(SoundConfigurationState& state, const char* name,
    SoundLevelNameHost& names)
{
    // 00A7F9F0 compares all equal-length nonempty names but ignores results:
    // duplicate listener entries are still appended.
    const std::string copied = name ? name : "";
    for (const auto& existing : state.listeners_ac) {
        if (existing.size() == copied.size() && !existing.empty())
            (void)names.compare_class_name_case_insensitive(
                existing.c_str(), copied.c_str());
    }
    if (state.listeners_ac.size() ==
        static_cast<std::size_t>(state.listener_capacity_b4)) {
        state.listener_capacity_b4 = std::max(2 * state.listener_capacity_b4, 1);
        state.listeners_ac.reserve(state.listener_capacity_b4);
    }
    state.listeners_ac.push_back(copied);
}

struct DspParameter {
    const char* key;
    float fallback;
};

// Each array is in native FMOD parameter-index order, including the unusual
// positive ReflectionsLevel default. Constants checked against image bytes.
constexpr DspParameter lowpass[]{{"Cutoff", 5000.0f}, {"Resonance", 1.0f}};
constexpr DspParameter lowpass_simple[]{{"Cutoff", 5000.0f}};
constexpr DspParameter distortion[]{{"Level", 0.5f}};
constexpr DspParameter compressor[]{
    {"Threshold", 0.0f}, {"Attack", 50.0f}, {"Release", 50.0f}, {"Gain", 0.0f}};
constexpr DspParameter reverb[]{
    {"DryLevel", 0.0f}, {"Room", 0.0f}, {"RoomHF", 0.0f},
    {"RoomRolloff", 10.0f}, {"DecayTime", 1.0f}, {"DecayHFRatio", 0.5f},
    {"ReflectionsLevel", 10000.0f}, {"ReflectionsDelay", 0.02f},
    {"ReverbLevel", 0.0f}, {"ReverbDelay", 0.04f}, {"Diffusion", 100.0f},
    {"Density", 100.0f}, {"HFReference", 5000.0f}, {"RoomLF", 0.0f},
    {"LFReference", 250.0f}};

struct DspPolicy {
    const char* name;
    std::int32_t type;
    const DspParameter* parameters;
    std::int32_t count;
};
constexpr DspPolicy dsp_policies[]{
    {"Lowpass", 3, lowpass, 2}, {"LowpassSimple", 19, lowpass_simple, 1},
    {"Distortion", 8, distortion, 1}, {"Compressor", 17, compressor, 4},
    {"SFXReverb", 18, reverb, 15}};

void configure_system_dsps_00a7e0d0(void* system, SoundConfigurationState& state,
    const GuiLuaRef& config, SoundConfigurationFmodHost& fmod, GuiLuaHost& lua)
{
    // 00A7E0D0 explicitly checks DSP is a table. The outer dictionaries use
    // the established LuaObject iterator's behavior instead.
    if (lua.type_of(config) != GuiLuaType::Table) return;
    each_lua_entry(lua, config, [&](const GuiLuaRef&, const GuiLuaRef& value) {
        void* dsp = create_configured_sound_dsp_00a7c740(system, value, fmod, lua);
        if (!dsp) return;
        if (state.system_dsps_134.size() ==
            static_cast<std::size_t>(state.system_dsp_capacity_13c)) {
            state.system_dsp_capacity_13c =
                std::max(state.system_dsp_capacity_13c + 16, 16);
            state.system_dsps_134.reserve(state.system_dsp_capacity_13c);
        }
        state.system_dsps_134.push_back(dsp);
        check_memory(fmod.add_system_dsp(system, dsp, nullptr), fmod);
    });
}

void configure_channel_group_00a7de10(void* system, SoundConfiguredChannelGroup& group,
    const char* name, const GuiLuaRef& config,
    SoundConfigurationFmodHost& fmod, GuiLuaHost& lua)
{
    // 00A7DE10: store the name, create the group, then Volume, Pitch, DSP.
    group.name = name;
    check_memory(fmod.create_channel_group(system, group.name.c_str(),
        &group.channel_group), fmod);
    call_with_number(lua, config, "Volume", 1.0f, fmod, [&](float value) {
        return fmod.set_channel_group_volume(group.channel_group, value);
    });
    call_with_number(lua, config, "Pitch", 1.0f, fmod, [&](float value) {
        return fmod.set_channel_group_pitch(group.channel_group, value);
    });
    LuaRefOwner dsps{lua, lua.get_by_name(config, "DSP")};
    if (lua.type_of(dsps.ref) != GuiLuaType::Table) return;
    each_lua_entry(lua, dsps.ref, [&](const GuiLuaRef&, const GuiLuaRef& value) {
        void* dsp = create_configured_sound_dsp_00a7c740(system, value, fmod, lua);
        if (!dsp) return;
        if (group.dsps.size() == static_cast<std::size_t>(group.dsp_capacity)) {
            group.dsp_capacity = std::max(group.dsp_capacity + 4, 8);
            group.dsps.reserve(group.dsp_capacity);
        }
        group.dsps.push_back(dsp);
        check_memory(fmod.add_channel_group_dsp(group.channel_group, dsp, nullptr), fmod);
    });
}

std::uint32_t float_bits(float value)
{
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

} // namespace

void* create_configured_sound_dsp_00a7c740(void* system,
    const GuiLuaRef& config, SoundConfigurationFmodHost& fmod, GuiLuaHost& lua)
{
    std::string type;
    {
        LuaRefOwner type_ref{lua, lua.get_by_name(config, "Type")};
        // NativeString constructor calls strlen: Type must be string-convertible.
        type = lua.to_string(type_ref.ref);
    }
    // An empty NativeString has a null buffer. Native skips every CRT compare.
    if (type.empty()) return nullptr;
    for (const auto& policy : dsp_policies) {
        if (fmod.compare_class_name_case_insensitive(type.c_str(), policy.name) != 0)
            continue;
        void* dsp{};
        check_memory(fmod.create_dsp_by_type(system, policy.type, &dsp), fmod);
        // The handle, not FMOD_RESULT, gates the parameter calls.
        if (!dsp) return nullptr;
        for (std::int32_t index = 0; index < policy.count; ++index) {
            const auto& parameter = policy.parameters[index];
            call_with_number(lua, config, parameter.key, parameter.fallback,
                fmod, [&](float value) { return fmod.set_dsp_parameter(dsp, index, value); });
        }
        return dsp;
    }
    return nullptr;
}

std::int32_t find_sound_listener_00a7ae00(const SoundConfigurationState& state,
    const char* name, SoundLevelNameHost& names)
{
    for (std::size_t index = 0; index < state.listeners_ac.size(); ++index)
        if (listener_matches(state.listeners_ac[index], name, names))
            return static_cast<std::int32_t>(index);
    return 0;
}

void select_sound_listener_00a7ae80(SoundConfigurationState& state,
    const char* name, SoundLevelNameHost& names)
{
    state.selected_listener_104 = -1;
    for (std::size_t index = 0; index < state.listeners_ac.size(); ++index) {
        if (!listener_matches(state.listeners_ac[index], name, names)) continue;
        state.selected_listener_104 = static_cast<std::int32_t>(index);
        state.selected_listener_108 = static_cast<std::int32_t>(index);
        return;
    }
}

void* find_sound_channel_group_00a7b120(const SoundConfigurationState& state,
    const NativeString& name, SoundLevelNameHost& names)
{
    for (const auto& group : state.channel_groups_128) {
        if (group.name.size() != name.length()) continue;
        if (group.name.empty() || names.compare_class_name_case_insensitive(
            group.name.c_str(), name.data()) == 0) return group.channel_group;
    }
    return nullptr;
}

void apply_sound_configuration_lua_00a7ff80_fragment(SoundSystemState& system,
    SoundConfigurationState& state, SoundClassOwnership& classes,
    SoundConfigurationFmodHost& fmod, GuiLuaHost& lua)
{
    struct ScalarField { const char* key; float SoundConfigurationScalars::* field; };
    constexpr ScalarField fields[]{
        {"PlayerSoundBoost", &SoundConfigurationScalars::player_sound_boost_04},
        {"HRTFMinAngle", &SoundConfigurationScalars::hrtf_min_angle_08},
        {"HRTFMaxAngle", &SoundConfigurationScalars::hrtf_max_angle_0c},
        {"HRTFFreq", &SoundConfigurationScalars::hrtf_frequency_10},
        {"DopplerConstant", &SoundConfigurationScalars::doppler_constant_14},
        {"DopplerMin", &SoundConfigurationScalars::doppler_min_18},
        {"DopplerMax", &SoundConfigurationScalars::doppler_max_1c},
        {"SoundDistance1", &SoundConfigurationScalars::sound_distance_1_20},
        {"SoundDistance2", &SoundConfigurationScalars::sound_distance_2_24},
        {"SoundDistance3", &SoundConfigurationScalars::sound_distance_3_28},
        {"SoundDistance4", &SoundConfigurationScalars::sound_distance_4_2c},
        {"PassDistance", &SoundConfigurationScalars::pass_distance_30},
        {"PassTime", &SoundConfigurationScalars::pass_time_34}};
    for (const auto& field : fields) {
        LuaRefOwner globals{lua, lua.globals()};
        LuaRefOwner value{lua, lua.get_by_name(globals.ref, field.key)};
        if (lua.type_of(value.ref) == GuiLuaType::Number)
            state.scalars.*field.field = static_cast<float>(lua.to_number(value.ref));
    }

    SoundFmodAdvancedSettings advanced;
    advanced.words[0] = 0x44;
    check_memory(fmod.get_advanced_settings(system.system, advanced), fmod);
    // 00A807DA..00A80810: account for two intervening PUSH instructions.
    advanced.words[0x24 / 4] = float_bits(state.scalars.hrtf_min_angle_08);
    advanced.words[0x28 / 4] = float_bits(state.scalars.hrtf_max_angle_0c);
    advanced.words[0x2c / 4] = float_bits(state.scalars.hrtf_frequency_10);
    check_memory(fmod.set_advanced_settings(system.system, advanced), fmod);

    {
        LuaRefOwner globals{lua, lua.globals()};
        LuaRefOwner groups{lua, lua.get_by_name(globals.ref, "Channelgroups")};
        each_lua_entry(lua, groups.ref, [&](const GuiLuaRef& key, const GuiLuaRef& value) {
            const bool is_system = lua.type_of(key) == GuiLuaType::String &&
                std::strcmp(lua.to_string(key), "System") == 0;
            if (is_system) {
                LuaRefOwner dsp{lua, lua.get_by_name(value, "DSP")};
                configure_system_dsps_00a7e0d0(system.system, state, dsp.ref, fmod, lua);
                return;
            }
            const std::string name(lua.to_string(key));
            if (state.channel_groups_128.size() ==
                static_cast<std::size_t>(state.channel_group_capacity_130)) {
                state.channel_group_capacity_130 =
                    std::max(state.channel_group_capacity_130 + 16, 16);
                state.channel_groups_128.reserve(state.channel_group_capacity_130);
            }
            // Count is increased before 00A7DE10 runs, including on FMOD error.
            state.channel_groups_128.emplace_back();
            configure_channel_group_00a7de10(system.system, state.channel_groups_128.back(),
                name.c_str(), value, fmod, lua);
        });
    }
    {
        LuaRefOwner globals{lua, lua.globals()};
        LuaRefOwner listeners{lua, lua.get_by_name(globals.ref, "Listeners")};
        each_lua_entry(lua, listeners.ref, [&](const GuiLuaRef&, const GuiLuaRef& value) {
            const char* text = lua.to_string(value);
            const std::string name = text ? text : "";
            append_sound_listener_00a7f9f0(state, name.c_str(), fmod);
            const auto index = find_sound_listener_00a7ae00(state, name.c_str(), fmod);
            if (state.current_listener_10c != index) {
                state.previous_listener_110 = state.current_listener_10c;
                state.current_listener_10c = index;
                select_sound_listener_00a7ae80(state, name.c_str(), fmod);
            }
        });
    }
    {
        LuaRefOwner globals{lua, lua.globals()};
        LuaRefOwner categories{lua, lua.get_by_name(globals.ref, "Categories")};
        each_lua_entry(lua, categories.ref, [&](const GuiLuaRef&, const GuiLuaRef& value) {
            auto release = [](SoundClassDescriptor* descriptor) { descriptor->release(); };
            std::unique_ptr<SoundClassDescriptor, decltype(release)> descriptor(
                create_sound_class_00a7c350(), release);
            {
                LuaRefOwner name{lua, lua.get_by_name(value, "Name")};
                const char* text = lua.to_string(name.ref);
                const auto length = text ? static_cast<std::uint32_t>(std::strlen(text)) : 0u;
                descriptor->name().resize_0041dd40(descriptor->name_storage(), length, false);
                if (descriptor->name().data())
                    std::memcpy(descriptor->name().data(), text, length);
            }
            descriptor->volume_10 = number_or_default(lua, value, "Volume", 1.0f);
            descriptor->retain(); // native stack smart-pointer temporary
            descriptor->class_index_08 = static_cast<std::uint32_t>(
                classes.manager().classes_98.size());
            try { classes.append_retained(*descriptor); }
            catch (...) { descriptor->release(); throw; }
            descriptor->release(); // table retains its reference; creator below
        });
    }
    {
        LuaRefOwner globals{lua, lua.globals()};
        LuaRefOwner types{lua, lua.get_by_name(globals.ref, "SoundTypes")};
        each_lua_entry(lua, types.ref, [&](const GuiLuaRef& key, const GuiLuaRef& value) {
            SoundConfiguredType type;
            if (const char* text = lua.to_string(key)) type.name = text;
            each_lua_entry(lua, value, [&](const GuiLuaRef& listener, const GuiLuaRef& group) {
                NativeName listener_name(lua.to_string(listener));
                NativeName group_name(lua.to_string(group));
                const char* name = listener_name.value.data();
                const auto index = find_sound_listener_00a7ae00(state, name ? name : "", fmod);
                if (type.by_listener.size() <= static_cast<std::size_t>(index))
                    type.by_listener.resize(static_cast<std::size_t>(index) + 1, nullptr);
                type.by_listener[index] = find_sound_channel_group_00a7b120(
                    state, group_name.value, fmod);
            });
            if (state.types_38.size() == static_cast<std::size_t>(state.type_capacity_40)) {
                state.type_capacity_40 = std::max(2 * state.type_capacity_40, 1);
                state.types_38.reserve(state.type_capacity_40);
            }
            state.types_38.push_back(std::move(type));
        });
    }
}

void initialize_sound_configuration_00a7ff80(SoundSystemState& system,
    SoundConfigurationState& state, SoundClassOwnership& classes,
    SoundConfigurationFmodHost& fmod, SoundConfigurationLuaOwner& lua_owner)
{
    check_memory(fmod.get_master_channel_group(system.system,
        &state.master_channel_group_140), fmod);
    GuiLuaHost& lua = lua_owner.construct_and_open(1);
    struct CloseOwner {
        SoundConfigurationLuaOwner& owner;
        ~CloseOwner() { owner.close(); }
    } close{lua_owner};
    lua_owner.load_and_call_chunk(kSoundConfigurationPath, 0);
    apply_sound_configuration_lua_00a7ff80_fragment(system, state, classes, fmod, lua);
}

} // namespace bsp
