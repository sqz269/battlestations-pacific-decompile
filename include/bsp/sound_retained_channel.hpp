#pragma once

#include "bsp/sound_instance.hpp"
#include "bsp/sound_retained_record.hpp"

namespace bsp {

// Behavioral projections of native D5AD58/D5ADE8 (70h) and D5ADA0 (74h).
// Base fields keep the canonical SoundChannelInstance interface. The embedded
// source is the actual 14h record, but these complete objects are NOT native
// layouts or ABI replacements. Names are hypotheses; lifetime is explicit.
struct RetainedSoundChannelInstance : SoundChannelInstance {
    SoundRetainedSourceRecord source_5c;
};
struct ScaledRetainedSoundChannelInstance : RetainedSoundChannelInstance {
    float scale_70{};
};

// Native constructors: ECX=self; stack source-record*, class-slot*; EAX=self;
// RET8. Air/Air/Underwater listener lookup supplies base type48, flag50=1.
// Complete ordinary and unwind behavior within the existing host contracts.
RetainedSoundChannelInstance& construct_retained_sound_channel_00a7da40(
    RetainedSoundChannelInstance&, const SoundRetainedSourceRecord&,
    SoundClassLevel* const*, SoundInstanceContext&, SoundLevelNameHost&);
ScaledRetainedSoundChannelInstance& construct_scaled_retained_sound_channel_00a7db80(
    ScaledRetainedSoundChannelInstance&, const SoundRetainedSourceRecord&,
    SoundClassLevel* const*, SoundInstanceContext&, SoundLevelNameHost&);
RetainedSoundChannelInstance& construct_underwater_retained_sound_channel_00a7dcd0(
    RetainedSoundChannelInstance&, const SoundRetainedSourceRecord&,
    SoundClassLevel* const*, SoundInstanceContext&, SoundLevelNameHost&);

// Ordinary destructors: ECX=self; RET. Record destruction precedes base-channel
// destruction, including its unconditional FMOD stop and sample/class/name
// teardown. The base is also destroyed if record destruction throws.
void destroy_retained_sound_channel_00a7dad0(
    RetainedSoundChannelInstance&, SoundInstanceContext&);
void destroy_scaled_retained_sound_channel_00a7dc20(
    ScaledRetainedSoundChannelInstance&, SoundInstanceContext&);
void destroy_underwater_retained_sound_channel_00a7dd60(
    RetainedSoundChannelInstance&, SoundInstanceContext&);

// Scalar destructors: ECX=self; stack flags; EAX=original self; RET4.
// Free the correctly typed projection only after destruction returns, iff bit0.
RetainedSoundChannelInstance* scalar_delete_retained_sound_channel_00a7db20(
    RetainedSoundChannelInstance*, std::uint32_t, SoundInstanceContext&);
ScaledRetainedSoundChannelInstance* scalar_delete_scaled_retained_sound_channel_00a7dc70(
    ScaledRetainedSoundChannelInstance*, std::uint32_t, SoundInstanceContext&);
RetainedSoundChannelInstance* scalar_delete_underwater_retained_sound_channel_00a7ddb0(
    RetainedSoundChannelInstance*, std::uint32_t, SoundInstanceContext&);

} // namespace bsp
