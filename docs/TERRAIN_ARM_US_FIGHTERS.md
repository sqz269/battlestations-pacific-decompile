# Why the terrain arm never fires for a US fighter (packet cc9_terrain_arm_us_fighters)

2026-09-25. Ghidra read-only. Names are hypotheses. The finding (docs/KATE_ENGAGEMENT.md section 6.1):
- the terrain arm `0099F1C0` records no tick for any US fighter, including the ones that dive into
  the sea;
- Kates and Vals get bands.

## 1. It is not the probe: the whole avoidance pass never runs for a US plane

`009A17D0` `BSP_PilotBot_UpdateAvoidance` runs the gunfire, vehicle and terrain arms. Its entry gate,
`009A1809`-`009A1871`, skips the pass when any of these holds:
- the pilot has no unit (`+2F4h` null);
- `unit+520h` is set, or `unit+5Dh` is set (dead);
- the unit is neither in free flight (`+72Ch` `vtable[38h]`) nor in state 6;
- `[[00E188A8]+5FCh]`, the scene record, is null (`009A1850`);
- **`[[00E188A8]+5FCh]+908h` equals `unit+54h`** (`009A1868`-`009A1871`).

The host's version (`gunfire_avoidance_009a17d0`) replaced that last comparison with "the controlled
unit's party equals this unit's party", labelled as a stand-in for "the local player's party". The
controlled unit is the Lexington, so **every US aircraft skipped the whole pass**:
- no gunfire avoidance;
- no vehicle avoidance;
- no terrain or water bands.

**The field is not the player's party.**
- `record+908h` is written only by `004DA4CF` in the scene-record constructor (the literal 3) and by
  the header property applier `004F1D70`.
- The applier looks up the key at `00CEA478`, **`DummyAIEnabled`** (`004F20EE`-`004F20F5`), stores
  its value (`004F2101`), and stores 3 when the key is absent (`004F2109`).
- `include/bsp/scene_record_side_blocks.hpp` already names the offset (`kSceneRecordDummyAiOffset`,
  default 3, `kSceneDefaultDummyAi`).
- `unit+54h` is the unit's side (`include/bsp/attack_commands.hpp`, `007EE917`).

So the image skips one side's planes only in a scene that authors `DummyAIEnabled`. That is a
multiplayer header key: the same block carries `CompetitiveModeParty`, `MaxPlayerNum` and
`MultiPlay`. In a mission without it the value is 3, no side matches, and every AI plane, US
included, runs the avoidance pass.

The same comparison also gates `unit+C50h`, the neighbour object, at `007D621F` (see the note in
`include/bsp/dogfight_task.hpp`). The host models that object's lists as scans, so it has no side
gate there to correct.

## 2. The other two questions in the brief

- **The surface the terrain arm samples.** It is the avoid-zone layer at `ctl+34Ch` through
  `0041BC20`. The host substitutes the water surface (labelled in `terrain_avoidance_0099f1c0`), which
  is exact over open sea. It is not the cause here: the arm never ran.
- **The look-ahead against a 100 m/s dive.**
  - `look_t = max(1.5 / class PitchSpd, 3.0)` seconds (`0099F46x`, double `00D7A2B0`).
  - The probe length is speed x `look_t`, so at 100 m/s the arm looks at least 300 m ahead along the
    velocity.
  - Its clearance `sa` grows with a nose-down pitch (`sa -= width * pitch * 4`) and with speed.
  - This is not measured here, because the arm never ran for a US plane. The pair's terrain-tick row
    is its first measurement.

## 3. The binding

`kAvoidanceDummyAiGateBound` (`src/game_hosts_units.cpp`, `gunfire_avoidance_009a17d0`) compares the
unit's side with the scene's `DummyAIEnabled`. **Substitution, labelled:** the key is not plumbed
from the scene header to the units host, so the image's default 3 stands in. No loose `.scn` in this
installation authors the key. USN04's scene is packaged, and it is a single-player mission.

The scene-record-present test (`009A1850`) keeps its existing stand-in, the controlled unit being
bound.

## 4. Predictions for the combined pair, written before the runs

As the lead briefed, the pair is:
- **all OFF:** `kAvoidanceDummyAiGateBound`, `kRateLawAttitudeTermsBound`,
  `kFlightIntegratorBound` and `kDogfightThrottleBound` all OFF;
- **all ON:** the same four switches all ON.

E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`. The all-OFF side should match
`local\KE0b_9000.log` apart from the harness window change.

| row | all OFF | all ON prediction |
| --- | --- | --- |
| US fighters in the avoidance census (terrain, vehicle or gunfire ticks) | 0 | at least 6 |
| terrain-avoidance ticks (all aircraft) | about 1,340 | above OFF |
| **US fighter depth kills and sea contacts (the gate)** | 0 | **0-1** |
| US fighter losses | 0 | 0-4 |
| Kate death rows with fighter-gun hits | 0 | at least 1 |
| Kate deaths to AA | 16 | 8-16 |
| fighter kills | 5 | 4-20, the image's laws' band |
| fighter fire ticks | about 160 | 100-600 |
| Kate / Val deaths | 16 / 16 | 12-16 each |
| hit records | about 570 | 450-750 |
| torpedo / dive-bomb releases | 4 / 0 | 2-8 / 0-4 |
| Lexington moved | about 6.0 km | 5.5-7.5 km |
| plane distance moved | about 1.09 Mm | +-15% |
| mission end | none | none |

The US fighters now also run gunfire avoidance against the Japanese gunners. Their attack runs
change, which shifts the fighter-kill and loss rows further. Those rows are judged by band.

**The flip rule.** If the sea-loss row holds, all four switches land ON. If it misses,
`kRateLawAttitudeTermsBound` stays OFF, and the other three are judged on their own rows.
