#pragma once

// cc13-seg67n adoption switches (docs/CC13_SEG67N.md). Each one is OFF: the
// host keeps its current behaviour until a pair run judges the ON arm.

namespace bsp {

// 009FAAD0, the bot-task gun block constructor. Every bot task is built through
// 0099C6F0 (or 0099C8B0 from a stream), which constructs a fresh gun block at
// task+314h (0099C790 / 0099C950); the old task's block dies with it (00999EE0
// deletes the retired task at 00999F0B). The host keeps one gun block per unit
// across tasks (game_hosts_units.cpp df_gun and its sibling fields), so a value
// the old task left survives the change. The one that matters is the hold timer
// +4Ch: a follow sight sets it to 2.0 (009C232A), and in the gun tick a hold > 0
// FORCES the envelope (009FCC33 JA -> 009FCC9E, skipping the range and cone
// tests) and skips only the fine-aim correction (009FCCB4), so the path FIRES
// (009FCDB4). A stale hold therefore makes the next task's guns fire, without aim
// correction, for up to 2.0 s whenever a target is in gun range.
// ON: each host task install (bot_task_ctor_seeds_0099c6f0, the 0099C6F0 hook)
// re-runs 009FAAD0's stores on the fields the host models, in the image's order:
// +28h = -1.0 (strafe point disarmed), +48h/+4Bh = 0, +74h = 0, +40h = 0.0,
// +4Ch = 0.0; the burst clock +50h is the sub-switch below. Not modelled, so not
// reset: +0h, the aim wander +4h..+Ch and its 009FA620 reseed (stream-1 draws),
// +2Ch..+3Ch (per-tick inputs in this host), +44h = 999999.0, +49h.
// OFF: the block persists across tasks, as before.
// cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 100). Pairs
// local\p24_off vs local\p24e_on: BSM09 moves in both envs (ref 50 -> 49 deaths,
// still ends 351.04 s; default still ends, 516.00 -> 459.01 s); USN01 u1i and
// USN21 are gameplay identical.
inline constexpr bool kTaskGunBlockReconstructBound = true;

// 009FAB50, +50h = 0.0, the burst clock, split out because it moves draws. In the
// image the burst draw is 009FCED1 00BD2F10 on ECX = 1, the shared stream 1, taken
// on each +4Bh edge (009FCE70-009FCED6). After 009FAAD0 (+4Bh = 0, +50h = 0.0) the
// new task's first draw comes on its first gun tick with +48h set: +50h has gone
// below 0, so 009FCEA8 sets +4Bh = 1 and 009FCED1 draws U(row+234h, row+238h).
// The host's draw (game_hosts_units.cpp, kDogfightGunBurstDrawBound, "#dfgb") is
// not the image's stream: by default it is the host's one shared LCG
// (db_release_rng, also used by the task constructor seeds and the release draws),
// and with BSP_GUNNERY_RNG_STREAMS=1 it is a private per-unit "#dfgb" stream. With
// both switches ON the host matches the image's count and order of burst draws
// for the new task (one draw at its first firing tick, then one per edge), on the
// ticks where the host runs the gun tick; the generator differs. ON: also +50h = 0.0 at each install. OFF: the old task's
// clock carries over.
// cc9-physics24 2026-10-19: ON by mechanism; local\p24e_on vs local\p24f_on is
// gameplay identical on USN01 u1i, BSM09 and USN21 in both envs.
inline constexpr bool kTaskGunBlockBurstTimerResetBound = true;
static_assert(!kTaskGunBlockBurstTimerResetBound || kTaskGunBlockReconstructBound,
              "kTaskGunBlockBurstTimerResetBound requires kTaskGunBlockReconstructBound");

}  // namespace bsp
