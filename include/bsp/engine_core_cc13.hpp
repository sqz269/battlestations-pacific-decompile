#pragma once

// Segment 96 engine-core pieces reconstructed by cc13-render5: the performance
// counter calibration and tick conversion, the standalone profiler counter
// record, the profiler history reset / PERFTIME dump / bar overlay, the startup
// memory report, and two VFS manager list edits. Names are hypotheses, not
// recovered symbols. Evidence, ABIs and open questions are in
// docs/CC13_ENGINE_CORE.md. None is a drop-in binary replacement.
//
// The profiler types come from bsp/app_frame_game_state.hpp (00BE3260,
// 00BE3640, 00BE3660, 00BE34D0 are reconstructed there). The VFS manager is the
// object at [0109CEEC]; its other members are reconstructed in the
// native_vfs_* sources.
//
// x87 model: the image runs with 53-bit precision; every FSTP to a float slot is
// a float cast and unspilled intermediates are doubles.

#include "bsp/app_frame_game_state.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace bsp::cc13_engine_core {

// Performance-counter services. Each method names its import.
class PerfClockHost {
public:
    virtual ~PerfClockHost() = default;
    virtual std::int64_t query_performance_counter() = 0;   // [00CE2270]
    virtual void sleep_ms(std::uint32_t ms) = 0;            // [00CE2230] Sleep
};

// 00BE3310, plain RET, result in ST0. Two QueryPerformanceCounter samples around
// Sleep(100); the 64-bit difference times 10 (__allmul 00BF7DF0), loaded as a
// double. The static initializer at 00CD8F30 stores it to the double [0109DB48],
// so that global is an estimate of counter ticks per second, and every
// converted profiler value is in seconds.
double calibrate_ticks_per_second_00be3310(PerfClockHost& clock);

// 00BE3360, ECX = 64-bit tick count, plain RET, ST0: (float)(ticks / [0109DB48]).
float ticks_to_seconds_00be3360(std::int64_t ticks, double ticks_per_second);

// 00BE32B0, ECX record, plain RET. Interlocked depth decrement (+20h); when it
// reaches zero: accumulated (+0h) += now - sample_start (+8h), last_exit (+18h) =
// now. The begin side is 00BE3260. Called by BSP_FrameStats_Accumulate.
void end_counter_record_00be32b0(ProfilerCounterRecord& record, PerfClockHost& clock);

// 00BE3300, ECX record: depth, hit count and accumulated ticks to zero (the
// sample start, first entry and last exit words are left alone).
void reset_counter_record_00be3300(ProfilerCounterRecord& record) noexcept;

// 00BE3380, ECX profiler, called by BSP_Profiler_Construct: zero both history
// rings (+18h, +1Ch; (n*5 + 0FAh)*10h bytes each) and the current array (+20h;
// n*4 + 200 bytes), n = the static counter count [00E15118]. That is
// (n + 50) * 20 floats per ring, the constructor's capacity.
void clear_profiler_history_00be3380(ProfilerCounters& profiler, int static_count) noexcept;

// One slot's 20-frame history sum as 00BE33E0/00BE3BF0 compute it: ten float
// adds per inner pass, every partial sum stored to a float.
float profiler_history_sum(const float* history20) noexcept;

// 00BE33E0, ECX profiler, plain RET; unreferenced. For i = 1 .. static_count-1
// formats "PERFTIME[%d]:%f" (00D68668) with (double)(float)(sum / 20.0) into a
// stack buffer that is then discarded. The history pointer starts at slot 0
// and advances 20 floats per line, so label i prints slot i-1 (the overlay
// starts at slot 1). Returned here instead of discarded.
std::vector<std::string> format_perftime_lines_00be33e0(const ProfilerCounters& profiler,
    int static_count);

// 2D debug quad the overlay emits through renderer vt+D4h
// (BSP_Renderer_AppendDebugRecord40 00B29330) with texture 0 and uv (0,0)-(1,1).
struct OverlayQuad { float x, y, w, h; std::uint32_t color; };

struct ProfilerOverlay {
    std::vector<OverlayQuad> quads;   // bars first, then the grid columns
    std::vector<std::string> labels;  // formatted and dropped by the image
};

// 00BE3BF0, ECX profiler, plain RET; called by BSP_Game_Render (004CA694) when
// game+2191h is set. slot_count = [0109CF18] + [00E15118]; names = the 1Ch
// std::string table at 0109D038 (slot i at index i-1); offset_mode = byte
// profiler+10h. Writes profiler.current[slot] for every slot >= 1.
ProfilerOverlay build_profiler_overlay_00be3bf0(ProfilerCounters& profiler, float bar_height,
    bool offset_mode, int slot_count, const std::vector<std::string>& names);

// Allocation statistics object [0109CEFC] as the report reads it.
class AllocationReportHost {
public:
    virtual ~AllocationReportHost() = default;
    virtual std::int32_t startup_bytes() = 0;     // stats+08h
    virtual std::int32_t allocated_bytes() = 0;   // stats vt+08h (called twice, first result dropped)
    virtual void log(const std::string& line) = 0; // 004254B0, an empty body in this build
};

// 00BE3FD0, cdecl (const char* caption), plain RET. Nothing happens when
// [0109CEFC] is null (has_stats false). Kilobytes are signed divisions by 1024
// rounded toward zero (the +3FFh bias for negatives).
void log_allocation_report_00be3fd0(AllocationReportHost* stats, const char* caption);

// Counted string {length, data} as the VFS manager stores names.
struct CountedName { std::int32_t length; const char* data; };

// 00BDD020, ECX VFS manager, stack name*, RET 4. Removes every entry of the
// manager+48h name vector (count +4Ch, 8-byte elements) equal to the name
// (same length, then _stricmp; two empty names match) through the vector erase
// 0043EA00. Callers log "Remove active content package (%s)" (00995150) and
// "Damaged content %s" (00995690), so the vector holds active content-package
// names. Returns the surviving names in order.
std::vector<CountedName> remove_content_package_name_00bdd020(
    const std::vector<CountedName>& names, const CountedName& name);

} // namespace bsp::cc13_engine_core
