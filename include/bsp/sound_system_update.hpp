#pragma once

#include "bsp/sound_instance.hpp"
#include "bsp/frame_clock.hpp"

namespace bsp {
class SoundSystemUpdateFmodHost {
public:
    virtual ~SoundSystemUpdateFmodHost() = default;
    virtual FmodResult system_get_channels_playing(void*, std::int32_t*) = 0;
    virtual FmodResult event_system_set_3d_listener_attributes(void*, std::int32_t,
        const std::array<float, 3>& position, const std::array<float, 3>& velocity,
        const std::array<float, 3>& forward, const std::array<float, 3>& up) = 0;
    virtual FmodResult update_event_system(void*) = 0;
};
class SoundSystemUpdateHost {
public:
    virtual ~SoundSystemUpdateHost() = default;
    // Current global01090AB0 virtual14. Native immediately copies all16 bytes.
    virtual const ClockTimestamp* current_timestamp_slot14() = 0;
    virtual void* current_alternate_00f8bbcc() = 0;
    virtual void update_alternate_slot04(void*, float dt) = 0;
};
class SoundActiveEntryHost : public VoiceReferenceHost {
public:
    virtual SoundInstance& instance_fields(SoundLevelEntry*) = 0;
    virtual void update_slot30(SoundLevelEntry*, float, SoundListenerOwnerState&) = 0;
    virtual bool transition_slot24(SoundLevelEntry*) = 0;
    virtual void refresh_slot10(SoundLevelEntry*) = 0;
    virtual bool completed_slot0c(SoundLevelEntry*) = 0;
    virtual bool nonvirtual_slot14(SoundLevelEntry*) = 0;
    virtual void stop_slot08(SoundLevelEntry*, std::uint8_t) = 0;
};
struct SoundSystemUpdateContext {
    SoundSystemUpdateHost& host;
    SoundSystemUpdateFmodHost& fmod;
    SoundActiveEntryHost& entries;
};

// Current-clock vslot14 for verifiedD68D50: ECX=FrameClock, EAX=this+20, RET.
const ClockTimestamp* sound_frame_clock_current_00bee050(FrameClock&) noexcept;
// NativeAL = instance58 != instance59, not an isPlaying query.
bool sound_channel_virtual_transition_00a79990(const SoundChannelInstance&) noexcept;

// Array functions use the existing canonical entries_8C and capacity94. Calls
// are serialized; valid native iterator storage must survive release callbacks.
// Callback slot replacements are observed; structural edits follow the existing
// array contract (see VOICE_SLOT_START.md). Native negative counts are invalid.
void resize_sound_entries_00a7c1c0(SoundSystemOwner&, std::int32_t, VoiceReferenceHost&);
void pop_sound_entry_00a7d640(SoundSystemOwner&, VoiceReferenceHost&);
// Native ECX=array, stack pointer-to-iterator, RET4. Iterator itself is unchanged.
void erase_sound_entry_swap_last_00a7d680(SoundSystemOwner&, std::size_t index, VoiceReferenceHost&);

// ECX manager, stack matrix pointer plus by-value3-float velocity, RET10.
// All timing comes from the clock; there is no input dt/register EBX argument.
void update_sound_system_base_00a7e630(SoundSystemOwner&, const CameraMatrix&,
    std::array<float, 3> velocity, SoundSystemUpdateContext&);
// D5B44C slot4: copy current listener10C to110 only when different, then base.
void update_sound_system_00a87bf0(SoundSystemOwner&, const CameraMatrix&,
    std::array<float, 3> velocity, SoundSystemUpdateContext&);
} // namespace bsp
