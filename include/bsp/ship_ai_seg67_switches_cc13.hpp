#pragma once
// Packet cc13-seg67h: OFF switches for the seg67 mismatches the seg67f audit
// confirmed (docs/CC13_SEG67D.md adoption notes C, D, F and I). (G) and (H)
// live in bsp/cc13_ship_ai_commit_band.hpp.
namespace bsp {

// (I) 009E08EF. The torpedo crossing margin of 009E04E0 reads the own class's
// Length: 009E089A [ESI+3FCh], 009E08CB [EDX+538h], 009E08EF FLD [EAX+0A0h],
// times 0.6 [00CEFF98]. The host bound unit+9C8h, which in the image equals
// class+A0h only for a class without a model box (00811058).
// True: the margin reads the class Length (GameUnitsHost::unit_class_extents,
// the slot's class+A0h). False: unit+9C8h, as before.
inline constexpr bool kShipAiTorpedoMarginClassLengthBound = true;  // ON: USN04/JM08/USN21 pairs identical (SHIP_AI 302)

// (C) 009EE586..009EE594. Every 009ED6B0 pass that reaches 009EE580 (every
// tick the station-keeping arm does not run) clears blk+38Ah, +388h and +389h
// before the path-pick gate. The host kept the latches across skipped ticks.
// True: the three station latches clear there. False: they persist.
inline constexpr bool kShipAiStationLatchClearBound = true;  // ON: JM08 moved from LSM 01 at 725.95 s, USN04/USN21 identical (SHIP_AI 302)

// (D) 009EE6F1..009EE750. After the path-pick gate opens and before the output
// block and the tail, the parked byte blk+2FDh is computed:
//   2FDh = ([[blk+2F4h]+1Ch] == 0) ? 1
//        : ([blk+310h] >= [blk+30Ch] (signed SETGE) && !blk+2FCh && record+20h)
// The tail's parked stop 009EEF72 reads it, and 009DA610 reads the same byte.
// The host set it only at 009EF045. A sibling of (C), not the same switch:
// different fields, different consumers, different addresses.
// True: the byte is computed there into both host copies (tail.parked_2fd and
// goal.flag_2fd). False: the host's previous single write at 009EF045.
inline constexpr bool kShipAiParkedByteBeforeTailBound = true;  // ON: gameplay identical on USN04/JM08/USN21 (SHIP_AI 302)

// (F) 009EF94E/009EF96F. Every turn-clearance refresh resets blk+37Ch to
// [00CE3D64] = 10000.0f (0x461C4000) before its lowerings. The host's
// kShipAiClearanceSentinel is 9999.0f. Of its readers only this store stands
// for 00CE3D64. The ShipAiClearanceBlock member default
// (ship_ai_clearance_profile.hpp:108) is the construction value: in the image
// 009DFCB0 (ECX = blk+1C4h) stores self+1B8h = blk+37Ch = 0.0f at 009DFDA9.
// That value is unobservable: 009DFCB0 also leaves +374h = 0.0f (self+1B0h),
// every other +374h writer stores -1.0f, so the first 009EF910 call always
// falls through its timer (009EF93D JA, not taken for 0 - dt or -1 - dt) and
// stores 37Ch at 009EF96F; 009F4D10 makes that call (009F4D87) before
// 009F50E0 reaches 009F4DA0 -> 009F3F80's reader at 009F416E (009F5227,
// then 009F5248, straight-line). So the default is left as it is.
// True: the refresh stores 10000.0f. False: 9999.0f, as before.
inline constexpr bool kShipAiClearanceRefreshSentinelBound = true;  // ON: gameplay identical on USN04/JM08/USN21 (SHIP_AI 302)
inline constexpr float kShipAiClearanceRefreshSentinel = 10000.0f; // 00CE3D64
static_assert(kShipAiClearanceRefreshSentinel == 0x1.388p+13f);    // 0x461C4000

// (D') 009ED7E6..009ED7F3 sets [ESP+4Fh] = (blk+1C4h == 3); 009EE756
// CMP [ESP+4Fh],0 / JNE 009EF206. In mode 3 (written by the attack-move
// approach step at 009F3375) the image still runs the 009ED6B0 prologue, the
// 009EE586 clears, the 009EE59B gate, the path pick 009EE5BA..009EE66C, the
// output prelude 009EE671..009EE6CB (A90h = 1, A94h/A98h/A9Ch = path x, 0,
// path z, and 1F0h = max(1F0h, 009D9E50([blk+2F4h]))) and the 2FDh writes
// 009EE6F1..009EE750, then skips the rest of the output block 009EE765..009EEAA2
// and the tail 009EEAAB..009EF1FE and goes straight to 009EF206: 009DE5B0(dt),
// AL = [ESP+37h] = 0 (set at 009EDA2F).
// True: the host does the same for NavigateAstern. False: the host runs the
// output block and the tail for mode 3 as for mode 2.
// ON (cc9-gunnery53, SHIP_AI_OPEN_ITEMS 300 ### 2): flipped by mechanism after an
// audit found the host's approach step makes every 009F3240 steering store. JM08
// changes from an HQ-capture failure at 977.93 s to a no-end; JM05 and USN04/07/13
// keep their outcomes.
// OFF again (cc9 lead, 2026-10-19, labelled INTERIM): cc9-lua70 found the ON
// path stalls every attackmove unit in mode 3. It publishes the goal and
// returns before the 009EE765 output, so d32c stays 0.00 on every step and the
// throttle swings 0.5/-0.625. Examples: LOMP10 Ashigara 3113/3113 steps;
// ESMP08 Iowa 1848/1848; USN02, USN15 and IJN18 attack forces. The l69 base has
// none. With only this switch OFF, LOMP10 completes at 1072.47 s. Whatever
// steers mode-3 units in the image is unhosted; re-flip once cc13 binds it.
// Name note (packet cc13-astern3a6): "NavigateAstern" is a misnomer kept here
// because cc9 flips this switch by name. Mode 3 is the caller-steered navigate
// the attackmove approach step enters (provisional NavigateCallerSteered; see
// ShipAiSteeringMode in ship_ai_states.hpp and docs/CC13_MODE3STEER.md).
inline constexpr bool kShipAiNavigateAsternSkipBound = true;  // re-ON with the arc-centre plan test (SHIP_AI 314)

// 009EE59B/009EE59F. When blk+1C4h is neither 2 nor 3 (Rudder or Heading),
// the path gate JZ 009EF206 still reaches 009EF206: 009DE5B0(dt), then
// AL = [ESP+37h] = 0. The host's closed-gate return
// (game_hosts_ship_ai.cpp:10956) skipped it under either value of
// kShipAiNavTailGateBookkeeping (packet cc9_ship_ai_tails_2).
// A sibling of kShipAiNavTailGateBookkeeping, which only chooses the
// bookkeeping record.
// True: the closed gate makes the same arm-final call before returning false.
// False: it returns without it, as before.
// ON (cc9-gunnery53) by mechanism, from pairs on bd61f8f4f at 30000 frames:
//   - 009EE59F JZ 009EF206 and 009EF213 CALL 009DE5B0 were read from the disk bytes.
//   - USN04, reference env: gameplay identical; 009DE5B0 539876 -> 540000 calls.
//   - JM08, reference env: still fails at the HQ check (CheckHQ, Party == 2), now at
//     958.94 s instead of 977.93 s.
//   - JM08, default env: the same failure, 941.94 s instead of 963.94 s.
// SHIP_AI_OPEN_ITEMS 300.
inline constexpr bool kShipAiNavGateClosedArmFinalBound = true;

// cc13-mode3steer (rev4's bytes; docs/CC13_MODE3STEER.md). The nav-block
// constructor 009E4330 calls 009DFCB0 at 009E43CC with ECX = blk+1C4h
// (009E43A4 LEA ECX,[ESI+1C4h]). 009DFD49 MOVSS XMM2,[00CE3804] (1000.0f) is
// stored at 009DFEA8 as blk+3E0h. Its only reader is the prologue: 009ED918
// copies it to 32Ch for any non-Stopped direction, and 330h = 32Ch + 2000. The
// host left ShipAiControlBlock::cruise_distance_3e0 at 0, hence d32c = 0 /
// d330 = 2000 on every mode-3 row.
// What 32Ch = 0 does (wire1, bytes): nothing on the mode-3 path reads 32Ch for
// throttle or arrival. The -0.625 hunt is the 009F3F80 sector-escape 364h
// ahead/astern flip (009EC9DB/009EC9E5 negate the ceiling and floor it at
// -0.625, [00D21A78]; a 1 s 368h dwell, 009F4AA0..009F4B95). 32Ch = 0 keeps it
// going through 009F0100's neighbour clear at 009F06D4..009F06EC (so no pass
// side is posted), the arm-final query range (009DE633 -> 009DC313), the
// leader corridor cap (009DEEBF) and the slot+40h crossing negotiation
// (009D8CE0).
// True: the nav-block build seeds 3E0h (for controllers created after load
// too). False: it stays 0, as before. The other 009DFCB0 mismatches are left to
// a separate switch (docs/CC13_MODE3STEER.md section 4).
// NOT neutral with the skip OFF: mode 0 and 1 ships with a non-Stopped
// direction also take 009ED918.
// Expected ON signature: mode-3 rows show d32c=1000.00 / d330=3000.00, and the
// l70_astern.py d32c0 count falls to 0 (today LOMP10 Ashigara 3113/3113; IJN18
// Gear3 406/406, Oma4 401/401, Gear1 366/366). Pass criteria: that signature;
// pass_posts and traffic_writes > 0; free bearing > 263; fewer -0.625 steps;
// LOMP10 completing near 1072 s (not guaranteed).
// Pairing plan: (1) this ON + kShipAiNavigateAsternSkipBound ON against both
// OFF, on LOMP10 and IJN18; (2) this ON against OFF with the skip OFF. Re-flip
// the skip only after (1) passes. cc9-lua52's local\l70_astern.py does the
// d32c census.
inline constexpr bool kShipAiCruiseDistanceSeedBound = true;  // ON: skip OFF, LOMP10 identical, USN21 d32c 0 -> 1000 and -0.625 71 -> 46 (SHIP_AI 311)

// Packet cc13-navseeds2 (docs/CC13_NAVSEEDS2.md). Three more 009DFCB0 seeds
// (ECX = blk+1C4h) the host starts at zero:
//  - 009DFDB7 MOV byte [ECX+1A0h],1: blk+364h = 1, the committed-ahead
//    direction. 009ED9D1 picks 00438B10 (subtract) on it and 009EC9CD keeps
//    the throttle ceiling positive; with it clear the ceiling is negated and
//    floored at -0.625. The host's ShipAiControlBlock::yaw_rate_subtracts_364
//    starts false, so a ship that never runs the escape (the only image
//    writers after the seed, 009F4B0F = 0 and 009F4B74 = 1) stays astern.
//  - 009DFD79: blk+384h = -1.0f. Seeded as stall_time_384 = -1 with the
//    direction_counter_384 marker = 1 (no direction-change zero yet), so the
//    stall 009F47EE accumulates from -1 until the first zero. Long term the
//    host should hold blk+384h as one float, not the int marker plus the float.
//  - 009DFD71: blk+354h = -1.0f, both host copies (blk.clamp_354 and
//    throttle_profile.hold_354). OFF starts the hold at 0, and 009E0591 counts
//    it down on the first 009E04E0 pass even when nothing raised it (0 >= 0
//    gives -dt), so OFF differs only until that first countdown. Separately,
//    the host sends the 009F00E3 raise to ShipAiClearanceBlock::hold_354,
//    which nothing reads, while the image raises the same blk+354h that
//    009DE8FF tests; this switch does not route that raise.
// Lifetime: written once in the controller build, before the first 009ED6B0;
// register_units rebuilds every controller, and register_generated_units runs
// the same build for units created after load. After the build only the
// escape (ship_ai_obstacle_tables.cpp, 009F4B0F/009F4B74) writes 364h.
// Reach: every mission with navigating ships, so a pair is needed. Pass
// signature: in p24_off_u21f the Iowa group (steps 5050+, no positive throttle
// after) should show positive throttle with this ON, and the count of
// throttle -0.625 lines should fall; check with BSP_SHIP_ESCAPE_TRACE=Iowa.
// Later items, not here: the 3A5h/3C1h dead copies (station_request.enable,
// Snapshot.station.enable; labelled, not removed, by packet cc13-deadcopy)
// and the plan pair's +DCh = -1 (009DBE1D, blk+300h),
// which has no host storage or reader. 009DFCB0's 3DCh = 400.0f has no image
// reader and 3C0h = 1 already matches Snapshot.station.making_way, so neither
// is seeded here.
inline constexpr bool kShipAiNavBlockCtorSeedsCc13Bound = true;  // ON with the sync: LOMP10 Ashigara never reverses, completes 1050.45 s (SHIP_AI 311)

// Packet cc13-navseeds2 (rev2 nit a). 009EF910 (ESI = ECX = blk at 009EF91B)
// reads blk+364h live at 009EFFDC (steering mode 1: committed = !364h) and
// 009EFFEC (mode 2: committed = 364h), and never writes it. The host models
// that read with ShipAiClearanceBlock::committed_ahead_364, which nothing in
// src/ writes, so it stays false while blk.yaw_rate_subtracts_364 is seeded
// (kShipAiNavBlockCtorSeedsCc13Bound) or flipped by the escape. ON copies the
// block's value in beside the other 009EF910 inputs, just before each call.
// Separate from the seed switch because it also mirrors later escape flips,
// so it changes behaviour on its own. Reach: every ship whose clearance
// refresh runs in steering mode 1 or 2; needs its own pair (and one with the
// seeds ON).
inline constexpr bool kShipAiClearance364SyncCc13Bound = true;  // ON with the ctor seeds (SHIP_AI 311)

// Packet cc13-helmlatch (docs/CC13_IJN18LOSS.md section 7). 009E1170's latched
// arm 009E11CE..009E1262 runs when +184h is set and role 1 is 8 or AI-held (the
// map-up case). While the cruise state's +8h is below -10.0f (00CE6848, COMISS
// at 009E11F9), it latches +8h = unit+994h (009E1207) and +0Ch = unit+998h
// (009E1237), or 0 for a kind-0Eh unit (009E121F). On every re-plan it re-sends
// +0Ch through 009DFFB0 (009E1249) and +8h through 009DBF90 (009E1257). The helm
// arm stores -99.0f (00CF5BFC) into both at 009E1407/009E140C, so a later fall
// to the latched arm latches again; the brain constructor stores the same -99.0f
// at 009F3A22/009F3A27. The latch state is tracked in both settings. ON: the
// re-send writes the control block. OFF writes only the controller's latch pair
// (read by nothing else) and the summary counts. kHelmPrepareMapUpGateCc13Bound
// (bsp/game_hosts_hud_world.hpp) requires this switch.
inline constexpr bool kCruiseLatchedPlayerArmCc13Bound = true;  // ON (cc9-lua70, 2026-10-19): IJN18 l70_i18_r trial, ref EndMission 1685.52 s (Zao 163 cat-6 shots / 111 hits vs 0); see WINS_TABLE

// Packet cc13-P3 (the 009ED6B0 row). The two NaN-only deviations the seg67f
// audit left in the station-keeping arm 009EDA28..009EE57B
// (ship_ai_station_keeping.cpp):
//  - 009EDC46 00415550(ECX = &120.0f, EDX = &unit+9C8h). 00415550 is
//    FCOMIP a,b / JBE: a when a > b, else b, so a NaN length yields NaN. The
//    host's max_00415550 was (b > a) ? b : a and yielded 120.
//  - 009EE11D, 009EE23A (COMISS across, XMM0 = 0) and 009EE37B, 009EE49F
//    (COMISS across, [00D7A218] = 0.0f), each followed by JC: a NaN across
//    takes the "across < 0" arm. The host's (0 > across) took the other one.
// True: both follow the image. False: as before.
// Reach: only a NaN unit length (unit+9C8h) or a NaN station offset; every
// finite input gives the same result either way.
inline constexpr bool kShipAiStationKeepingNanArmsCc13Bound = true;  // ON (cc9-sweep2b, 2026-10-27): prune flip, zero NaN operands on every OFF run; see commit

// Packet cc13-P3 (the 009E04E0 row). The NaN-only deviations the seg67e/f
// audits left in 009E04E0 (ship_ai_clearance_profile.cpp), each from the disk
// bytes:
//  - 009E06DA: the arc-cosine clamp sends a NaN heading error to 00BF9940
//    (acos); the host gave pi.
//  - 009E0767/009E0778: a NaN |error| takes the steer-only arm; the host banded.
//  - 009E099F/009E0AB6: only an ordered negative discriminant returns the
//    average; a NaN one runs the square root. The host returned the average.
//  - 009E0BB9 JBE: a NaN compare counts the track as stand-on; the host counted
//    give-way.
//  - 009E0C8A JBE: a NaN |error| takes the inner (quarter-turn) arm; the host
//    took the wide one.
//  - 009E0CA2/009E0E54 JB: a NaN lateral side picks the left normal; the host
//    picked its negation.
//  - 009E0D07/009E0EA4 COMISS 0,e / JBE: a NaN error gives gap + lateral; the
//    host gave gap - lateral.
// True: all of them follow the image. False: as before.
// Reach: only a NaN track heading, position or speed; every finite input gives
// the same result either way.
inline constexpr bool kShipAiThrottleProfileNanArmsCc13Bound = true;  // ON (cc9-sweep2b, 2026-10-27): prune flip, zero NaN operands on every OFF run; see commit

// Packet cc13-P3 follow-up (rev8 F2, the 009ED6B0 row). The NaN-only deviations
// left in the navigation arm 009EE580..009EEAAB:
//  - 00415510 min (ship_ai_navigation.cpp): ST0 = b, ST1 = a, FCOMIP, JBE
//    returns b, so an unordered compare yields b. The host's (b <= a) ? b : a
//    yielded a. Used at 009EE96F and 009EEA9D and for the turn-lead denominator.
//  - 00415550 max: ST0 = a, ST1 = b, FCOMIP, JBE returns b; same NaN fault in
//    the host's (a <= b) ? b : a. Used at 009EE9AA and the cosine floor.
//  - 00415620 clamp: the second test is FCOMI a,c / JBE returning a, so a NaN
//    there yields a; the host's (a <= high) yielded high.
//  - 009EE62B 00415550(&floor, &path+2F4h+8) (ship_ai_goal_vector.cpp): a NaN
//    width wins; the host's (low < width) kept the floor.
// True: all of them follow the image. False: as before.
// Reach: only NaN path, turn or speed inputs; every finite input gives the same
// result either way.
inline constexpr bool kShipAiNavigationNanArmsCc13Bound = true;  // ON (cc9-sweep2b, 2026-10-27): prune flip, zero NaN operands on every OFF run; see commit

// Packet cc13-P3 follow-up (rev8 F4, the 009ED6B0 row). The main thread runs
// the image's x87 at PC24 (docs/CC13_FPU_PRECISION.md), so every FMUL, FADD,
// FSUB and FDIV in the station-keeping arm 009EDA28..009EE57B rounds to float.
// The host evaluated several chains in double and rounded once (or compared
// unrounded). True: each operation rounds to float in listing order
// (ship_ai_station_keeping.cpp). False: as before.
inline constexpr bool kShipAiStationKeepingPc24Cc13Bound = false;

} // namespace bsp
