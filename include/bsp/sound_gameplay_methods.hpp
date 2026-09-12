#pragma once
#include "bsp/sound_lifetime_access.hpp"
#include "bsp/sound_event_instance.hpp"
#include "bsp/random_threads.hpp"

namespace bsp {
// Additional installed-FMOD boundary for the three supported sound profiles.
// Empty optional outputs represent native uninitialized stack words. A call
// that leaves one unwritten is outside the subsequent valid-storage domain.
class SoundGameplayFmodHost {
public:
    virtual ~SoundGameplayFmodHost() = default;
    virtual FmodResult channel_set_3d_minmax_distance(void*, float, float) = 0;
    virtual FmodResult channel_get_mode(void*, std::optional<std::uint32_t>&) = 0;
    virtual FmodResult channel_set_mode(void*, std::uint32_t) = 0;
    virtual FmodResult channel_get_position(void*, std::uint32_t*, std::uint32_t) = 0;
    virtual FmodResult channel_group_get_num_groups(void*, std::int32_t*) = 0;
    virtual FmodResult channel_group_get_group(void*, std::int32_t, std::optional<void*>&) = 0;
    virtual FmodResult channel_group_get_num_channels(void*, std::int32_t*) = 0;
    virtual FmodResult channel_group_get_channel(void*, std::int32_t, std::optional<void*>&) = 0;
};

// Canonical projection of the native eight-byte singleton at F8BBDC. The
// lifetime domain must dispatch its registered pointer to the deleting dtor.
struct SoundEventQueryLock {
    std::uint32_t native_vtable_00{};
    TrackedCriticalSection* section_04{};
};
struct SoundEventQueryLockBindings {
    SoundLifetimeAccess domain;
    SoundEventQueryLock* volatile& global_00f8bbdc;
};
SoundEventQueryLock& construct_sound_event_query_lock_00a89460(
    SoundEventQueryLock&, SoundEventQueryLockBindings&);
SoundEventQueryLock* get_sound_event_query_lock_00a89a40(SoundEventQueryLockBindings&);
void destroy_sound_event_query_lock_base_00a89390(SoundEventQueryLock&, SoundEventQueryLockBindings&) noexcept;
SoundEventQueryLock* scalar_delete_sound_event_query_lock_00a89b40(
    SoundEventQueryLock*, std::uint32_t, SoundEventQueryLockBindings&);

// Actual 48h options and actual 12h array headers of 16h descriptors. Copy
// construction leaves padding untouched and follows native per-field ordering.
void* assign_sound_sample_parameters_0093fd00(void* destination, const void* source);
void* copy_construct_sound_sample_options_0093fdf0(void* destination, const void* source);
void destroy_sound_sample_options_0093fdb0(void*);
void configure_spatial_sound_channel_00a8a700(SpatialSoundChannelInstance&,
    SoundInstanceContext&, SoundGameplayFmodHost&);
void sound_instance_noop_00a7be90() noexcept;
void* sound_event_handle_00a88b90(const SpatialSoundEventInstance&) noexcept;
void* sound_channel_handle_00a79a30(const SoundChannelInstance&) noexcept;
float sound_event_progress_00a89c60() noexcept;
float sound_channel_progress_00a7a6b0(SoundChannelInstance&, SoundGameplayFmodHost&);
float sound_channel_audibility_00a7a710(SoundChannelInstance&, SoundInstanceContext&);
float sound_group_max_audibility_00a89250(void* group,
    SoundInstanceContext&, SoundGameplayFmodHost&);
float sound_event_audibility_00a89d00(SpatialSoundEventInstance&,
    SoundEventInstanceContext&, SoundGameplayFmodHost&, SoundEventQueryLockBindings&);
void set_sound_channel_paused_00a799a0(SoundChannelInstance&, std::uint8_t) noexcept;
void set_sound_event_paused_00a891f0(SpatialSoundEventInstance&, std::uint8_t,
    SoundEventInstanceContext&);
} // namespace bsp
