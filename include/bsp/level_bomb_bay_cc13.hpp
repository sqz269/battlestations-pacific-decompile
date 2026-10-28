#pragma once

// Packet cc13-lbbay: the level bomber's bomb bay. Two switches, both OFF.
// Background: docs/CC13_RELEASE_TIMER.md section 10.

namespace bsp {

// kLevelBombBayOpenerCc13Bound (a): the level-bomb task as a mode source of the
// bay opener (GameUnitsHost::Impl::bay_open_step_cc13, 0099AFCC-0099B055).
// 009B8450 BSP_BotTaskLevelBomb_StepResult, the task's vtable[+50h] (disk listing
// 009B8450-009B849B):
//   - task+310h (the current state) is one of attackrun +5CCh, aim +5F4h,
//     prepare +610h, release +6B4h or goaway +6D8h: answer task+4C4h ? 2 : 0
//     (009B8488 MOV AL,[ECX+4C4h]; NEG; SBB; AND 2);
//   - otherwise answer 1.
// task+4C4h is approach+CCh (the approach sits at task+3F8h; +C8h = task+4C0h,
// +CEh = task+4C6h). The host keeps it as GameUnitSlot::lb_ap.close_cc.
// Its writers, from a scan of 009B3E00-009B9200 for byte stores at +CCh:
//   009B45E8 (the approach constructor), 009B7CAC (cleared by the approach
//   update 009B7C90), 009B6394 (aim: kAimBayClose > impact distance),
//   009B688A (prepare's wait arm: the leader's 9C1h), 009B59FC (release tick:
//   1), and the setter 009B59D0.
// The host already writes all of them except the setter (src/plane_level_bomb_task.cpp
// and lb_release_tick_009b59f0). Mode 2 makes unit+DF0h = 9C3h (007BB150), and
// 007C6A8D then sets channel C's target to 9C3h.
// The wingmen's +424h passthrough 009B8BD0 -> 007BBBA0 (kLevelBombReleaseOrderSpendBound)
// raises unit+C20h, which 007C6A8D also reads. With this switch, a level bomber
// reaches 007C6A8D, so that path opens the bay as well.
// OFF: a plane with neither a torpedo nor a dive-bomb task is left alone, as before.
// ON-only line: "summary level-bomb bay opener steps= mode2=".
inline constexpr bool kLevelBombBayOpenerCc13Bound = false;

// kLevelBombDropBayGateCc13Bound (b): the bay test of 007CC8E0
// BSP_Plane_DropAttitudeGate for a level bomber (IsKindOf 10h), applied before
// the |bank| / |pitch| <= tuning+550h test in the rack tick 006E56F0 (called at
// 006E585F). 007CC8F4-007CC928 is 007BB110 inlined:
//   - channel = [plane+DECh]+60h; if it is disabled, pass;
//   - if its value +64h is exactly [00D7A24C] = 1.0f (ordered equal,
//     LAHF / TEST AH,44h / JNP), pass;
//   - otherwise refuse while byte [plane + 9C3h + 8*word[00F876B8]] != 0.
// The test covers the whole plane, so every level-bomber rack is gated,
// paratrooper (31h) racks included. The rack slot argument reaches only 007C7600.
// The host evaluates it with cc13_release_timer::unit_can_release_007bb110
// (channel_c, bay_rack_holds_round_9c3, read live). A refusal counts in
// gate_refused and in the ON-only per-unit "levelbomb ... bay_refused=" line.
// OFF: the bay test is skipped, as before.
inline constexpr bool kLevelBombDropBayGateCc13Bound = false;

// Without the opener, a BayDoor level bomber's channel C stays at target 0,
// value 0, and the gate would refuse every drop while a bay rack holds a round.
static_assert(!kLevelBombDropBayGateCc13Bound || kLevelBombBayOpenerCc13Bound,
              "the level-bomb bay gate needs the level-bomb bay opener");

// Expected movers when ON:
//   (a) bay target writes and the bay animation for level bombers with
//       PartAnims.BayDoor. The bay opens in aim (once the impact point is
//       within kAimBayClose), in release and in prepare (following the
//       leader), or after a passthrough release, and closes once the bay racks
//       are empty. Alone, (a) changes no drop.
//   (b) level-bomb and paratrooper drop times for those classes: a drop waits
//       for channel C to reach 1.0f (1/rate seconds after it opens). gate_refused
//       rises. Classes without a BayDoor anim (channel disabled) and planes
//       whose bay racks are empty do not move.

// 009B8450 for the host's state: 2 or 0 from close_cc in the attacking
// states, 1 otherwise.
constexpr int level_bomb_step_result_009b8450(bool attacking, bool close_cc) noexcept {
    return attacking ? (close_cc ? 2 : 0) : 1;
}

}  // namespace bsp
