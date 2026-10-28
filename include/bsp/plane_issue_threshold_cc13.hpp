#pragma once

// Packet cc13-issuea0 (substitution census S11).
namespace bsp {

// kPlaneIssueThresholdZeroCc13Bound (OFF): the pilot control block's ctl+390h, the
// threshold 007EEF40 compares against the armed fraction ctl+374h.
//
// The control block is the plane squadron at unit+9D4h. Each of its allocators does
// new(414h), then memset 0, then 007F2C60: 004F0AD0, 009483D0, and 007F3500 (clone).
// 007F2C60 writes no +390h.
// The only writer of +390h is the Lua binding 008A1F40, SquadronSetDropAllBombPercent:
// 00888AA0 on argument 0 (the same squadron), then FSTP float [ESI+390h] at 008A2091.
// No script in this installation calls it, so here the value stays 0.0.
// The whole-.text scan for disp32 0x390 stores lists 008A2091 alongside the cameras,
// the GameTuning singleton (007E2A20) and unrelated objects. In 007D1E50-007F8390 the
// block's +390h is only read, at 007EEF46 and 007EFB3A.
// With the default 0.0:
// - At 007EEF40, 0.0 > +374h is never true (JBE also takes the unordered case), so
//   007EEF54's whole-flight loop never issues.
// - 007EFB30 always sets +378h.
// The old seed, 0079CD36 = [[unit+538h]+A0h] * 0.95, writes the CAMERA that
// 0079CBD0 BSP_GameCameraModifier_EnsureCamera builds at [modifier+8], not this block.
//
// ON: authorise_value_390 = the squadron slot's sq_drop_all_bomb_percent_390. That
// field is 0.0 until the Lua native SquadronSetDropAllBombPercent stores into it;
// the native is bound in game_hosts_lua.cpp only under this switch.
// OFF: 1.0 * 0.95 (kIssueThresholdScale_00ceffb0), the cc8 stand-in (afb2fb184,
// 2026-09-17), unchanged. The native stays unbound.
//
// Where the 0.95 came from: the camera row, by mistake; it was not measured.
// fb038d7a5 took 0079CD36 as the control block's seed because two of the five
// fields were "the altitudes 009D4A70 later overwrites". 009D4A70 only READS
// [task+404h]+398h (009D4B57), which is another object's field, not this block's.
// afb2fb184 then kept 0.95 and stood 1.0 in for the unread class +A0h. Its
// evidence is a USN01 census (+390h = 0.95, +374h = 0.80, open on every
// aircraft). No game validation exists.
//
// PAIRING (required): flip this together with kReleaseOrderLeaderBlockBound
// (torpedo_release_orders.hpp, packet 80da020b3).
// - In the image, 007EEF50's JBE on a failed gate lands on 007EEF96, the leader
//   block.
// - With that switch OFF, torpedo_issue_release_orders_007c0d90 returns on a failed
//   gate. So this switch ON alone issues NOTHING from the 007EEF30 tail: it also
//   drops the leader block's raises, which the image keeps.
// - For an AI flight (unit+1B0h == 8) the block issues nothing either way.
// Apart from the gate's value, a0ccc4569's wiring stays as it was: the 007BCBE0
// budget writer, unit+C58h, the 0099AF53 spend and the attack mode ctl+370h
// (0099B740).
//
// Every host reader of authorise_value_390:
//   - game_hosts_units.cpp read_issue_inputs (the switched site), which feeds
//     torpedo_release_orders.cpp release_orders_should_issue_007eef30 (the 007EEF40
//     gate) through torpedo_issue_release_orders_007c0d90.
//   - game_hosts_units.cpp slot torpedo_issue_threshold_390, a copy for the
//     diagnostic summary only.
// 007EFB30 has no live host path: cc15_seg48.cpp and cc13_seg67b.cpp are standalone,
// and the host's squadron force_flag_378 stays at its 007F2D1E seed (true).
// SHARED-STATE NOTE: no RNG draw is involved.
//
// Movers when ON, paired with kReleaseOrderLeaderBlockBound: when planes issue
// (release) their ordnance.
// - The 007EEF54 loop, 007BCBE0(unit_i, 999) for every controlled plane, stops
//   running. A flight's unit+C58h release budget then comes only from the 007EEF96
//   leader block (player-held flights) and each plane's own path.
// - Torpedo and bomb releases in multi-plane flights change.
// Without the pairing, the leader block's raises are lost as well.
inline constexpr bool kPlaneIssueThresholdZeroCc13Bound = false;

}  // namespace bsp
