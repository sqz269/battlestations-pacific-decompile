// The CommandBuilding capture: the neutralize at health 0, the one-second
// countdown, the capture tick's strength sums and progress rule, and the flip.
//
// Packet cc9_command_building_capture_bind, worker agent/cc9-ships21. Project
// C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra was
// read-only for this packet. Every descriptive name is a hypothesis, not a
// recovered symbol. docs/SHIP_AI_OPEN_ITEMS.md sections 78 and 80 carry the
// evidence.
//
// | routine | address | coverage |
// | --- | --- | --- |
// | health-changed slot vtable[1B0h] | 006F3270-006F32A1 | complete (the health > 0 arm is the base 00958A30, not modelled) |
// | neutralize route | 006F2940-006F29C4 | complete |
// | fixed-step countdown | 006F755D-006F761C inside 006F7360 | partial: the neutral arm only; the level clock and the garrison scan are not modelled |
// | capture tick | 006F6760-006F7353 | partial: the progress rule, the side and the completion test; the strength sums are the caller's |
// | D3h handler | 006F4D10-006F5454 | partial: the party, +528h, +7B0h and the repair decision; announcements, D4h, the flag model and the per-pad 006AC4D0 are records |
#pragma once

#include <cstdint>

namespace bsp {

// 006F2780: `CaptureValue` stored at +7A4h, 1000 when unauthored (006F2780's
// find arm, local\s21_2780.txt line 36).
inline constexpr std::int32_t kCommandBuildingCaptureValueDefault = 1000;
// The constructor stores +7BCh = 1.0f (docs/LAND_AND_STRUCTURES.md section 4);
// 006F7603 reloads the countdown from it.
inline constexpr float kCommandBuildingCapturePeriod = 1.0f;
// [MultiLobbyOptionRegistry+0C0h], CaptureSettings.FallbackCapturePower:
// 008D3C3E..008D3C6D reads it with the default 20.0f (00CE3930). LABELLED: this
// process does not load MultiGlobals.lua (game_hosts_lua.cpp, 008d2f50 is a
// record), so the value is this installation's scripts/datatables/
// multiglobals.lua line 1222 (mtime 2024-07-13), which authors 20.
inline constexpr float kCommandBuildingFallbackCapturePower = 20.0f;

// The building's capture cells.
struct CommandBuildingCaptureState {
    std::int32_t party_54{2};           // unit+54h
    std::int32_t capture_value_7a4{kCommandBuildingCaptureValueDefault};
    float progress_7a8{0.0f};           // positive is party 0's
    std::int32_t side_7ac{2};           // the last tick's winning side
    std::int32_t prior_party_7b0{2};    // 006F3284 / 006F5099
    std::int32_t prior_slot_7b4{-1};    // 006F3290, from unit+2D8h
    float countdown_7c0{0.0f};
    std::int32_t first_slot_528{-1};    // 006F4D48
};

// 006F755D..006F761C: the neutral arm of the fixed step. With the building
// neutral, `step >= countdown` (006F75FF FCOMI, JB) reloads the countdown as
// (+7BCh - step) + countdown (006F7603 FSUBR, 006F760F FADDP) and answers true
// (the tick 006F6760 runs); otherwise countdown -= step (006F763C). A building
// that is not neutral answers false and leaves the countdown alone (the level
// clock arm is not modelled).
bool command_building_capture_countdown_006f75ed(CommandBuildingCaptureState& state,
                                                 float step);

// One party's strength sum as 006F69D2..006F69ED builds it: ftol((float)power *
// modifier + (float)sum), with the x87 extended precision taken as double.
std::int32_t command_building_add_capture_power_006f69d2(std::int32_t sum, float power,
                                                         float modifier);

struct CommandBuildingTickOutcome {
    bool progress_changed{false};  // 006F6FC8..: |new - old| > 0, so D5h is routed
    bool completed{false};         // |progress| >= (float)CaptureValue
    std::int32_t side{2};          // local_174: 0, 1, or 2 on a tie
};

// 006F6DEF..006F7334 over the two strength sums (the arms are the caller's),
// for a neutral building: the side, the progress rule with the fallback decay
// `fallback`, the D5h change test and the completion test. On completion the
// progress is zeroed (006F71FB..) and +7ACh gets the side; otherwise +7ACh gets
// the side too (006F70E0 branch). The owner's-opponent division at
// 006F6DB0..006F6DEE is not modelled: it runs only for a party < 2 building,
// which the only caller 006F7617 never passes.
CommandBuildingTickOutcome command_building_capture_tick_006f6760(
    CommandBuildingCaptureState& state, std::int32_t strength0, std::int32_t strength1,
    float fallback);

// 006F3270 at health <= 0: answers true when the building was owned and is now
// neutralized (006F3284..006F3296 then 006F2940 -> D3h slot 9 -> 006F4D10's
// slot-9 arm). `prior_slot_2d8` is unit+2D8h. A neutral building answers false.
bool command_building_neutralize_006f3270(CommandBuildingCaptureState& state,
                                          std::int32_t prior_slot_2d8);

struct CommandBuildingFlipOutcome {
    std::int32_t new_party{2};
    bool repair{false};   // 006F5019..006F5073: 006F47F0 on the building and garrison
};

// 006F4D10 for a slot 0..7 whose record's Party is `slot_party`, or slot 8
// (none eligible) with slot_party ignored: the new party, +528h, and the repair
// decision. `prior_slot_party` is [slot record +7B4h]+28h, or -1 when +7B4h is
// not a slot (006F4FC2 JA on the unsigned compare).
CommandBuildingFlipOutcome command_building_flip_006f4d10(CommandBuildingCaptureState& state,
    std::int32_t slot, std::int32_t slot_party, std::int32_t prior_slot_party);

}  // namespace bsp
