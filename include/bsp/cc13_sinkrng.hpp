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
// GameGunneryHost::sink_leak_draw_0074eca5(ship, 0, 1). On the shared default
// that is the same random_range_00bd2f10 call in call order, as every other
// units-host stream-1 draw routed to gunnery is; under BSP_GUNNERY_RNG_STREAMS=1
// it has its own key (Draw::sink_leak, ship), so each ship's six draws are
// isolated (packet cc13-sinkfix).
//
// ON is the closer model (call order: the sink's deterministic slot-0
// co-tenants are on H-G). The flip is chaos-only and low priority. cc9: flip it
// in a batch that already resets shared-mode baselines; do not spend a
// dedicated pair on it. The draws share slot 0 with the plane-AI and
// gunnery-pass jobs only when the main thread wins the job race
// (00E0B6CE = 1); the H-G/H-U split stays a labelled substitution. The count
// is exact: 0074F4DD fixes [manager+14h] at 6 (read from 0074F490, 0074E7B0
// and 0074EDC0 only, not a program-wide scan).
//
// OFF is identical: the ON-only call is behind `if constexpr`. Not yet
// game-validated.

namespace bsp {

inline constexpr bool kSinkLeakDrawStreamCc13Bound = true;  // ON: rides with the nav seed flip, rev4 closer model (SHIP_AI 311)

}  // namespace bsp
