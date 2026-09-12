#pragma once
#include "bsp/sound_instance.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Projection of the native84h D5B510 channel. Offsets describe the original,
// not this C++ object's layout. Position/velocity are by-value float3 inputs.
struct SpatialSoundChannelInstance : SoundChannelInstance {
    std::array<float, 3> position_5c{}, velocity_68{};
    std::uint8_t spatial_dirty_74{};
    std::uint32_t distance_band_78{};
    void* channel_group_7c{};
    void* dsp_80{};
};
class SoundSpatialChannelFmodHost {
public:
    virtual ~SoundSpatialChannelFmodHost() = default;
    virtual FmodResult channel_set_group(void*, void*) = 0;
    virtual FmodResult channel_set_3d_attributes(void*, const std::array<float, 3>&,
        const std::array<float, 3>&) = 0;
    virtual FmodResult dsp_remove(void*) = 0;
    virtual FmodResult dsp_release(void*) = 0;
};
struct SoundSpatialChannelContext {
    SoundInstanceContext& instance;
    SoundSpatialChannelFmodHost& fmod;
    SoundLevelNameHost& names;
    const CameraAxesCrtAccess& crt;
};
// Same slot10/14 methods in D5ABF8/D5B510 and retained D5AD58/D5ADA0/D5ADE8.
class BankSoundChannelVirtuals final : public SoundChannelVirtualHost {
public:
    bool query_slot_14(SoundChannelInstance&, SoundInstanceContext&) override;
    void refresh_slot_10(SoundChannelInstance&, SoundInstanceContext&) override;
};

SpatialSoundChannelInstance& construct_spatial_sound_channel_00a8a2e0(
    SpatialSoundChannelInstance&, void*, SoundClassLevel* const*, std::uint32_t,
    std::uint8_t, std::array<float, 3> position, std::array<float, 3> velocity,
    SoundInstanceContext&);
// Compatibility entry exposing only the A818C0=true bank branch of A7F710.
// A false gate throws after counting. The complete factory is declared in
// sound_event_instance.hpp and accepts both bank and event resources.
SpatialSoundChannelInstance* create_spatial_bank_sound_00a7f710_fragment(
    SoundSystemOwner&, void*, std::int32_t, std::uint32_t, std::uint8_t, SoundInstanceContext&);
// Internal true-branch allocation, after the full factory has counted/gated.
SpatialSoundChannelInstance* allocate_spatial_bank_sound_fragment(
    SoundSystemOwner&, void*, std::int32_t, std::uint32_t, std::uint8_t, SoundInstanceContext&);
void destroy_spatial_sound_channel_00a8a3d0(SpatialSoundChannelInstance&, SoundSpatialChannelContext&);
SpatialSoundChannelInstance* scalar_delete_spatial_sound_channel_00a8a460(
    SpatialSoundChannelInstance*, std::uint32_t, SoundSpatialChannelContext&);
void set_spatial_sound_attributes_00a8a290(SpatialSoundChannelInstance&,
    std::array<float, 3> position, std::array<float, 3> velocity) noexcept;
std::array<float, 3>& get_spatial_sound_position_00a8a3a0(
    const SpatialSoundChannelInstance&, std::array<float, 3>&) noexcept;
std::uint32_t sound_distance_band_00a79a40(const SoundConfigurationScalars&, float) noexcept;
std::int32_t sound_distance_type_00a7ec60(const SoundConfigurationState&, std::int32_t,
    std::uint32_t band, NativeStringStorage&, SoundLevelNameHost&);
void* sound_type_listener_group_00a7ba40(SoundConfigurationState&, std::int32_t);
void update_spatial_sound_channel_00a8a480(SpatialSoundChannelInstance&, float,
    const SoundListenerOwnerState&, SoundSpatialChannelContext&);
} // namespace bsp
