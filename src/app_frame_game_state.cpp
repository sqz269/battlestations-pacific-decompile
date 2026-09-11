#include "bsp/app_frame_game_state.hpp"

namespace bsp {
namespace {

// 00be364b and 00be366f. The record address is base + slot * 28h; 00be34d0
// reaches the same records through an induction variable that starts at 28h and
// steps by 28h, which is the same mapping.
ProfilerCounterRecord* record_for_slot(ProfilerCounters& profiler, int slot) noexcept {
    if (profiler.records == nullptr || slot <= 0 || slot >= profiler.slot_count) {
        return nullptr;
    }
    return profiler.records + slot;
}

// 00be3530 and 00be3595. Both float rings are indexed by ring_index + slot * 14h,
// so each slot owns a contiguous run of kProfilerHistoryFrames entries.
int ring_offset(const ProfilerCounters& profiler, int slot) noexcept {
    return profiler.ring_index + slot * kProfilerHistoryFrames;
}

} // namespace

int read_game_state_00737acc(const GameStateSlot& slot) noexcept {
    return slot.value;
}

void apply_drained_game_state(GameStateSlot& slot, int request) noexcept {
    slot.value = request;
}

void profiler_begin_counter_00be3260(ProfilerCounterRecord& record, ProfilerClockHost& clock) {
    // 00be3266, the hit count is raised whatever the depth.
    record.hit_count += 1;
    // 00be3270, InterlockedExchangeAdd(&depth, 1) returns the previous value; a
    // nonzero one means this entry is nested and nothing else happens.
    const std::int32_t previous_depth = record.depth;
    record.depth = previous_depth + 1;
    if (previous_depth != 0) {
        return;
    }
    // 00be3285 tests the 64 bit accumulator as two dwords before the sample is
    // stored, and the branch that uses the result is taken after the store.
    const bool accumulator_empty = record.accumulated_ticks == 0;
    const std::int64_t now = clock.query_performance_counter();
    record.sample_start = now; // 00be3292
    if (accumulator_empty) {
        record.first_entry = now; // 00be329a, first entry of this frame
    }
}

void profiler_begin_frame_slot_00be3640(ProfilerCounters& profiler, int slot,
    ProfilerClockHost& clock) {
    ProfilerCounterRecord* record = record_for_slot(profiler, slot);
    if (record == nullptr) {
        return;
    }
    profiler_begin_counter_00be3260(*record, clock);
}

void profiler_end_frame_slot_00be3660(ProfilerCounters& profiler, int slot,
    ProfilerClockHost& clock) {
    ProfilerCounterRecord* record = record_for_slot(profiler, slot);
    if (record == nullptr) {
        return;
    }
    // 00be3679, InterlockedDecrement(&depth) returns the new value; a nonzero
    // one means an outer pair is still open.
    record->depth -= 1;
    if (record->depth != 0) {
        return;
    }
    const std::int64_t now = clock.query_performance_counter();
    // 00be3699..00be36a9. The subtraction and the accumulate are one 64 bit
    // SUB/SBB and ADD/ADC pair over the two halves.
    record->accumulated_ticks += now - record->sample_start;
    record->last_exit = now;
}

void profiler_set_slot_color(ProfilerCounters& profiler, int slot, std::uint32_t argb) noexcept {
    if (profiler.colors == nullptr || slot < 0 || slot >= profiler.slot_count) {
        return;
    }
    profiler.colors[slot] = argb;
}

int profiler_end_frame_00be34d0(ProfilerCounters& profiler, int registered_slot_count,
    int app_update_slot, double tick_scale) {
    // 00be34ef. Nothing runs when the bound leaves no slot above zero, and the
    // ring index still advances at 00be3627.
    const ProfilerCounterRecord* app_record = record_for_slot(profiler, app_update_slot);
    const std::int64_t frame_origin = app_record != nullptr ? app_record->first_entry : 0;
    const double display_scale = static_cast<double>(profiler.display_scale);

    for (int slot = 1; slot < registered_slot_count; ++slot) {
        ProfilerCounterRecord* record = record_for_slot(profiler, slot);
        if (record == nullptr) {
            continue;
        }
        const int offset = ring_offset(profiler, slot);

        if (profiler.display != nullptr) {
            profiler.display[slot].hit_count = record->hit_count; // 00be3520
        }

        // 00be3527..00be353c. The native converts through the x87 stack, so the
        // divide happens at 80 bit precision and rounds once on the store to
        // float32. This rounds a double instead, which can differ in the last
        // bit for values that fall on a tie.
        const float seconds
            = static_cast<float>(static_cast<double>(record->accumulated_ticks) / tick_scale);
        if (profiler.history != nullptr) {
            profiler.history[offset] = seconds;
            if (profiler.current != nullptr) {
                // 00be354a, the current value is reloaded from the ring.
                profiler.current[slot] = profiler.history[offset];
            }
        }

        // 00be3556..00be3561, in native order: the hit count, the depth and both
        // halves of the accumulator. The two timestamps survive, which is what
        // lets the offsets below be taken after the clear.
        record->hit_count = 0;
        record->depth = 0;
        record->accumulated_ticks = 0;

        // 00be356d..00be35a6 and 00be35a9..00be35f4. Both offsets are measured
        // from the PERF_APP_UPDATE record's first entry, divided by the tick
        // scale and then by profiler+4h, and biased.
        const double start_delta = static_cast<double>(record->first_entry - frame_origin);
        const double end_delta = static_cast<double>(record->last_exit - frame_origin);
        if (profiler.start_offsets != nullptr) {
            profiler.start_offsets[offset] = static_cast<float>(
                (start_delta / tick_scale) / display_scale + kProfilerNormalizedBias);
        }
        if (profiler.display != nullptr) {
            profiler.display[slot].end_offset = static_cast<float>(
                (end_delta / tick_scale) / display_scale + kProfilerNormalizedBias);
        }
    }

    // 00be3627..00be3637. The remainder becomes the new ring index and the
    // quotient is returned, so the result is 1 exactly on the wrapping frame.
    const int advanced = profiler.ring_index + 1;
    profiler.ring_index = advanced % kProfilerHistoryFrames;
    return advanced / kProfilerHistoryFrames;
}

bool platform_pretranslate_consumes_00bec1d8(bool xlive_library_bound,
    bool xlive_result_nonzero) noexcept {
    // 00bec1d8 then the TEST EAX,EAX at 00bec1dd. With no library bound there is
    // no call and no consumer, so the message goes on to TranslateMessage.
    return xlive_library_bound && xlive_result_nonzero;
}

} // namespace bsp
