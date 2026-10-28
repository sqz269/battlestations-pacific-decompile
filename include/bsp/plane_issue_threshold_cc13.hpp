#pragma once

// Packet cc13-issuea0 (substitution census S11).
namespace bsp {

// kPlaneIssueThresholdZeroCc13Bound (OFF): the pilot control block's ctl+390h, the
// threshold 007EEF40 compares against the armed fraction ctl+374h.
//
// The control block is the plane squadron at unit+9D4h. Each of its allocators does
// new(414h), then memset 0, then 007F2C60: 004F0AD0, 009483D0, and 007F3500 (clone).
// 007F2C60 writes no +390h. A whole-.text scan for disp32 0x390 stores (packet
// cc13-issuea0) finds no writer on this block: each writer is a camera, the
// GameTuning singleton (007E2A20), or an unrelated object. The only +390h accesses
// in 007D1E50-007F8390 on the block are the reads 007EEF46 and 007EFB3A. So the
// image's ctl+390h is 0.0.
// - At 007EEF40, 0.0 > +374h is never true (JBE also takes the unordered case), so
//   007EEF54's whole-flight loop never issues.
// - 007EFB30 always sets +378h.
// The old seed, 0079CD36 = [[unit+538h]+A0h] * 0.95, writes the CAMERA that
// 0079CBD0 BSP_GameCameraModifier_EnsureCamera builds at [modifier+8], not this block.
//
// ON: authorise_value_390 = 0.0f. OFF: 1.0 * 0.95 (kIssueThresholdScale_00ceffb0),
// the cc8 stand-in (afb2fb184, 2026-09-17), unchanged.
// Where the 0.95 came from: the camera row, by mistake; it was not measured.
// fb038d7a5 took 0079CD36 as the control block's seed because two of the five
// fields were "the altitudes 009D4A70 later overwrites". 009D4A70 only READS
// [task+404h]+398h (009D4B57), which is another object's field, not this block's.
// afb2fb184 then kept 0.95 and stood 1.0 in for the unread class +A0h. Its
// evidence is a USN01 census (+390h = 0.95, +374h = 0.80, open on every
// aircraft). No game validation exists.
// Scope: ON changes only the gate's value. a0ccc4569's wiring stays as it was:
// the 007BCBE0 budget writer, unit+C58h, the 0099AF53 spend, the leader block
// 007EEF96 and the attack mode ctl+370h (0099B740). Only the 007EEF54 loop,
// which needs the gate, stops issuing.
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
// Movers when ON: when planes issue (release) their ordnance. The 007EEF54 loop,
// 007BCBE0(unit_i, 999) for every controlled plane, stops running, so a flight's
// unit+C58h release budget comes only from each plane's own path (the 007EEF96
// leader block and elsewhere). Torpedo and bomb releases in multi-plane flights
// change.
inline constexpr bool kPlaneIssueThresholdZeroCc13Bound = false;

}  // namespace bsp
