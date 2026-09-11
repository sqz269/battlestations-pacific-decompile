#include "bsp/scheduled_voice.hpp"

namespace bsp {
namespace {
std::int32_t choose_scheduled_slot(ScheduledVoiceContext& context)
{
    // Native stride18h is present, but CMP index,1 proves a single probe.
    return poll_voice_slot_007027b0(context.manager.slot_08, context.playback) == 0 ? 0 : -1;
}
VoicePlaybackSlot& selected_slot(ScheduledVoiceContext& context, std::int32_t index)
{
    return index == 0 ? context.manager.slot_08
        : context.playback.slot_start_context().host.resolve_nonzero_slot(context.manager, index);
}
} // namespace

void start_scheduled_voice_record_005b9300(ScheduledVoiceContext& context,
    const VoiceClipRecord& record)
{
    if (record.sound_id_08 >= 0) {
        const VoiceClip clip{0x00cf0dd0, &record, 0};
        const auto index = context.fields.slot_index_70;
        start_voice_clip_005b9050(context.manager, index, clip, nullptr,
            context.playback, context.playback.slot_start_context());
    }
    // Native005B9344..376 checks manager+94/current+84 AFTER playback.
    // at() is the standard-container bounds boundary; invalid native iterator
    // handler behavior and malformed vector layouts are not reproduced.
    auto& row = context.fields.rows_94.at(context.fields.selected_row_84);
    row.slot_index_30 = context.fields.slot_index_70;
    const bool has_keys = !context.records.timed_keys_34(record).empty();
    row.keys_active_20 = has_keys ? 1 : 0;
    if (has_keys) {
        row.record_24 = &record;
        row.key_index_28 = 0;
        row.started_at_2c = context.playback.mission_clock_00f876a4();
    }
}

void admit_scheduled_voice_record_005b93d0(ScheduledVoiceContext& context,
    const VoiceClipRecord& record)
{
    if (record.sound_id_08 >= 0) {
        const auto index = choose_scheduled_slot(context);
        context.fields.slot_index_70 = index;
        if (index < 0) {
            context.fields.pending_record_74 = &record;
            return;
        }
    }
    start_scheduled_voice_record_005b9300(context, record);
}

bool poll_scheduled_voice_005b9420(ScheduledVoiceContext& context)
{
    if (context.fields.pending_record_74) {
        const auto index = choose_scheduled_slot(context);
        context.fields.slot_index_70 = index;
        if (index >= 0) {
            // Reload current pending pointer after retirement callbacks. A
            // callback-cleared null is a native fault, not a successful skip.
            const auto* pending = context.fields.pending_record_74;
            start_scheduled_voice_record_005b9300(context, *pending);
            context.fields.pending_record_74 = nullptr; // after start callbacks
        }
        return true;
    }
    const auto index = context.fields.slot_index_70;
    if (index >= 0) {
        if (poll_voice_slot_007027b0(selected_slot(context, index), context.playback) != 0)
            return true;
        context.fields.slot_index_70 = -1;
    }
    return false;
}

} // namespace bsp
