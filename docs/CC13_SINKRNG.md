# CC13 sinkrng: the generator for the wreck's leak draws

Packet cc13-sinkrng. The switch is `kSinkLeakDrawStreamCc13Bound` in
`include/bsp/cc13_sinkrng.hpp`. It is committed OFF. Not game-validated. This packet follows the
gap that docs/CC13_LEAK92.md section 5 flagged, and it was revised after rev4's leak92 review.

## Verdict

- **Count.** The image makes `[manager+14h]` draws, one per leak. `0074F490` always sets that
  count to 6 (`0074F4DD MOV [ESI+14h],6`), and nothing else writes it, so the host's fixed six
  are exact. The count gap rev4 raised is not live, so no `kSinkLeakDrawCountCc13Bound` is built
  (section 4).
- **Stream.** The six draws are image stream 1 (`ECX = 1`, `0074ECA5`). They are made in the
  fan-out's row 15 flush on the main thread, so they use slot 0's stream 1.
  - In the image, that one stream also carries every other main-thread stream-1 draw. That
    includes the row 6/14 hit dispatch and the death break-up, which are always on the main
    thread. It also includes the draws of whichever fixed-step jobs (gunnery passes, pilot-bot
    tasks, ship AI) the main thread wins in the job race. In the image, therefore, the sink draws
    advance both the gunnery sequence and the plane-AI sequence, each only for the jobs that run
    on the main thread.
  - The host splits image stream 1 into a gunnery generator (H-G) and a plane-AI generator (H-U).
    So the six draws can perturb only one of the two.
- **ON is the closer model (call order: the sink's deterministic slot-0 co-tenants are on H-G).
  The flip is chaos-only and low priority. cc9: flip it in a batch that already resets
  shared-mode baselines; do not spend a dedicated pair on it.** Neither generator is exact,
  because the host splits one image stream in two. The difference is in which draws are
  guaranteed to share slot 0 with the sink draws:
  - *Always on slot 0:* the sink draws, the row 6/14 hit-effect draws, the break-up draw
    `00827B4D` and the movie reseeds. All of them run in the main-thread fan-out or on the main
    thread. Every one of them is on H-G in the host.
  - *On slot 0 only when the main thread wins the job race:* the pilot-bot draws (H-U) and the
    gunnery-pass draws (H-G). They are pool jobs (`00875BB0`, `00E0B6CE = 1`), so each lands on
    the stream of whichever thread claims it.
  - So H-G holds every guaranteed partner, while H-U holds only race-dependent ones. ON
    (`kSinkLeakDrawStreamCc13Bound`) moves the six draws to H-G. The H-G/H-U split itself
    remains a labelled substitution.
  - Keeping the draws on H-U by "convention" does not hold either: `death_mode_draw_00bd2f10`
    already routes units-host stream-1 draws to H-G, the torpedo goaway jitter among them.
- **The gunnery stream.** The image's gunnery draws are stream 1 and H-G models stream 1
  (section 1). Only the projectile-construction draw `006E7B97` is stream 0, and the host does not
  take it. rev4 has withdrawn its "gunnery is stream 0" note.
- **LIVE?** Only with `BSP_GUNNERY_RNG_STREAMS` unset. In that mode every sink moves one
  host generator by six that the image would not move in the same way. Section 3 has the
  per-mission reach.

## 1. Which host generator models which image stream

Source: docs/CC13_RNG_STREAM_CENSUS.md sections 2-5 and `reports/cc13_rng_stream_census.json`,
rechecked against the code.

| image | host generator | code | users |
| --- | --- | --- | --- |
| stream 1 | H-G, `GameGunneryHost::Impl::rng` (`Impl::draw`, `random_range_00bd2f10`) | `game_hosts_gunnery.cpp` `Draw` purposes (`:2364`) | `aim_error` (`006DEFF3`, `006DF5C6`; `:2928`..`:2938`), `fire_stagger`, `hit_effect` (`:11620`), `hull_damage`, `blast_damage`, `ship_breakup` (`00827B4D`, `:13851`), `ship_ai_torpedo` / `ship_ai_draw` (`:18365`), `death_mode` (`007CA914`; units-host stream-1 draws routed through `death_mode_draw_00bd2f10(1, ...)`, such as the torpedo goaway jitter); the only generator `reseed_shared_stream_00bd2fd0` (`:18389`) reseeds |
| stream 1 | H-U, `GameUnitsHost::Impl::db_release_rng` (`release_altitude_draw_00bd2f10`) | `game_hosts_units.cpp` | the pilot-bot tasks: dive bombing, torpedo, goaway (`#ga`), retreat, take-off, landing, dogfight, moveto, about 69 sites; the level-bomb periods (image stream 0, labelled); **the six sink draws** (`<name>#leak`) |
| stream 1 | H-AI, H-SO | `game_hosts_ai.cpp`, `GameScriptOrdersHost::random_state_` | AI group think, script orders |
| stream 0 | H-S0, the static `stream0` in `death_mode_draw_00bd2f10` | `game_hosts_gunnery.cpp` | `007BBFA0` explosion delay, the fort rolls, the ship-AI navigation block |

In the census report, gunnery sites are stream 1: `BSP_Gun_Fire` (2), `BSP_GunBot_LeadAimTick`
(4), `BSP_GunBot_TurretAimAndFireTick` (3), `BSP_AAFlakBot_RollError` (13),
`BSP_Projectile_OnImpact` and others. The projectile constructor's `006E7B97` U(0, 0.1) is
stream 0, and its only reader is the bullet serializer (census section 0). H-G is seeded `9E3779B9h`
and models stream 1. H-S0 is the host's stream-0 model.

**Main-thread stream 1 for rows 6, 14 and 15.**
- The fan-out runs on the main thread, and the main thread is registered as slot 0 (`BSP_WinMain`
  -> `00BD2FE0`, census section 3).
- The row 6/14 deferred hit dispatch (`00926700` -> `009239A0`) draws its fire/leak chance on
  stream 1 (`00826F10`). The host takes that from H-G as `Draw::hit_effect`.
- The death break-up `00827B4D` is stream 1, `ECX = 1` in the census report. The host takes it
  from H-G as `Draw::ship_breakup`.
- Every reseed runs on the main thread and sets slot 0 only. The host applies the stream-1
  reseeds to H-G only.
- So the host generator that models main-thread stream 1 for these rows is **H-G**.

**Is H-U a different image stream or thread?**
- It is not a different stream: the pilot-bot draws are stream 1, as the sink draws are.
- Its thread varies. `00875BB0` queues every fixed-step element into the frame-job pool and
  dispatches with `00E0B6CE = 1`, so the main thread and the workers claim jobs by a LIFO race,
  and each registered thread has its own stream pair (census section 3).
- A pilot-bot task that the main thread runs draws from slot 0's stream 1, the one the sink
  draws advanced. One that a worker runs does not.
- The gunnery pass is also a fixed-step element (a wave-2 sub-node), so the same holds for it.
- Which consumers sit inside wave virtuals was not traced per site (census open item 2).
- So yes: in the image the sink draws do advance the plane-AI sequence, but only for the jobs
  the main thread claims. The draws the sink always shares slot 0 with are the fan-out and
  main-thread draws, and those are on H-G. That is why ON is the closer model.
- A faithful model would merge H-G and H-U into one stream-1 generator. That would move every
  pair at once, and the job race would still make it inexact. Neither the switch nor the OFF
  state claims to be that model.

## 2. The binding

- **OFF:** `ship_wreck_sink_block_00824fe5` draws `release_altitude_draw_00bd2f10(s.row.name +
  "#leak", 0, 1)` six times, on H-U. No change.
- **ON:** the six draws are `gunnery->sink_leak_draw_0074eca5(s.process_index, 0, 1)`
  (packet cc13-sinkfix).
  - On the default path that is H-G in call order: `Impl::draw` returns
    `random_range_00bd2f10` before it looks at the purpose. That is the same call the earlier ON
    (`death_mode_draw_00bd2f10(1, ...)`) made, so ON-shared is unchanged by cc13-sinkfix.
  - Under `BSP_GUNNERY_RNG_STREAMS=1` the key is `(Draw::sink_leak, ship)`, a purpose of its
    own (35). The earlier ON key `(Draw::death_mode, ship)` was shared with Lua `BreakShip`
    (`game_hosts_lua.cpp:7591`-`7594`, `:7609`) and `SetDeadMeat` (`008AC94B`,
    `game_hosts_lua.cpp:9897`) draws on the same ship. The new key is not, so each ship's six
    draws are isolated, as they are OFF on the `<name>#leak` H-U key.
  - With no gunnery host the draws fall back to H-U, as OFF. In a mission a gunnery host is
    always present.
- **Lifetime.** ON adds no state. The gunnery host lives until `~GameUnitsHost` resets it, and
  the row 15 flush runs inside the mission. `s.process_index` is stable, because slots are never
  erased.
- **With leak92 (main 07c034f0f).** leak92 edits the rate loop of the same function and this
  packet edits the draw loop. The two merged without a conflict.

## 3. Reach

The log set is the frozen list `reports/cc13_sink_logs.txt` (471 logs; see docs/CC13_LEAK92.md
section 4), produced by `python tools/cc13_sink_log_scan.py scan reports/cc13_sink_logs.txt`.
Each line of the list is "path, size, mtime" (tab-separated, mtime in UTC), and `scan` prints
"WARNING changed since freeze" for any log whose size or mtime no longer matches, then a final
`changed_since_freeze` count. All 471 were stamped 2026-10-19 with no log modified after the
12:30Z cutoff; the re-scan reports `changed_since_freeze 0`.
Columns:
- *shared*: logs without the "per-consumer random streams ON" summary;
- *live*: shared logs with a wreck death and plane activity after it (plane death rows, val rack
  drops, bomb drop scatters). In those, H-U's later plane-AI draws are offset now, and ON would
  offset H-G's later draws instead.
- *92h posts* is the number of sinking ships, six draws each.

| mission | logs | shared | live | 92h posts min/median/max |
| --- | --- | --- | --- | --- |
| BSM01 | 28 | 6 | 6 | 4 / 7 / 12 |
| BSM09 | 61 | 18 | 18 | 1 / 2 / 10 |
| JM03 | 6 | 1 | 0 | 0 / 16 / 26 |
| JM05 | 12 | 1 | 0 | 0 / 1 / 10 |
| JM06 | 12 | 4 | 4 | 17 / 19 / 20 |
| JM08 | 33 | 11 | 8 | 1 / 25 / 39 |
| JM16 | 2 | 0 | 0 | 51 / 51 / 51 |
| PRCP01 | 5 | 0 | 0 | 9 / 9 / 20 |
| PRCP05 | 6 | 4 | 0 | 1 / 3 / 5 |
| PRCP07 | 1 | 0 | 0 | 2 / 2 / 2 |
| PRCP08 | 5 | 0 | 0 | 1 / 1 / 1 |
| USN01 | 88 | 9 | 9 | 0 / 4 / 7 |
| USN10 | 2 | 0 | 0 | 24 / 24 / 24 |
| USN11 | 3 | 0 | 0 | 3 / 3 / 3 |
| USN12 | 2 | 0 | 0 | 21 / 21 / 21 |
| USN13 | 4 | 0 | 0 | 2 / 5 / 5 |
| USN15 | 7 | 0 | 0 | 15 / 23 / 54 |
| USN21 | 65 | 21 | 14 | 0 / 15 / 17 |
| USNOS | 20 | 2 | 2 | 19 / 24 / 37 |
| USNRM01 | 2 | 0 | 0 | 10 / 11 / 11 |

- Missions that post no 92h are not listed. There the switch changes nothing.
- **Most of the table runs in streams mode:** 77 of the 364 listed logs are shared. In the rest,
  ON changes the six values directly, and from there whatever the sink physics feeds (section 5).
- **cc9's current re-pairs run in streams mode.** The evidence is the manifests: all 22 of the 22
  `s60_m1*` / `s60_m2*` `*.log.manifest.json` files in cc9-ships44 (rechecked 2026-10-19) carry
  `"BSP_GUNNERY_RNG_STREAMS": "1"` in their env. No startup line prints the mode, so the manifest
  env is the record of it. As a cross-check, the "per-consumer random streams ON" summary was
  read in `s60_m2off_j06.log` before that log was removed; only its `.done`, launch file and
  manifest remain. cc9-physics1's `p23j` pairs run both modes (the `*_def` logs are shared).
- JM06's first sink at 0.05 s is a death on the first step with no damage (`first_damage=-1.00`),
  for example `Gato-class Submarine 01` in `p17htd_jm06w.log`. It is a wreck that is instant at
  load, not one that sank in combat.
- First sinks fall at 117 s (BSM01), about 155 s (BSM09), 0.05 s (JM06),
  71-450 s (JM08), about 162 s (USN01), 149-165 s (USN21) and 22-30 s (USNOS). Nothing before
  the first sink moves.
- In streams mode (every log not counted as shared), both generators are keyed, and no other
  key's draws move. The six values still change, with the consequences in section 5.
- The *live* column is a proxy: the host has no H-U draw counter.

## 4. The draw count: `[manager+14h]` against a fixed six

- `0074EC50` loops `[ESI+14h]` times (`0074ECBA CMP EDI,[ESI+14h]`) and copies the same count
  into the 92h message.
- `0074F490` writes it once, as the constant 6 (`0074F4DD MOV dword ptr [ESI+14h],0x6`). The
  host's comment at its `LeakManager::init_0074f490` site says the same.
- The constructor `0074E7B0` writes `+18h`, `+1Ch`, `+20h`, `+24h`, `+28h`, `+2Ch`, `+38h` and
  `+3Ch`, but not `+14h`.
- The save path `0074EDC0` only compares `+14h` (`0074EDE6`, `0074EE49`).
- So every leak manager that reaches `0074EC50` with its arrays set has six leaks. Every ship
  class in this installation gets six: the count is a constant, not data.
- The host draws only when `leak_ready`, which is set after its `0074F490` model. No wreck in
  the `p24*` and `g54*` logs of the frozen set reports `leaks bound=1 built=0`.
- **Scope of that finding.** "Only `0074F4DD` writes `+14h`" rests on these three bodies
  (`0074F490`, `0074E7B0`, `0074EDC0`), not on a program-wide displacement or block-copy scan.
- **Not live.** No switch is built.

## 5. Expected movers with the switch ON

- **Shared mode, per sinking ship:**
  - H-U loses six draws, so every later plane-AI draw shifts.
  - H-G gains six draws at the row 15 flush, so every later gunnery, hit-effect, break-up,
    ship-AI and routed death-mode draw shifts.
  - The leak draw values change, so the rates, the sink trajectory and the kill-depth time move
    slightly.
  - Divergence starts at the first sink, with the per-mission counts in section 3.
- **Streams mode:** no other key's draws move, but the six values change. Through the rates
  the sink physics changes (trajectory, kill-depth time). Through the wreck pose, so does any
  shell-on-wreck hit (docs/CC13_LEAK92.md section 4). Anything downstream of those events can
  then move too.
- **Draw totals:** unchanged at six per sink. They move from H-U to H-G.
- **Log:** no new line.

## Uncertainty

- The job race decides, per step and per element, whether a pilot-bot or gunnery draw shares
  slot 0 with the sink draws. Its outcome is not modelled anywhere in the host.
- The `Draw::sink_leak` key is a host measurement construct, like every streams-mode key. The
  image has one stream-1 sequence per thread slot.
- The no-unbuilt-wreck check covered the `p24*` and `g54*` logs of the frozen set, not all 471.
