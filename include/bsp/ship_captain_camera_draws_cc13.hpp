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
// Movers: every later stream-0 draw in the process shifts by three per ShipCaptain
// construction: the damage-smoke effect pick (00822A89), the stream-0 death-delay draws,
// the HUD and other stream-0 consumers that use death_mode_draw_00bd2f10(0, ...). Stream 1
// (ship AI, gunnery, GameGunneryHost::ship_ai_draw) does not move: the image's streams are
// separate states per registered thread (00BD2ED0, docs/RANDOM_STREAMS.md). The phases
// themselves are advanced in publish_pose_004329d0 but nothing modelled reads them, so the
// camera pose does not move.
inline constexpr bool kShipCaptainCameraDrawsCc13Bound = false;

}  // namespace bsp
