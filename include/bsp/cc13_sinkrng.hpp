#pragma once

// Packet cc13-sinkrng. docs/CC13_SINKRNG.md.
//
// kSinkLeakDrawStreamCc13Bound: the generator for the wreck's six leak draws.
// 0074EC50 draws U(0, 1) per leak from 00BD2F10 with ECX = 1 (0074ECA5): stream 1
// of the main thread (slot 0), since the wreck handler 00824B60 runs in the
// fan-out's row 15 flush (009273A0). The host splits image stream 1 into several
// generators (docs/CC13_RNG_STREAM_CENSUS.md section 5). The main-thread stream-1
// draws of the fixed step (the row 6/14 hit effects, the death break-up, the ship
// AI) and the movie reseeds go to the gunnery generator (H-G,
// GameGunneryHost::Impl::rng).
//
// OFF (today): the six draws come from the units host's release generator (H-U,
// release_altitude_draw_00bd2f10, key "<name>#leak"), which carries the plane
// AI's draws (dive bombing, torpedo, goaway, take-off, dogfight). With the shared
// default, each sinking ship moves every later plane-AI draw and none of the
// gunnery draws. ON: they come from H-G through
// GameGunneryHost::death_mode_draw_00bd2f10(1, ship, 0, 1), in call order, as
// every other units-host stream-1 draw routed to gunnery is.
//
// H-G is the better approximation, not an exact one: the sink draws always share
// slot 0 with the row 6/14 hit effects, the break-up and the reseeds (all H-G),
// but with the plane-AI and gunnery-pass jobs only when the main thread wins the
// job race (00E0B6CE = 1). The H-G/H-U split stays a labelled substitution.
// Under BSP_GUNNERY_RNG_STREAMS=1 the ON key (death_mode, ship) is shared with
// Lua BreakShip / SetDeadMeat draws on the same ship. The count is exact:
// 0074F4DD fixes [manager+14h] at 6.
//
// OFF is identical: the ON-only call is behind `if constexpr`. Not yet
// game-validated.

namespace bsp {

inline constexpr bool kSinkLeakDrawStreamCc13Bound = false;

}  // namespace bsp
