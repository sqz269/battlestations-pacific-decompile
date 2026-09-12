#include "bsp/sound_instance.hpp"
#include "bsp/sound_resource_asset.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/unit_motion.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T get(const void* p, std::size_t n) noexcept {
    T v; std::memcpy(&v, static_cast<const unsigned char*>(p) + n, sizeof v); return v;
}
SoundOwnedResource& resource(void* sample) noexcept { return *get<SoundOwnedResource*>(sample, 0x78); }
SoundClassDescriptor& descriptor(SoundClassLevel* p) {
    if (!p) throw std::logic_error("Sound channel requires a live class descriptor");
    return *static_cast<SoundClassDescriptor*>(p);
}
void release_sample(void* sample, SoundInstanceContext& c) {
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(static_cast<unsigned char*>(sample) + 4)) == 0)
        c.sample_lifetime.zero_references_slot_00(sample);
}
float product(float a, float b) noexcept {
    float result;
    __asm { fld a }
    __asm { fmul b }
    __asm { fstp result }
    return result;
}
float difference(float a, float b) noexcept {
    float result;
    __asm { fld a }
    __asm { fsub b }
    __asm { fstp result }
    return result;
}
float copied(float value) noexcept {
    float result;
    __asm { fld value }
    __asm { fstp result }
    return result;
}
void memory_result(FmodResult result, SoundInstanceContext& c) {
    if (static_cast<std::uint32_t>(result) == 0x2b) {
        std::int32_t current, maximum;
        c.fmod.memory_get_stats(&current, &maximum);
    }
}
void require_nonspatial(const SoundChannelInstance& instance) {
    if (instance.native_vtable_00 != 0x00d5abf8 && instance.native_vtable_00 != 0x00d5ad58 &&
        instance.native_vtable_00 != 0x00d5ada0 && instance.native_vtable_00 != 0x00d5ade8)
        throw std::logic_error("Unbound derived sound-channel virtual table");
}
}
void* sound_sample_options_00a81860(void* sample) noexcept {
    return static_cast<unsigned char*>(sample) + 8;
}
std::uint32_t sound_sample_pcm_length_00a81870(void* sample) {
    return resource(sample).pcm_length_1c.value();
}
bool sound_sample_has_fmod_sound_00a818c0(void* sample) noexcept {
    return resource(sample).fsb_subsound_10 != nullptr;
}
void* sound_sample_fmod_sound_00a818e0(void* sample) noexcept { return resource(sample).fsb_subsound_10; }

SoundInstance& construct_sound_instance_00a7c480(SoundInstance& s, void* sample,
    SoundClassLevel* const* class_slot, std::uint32_t type, std::uint8_t flag, SoundInstanceContext& c) {
    s.native_vtable_00 = 0x00ceb130; s.references_04 = 1;
    s.native_vtable_00 = 0x00d5abb8; new(&s.name_0c) NativeString(); s.class_44 = nullptr;
    s.id_08 = c.next_id_00f8bbd4++;
    s.sample_4c = sample;
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(static_cast<unsigned char*>(sample) + 4));
    try {
        auto* next = *class_slot; auto* old = s.class_44;
        if (old != next) {
            s.class_44 = next;
            if (next) descriptor(next).retain();
            if (old) descriptor(old).release();
        }
        const auto& source = *reinterpret_cast<const NativeString*>(static_cast<const unsigned char*>(sample) + 0x50);
        s.name_0c.copy_from_00be0a30_fragment(c.strings, source);
        s.dirty_14 = 1; s.stopped_15 = 0; s.updates_18 = 0;
        s.volume_24 = 1; s.target_30 = 1; s.fade_2c = 1; s.scale_28 = 1; s.frequency_scale_20 = 1;
        s.duration_3c = 0; s.pan_level_40 = 1; s.rate_34 = 0; s.enabled_38 = 0; s.paused_1c = 0;
        s.volume_24 = copied(get<float>(sound_sample_options_00a81860(s.sample_4c), 4));
        s.type_48 = type; s.flag_50 = flag; s.stop_requested_51 = 0;
    } catch (...) {
        // C480 FuncInfo DEB100, states2..0: class44, name0C, base.
        // Raw sample4C is absent from that map despite the earlier retain.
        if (auto* entry_class = s.class_44) { descriptor(entry_class).release(); s.class_44 = nullptr; }
        destroy_native_string_header_0041dd20(&s.name_0c, c.strings);
        s.native_vtable_00 = 0x00ceb130;
        throw;
    }
    return s;
}
void destroy_sound_instance_00a7bd90(SoundInstance& s, SoundInstanceContext& c) {
    s.native_vtable_00 = 0x00d5abb8;
    if (auto* sample = s.sample_4c) { release_sample(sample, c); s.sample_4c = nullptr; }
    if (auto* entry_class = s.class_44) { descriptor(entry_class).release(); s.class_44 = nullptr; }
    destroy_native_string_header_0041dd20(&s.name_0c, c.strings);
    s.native_vtable_00 = 0x00ceb130;
}
SoundInstance* scalar_delete_sound_instance_00a7c5d0(SoundInstance* s, std::uint32_t flags, SoundInstanceContext& c) {
    destroy_sound_instance_00a7bd90(*s, c);
    if (flags & 1u) delete s;
    return s;
}
SoundChannelInstance& construct_sound_channel_00a7d560(SoundChannelInstance& s, void* sample,
    SoundClassLevel* const* cls, std::uint32_t type, std::uint8_t flag, SoundInstanceContext& c) {
    construct_sound_instance_00a7c480(s, sample, cls, type, flag, c);
    s.ended_5a = 0; s.channel_54 = nullptr; s.virtual_59 = 1; s.was_virtual_58 = 1;
    s.native_vtable_00 = 0x00d5abf8;
    return s;
}
SoundChannelInstance* create_nonspatial_sound_00a7f640(SoundSystemOwner& owner, void* sample,
    std::int32_t class_index, std::uint32_t type, std::uint8_t flag, SoundInstanceContext& c) {
    ++owner.words_160[0];
    if (!sound_sample_has_fmod_sound_00a818c0(sample)) return nullptr;
    auto* s = new(std::nothrow) SoundChannelInstance;
    if (!s) return nullptr;
    try {
        // Native signed negative indices and overflowing index+1 are outside
        // the existing class-table contract, not normalized to another class.
        if (class_index < 0 || class_index == INT32_MAX) throw std::out_of_range("Sound class index");
        if (static_cast<std::size_t>(class_index) >= owner.levels.classes_98.size())
            owner.classes.resize_00a7c2c0(class_index + 1);
        construct_sound_instance_00a7c480(*s, sample, &owner.levels.classes_98[static_cast<std::size_t>(class_index)], type, flag, c);
        s->native_vtable_00 = 0x00d5abf8; s->ended_5a = 0; s->channel_54 = nullptr;
        s->virtual_59 = 1; s->was_virtual_58 = 1;
    } catch (...) { delete s; throw; }
    return s;
}
void destroy_sound_channel_00a7bf40(SoundChannelInstance& s, SoundInstanceContext& c) {
    s.native_vtable_00 = 0x00d5abf8;
    try { c.fmod.channel_stop(s.channel_54); }
    catch (...) { destroy_sound_instance_00a7bd90(s, c); throw; }
    destroy_sound_instance_00a7bd90(s, c);
}
SoundChannelInstance* scalar_delete_sound_channel_00a7d5a0(SoundChannelInstance* s, std::uint32_t flags, SoundInstanceContext& c) {
    destroy_sound_channel_00a7bf40(*s, c);
    if (flags & 1u) delete s;
    return s;
}
void scale_sound_instance_volume_00a79880(SoundInstance& s, float multiplier) {
    const float volume = product(get<float>(sound_sample_options_00a81860(s.sample_4c), 4), multiplier);
    s.dirty_14 = 1; s.volume_24 = volume;
}
void advance_sound_instance_fade_00a7a2e0(SoundInstance& s, float dt) {
    if (s.rate_34 > 0) {
        s.fade_2c = unit_step_towards_0042ac60(s.fade_2c, s.target_30, product(s.rate_34, dt));
        if (s.target_30 == s.fade_2c) s.rate_34 = 0;
        s.dirty_14 = 1;
    }
}
float sound_instance_volume_00a7aaa0(const SoundInstance& s, SoundInstanceContext& c) {
    const auto& owner = *c.current_owner_00f8bbd8;
    const auto& cls = descriptor(s.class_44);
    // Three explicit32-bit stores at A7AAC2,A7AAC8,A7AAD9. Intermediate
    // class and instance products remain x87 extended between those stores.
    const float global = owner.flag_50 ? 0.0f : product(owner.level_6c, owner.levels.global_4c);
    const float secondary = cls.secondary_level_14, level = cls.volume_10;
    const float fade = s.fade_2c, scale = s.scale_28, volume = s.volume_24;
    float combined, result;
    __asm { fld secondary }
    __asm { fmul level }
    __asm { fmul global }
    __asm { fstp combined }
    __asm { fld combined }
    __asm { fld fade }
    __asm { fmul scale }
    __asm { fmul volume }
    __asm { fmulp st(1), st(0) }
    __asm { fstp result }
    return result;
}
void create_sound_channel_paused_00a7a4c0(SoundChannelInstance& s, SoundInstanceContext& c) {
    auto& owner = *c.current_owner_00f8bbd8;
    auto* sound = sound_sample_fmod_sound_00a818e0(s.sample_4c);
    memory_result(c.fmod.system_play_sound(owner.system.system, -1, sound, true, &s.channel_54), c);
    if (get<std::uint8_t>(sound_sample_options_00a81860(s.sample_4c), 0x15) == 0)
        memory_result(c.fmod.channel_set_loop_count(s.channel_54, 0), c);
    memory_result(c.fmod.channel_set_speaker_mix(s.channel_54, {1,1,1,1,1,1,1,1}), c);
}
void stop_sound_channel_00a7a570(SoundChannelInstance& s, std::uint8_t immediate, SoundInstanceContext& c) {
    const bool had_channel = s.channel_54 != nullptr;
    s.stop_requested_51 = 1;
    if (had_channel) {
        if (c.virtuals.query_slot_14(s, c)) {
            if (!immediate) {
                if (get<std::uint8_t>(sound_sample_options_00a81860(s.sample_4c), 0x21)) {
                    c.fmod.channel_set_loop_count(s.channel_54, 0); return;
                }
                if (get<std::uint8_t>(sound_sample_options_00a81860(s.sample_4c), 0x22)) {
                    c.fmod.channel_set_loop_count(s.channel_54, 0);
                    const auto end = sound_sample_pcm_length_00a81870(s.sample_4c) - 1u;
                    c.fmod.channel_set_loop_points(s.channel_54, 0, 2, end, 2); return;
                }
            }
        }
        c.fmod.channel_stop(s.channel_54);
    }
    s.stopped_15 = 1;
}
bool sound_channel_nonvirtual_00a7a630(SoundChannelInstance& s, SoundInstanceContext& c) {
    if (!s.channel_54) return false;
    std::optional<bool> is_virtual;
    c.fmod.channel_is_virtual(s.channel_54, is_virtual);
    if (!is_virtual) throw std::runtime_error("FMOD left native isVirtual output indeterminate");
    return !*is_virtual;
}
bool refresh_sound_channel_ended_00a7a660(SoundChannelInstance& s, SoundInstanceContext& c) {
    bool ended = true;
    if (!s.stopped_15) {
        if (!s.channel_54) ended = false;
        else {
            bool playing = false;
            const auto result = static_cast<std::uint32_t>(c.fmod.channel_is_playing(s.channel_54, &playing));
            ended = result == 0x24 || result == 0x0b || !playing;
        }
    }
    s.ended_5a = static_cast<std::uint8_t>(ended); return ended;
}
bool sound_channel_completed_00a799b0(SoundChannelInstance& s, SoundInstanceContext& c) {
    if (s.stop_requested_51) c.virtuals.refresh_slot_10(s, c);
    if (s.channel_54) return s.stopped_15 || s.ended_5a;
    return s.stop_requested_51 != 0;
}
void update_sound_channel_00a7af10(SoundChannelInstance& s, float dt, std::uint32_t, SoundInstanceContext& c) {
    if (s.stopped_15) return;
    advance_sound_instance_fade_00a7a2e0(s, dt);
    if (s.duration_3c > 0) {
        s.duration_3c = difference(s.duration_3c, dt);
        // FCOMIP(0,remaining) sets CF for positive OR unordered; JC skips
        // the clear on both. NaN therefore preserves the delay flag.
        if (s.duration_3c <= 0) s.enabled_38 = 0;
    }
    if (!s.channel_54) create_sound_channel_paused_00a7a4c0(s, c);
    ++s.updates_18;
    if (s.dirty_14) {
        const float frequency = product(resource(s.sample_4c).frequency_20.value(), s.frequency_scale_20);
        c.fmod.channel_set_frequency(s.channel_54, frequency);
        const float volume = sound_instance_volume_00a7aaa0(s, c);
        c.fmod.channel_set_volume(s.channel_54, volume);
        if (get<std::int32_t>(sound_sample_options_00a81860(s.sample_4c), 0) == 1) {
            memory_result(c.fmod.channel_set_3d_pan_level(s.channel_54, s.pan_level_40), c);
            if (s.pan_level_40 != 1.0f) {
                const float mix = sound_instance_volume_00a7aaa0(s, c);
                if (static_cast<std::uint32_t>(c.fmod.channel_set_speaker_mix(s.channel_54, {mix,mix,mix,mix,0,0,0,0})) == 0x2b)
                    c.fmod.on_out_of_sound_memory("Out of sounjd memory:");
            }
        }
        s.dirty_14 = 0;
    }
    const bool paused = s.paused_1c || descriptor(s.class_44).flags_0c;
    c.fmod.channel_set_paused(s.channel_54, static_cast<std::uint8_t>(s.enabled_38 | static_cast<std::uint8_t>(paused)));
    float ignored;
    c.fmod.channel_get_audibility(s.channel_54, &ignored);
    s.was_virtual_58 = s.virtual_59;
    s.virtual_59 = static_cast<std::uint8_t>(!c.virtuals.query_slot_14(s, c));
}
bool NonspatialSoundChannelVirtuals::query_slot_14(SoundChannelInstance& s, SoundInstanceContext& c) {
    require_nonspatial(s); return sound_channel_nonvirtual_00a7a630(s, c);
}
void NonspatialSoundChannelVirtuals::refresh_slot_10(SoundChannelInstance& s, SoundInstanceContext& c) {
    require_nonspatial(s); refresh_sound_channel_ended_00a7a660(s, c);
}
} // namespace bsp
