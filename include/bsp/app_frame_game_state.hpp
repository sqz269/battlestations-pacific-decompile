#pragma once
#include <cstdint>

#include "bsp/app_frame.hpp"

// The three application-frame host methods that had no reconstruction behind
// them: the game-state slot the frame reads twice per frame, the profiler
// counter pair that brackets the frame, and the message-loop pretranslation.
// Evidence and uncertainties: docs/APP_FRAME_GAME_STATE.md.
//
// bsp::ApplicationFrameHost::game_state is not a call. 00e188a8 is the GGame
// singleton pointer and 5D4h is a field inside that object, so the frame reads
// an int through one indirection at 00737acc and again at 00737b33. There is no
// vtable at 00e188a8 and no function pointer at 00e18e7c, which holds four zero
// bytes in the image (00e18e6c is the unrelated menu-command singleton cache).
namespace bsp {

// ---------------------------------------------------------------------------
// The game-state slot, *(00e188a8)+5D4h
// ---------------------------------------------------------------------------

// The field the frame reads. It is owned by the game object, written by
// GGame::OnInit (3), GGame::OnInitTitle (2), the logo sequence entry (1) and
// the request drain 004e4430, which stores the dequeued value at 004e449e
// before dispatching on it. No other writer reaches it, so a host that owns
// this one integer answers the frame's read correctly.
//
// The value meanings live in the enumerations that already recovered them:
// bsp::GameFrontEndState (bsp/frontend_states.hpp) for 1, 2 and 4,
// bsp::GameStateId (bsp/game_frame_control.hpp) for the drain's set, and
// bsp::kGameStateFrontEndInit / kGameStateFrontEndShellReady
// (bsp/frontend_entry.hpp) for 3 and 5. Nothing new is enumerated here.
struct GameStateSlot {
    int value{0}; // game+5D4h
    // game+5E8h, the pending request count. The frame does not read it, but the
    // front-end branch does at 004e4ccb and every enqueue guard tests it.
    int pending_requests{0};
    // game+5ECh. While set, 004e4d03 skips the drain entirely.
    bool requests_held{false};
};

// 00737acc and 00737b33, the two reads. Both load the same field; the second
// result is dead in the native body, which is why the 60 frame run counts 120
// calls for one field.
int read_game_state_00737acc(const GameStateSlot& slot) noexcept;

// 004e449e, the drain's store. Separated from the queue mechanics in
// bsp::drain_state_requests_004e4430 so the executable can hold the field
// without also holding the deque.
void apply_drained_game_state(GameStateSlot& slot, int request) noexcept;

// ---------------------------------------------------------------------------
// Profiler counter records
// ---------------------------------------------------------------------------

// Layout constants read from the image. The record stride comes from the
// LEA EAX,[EAX+EAX*4] / LEA ECX,[ECX+EAX*8] pair in 00be3640 and 00be3660 and
// from the ADD EAX,0x28 induction step in 00be34d0.
inline constexpr int kProfilerRecordStride = 0x28;
inline constexpr int kProfilerDisplayStride = 0x1c;
// 00be362e, IDIV by 14h; the constructor sizes the two float rings as
// capacity * 14h elements (00be3896, LEA EBP,[EDI+EDI*4] then two ADD EBP,EBP).
inline constexpr int kProfilerHistoryFrames = 0x14;
// 00be384e. The constructor reserves the static counter count at 00e15118 plus
// this many spare slots for counters registered at run time.
inline constexpr int kProfilerSpareSlots = 0x32;
// 00d7a2f0, 3DCCCCCDh. The divisor the normalised start and end offsets use, so
// the profiler's bar graph is full scale at one tenth of a second.
inline constexpr float kProfilerDisplayScaleSeconds = 0.1f;
// 00d68680, 3CB60B61h. Stored at profiler+8h by the constructor and not read by
// any routine in this packet; 1/45 to seven digits.
inline constexpr float kProfilerReferenceFrameSeconds = 0.0222222218f;
// 00d7a270, 3FA99999A0000000h. The additive bias on both normalised offsets.
// The same .rdata cell is the single-player step threshold in
// bsp/game_frame_control.hpp; the two uses are unrelated.
inline constexpr double kProfilerNormalizedBias = static_cast<double>(0.05f);
// 00be3965..00be3975. Every slot colour starts at this dword. The channel order
// is not established: the application frame's own colour at 00e1ae94 is
// FF000000h, which is the opposite corner of the same layout.
inline constexpr std::uint32_t kProfilerDefaultSlotColor = 0xffffff0fu;

// One counter record, 0x28 bytes, in the array at profiler+14h.
struct ProfilerCounterRecord {
    std::int64_t accumulated_ticks{0}; // +00h
    std::int64_t sample_start{0}; // +08h
    std::int64_t first_entry{0}; // +10h
    std::int64_t last_exit{0}; // +18h
    std::int32_t depth{0}; // +20h, a LONG the native touches with interlocked ops
    std::int32_t hit_count{0}; // +24h
};

// One display record, 0x1C bytes, in the array at profiler+28h. 00be34d0 writes
// two of its fields; the rest were not identified and are not modelled.
struct ProfilerDisplayRecord {
    std::int32_t hit_count{0}; // +14h
    float end_offset{0.0f}; // +18h
};

// The profiler object's own fields, as pointers into storage the caller owns.
// This mirrors the native allocation rather than owning it: the constructor
// 00be3820 makes six separate allocations sized from one capacity.
struct ProfilerCounters {
    float display_scale{kProfilerDisplayScaleSeconds}; // +04h
    float reference_frame_seconds{kProfilerReferenceFrameSeconds}; // +08h
    int ring_index{0}; // +0Ch, advanced once per frame by 00be34d0
    int slot_count{0}; // the capacity every array below is sized from
    ProfilerCounterRecord* records{nullptr}; // +14h, slot_count entries
    float* history{nullptr}; // +18h, slot_count * kProfilerHistoryFrames
    float* start_offsets{nullptr}; // +1Ch, same shape as history
    float* current{nullptr}; // +20h, slot_count entries
    std::uint32_t* colors{nullptr}; // +24h, slot_count entries
    ProfilerDisplayRecord* display{nullptr}; // +28h, slot_count entries
};

// QueryPerformanceCounter, reached through the indirect slot 00ce2270. It is a
// host call because the native samples it only on the outermost entry and exit;
// sampling eagerly would change the observable call count.
struct ProfilerClockHost {
    virtual ~ProfilerClockHost() = default;
    virtual std::int64_t query_performance_counter() = 0;
};

// 00be3260. __thiscall, ECX = the record, no stack arguments, RET. Counts the
// hit, raises the depth, and on the outermost entry only samples the counter
// into sample_start, additionally into first_entry when the accumulator is
// still zero. The depth is raised with InterlockedExchangeAdd through 00ce22c4,
// whose previous value decides "outermost"; the API identity is inferred from
// the two pushed arguments and the use of the return value, not from a symbol.
void profiler_begin_counter_00be3260(ProfilerCounterRecord& record, ProfilerClockHost& clock);

// 00be3640. __thiscall(int slot), ECX = the profiler, RET 4. Converts the slot
// to a record and forwards. Slot 0 is skipped: the TEST EAX,EAX at 00be3644
// returns before touching anything.
void profiler_begin_frame_slot_00be3640(ProfilerCounters& profiler, int slot,
    ProfilerClockHost& clock);

// 00be3660. __thiscall(int slot), ECX = the profiler, RET 4. Lowers the depth
// with InterlockedDecrement through 00ce2220 and, when the new value is zero,
// samples the counter, adds the 64 bit delta into the accumulator and stores
// the end timestamp. Slot 0 is skipped the same way. Nested pairs therefore
// accumulate once, around the outermost pair.
void profiler_end_frame_slot_00be3660(ProfilerCounters& profiler, int slot,
    ProfilerClockHost& clock);

// 00737a9e and the one-time init at 00737a6c. The application frame writes the
// colour of its own slot into the array at profiler+24h; the value comes from
// bsp::frame_marker_color_00737a6c.
void profiler_set_slot_color(ProfilerCounters& profiler, int slot, std::uint32_t argb) noexcept;

// 00be34d0. __thiscall, ECX = the profiler, no stack arguments, RET, returns an
// int. Closes the frame for slots 1 through registered_slot_count - 1: slot 0
// is skipped by the loop's initial EDI, and the bound is the sum of the static
// count at 00e15118 and the run-time count at 0109cf18. The native also clears
// 0109cf18 at 00be361d; that global is not modelled here, so a caller that
// tracks run-time registrations must clear its own copy after this call.
// app_update_slot is the PERF_APP_UPDATE index held at 0109d014; every
// normalised offset is measured from that record's first entry.
// tick_scale is the double at 0109db48, zero in the image and filled at run
// time; a zero scale makes every converted value non finite, so the caller must
// supply the real one.
// The return value is (ring_index + 1) / kProfilerHistoryFrames, which is 1 on
// the frame the ring wraps and 0 otherwise. The application frame discards it.
int profiler_end_frame_00be34d0(ProfilerCounters& profiler, int registered_slot_count,
    int app_update_slot, double tick_scale);

// ---------------------------------------------------------------------------
// Message-loop pretranslation
// ---------------------------------------------------------------------------

// The native pretranslation is XLivePreTranslateMessage, called at 00bec1d8
// through the thunk 00c2f1d2, which jumps through the import slot 00ce25dc.
// 00bec20a is a different instruction: MOV byte ptr [ESI+43h],1, the loop
// finished store reached once at loop exit. docs/GAME_EXECUTABLE.md attributes
// the host method to 00bec20a; the corrected site is 00bec1d8.
inline constexpr std::uint32_t kPlatformPretranslateCallSite = 0x00bec1d8u;
inline constexpr std::uint32_t kPlatformPretranslateThunk = 0x00c2f1d2u;
inline constexpr std::uint32_t kPlatformPretranslateImportSlot = 0x00ce25dcu;
inline constexpr int kXLivePreTranslateMessageOrdinal = 5030;

// 00bec1dd. A nonzero result skips TranslateMessage and DispatchMessageA
// entirely, so a consumed message never reaches the window procedure. With no
// XLive library bound there is nothing to consume the message and the loop must
// dispatch it, which is what the existing executable already does by returning
// false; this states the rule rather than leaving it implicit.
bool platform_pretranslate_consumes_00bec1d8(bool xlive_library_bound,
    bool xlive_result_nonzero) noexcept;

} // namespace bsp
