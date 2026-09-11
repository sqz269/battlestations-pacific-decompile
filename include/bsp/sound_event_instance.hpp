#pragma once
#include "bsp/sound_spatial_instance.hpp"

namespace bsp {
// Actual12h header and12h records: float value+0, dirty byte+4, borrowed FMOD
// parameter pointer+8. Growth leaves value and padding bytes untouched. A value
// read requires initialized backing bytes; do not invent a zero default.
struct SoundEventParameterArray {
    void* data{};
    std::int32_t count{}, capacity{};
};
static_assert(sizeof(SoundEventParameterArray) == 12);
// Projection of the native90h D5B4C8 object, not its native layout.
struct SpatialSoundEventInstance : SoundInstance {
    void* event_54{};
    SoundEventParameterArray parameters_58;
    std::uint8_t parameters_dirty_64{};
    void* channel_group_68{};
    std::array<float, 3> position_6c{}, velocity_78{};
    std::uint8_t spatial_dirty_84{};
    std::uint32_t distance_band_88{};
    std::uint8_t ended_8c{};
};
class SoundEventFmodHost {
public:
    virtual ~SoundEventFmodHost() = default;
    virtual FmodResult event_system_get_event(void*, const char*, std::uint32_t, void**) = 0;
    virtual FmodResult event_start(void*) = 0;
    virtual FmodResult event_stop(void*, bool immediate) = 0;
    virtual FmodResult event_get_state(void*, std::uint32_t*) = 0;
    virtual FmodResult event_get_parameter(void*, const char*, void**) = 0;
    virtual FmodResult event_get_parameter_by_index(void*, std::int32_t, void**) = 0;
    virtual FmodResult event_parameter_key_off(void*) = 0;
    virtual FmodResult event_parameter_set_value(void*, float) = 0;
    virtual FmodResult event_get_channel_group(void*, void**) = 0;
    virtual FmodResult channel_group_add_group(void*, void*) = 0;
    virtual FmodResult event_set_pitch(void*, float, std::uint32_t units) = 0;
    virtual FmodResult event_set_volume(void*, float) = 0;
    virtual FmodResult event_set_3d_attributes(void*, const std::array<float, 3>&,
        const std::array<float, 3>&, const std::array<float, 3>&) = 0;
    virtual FmodResult event_set_paused(void*, std::uint8_t) = 0;
};
struct SoundEventInstanceContext;
class SoundEventVirtualHost {
public:
    virtual ~SoundEventVirtualHost() = default;
    virtual void refresh_slot10(SpatialSoundEventInstance&, SoundEventInstanceContext&) = 0;
};
struct SoundEventInstanceContext {
    SoundInstanceContext& instance;
    SoundEventFmodHost& fmod;
    SoundLevelNameHost& names;
    const CameraAxesCrtAccess& crt;
    const char* null_data_00f8bbef;
    SoundEventVirtualHost& virtuals;
};
class SpatialSoundEventVirtuals final : public SoundEventVirtualHost {
public:
    void refresh_slot10(SpatialSoundEventInstance&, SoundEventInstanceContext&) override;
};

void reserve_sound_event_parameters_00a893d0(SoundEventParameterArray&, std::int32_t);
void resize_sound_event_parameters_00a899e0(SoundEventParameterArray&, std::int32_t);
void destroy_sound_event_parameters_00a89be0(SoundEventParameterArray&);
void set_sound_event_parameter_00a89b80(SpatialSoundEventInstance&, std::int32_t, float);
// Actual16h output: index, min, max, borrowed name. Miss copies first record;
// the native empty-array miss is outside the valid-storage domain.
void* find_sound_event_parameter_00a896e0(const SpatialSoundEventInstance&, void* output,
    const char* name, SoundLevelNameHost&);
SpatialSoundEventInstance& construct_spatial_sound_event_00a89dc0(
    SpatialSoundEventInstance&, void*, SoundClassLevel* const*, std::uint32_t, std::uint8_t,
    std::array<float, 3>, std::array<float, 3>, SoundInstanceContext&);
void destroy_sound_event_00a89c80(SpatialSoundEventInstance&, SoundEventInstanceContext&);
SpatialSoundEventInstance* scalar_delete_spatial_sound_event_00a89ee0(
    SpatialSoundEventInstance*, std::uint32_t, SoundEventInstanceContext&);
void create_sound_event_00a894e0(SpatialSoundEventInstance&, SoundEventInstanceContext&);
void update_sound_event_parameters_00a89560(SpatialSoundEventInstance&, SoundEventInstanceContext&);
void stop_sound_event_00a89170(SpatialSoundEventInstance&, std::uint8_t, SoundEventInstanceContext&);
bool refresh_spatial_sound_event_ended_00a89340(SpatialSoundEventInstance&, SoundEventInstanceContext&);
bool spatial_sound_event_completed_00a88ba0(SpatialSoundEventInstance&, SoundEventInstanceContext&);
bool sound_event_nonvirtual_00a89c40() noexcept;
bool sound_event_virtual_transition_00a89c70() noexcept;
void set_spatial_sound_event_attributes_00a89120(SpatialSoundEventInstance&,
    std::array<float, 3>, std::array<float, 3>) noexcept;
std::array<float, 3>& get_spatial_sound_event_position_00a89e80(
    const SpatialSoundEventInstance&, std::array<float, 3>&) noexcept;
// Native ECX source, stack hidden output, EAX output, RET4. EDX adds borrowed
// CRT access. Preserves native x87 spills and component reload/alias behavior.
float* __fastcall normalize_sound_event_velocity_00a88ff0(const float*,
    const CameraAxesCrtAccess*, float*);
void update_spatial_sound_event_00a89770(SpatialSoundEventInstance&, float,
    const SoundListenerOwnerState&, SoundEventInstanceContext&);
// Complete manager slot10: counter then bank/event allocation and construction.
SoundInstance* create_spatial_sound_00a7f710(SoundSystemOwner&, void*, std::int32_t,
    std::uint32_t, std::uint8_t, SoundInstanceContext&);
} // namespace bsp
