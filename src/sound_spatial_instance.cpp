#include "bsp/sound_spatial_instance.hpp"
#include "bsp/sound_resource_asset.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_append.hpp"
#include "bsp/native_string_compare.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t offset) noexcept {
    T value; std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, sizeof value); return value;
}
float difference(float a, float b) noexcept {
    float value;
    __asm { fld a }
    __asm { fsub b }
    __asm { fstp value }
    return value;
}
float product(float a, float b) noexcept {
    float value;
    __asm { fld a }
    __asm { fmul b }
    __asm { fstp value }
    return value;
}
struct Text {
    NativeString value;
    NativeStringStorage& storage;
    explicit Text(NativeStringStorage& s) noexcept : storage(s) {}
    ~Text() { destroy_native_string_header_0041dd20(&value, storage); }
};
void require_bank(const SoundChannelInstance& s) {
    if (s.native_vtable_00 != 0x00d5abf8 && s.native_vtable_00 != 0x00d5b510)
        throw std::logic_error("Unbound bank sound-channel virtual table");
}
}
bool BankSoundChannelVirtuals::query_slot_14(SoundChannelInstance& s, SoundInstanceContext& c) {
    require_bank(s); return sound_channel_nonvirtual_00a7a630(s, c);
}
void BankSoundChannelVirtuals::refresh_slot_10(SoundChannelInstance& s, SoundInstanceContext& c) {
    require_bank(s); refresh_sound_channel_ended_00a7a660(s, c);
}
SpatialSoundChannelInstance& construct_spatial_sound_channel_00a8a2e0(
    SpatialSoundChannelInstance& s, void* sample, SoundClassLevel* const* cls,
    std::uint32_t type, std::uint8_t flag, std::array<float, 3> position,
    std::array<float, 3> velocity, SoundInstanceContext& c) {
    s.velocity_68 = {}; s.position_5c = {}; s.velocity_68 = s.position_5c; s.distance_band_78 = 0;
    construct_sound_channel_00a7d560(s, sample, cls, type, flag, c);
    s.native_vtable_00 = 0x00d5b510; s.channel_group_7c = nullptr; s.dsp_80 = nullptr;
    s.velocity_68 = velocity; s.position_5c = position; s.spatial_dirty_74 = 1;
    return s;
}
SpatialSoundChannelInstance* create_spatial_bank_sound_00a7f710_fragment(
    SoundSystemOwner& owner, void* sample, std::int32_t cls, std::uint32_t type,
    std::uint8_t flag, SoundInstanceContext& c) {
    ++owner.words_160[0];
    if (!sound_sample_has_fmod_sound_00a818c0(sample))
        throw std::logic_error("A7F710 event-instance branch is not bound");
    auto* s = new(std::nothrow) SpatialSoundChannelInstance;
    if (!s) return nullptr;
    try {
        if (cls < 0 || cls == INT32_MAX) throw std::out_of_range("Sound class index");
        if (static_cast<std::size_t>(cls) >= owner.levels.classes_98.size()) owner.classes.resize_00a7c2c0(cls + 1);
        construct_spatial_sound_channel_00a8a2e0(*s, sample,
            &owner.levels.classes_98[static_cast<std::size_t>(cls)], type, flag, {}, {}, c);
    } catch (...) { delete s; throw; } // DEB7E8 state0 frees allocation only.
    return s;
}
void destroy_spatial_sound_channel_00a8a3d0(SpatialSoundChannelInstance& s, SoundSpatialChannelContext& c) {
    s.native_vtable_00 = 0x00d5b510;
    try {
        if (s.dsp_80) {
            c.fmod.dsp_remove(s.dsp_80);
            const auto result = c.fmod.dsp_release(s.dsp_80); // reload after remove
            if (static_cast<std::uint32_t>(result) == 0x2b) {
                std::int32_t current, maximum;
                c.instance.fmod.memory_get_stats(&current, &maximum);
            }
            s.dsp_80 = nullptr;
        }
        c.instance.fmod.channel_stop(s.channel_54);
    } catch (...) { destroy_sound_channel_00a7bf40(s, c.instance); throw; }
    destroy_sound_channel_00a7bf40(s, c.instance); // second stop is native
}
SpatialSoundChannelInstance* scalar_delete_spatial_sound_channel_00a8a460(
    SpatialSoundChannelInstance* s, std::uint32_t flags, SoundSpatialChannelContext& c) {
    destroy_spatial_sound_channel_00a8a3d0(*s, c);
    if (flags & 1u) delete s;
    return s;
}
void set_spatial_sound_attributes_00a8a290(SpatialSoundChannelInstance& s,
    std::array<float, 3> position, std::array<float, 3> velocity) noexcept {
    s.velocity_68 = velocity; s.position_5c = position; s.spatial_dirty_74 = 1;
}
std::array<float, 3>& get_spatial_sound_position_00a8a3a0(
    const SpatialSoundChannelInstance& s, std::array<float, 3>& output) noexcept {
    const auto position = s.position_5c; output = position; return output;
}
std::uint32_t sound_distance_band_00a79a40(const SoundConfigurationScalars& c, float distance) noexcept {
    if (distance < c.sound_distance_1_20) return 0;
    if (distance < c.sound_distance_2_24) return 1;
    if (distance < c.sound_distance_3_28) return 2;
    // JA selects3 only for ordered threshold4 > distance; unordered selects4.
    return distance < c.sound_distance_4_2c ? 3u : 4u;
}
std::int32_t sound_distance_type_00a7ec60(const SoundConfigurationState& config,
    std::int32_t type, std::uint32_t band, NativeStringStorage& storage, SoundLevelNameHost& names) {
    if (type < 0) throw std::out_of_range("Sound type index");
    const auto& source = config.types_38.at(static_cast<std::size_t>(type)).name;
    if (source.size() >= INT32_MAX) throw std::out_of_range("Sound type name length");
    Text full(storage);
    full.value.resize_0041dd40(storage, static_cast<std::uint32_t>(source.size()), true);
    if (!source.empty()) std::memcpy(full.value.data(), source.data(), source.size());
    std::int32_t position;
    {
        Text needle(storage); needle.value.assign_0041e870(storage, "_");
        position = reverse_find_native_string_header_00467cf0(&full.value, &needle.value, INT32_MAX);
    }
    Text suffix(storage);
    {
        Text temporary(storage);
        if (position == -1) temporary.value.assign_0041e870(storage, "");
        else construct_native_string_substring_00469840(&full.value, &temporary.value,
            static_cast<std::uint32_t>(position) + 1u, INT32_MAX, storage);
        suffix.value.copy_from_00be0a30_fragment(storage, temporary.value);
    }
    if (equal_native_string_header_00425850(&suffix.value, "dist1") ||
        equal_native_string_header_00425850(&suffix.value, "dist2") ||
        equal_native_string_header_00425850(&suffix.value, "dist3") ||
        equal_native_string_header_00425850(&suffix.value, "dist4")) {
        const auto offset = static_cast<std::uint32_t>(position);
        if (offset < full.value.length()) full.value.resize_0041dd40(storage, offset, true);
    }
    if (band >= 1 && band <= 4) {
        static const char* const tails[] = {"_dist1", "_dist2", "_dist3", "_dist4"};
        Text tail(storage); tail.value.assign_0041e870(storage, tails[band - 1]);
        if (band <= 2) {
            const auto length = tail.value.length(), before = full.value.length();
            const char* const data = tail.value.data();
            if (length) {
                full.value.resize_0041dd40(storage, length + before, true);
                std::memcpy(full.value.data() + before, data, length);
            }
        } else append_native_string_00425e10(full.value, tail.value, storage);
    }
    return find_sound_type_00a7b0a0(config, full.value, names);
}
void* sound_type_listener_group_00a7ba40(SoundConfigurationState& config, std::int32_t type) {
    const auto listener = config.selected_listener_108;
    if (type < 0 || listener < 0 || listener == INT32_MAX) throw std::out_of_range("Sound type/listener index");
    auto& groups = config.types_38.at(static_cast<std::size_t>(type)).by_listener;
    if (static_cast<std::size_t>(listener) >= groups.size()) groups.resize(static_cast<std::size_t>(listener) + 1, nullptr);
    return groups[static_cast<std::size_t>(listener)];
}
void update_spatial_sound_channel_00a8a480(SpatialSoundChannelInstance& s, float dt,
    const SoundListenerOwnerState& listener, SoundSpatialChannelContext& c) {
    auto& base = c.instance;
    if (s.stopped_15) return;
    advance_sound_instance_fade_00a7a2e0(s, dt);
    if (s.duration_3c > 0) {
        s.duration_3c = difference(s.duration_3c, dt);
        if (s.duration_3c <= 0) s.enabled_38 = 0;
    }
    auto& captured_owner = *base.current_owner_00f8bbd8;
    if (!s.channel_54) {
        create_sound_channel_paused_00a7a4c0(s, base);
        s.channel_group_7c = captured_owner.configuration.master_channel_group_140;
    }
    ++s.updates_18;
    const auto position = s.position_5c;
    const std::array<float, 3> relative{difference(listener.transform_c4[12], position[0]),
        difference(listener.transform_c4[13], position[1]), difference(listener.transform_c4[14], position[2])};
    const float distance = camera_vector_length_00419440(relative.data(), &c.crt);
    const auto band = sound_distance_band_00a79a40(base.current_owner_00f8bbd8->configuration.scalars, distance);
    if (band != s.distance_band_78) {
        s.distance_band_78 = band;
        s.type_48 = static_cast<std::uint32_t>(sound_distance_type_00a7ec60(
            base.current_owner_00f8bbd8->configuration, static_cast<std::int32_t>(s.type_48), band, base.strings, c.names));
    }
    void* const group = sound_type_listener_group_00a7ba40(base.current_owner_00f8bbd8->configuration, static_cast<std::int32_t>(s.type_48));
    if (group != s.channel_group_7c) {
        if (group) {
            c.fmod.channel_set_group(s.channel_54, group);
            s.channel_group_7c = group;
            if (s.scale_28 != 1) { s.dirty_14 = 1; s.scale_28 = 1; }
        } else if (s.scale_28 != 0) { s.dirty_14 = 1; s.scale_28 = 0; }
        s.channel_group_7c = group;
    }
    if (s.dirty_14) {
        auto& resource = *read<SoundOwnedResource*>(s.sample_4c, 0x78);
        const float frequency = product(resource.frequency_20.value(), s.frequency_scale_20);
        base.fmod.channel_set_frequency(s.channel_54, frequency);
        const float volume = sound_instance_volume_00a7aaa0(s, base);
        base.fmod.channel_set_volume(s.channel_54, volume);
        if (read<std::int32_t>(sound_sample_options_00a81860(s.sample_4c), 0) == 1) {
            base.fmod.channel_set_3d_pan_level(s.channel_54, s.pan_level_40);
            if (s.pan_level_40 != 1) {
                const float mixed = sound_instance_volume_00a7aaa0(s, base);
                const float mix = product(mixed, base.current_owner_00f8bbd8->configuration.scalars.player_sound_boost_04);
                base.fmod.channel_set_speaker_mix(s.channel_54, {mix,mix,mix,mix,0,0,0,0});
            }
        }
        s.dirty_14 = 0;
    }
    if (s.spatial_dirty_74) {
        const auto velocity = s.velocity_68, current_position = s.position_5c;
        c.fmod.channel_set_3d_attributes(s.channel_54, current_position, velocity);
        s.spatial_dirty_74 = 0;
    }
    if (!s.paused_1c && !s.class_44) throw std::logic_error("Spatial sound requires a live class descriptor");
    const bool paused = s.paused_1c || static_cast<SoundClassDescriptor*>(s.class_44)->flags_0c;
    base.fmod.channel_set_paused(s.channel_54, static_cast<std::uint8_t>(s.enabled_38 | static_cast<std::uint8_t>(paused)));
    s.was_virtual_58 = s.virtual_59;
    s.virtual_59 = !base.virtuals.query_slot_14(s, base);
}
} // namespace bsp
