#pragma once

#include "bsp/voice_slot_start.hpp"
#include "bsp/gameplay_effect_definition.hpp"
#include <optional>

namespace bsp {

// Behavioral projection of native52h base and5Ch channel objects. The inherited
// fields are the canonical fields used by manager walkers and voice start.
// These are NOT native layouts; never use raw object+4 reference operations here.
// Names are hypotheses. Explicit recovered destruction owns sample/class/name.
struct SoundInstance : SoundLevelEntry, VoiceSoundStartFields {
    std::uint32_t native_vtable_00{}, references_04{}, id_08{};
    NativeString name_0c;
    std::uint8_t stopped_15{}, paused_1c{};
    std::uint32_t updates_18{};
    float frequency_scale_20{}, volume_24{}, scale_28{}, fade_2c{}, target_30{}, rate_34{}, pan_level_40{};
    std::uint32_t type_48{};
    void* sample_4c{}; // actual7Ch SoundSampleStorage, not SoundOwnedResource
    std::uint8_t flag_50{}, stop_requested_51{};
};
struct SoundChannelInstance : SoundInstance {
    void* channel_54{};
    std::uint8_t was_virtual_58{}, virtual_59{}, ended_5a{};
};

class SoundChannelFmodHost {
public:
    virtual ~SoundChannelFmodHost() = default;
    virtual void memory_get_stats(std::int32_t*, std::int32_t*) = 0;
    virtual void on_out_of_sound_memory(const char*) = 0;
    virtual FmodResult system_play_sound(void*, std::int32_t, void*, bool, void**) = 0;
    virtual FmodResult channel_set_loop_count(void*, std::int32_t) = 0;
    virtual FmodResult channel_set_loop_points(void*, std::uint32_t, std::uint32_t,
        std::uint32_t, std::uint32_t) = 0;
    virtual FmodResult channel_set_speaker_mix(void*, const std::array<float, 8>&) = 0;
    virtual FmodResult channel_stop(void*) = 0;
    // Empty is an unwritten native bool output; caller630 has no initializer.
    virtual FmodResult channel_is_virtual(void*, std::optional<bool>&) = 0;
    virtual FmodResult channel_is_playing(void*, bool*) = 0;
    virtual FmodResult channel_set_frequency(void*, float) = 0;
    virtual FmodResult channel_set_volume(void*, float) = 0;
    virtual FmodResult channel_set_3d_pan_level(void*, float) = 0;
    virtual FmodResult channel_set_paused(void*, std::uint8_t) = 0;
    virtual FmodResult channel_get_audibility(void*, float*) = 0;
};
struct SoundInstanceContext;
class SoundChannelVirtualHost {
public:
    virtual ~SoundChannelVirtualHost() = default;
    virtual bool query_slot_14(SoundChannelInstance&, SoundInstanceContext&) = 0;
    virtual void refresh_slot_10(SoundChannelInstance&, SoundInstanceContext&) = 0;
};
struct SoundInstanceContext {
    SoundSystemOwner* volatile& current_owner_00f8bbd8;
    std::uint32_t& next_id_00f8bbd4;
    NativeStringStorage& strings;
    GameplayEffectComponentLifetime& sample_lifetime;
    SoundChannelFmodHost& fmod;
    SoundChannelVirtualHost& virtuals;
};
// Concrete D5ABF8 dispatch; other derived tables remain explicit boundaries.
class NonspatialSoundChannelVirtuals final : public SoundChannelVirtualHost {
public:
    bool query_slot_14(SoundChannelInstance&, SoundInstanceContext&) override;
    void refresh_slot_10(SoundChannelInstance&, SoundInstanceContext&) override;
};

void* sound_sample_options_00a81860(void*) noexcept;
std::uint32_t sound_sample_pcm_length_00a81870(void*);
bool sound_sample_has_fmod_sound_00a818c0(void*) noexcept;
void* sound_sample_fmod_sound_00a818e0(void*) noexcept;

// C480 ECX=this, stack sample/class-slot/type/flag, EAX=this, RET10.
SoundInstance& construct_sound_instance_00a7c480(SoundInstance&, void*,
    SoundClassLevel* const* class_slot, std::uint32_t type, std::uint8_t flag, SoundInstanceContext&);
void destroy_sound_instance_00a7bd90(SoundInstance&, SoundInstanceContext&);
SoundChannelInstance& construct_sound_channel_00a7d560(SoundChannelInstance&, void*,
    SoundClassLevel* const*, std::uint32_t, std::uint8_t, SoundInstanceContext&);
SoundInstance* scalar_delete_sound_instance_00a7c5d0(SoundInstance*, std::uint32_t, SoundInstanceContext&);
// F640 manager slot0C: increment +160 even on rejected resource/allocation.
SoundChannelInstance* create_nonspatial_sound_00a7f640(SoundSystemOwner&, void*,
    std::int32_t class_index, std::uint32_t type, std::uint8_t flag, SoundInstanceContext&);
void destroy_sound_channel_00a7bf40(SoundChannelInstance&, SoundInstanceContext&);
SoundChannelInstance* scalar_delete_sound_channel_00a7d5a0(SoundChannelInstance*, std::uint32_t, SoundInstanceContext&);
void scale_sound_instance_volume_00a79880(SoundInstance&, float);
void advance_sound_instance_fade_00a7a2e0(SoundInstance&, float);
float sound_instance_volume_00a7aaa0(const SoundInstance&, SoundInstanceContext&);
void create_sound_channel_paused_00a7a4c0(SoundChannelInstance&, SoundInstanceContext&);
void stop_sound_channel_00a7a570(SoundChannelInstance&, std::uint8_t immediate, SoundInstanceContext&);
bool sound_channel_nonvirtual_00a7a630(SoundChannelInstance&, SoundInstanceContext&);
bool refresh_sound_channel_ended_00a7a660(SoundChannelInstance&, SoundInstanceContext&);
bool sound_channel_completed_00a799b0(SoundChannelInstance&, SoundInstanceContext&);
// AF10 consumes two stack words (RET8); the second is unused, not a dropped dt.
void update_sound_channel_00a7af10(SoundChannelInstance&, float dt, std::uint32_t unused, SoundInstanceContext&);

} // namespace bsp
