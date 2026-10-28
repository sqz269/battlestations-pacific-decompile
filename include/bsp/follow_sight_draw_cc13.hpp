#pragma once

// Packet cc13-sightdraw (cc9 lead 4, census S10). Header-only switch.

namespace bsp {

// kFollowSightCountdownDrawCc13Bound (OFF). SHARED-STREAM MOVER: ON moves draws onto
// the host's shared stream-1 generator, so every later consumer of that stream
// (ship AI, gunnery, planners) shifts. Flip it together with the other shared-stream
// movers cc9 pairs, not alone.
//
// 009C2980 BSP_BotStateFollow_Construct seeds the follow state's sight countdown +74h:
// 009C299D XOR EBX,EBX; 009C29E0 FLD dword [00CE3D30] (0.6f, 3F19999Ah), FSTP the high
// argument; 009C29FA FLDZ, FSTP the low argument; 009C29FF LEA ECX,[EBX+1] (stream 1);
// 009C2A0A CALL 00BD2F10 BSP_Random_UniformFloatRange; 009C2A0F FCHS; 009C2A19 FSTP
// float [ESI+74h]. So +74h = -U(0.0f, 0.6f), one stream-1 draw, the negation exact.
// The host draws it at each task install from release_altitude_draw_00bd2f10 (its own
// LCG, keyed per unit and purpose under BSP_GUNNERY_RNG_STREAMS), not from stream 1.
// ON: GameGunneryHost::ship_ai_draw(unit, 0.0f, 0.6f), the host's shared stream-1
// generator (00BD2F10 with ECX = 1), negated, at the same install points; with no
// gunnery host the old draw answers. OFF: the old draw.
// Sites (src/game_hosts_units.cpp), each a 009C2980 construction:
//   bot_task_ctor_seeds_0099c6f0 with a follow state (#tb74; dogfight, strafe, level
//     bomb, kamikaze, drop kamikaze, depth charge, dive bomb, torpedo, moveonpath)
//   the retreat install (#rf, 009C9C5D), the land install (#l74, 009B2EE4) and the
//     moveto install (#mt74, run_moveto_task_install_009c3000)
// One draw per 009C2980 call, in call order. The construct paths' rel32 callers of
// 009C2980 (PE scan) give the counts: dogfight 009A94E0, strafe 009CC020, level bomb
// 009B75E0, kamikaze 009AED20 2; drop kamikaze 009AD720, depth charge 009A4DC0, dive
// bomb 009C73A0, torpedo 009D2DA0 3; moveonpath 009BCC60, moveto 009C2DF0, land
// 009B2E50, retreat 009C9BB0 1. LABELLED: the host keeps one +74h per unit, so it
// takes the first state's draw and the later draws only advance the stream; the
// image's other stream-1 draws in the same constructors (and 0099C6F0's +308h) are
// not on the shared stream, so the interleaving is the host's, not the image's.
// Not hosted: 007B6830 rocket (2) and 009A2500 close-to-ship (1) approach builders.
// Not changed: the other task-constructor draws (+308h stream 1, +3E8h stream 0 and
// the per-task ones) stay on release_altitude_draw_00bd2f10.
// Movers when ON: the first follow sight search time of every follow state (+74h),
// and through the shared stream every later stream-1 draw.
inline constexpr bool kFollowSightCountdownDrawCc13Bound = false;

}  // namespace bsp
