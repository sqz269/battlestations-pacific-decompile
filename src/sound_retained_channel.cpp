#include "bsp/sound_retained_channel.hpp"

namespace bsp {
namespace {
void construct_retained_channel(RetainedSoundChannelInstance& sound,
    const SoundRetainedSourceRecord& source, SoundClassLevel* const* class_slot,
    const char* listener, std::uint32_t table, SoundInstanceContext& context,
    SoundLevelNameHost& names) {
    const auto type = find_sound_listener_00a7ae00(
        context.current_owner_00f8bbd8->configuration, listener, names);
    construct_sound_instance_00a7c480(sound, source.sample_00, class_slot,
        static_cast<std::uint32_t>(type), 1, context);
    sound.ended_5a = 0;
    sound.channel_54 = nullptr;
    sound.virtual_59 = 1;
    sound.was_virtual_58 = 1;
    sound.native_vtable_00 = table;
    try {
        copy_sound_retained_source_record_00a7c5f0(&sound.source_5c, &source,
            context.strings, context.sample_lifetime);
    } catch (...) {
        // Ctor FuncInfo state0 owns the channel base. The throwing record
        // copy performs its own partial-record cleanup before reaching here.
        destroy_sound_channel_00a7bf40(sound, context);
        throw;
    }
    sound.paused_1c = 0;
}

void destroy_retained_channel(RetainedSoundChannelInstance& sound,
    SoundInstanceContext& context) {
    try {
        destroy_sound_retained_source_record_004c7fa0(&sound.source_5c,
            context.strings, context.sample_lifetime);
    } catch (...) {
        // Each ordinary/scalar dtor has state0 -> A7BF40. No derived-vtable
        // store precedes the record release; preserve callback observation.
        destroy_sound_channel_00a7bf40(sound, context);
        throw;
    }
    destroy_sound_channel_00a7bf40(sound, context);
}
} // namespace

RetainedSoundChannelInstance& construct_retained_sound_channel_00a7da40(
    RetainedSoundChannelInstance& sound, const SoundRetainedSourceRecord& source,
    SoundClassLevel* const* class_slot, SoundInstanceContext& context,
    SoundLevelNameHost& names) {
    construct_retained_channel(sound, source, class_slot, "Air", 0x00d5ad58,
        context, names);
    return sound;
}

ScaledRetainedSoundChannelInstance& construct_scaled_retained_sound_channel_00a7db80(
    ScaledRetainedSoundChannelInstance& sound, const SoundRetainedSourceRecord& source,
    SoundClassLevel* const* class_slot, SoundInstanceContext& context,
    SoundLevelNameHost& names) {
    construct_retained_channel(sound, source, class_slot, "Air", 0x00d5ada0,
        context, names);
    sound.scale_70 = 1.0f; // MOVSS from 00D7A24C, native 00A7DBEE..00A7DBFE.
    return sound;
}

RetainedSoundChannelInstance& construct_underwater_retained_sound_channel_00a7dcd0(
    RetainedSoundChannelInstance& sound, const SoundRetainedSourceRecord& source,
    SoundClassLevel* const* class_slot, SoundInstanceContext& context,
    SoundLevelNameHost& names) {
    construct_retained_channel(sound, source, class_slot, "Underwater", 0x00d5ade8,
        context, names);
    return sound;
}

void destroy_retained_sound_channel_00a7dad0(
    RetainedSoundChannelInstance& sound, SoundInstanceContext& context) {
    destroy_retained_channel(sound, context);
}
void destroy_scaled_retained_sound_channel_00a7dc20(
    ScaledRetainedSoundChannelInstance& sound, SoundInstanceContext& context) {
    destroy_retained_channel(sound, context);
}
void destroy_underwater_retained_sound_channel_00a7dd60(
    RetainedSoundChannelInstance& sound, SoundInstanceContext& context) {
    destroy_retained_channel(sound, context);
}

RetainedSoundChannelInstance* scalar_delete_retained_sound_channel_00a7db20(
    RetainedSoundChannelInstance* sound, std::uint32_t flags, SoundInstanceContext& context) {
    destroy_retained_channel(*sound, context);
    if (flags & 1u) delete sound;
    return sound;
}
ScaledRetainedSoundChannelInstance* scalar_delete_scaled_retained_sound_channel_00a7dc70(
    ScaledRetainedSoundChannelInstance* sound, std::uint32_t flags, SoundInstanceContext& context) {
    destroy_retained_channel(*sound, context);
    if (flags & 1u) delete sound;
    return sound;
}
RetainedSoundChannelInstance* scalar_delete_underwater_retained_sound_channel_00a7ddb0(
    RetainedSoundChannelInstance* sound, std::uint32_t flags, SoundInstanceContext& context) {
    destroy_retained_channel(*sound, context);
    if (flags & 1u) delete sound;
    return sound;
}

} // namespace bsp
