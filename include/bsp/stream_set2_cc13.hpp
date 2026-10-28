#pragma once

// Packet cc13-stream2 ("stream set 2", cc9 request; census local/rev8_s2sites.tsv).
// Header-only switches.
//
// Both switches move host draws that were keyed per unit (release_altitude_draw_00bd2f10,
// the units host's own LCG, one key per unit and site) onto the host's shared stream-1
// generator, GameGunneryHost::death_mode_draw_00bd2f10(1, ...), which is the same
// generator ship_ai_draw uses (00BD2F10 with ECX = 1). The routing is in
// GameUnitsHost::Impl::stream_set2_draw_00bd2f10; release_altitude_draw_00bd2f10 itself is
// unchanged. With no gunnery host the old keyed draw answers.
//
// SHARED-STREAM MOVERS: when ON, every later stream-1 draw shifts (ship AI torpedo and
// avoidance draws, gunnery death-mode draws, the issue-interval and rack-delay draws, and
// the rest of this list). Flip them with the other shared-stream movers cc9 pairs, not
// alone.

namespace bsp {

// kStreamSet2SharedDrawsCc13Bound (OFF). Sites, by family (src/game_hosts_units.cpp; the
// OFF key in quotes). One draw per call; the count is calls, not keys.
//   Bot task constructor base (0099C6F0): "#tb308" (+308h), in bot_task_ctor_seeds_0099c6f0
//     and the strafe re-seed; dogfight "#tbmv2c" (+2Ch); torpedo "#tbt1c" (+1Ch). 4 sites.
//   Dive bomb: release altitude +A8h (009C3F29, the unit-name key), attack distance
//     U(0.6, 0.8) (009C3F63, unit-name key), "#b8" (009C3F97), profile "#dp" (009C8A66),
//     "#db_side" (009C8587), "#db_ad1c" (009C6114), "#db_ag1c" (009C579E), "#db_roll"
//     (009C7839). 8 sites. LABELLED: the host draws the side coin after the roll
//     magnitude; the image draws it before.
//   Aim error redraw (009C3DA0 dive / 009D02A0 torpedo): h, v, time, unit-name key.
//     3 sites.
//   Kamikaze: the KzHost wrapper, "#kz" + 4c, 50 (009AB885, 009AB8DF), h, v, 64, t, mul.
//     1 wrapper.
//   Drop kamikaze: the 009AE9D0 cruise-profile draw, "#dkz" + ac. 009AEA6E MOV ECX,1;
//     009AEA82 CALL 00BD2F10, U(0.9f [00CE3860], 1.2f [00CE3814]). The image calls +54h
//     only when the task becomes the bot's head (0099AE28, 0099AEAE); the host runs it
//     once at dkz_install_009aebe0, after the constructor's draws. 1 wrapper.
//   Takeoff: "#t20", "#t24s", "#t24" (009CDD50), "#t524" (009CE1D0), "#t518". 5 sites.
//   Torpedo reset (009D0380): "#t88", "#t12c", "#t70", "#t78", "#t7c", "#t80". 6 sites.
//   Land (009B41C0 -> 009AFE70): "#l54", "#l58", "#lb0". 3 sites.
//   Strafe: st_draw, "#st" + key. 1 wrapper.
//   Level bomb: lb_draw, "#lb" + key, except mv44 and ar18 (stream 0) and rk (the rack
//     delay, kPlaneDeviceWalkSharedDrawBound). 1 wrapper.
//   Dogfight: df_enter_draw ("#dfmr", "#dfav", "#dfae"), "#dfta" (009A8712), "#dfat"
//     (009A7D72), "#dfap" (009A99C6), "#dfgb" (009FCED6). 1 wrapper + 4 sites.
//   Gun avoidance (0099EF2D): "#ga", "#gr". 2 sites.
//   Neighbour clocks (007E1E20): "#nb7c", "#nb84", "#nb80"; the formation noise
//     "#fnoise" (007E21AA). 4 sites. LABELLED: the clocks are seeded at first use.
//   Dead-meat spin (007D7EA0): "#ds_m", and "#ds_z" / "#ds_p" through 00439950. 2 sites.
//   Moveto: "#mt2c", "#mt58", "#mt18", "#mt54c" (009C1C30), "#mt68", "#mt424" (009C3650).
//     6 sites. #mt74 is kFollowSightCountdownDrawCc13Bound's.
//   Retreat (009C9D26 -> 009C91B0): "#r2c", "#r30", "#r38", "#r560". 4 sites. #rf is
//     kFollowSightCountdownDrawCc13Bound's.
//   Depth charge: the adapter, "#dc" + key, including the approach target ref timer
//     ref60. 1 wrapper.
//   Opportunistic rocket: the adapter, "#or" + key. 1 wrapper.
// Not moved here:
//   Stream 0 (XOR ECX,ECX), left on the keyed draw: "#tb3e8" (0099C7BA / 0099C97A, both
//     its constructor and strafe re-seed calls), level bomb mv44 (009B6D03) and ar18
//     (009B4BF3).
//   Other switches: "#leak" (kSinkLeakDrawStreamCc13Bound); "#tb74", "#rf", "#l74",
//     "#mt74" (kFollowSightCountdownDrawCc13Bound); issue interval and rack delay
//     (kPlaneDeviceWalkSharedDrawBound).
//   Moveonpath "#mp" (0099A170 / 009BD990): excluded. The host draws +74h twice (once
//     through the follow-state seed and once as "74"), and its draw order is reversed
//     against the image's; it needs its own packet (medium effort).
//   Image stream-1 sites with no keyed host draw (nothing to move; the host stream
//     misses them): dive 009C3A18, 009C419A, 009C49FC, 009C4D13, 009C4D33; land
//     009B0077; the rocket task (not hosted).
// Movers when ON: bot task phases, dive bomb release altitude and attack distances,
// aim error offsets, kamikaze and drop-kamikaze altitudes and release distances, takeoff
// phases, torpedo reset legs, land approach angles, strafe, level bomb and dogfight
// timers, gun avoidance, neighbour clocks, dead-meat spin, moveto and retreat altitudes,
// depth charge and opportunistic rocket draws; and every later stream-1 draw.
inline constexpr bool kStreamSet2SharedDrawsCc13Bound = false;

// kStreamSet2WandererCc13Bound (OFF). The plane wanderer 007BE060's draws, key
// "<unit>#wander": 007BE186 (MOV ECX,1 at 007BE172) and 007BE2CF (007BE2C5), and the
// 007C4560 constructor's draw (007C457D), which the host takes at the first step
// (LABELLED; the image draws it at construction, 007CFDC6). Census liveness: 37235
// wanderer draws in u21, 57365 in usn21, so this is the largest single mover.
// Movers when ON: wanderer drift, and every later stream-1 draw.
inline constexpr bool kStreamSet2WandererCc13Bound = false;

}  // namespace bsp
