// Segment 96 engine-core pieces, reconstructed by cc13-render5.
// Evidence and ABIs: docs/CC13_ENGINE_CORE.md.

#include "bsp/engine_core_cc13.hpp"

#include <cstdio>
#include <cstring>

namespace bsp::cc13_engine_core {

namespace {

constexpr int kHistoryFrames = 20;                  // two passes of ten adds
constexpr double kHistoryFramesDouble = 20.0;       // 00CE3D88
constexpr double kOverlayTop = static_cast<double>(0.8f);        // 00CE3D40
constexpr float kOverlayLeft = 0.05f;                            // 00CE7638
constexpr double kBarPitch = static_cast<double>(1.3f);          // 00D21298
constexpr double kMilliseconds = 1000.0;                         // 00CE47A0
constexpr double kGridBottomMargin = static_cast<double>(0.2f);  // 00CE3D10
constexpr double kGridStepSeconds = static_cast<double>(1.0f / 60.0f); // 00D68688
constexpr float kGridLineWidth = 0.00156499993f;                 // 00D68690 (3ACD1ED9h)
constexpr std::uint32_t kGridColor = 0x701F1F1Fu;                // bytes 1F 1F 1F 70

bool names_equal(const CountedName& a, const CountedName& b) {
    // Native compare: lengths first; both empty match; one empty never matches.
    if (a.length != b.length)
        return false;
    if (a.length == 0)
        return true;
#if defined(_MSC_VER)
    return _stricmp(a.data, b.data) == 0;
#else
    return strcasecmp(a.data, b.data) == 0;
#endif
}

std::int32_t kilobytes(std::int32_t bytes) {
    return bytes / 1024;   // (x + (x >> 31 & 3FFh)) >> 10 truncates toward zero
}

} // namespace

double calibrate_ticks_per_second_00be3310(PerfClockHost& clock) {
    const std::int64_t before = clock.query_performance_counter();
    clock.sleep_ms(100);
    const std::int64_t after = clock.query_performance_counter();
    return static_cast<double>((after - before) * 10);
}

float ticks_to_seconds_00be3360(std::int64_t ticks, double ticks_per_second) {
    return static_cast<float>(static_cast<double>(ticks) / ticks_per_second);
}

void end_counter_record_00be32b0(ProfilerCounterRecord& record, PerfClockHost& clock) {
    if (--record.depth != 0)   // InterlockedDecrement on +20h
        return;
    const std::int64_t now = clock.query_performance_counter();
    record.accumulated_ticks += now - record.sample_start;
    record.last_exit = now;
}

void reset_counter_record_00be3300(ProfilerCounterRecord& record) noexcept {
    record.depth = 0;
    record.hit_count = 0;
    record.accumulated_ticks = 0;
}

void clear_profiler_history_00be3380(ProfilerCounters& profiler, int static_count) noexcept {
    const std::size_t ring_bytes = static_cast<std::size_t>(static_count * 5 + 0xFA) * 0x10;
    std::memset(profiler.history, 0, ring_bytes);
    std::memset(profiler.start_offsets, 0, ring_bytes);
    std::memset(profiler.current, 0, static_cast<std::size_t>(static_count * 4 + 200));
}

float profiler_history_sum(const float* history20) noexcept {
    float sum = 0.0f;
    for (int i = 0; i < kHistoryFrames; ++i)
        sum = sum + history20[i];   // FLD/FADD float/FSTP float per element
    return sum;
}

std::vector<std::string> format_perftime_lines_00be33e0(const ProfilerCounters& profiler,
    int static_count) {
    std::vector<std::string> lines;
    const float* history = profiler.history;   // slot 0, not slot 1
    for (int i = 1; i < static_count; ++i) {
        const float average =
            static_cast<float>(static_cast<double>(profiler_history_sum(history)) / kHistoryFramesDouble);
        history += kHistoryFrames;
        char buffer[0x100];
        std::snprintf(buffer, sizeof buffer, "PERFTIME[%d]:%f", i, static_cast<double>(average));
        lines.emplace_back(buffer);
    }
    return lines;
}

ProfilerOverlay build_profiler_overlay_00be3bf0(ProfilerCounters& profiler, float bar_height,
    bool offset_mode, int slot_count, const std::vector<std::string>& names) {
    ProfilerOverlay out;
    float y = static_cast<float>(kOverlayTop - bar_height);
    float x = kOverlayLeft;
    for (int slot = 1; slot < slot_count; ++slot) {
        const float* history = profiler.history + slot * kHistoryFrames;
        const float* starts = profiler.start_offsets + slot * kHistoryFrames;
        const float average =
            static_cast<float>(static_cast<double>(profiler_history_sum(history)) / kHistoryFramesDouble);
        profiler.current[slot] = average;
        const float start_average =
            static_cast<float>(static_cast<double>(profiler_history_sum(starts)) / kHistoryFramesDouble);
        if (average >= 0.0f) {   // FCOMI/JC: negative or NaN draws nothing
            const float width = static_cast<float>(
                static_cast<double>(average) / static_cast<double>(profiler.display_scale));
            if (offset_mode)
                x = start_average;   // persists into later slots, as the native local does
            out.quads.push_back({x, y, width, bar_height, profiler.colors[slot]});
            const std::string& name = names[static_cast<std::size_t>(slot - 1)];
            char buffer[0x100];
            std::snprintf(buffer, sizeof buffer, "%s %.2f ms count:%d", name.c_str(),
                static_cast<double>(average) * kMilliseconds, profiler.records[slot].hit_count);
            out.labels.emplace_back(buffer);   // the image calls 004C14C0 and drops it
        }
        y = static_cast<float>(static_cast<double>(y) - static_cast<double>(bar_height) * kBarPitch);
    }
    // Grid columns one 60 Hz frame apart, at least one.
    const float grid_height =
        static_cast<float>(static_cast<double>(static_cast<float>(1.0 - y)) - kGridBottomMargin);
    float gx = kOverlayLeft;
    do {
        out.quads.push_back({gx, y, kGridLineWidth, grid_height, kGridColor});
        gx = static_cast<float>(kGridStepSeconds / static_cast<double>(profiler.display_scale) +
                                static_cast<double>(gx));
    } while (1.0f > gx);
    return out;
}

void log_allocation_report_00be3fd0(AllocationReportHost* stats, const char* caption) {
    if (stats == nullptr)
        return;
    stats->log("=========================================================================\n");
    if (caption != nullptr)
        stats->log(caption);   // "%s" (00CEB484)
    char buffer[64];
    std::snprintf(buffer, sizeof buffer, "Alloc at startup:%8dK ", kilobytes(stats->startup_bytes()));
    stats->log(buffer);
    (void)stats->allocated_bytes();
    std::snprintf(buffer, sizeof buffer, "Phy allocated:%8dK", kilobytes(stats->allocated_bytes()));
    stats->log(buffer);
}

std::vector<CountedName> remove_content_package_name_00bdd020(
    const std::vector<CountedName>& names, const CountedName& name) {
    std::vector<CountedName> kept;
    for (const CountedName& entry : names)
        if (!names_equal(entry, name))
            kept.push_back(entry);
    return kept;
}

} // namespace bsp::cc13_engine_core
