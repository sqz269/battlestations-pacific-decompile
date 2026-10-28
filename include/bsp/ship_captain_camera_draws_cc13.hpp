#pragma once

// Packet cc13-camdraws (census S3, src/mission_camera.cpp construct_ship_captain_0064b650).

namespace bsp {

// kShipCaptainCameraDrawsCc13Bound (OFF). SHARED-STREAM MOVER.
// The ShipCaptain camera constructor 00432750 (reached from 0064B650 when 0064DA40 binds
// the player's ship view with no mover installed, screen +20h == 0) draws three phases
// U(0, 2 pi) ([00CE3D9C] = 40C90FDBh) through 00BD2F10 at 00432938, 00432955 and 00432972,
// each with XOR ECX,ECX: stream 0, not the gameplay stream 1. They land in +1E8h, +1ECh
// and +1F0h in draw order (00432983, 00432990 after the POP EDI, 0043299A).
// ON: game_hosts_hud.cpp takes the three draws from the host's stream-0 generator,
// GameGunneryHost::death_mode_draw_00bd2f10(0, ...) (a labelled substitution: its seed and
// sequence are not the image's), and seed_ship_captain_phases_00432938 stores them.
// OFF: no draw, the phases stay 0 (the earlier substitution).
// Movers. Not display-only. By default the host's stream 0 is a single static LCG
// (game_hosts_gunnery.cpp death_mode_draw_00bd2f10) that ignores the unit key, so each
// ShipCaptain construction moves every later stream-0 draw in the process along by three.
// The stream-0 consumers (file:line on this branch's base):
//   - game_hosts_units.cpp:23288, 007BBFA0 ExplosionDelayed: U(ExplosionExplosionDelay)
//     becomes plane_death_timer_c10 through 007BBFA0's timer, so a plane's death timing.
//   - game_hosts_ship_ai.cpp:13506..13584, the land fort fire rolls (00746320 start/extend,
//     00745BE0 / 007470B0 frame update): the fire extend and start durations into fire_time_748
//     (007464FD), the 007463C1 effect choice, the secondary chance and effect draws
//     (00745D5C, 00745D79) and the U(5, 15) secondary back-off into secondary_clock_750.
//   - game_hosts_ship_ai.cpp:12488, ShipAiNavBlock::uniform_00bd2f10: a ship-AI mover on
//     stream 0, the 009E465F timer draw of the nav-block constructor 009E4330
//     (BSP_ShipAi_NavigationBlockConstruct), under kShipAiTimerDrawsBound; negated into
//     blk+148h, which nothing traced reads.
//   - game_hosts_units.cpp:3358, 007BCABB U(DeadEngineFireDelay); the value is dropped
//     (+C14h in the image, nothing reads it), only the draw counts.
//   - game_hosts_gunnery.cpp:16937, 00822A89 the damage-smoke effect pick (default mode).
//   - game_hosts_units.cpp:11705 ParatrooperRackBinding::uniform and :11765
//     ParaServices::uniform pass their caller's stream. Stream-0 callers exist for both:
//     006E541F's drop jitter (gameplay_seg46_cc13.cpp:340/343, through 11705, under
//     kLevelBombParatrooperDropDrawBound, OFF) and 007AB6AC's soldier start draw
//     (cc13_paratrooper_round.cpp:325, through 11765, under kParatrooperRoundBound and
//     kParatrooperSoldierAnimDrawBound, both OFF), so neither draws today.
// Under BSP_GUNNERY_RNG_STREAMS=1 stream 0 is keyed per (unit, 0); the FFFFFFh key here is
// no unit's index (units and paratrooper rounds, F00000h + id, stay below it), so no
// unit-keyed draw moves in that mode.
// The stream-1 draws (GameGunneryHost::ship_ai_draw, the gunnery and the other stream-1
// sites) do not move: the image keeps a separate state per stream and registered thread
// (00BD2ED0, docs/RANDOM_STREAMS.md). Ship AI can still move through its stream-0 draw
// above. The
// phases are advanced in publish_pose_004329d0, but nothing modelled reads them, so the
// camera pose does not move.
inline constexpr bool kShipCaptainCameraDrawsCc13Bound = false;

}  // namespace bsp
