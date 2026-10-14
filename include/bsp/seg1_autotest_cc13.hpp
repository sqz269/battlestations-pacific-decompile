#pragma once
// Segment 1 (00413D10-0042A830), cc13-camera2: the command-line automation
// driver ("auto mpak classes", "auto mpak scenes", "auto test <name>", see
// docs/APP_INIT_BOOTSTRAP.md) and its watchdog, crash hook and _auto_.log
// writers. docs/CC13_SEG1_AUTOTEST.md has the evidence. Names are hypotheses.
//
// The driver object is the 50h-byte singleton returned by 004C1E90 (ledger
// name BSP_ProfileHintsOwner_GetSingleton, constructed by 00426250); the
// automation fields are:
//   +4h  watchdog thread handle      +8h  mode: 1 mpak classes, 2 mpak scenes, 3 test
//   +Ch  frames until the next step  +10h movie frames while a movie plays
//   +14h word, +16h byte             +18h campaign: 0 usa, 1 japan, 2 multi, 3 training
//   +1Ch mission index (0-based)     +24h, +28h, +30h counters
#include "bsp/frame_clock.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace bsp::seg1_cc13 {

struct AutoTestDriver {
    void* watchdog_thread_04{nullptr};
    std::int32_t mode_08{0};
    std::int32_t countdown_0c{0};
    std::int32_t movie_frames_10{0};
    std::uint16_t word_14{0};
    std::uint8_t byte_16{0};
    std::int32_t campaign_18{0};
    std::int32_t mission_1c{0};
    std::uint8_t byte_20{0};            // 00427190: 0 on screen 8, 1 on screen 10; screen 9 tests it
    std::int32_t f_24{0};
    std::int32_t f_28{0};
    std::int32_t f_30{0};
};

// 00425560 __fastcall(driver): "auto mpak classes": mode 1, first step after 200 frames.
void autotest_select_mpak_classes_00425560(AutoTestDriver& d) noexcept;
// 00425570 __fastcall(driver): "auto mpak scenes": mode 2, +18h/+1Ch/+28h = 0,
// +14h/+16h = 0, and +24h = 1 when the byte 00E17659 is set.
void autotest_select_mpak_scenes_00425570(AutoTestDriver& d, bool flag_00e17659) noexcept;

// 004256F0 __thiscall(left, ClockTimestamp* out, const ClockTimestamp* right)
// RET 8: out = left + right in left's frequency (right rescaled through
// __allmul / __alldiv when the frequencies differ). Differs from 00BEDB70,
// which adds in place.
ClockTimestamp add_timestamp_to_004256f0(const ClockTimestamp& left, const ClockTimestamp& right) noexcept;
// 00425780 __thiscall(left, const ClockTimestamp* right) RET 4 -> AL: left <
// right; with different frequencies only whole seconds (count / frequency,
// __alldiv) are compared.
bool timestamp_less_00425780(const ClockTimestamp& left, const ClockTimestamp& right) noexcept;

struct AutoTestHost {
    virtual ~AutoTestHost() = default;
    // [01090AB0]->vtable[20h](&out): the frame clock's current time.
    virtual ClockTimestamp clock_now() = 0;
    // 01090AB8 = 00425B80: the fatal-message hook.
    virtual void install_fatal_hook_00425b80() = 0;
    virtual std::uint32_t current_thread_id() = 0;   // [00CE223C] GetCurrentThreadId
    // [00CE2238] CreateThread(0, 0, 004258D0, 0, 0, 0).
    virtual void* create_watchdog_thread_004258d0() = 0;
    virtual void sleep_ms(std::uint32_t ms) = 0;     // [00CE2230] Sleep
    virtual bool finished_00e1ae75() = 0;            // byte 00E1AE75
    // fopen(path, "w") / fprintf / fclose; false when fopen fails.
    virtual bool write_file(const char* path, const std::string& text) = 0;
    // [00CE2224](GetCurrentProcess(), code): TerminateProcess.
    virtual void terminate_process(std::uint32_t code) = 0;
    // A finished StringBuilder line (00426500 ... 00425F80), the _auto_.log sink.
    virtual void log_line(const std::string& line) = 0;
};

// The globals 004278A0 writes before parsing.
struct AutoTestGlobals {
    ClockTimestamp start_00e08060;
    std::uint32_t main_thread_00e17660{0};
};

// 004278A0 __thiscall(driver, const NativeString* name) RET 4 ("auto test
// <name>"): installs the fatal hook, records the start time and the main
// thread id, starts the watchdog thread (+4h), then parses
// "LoadWaitQuit:<campaign>,<mission>" (split on ':' then ',' by 00BD20A0;
// the test name and the campaign compare case-insensitively): mission + 1 is
// stored as +1Ch = atol - 1 ("" when missing), +18h from usa / japan / multi /
// training (unchanged otherwise), mode 3. Any other test name leaves the mode.
void autotest_parse_test_004278a0(AutoTestDriver& d, const std::string& name, AutoTestGlobals& g,
                                  AutoTestHost& host);

// 004258D0, the watchdog thread (RET 4): while now < start + 900 s (00425780
// against 004256F0(00E08060, {900, 1})) sleep 1 s and stop when 00E1AE75 is
// set; on time-out write "Timed out.\n" to crash.log and terminate with exit
// code 3. Returns 0.
std::uint32_t autotest_watchdog_004258d0(const AutoTestGlobals& g, AutoTestHost& host);

// 00425B80 __fastcall(const NativeString* message): the fatal hook: crash.log
// gets the message ("%s\n", "" for an empty string), then exit code 2.
void autotest_fatal_hook_00425b80(const std::string& message, AutoTestHost& host);

// _auto_.log writers (StringBuilder lines): 00426E60 "GROUP <name>" (RET 4),
// 00426ED0 "ENDGROUP", 00426F30 "LOG <name with spaces as '_'> <value>"
// (RET 8), 00427040 "GROUP Memory" then "ENDGROUP" (no lines between in this
// build).
void autolog_group_00426e60(const std::string& name, AutoTestHost& host);
void autolog_end_group_00426ed0(AutoTestHost& host);
void autolog_value_00426f30(const std::string& name, const std::string& value, AutoTestHost& host);
void autolog_memory_00427040(AutoTestHost& host);

// 00425B20 __fastcall(vector of 18h-byte records) -> count of records whose
// byte +11h is nonzero.
struct FlaggedRecord {
    std::uint8_t bytes[0x18]{};
};
std::uint32_t count_flagged_records_00425b20(const std::vector<FlaggedRecord>& records) noexcept;

// 00426350 __thiscall(driver, int screen) RET 4 -> AL: when the active GUI
// screen ([[[00E198ACh]+58h]+1B8h], 00A9C990) is `screen`, invoke the
// screen's +8h object slot 4 with 00425E50's value and return 1; otherwise ask
// for `screen` (00A9C860) and return 0.
struct ScreenHost {
    virtual ~ScreenHost() = default;
    virtual std::int32_t active_screen_00a9c990() = 0;
    virtual void activate_current() = 0;            // [screen+8h]->vtable[4](00425E50(...))
    virtual void request_screen_00a9c860(std::int32_t screen) = 0;
};
bool autotest_ensure_screen_00426350(std::int32_t screen, ScreenHost& host);

}  // namespace bsp::seg1_cc13
