# CC13 sinkrng: the generator for the wreck's leak draws

Packet cc13-sinkrng. The switch is `kSinkLeakDrawStreamCc13Bound` in
`include/bsp/cc13_sinkrng.hpp`. It is committed OFF. Not game-validated. This is the gap that
docs/CC13_LEAK92.md section 5 flagged.

## Verdict

- **Image.** `0074EC50` draws six `U(0, 1)` on stream 1 (`ECX = 1`, `0074ECA5`). It runs from
  the wreck handler at row 15 of the fan-out, on the main thread, so the draws use slot 0's
  stream 1.
- **Host, today.** The six draws come from the units host's release generator, H-U. That is the
  plane AI's generator.
- **What happens on the default (shared) path.** Every sinking ship advances the plane AI's
  generator by six and leaves the gunnery generator untouched. The image does the reverse.
  This is **LIVE** in every shared-mode log of the missions that post a 92h. Every one of them
  has plane activity after its first sink (section 3).
- **Under `BSP_GUNNERY_RNG_STREAMS=1`,** which is a measurement option, the "#leak" key
  isolates each ship's draws and nothing else couples.
- **The fix.** `kSinkLeakDrawStreamCc13Bound` ON takes the six draws from the gunnery
  generator, H-G, in call order.

## 1. The host's model of image stream 1

docs/CC13_RNG_STREAM_CENSUS.md section 5 covers the full census. Rechecked against the code:

| host generator | code | users | models |
| --- | --- | --- | --- |
| H-G | `GameGunneryHost::Impl::rng`, through `Impl::draw` / `random_range_00bd2f10` | gunnery, `Draw::hit_effect` (the deferred dispatch at rows 6 and 14), `Draw::ship_breakup`, `ship_ai_draw`, `death_mode_draw_00bd2f10(1, ...)` (the plane death mode, the torpedo goaway jitter) | main-thread stream 1: the only generator that takes the movie reseeds (`reseed_shared_stream_00bd2fd0`) |
| H-U | `GameUnitsHost::Impl::db_release_rng`, through `release_altitude_draw_00bd2f10` | plane AI: dive bombing (`#r2c`, `db_dive_alt_a8`, ...), torpedo (`#t70`..`#t88`), goaway (`#ga`, `#gr`), take-off, retreat, dogfight, moveto, level bombing; **and the six sink draws** (`<name>#leak`) | a stream-1 stand-in for the pilot bots |
| H-AI | `game_hosts_ai.cpp` | AI group think intervals | |
| H-SO | `GameScriptOrdersHost::random_state_` | script orders | |
| H-S0 | static `stream0` in `death_mode_draw_00bd2f10` | stream 0 gameplay | stream 0 |

The image has one stream 1 per thread slot. The fixed-step fan-out, the message handlers and Lua
run on slot 0 (census section 3). The pilot-bot waves may run on frame-job workers, so their
draws can land on a worker's stream. The wreck handler `00824B60` runs from the row 15 flush
`009273A0`, in the fan-out, so its draws are slot-0 draws. The host's slot-0 stand-in is H-G.
The same fan-out's row 6 and row 14 hit dispatch already draws there (`Draw::hit_effect`), and
H-G is the generator the movie reseeds. So the sink draws belong on H-G, not H-U.

Under `BSP_GUNNERY_RNG_STREAMS=1` both generators are keyed. H-U's key is FNV-1a of
"`<name>#leak`" and H-G's is `(purpose, unit)`. Each ship's six draws then sit on their own
stream, and nothing else moves.

## 2. The binding

- **OFF:** `ship_wreck_sink_block_00824fe5` draws `release_altitude_draw_00bd2f10(s.row.name +
  "#leak", 0, 1)` six times. No change.
- **ON:** the six draws are `gunnery->death_mode_draw_00bd2f10(1, s.process_index, 0, 1)`.
  - On the default path that is H-G in call order. It is the same entry the units host already
    uses for its other routed stream-1 draws (`00BD2F10(1, ...)` in the plane death mode at
    `007CA914` and the torpedo goaway jitter).
  - Under the streams option the key is `(Draw::death_mode, ship)`. No other consumer uses a
    ship's index with that purpose, so each ship is still isolated. The key is labelled
    `death_mode`: a naming SUBSTITUTION, not a coupling.
  - If the units host has no gunnery host, the draws fall back to H-U, as OFF. In a mission the
    gunnery host is always present, so this does not happen.
- **Lifetime.** ON adds no state. It needs `owner gunnery` to be alive during the row 15 flush,
  and it is: it is reset only in `~GameUnitsHost`. `s.process_index` is the slot's stable index,
  because slots are never erased.
- **Combined with packet cc13-leak92 (agent/cc13-leak92, under review).** That branch edits the
  same function's rate loop. This one edits only the draw loop. The two are independent, and a
  merge should apply both hunks.

## 3. Reach: is it LIVE?

These are the bsp_game logs from the last 36 hours in `cc9-physics1\local` and
`cc9-gunnery52\local`: 343 logs with at least one redistributed wreck (script
`local\sinkrng_scan.py` in this tree). Columns:
- *mode*: `shared` when the log has no "per-consumer random streams ON" summary;
- *sinks*: the number of death rows of ships whose leaks were redistributed, one 92h each;
- *first sink*: the earliest of those death times;
- *plane activity after*: plane death rows, val rack drops and bomb drop scatters after the first
  sink. That shows the plane AI is alive and drawing from H-U.

| mission | mode | logs | sinks | first sink (s) | logs with plane activity after |
| --- | --- | --- | --- | --- | --- |
| BSM01 | shared | 6 | 7-10 | 117.05 | 6 |
| BSM01 | streams | 23 | 4-12 | 117.05 | 23 |
| BSM09 | shared | 20 | 1-2 | 154.85-155.0 | 20 |
| BSM09 | streams | 44 | 1-10 | 154.5-154.85 | 44 |
| JM03 | streams | 5 | 16-26 | 29.3 | 0 |
| JM05 | streams | 11 | 1-10 | 298.15-331.34 | 9 |
| JM06 | shared | 4 | 18-19 | 0.05 | 4 |
| JM06 | streams | 7 | 19-20 | 0.05 | 5 |
| JM08 | shared | 11 | 1-32 | 71.1-449.66 | 8 |
| JM08 | streams | 22 | 1-39 | 241.21-341.49 | 16 |
| JM16 | streams | 2 | 51 | 239.16 | 2 |
| PRCP01 | streams | 5 | 9-20 | 3.05 | 5 |
| PRCP05 | streams | 2 | 3-5 | 12.05 | 2 |
| PRCP07 | streams | 1 | 2 | 80.4 | 0 |
| PRCP08 | streams | 5 | 1 | 254.86 | 0 |
| USN01 | shared | 9 | 4-7 | 161.66-163.26 | 9 |
| USN01 | streams | 71 | 4-6 | 167.96-168.81 | 71 |
| USN10 | streams | 2 | 24 | 26.0 | 0 |
| USN11 | streams | 3 | 3 | 25.55 | 0 |
| USN12 | streams | 2 | 21 | 364.93 | 2 |
| USN13 | streams | 4 | 2-5 | 347.09-368.28 | 4 |
| USN15 | streams | 7 | 15-54 | 83.5 | 0 |
| USN21 | shared | 16 | 14-17 | 149.2-165.21 | 16 |
| USN21 | streams | 40 | 5-17 | 148.65 | 40 |
| USNOS | shared | 2 | 19 | 21.75-29.7 | 2 |
| USNOS | streams | 17 | 20-24 | 23.45-23.5 | 17 |
| USNRM01 | streams | 2 | 10-11 | 98.85 | 2 |

**LIVE (shared mode):** BSM01, BSM09, JM06, JM08, USN01, USN21 and USNOS. In each, every
plane-AI draw after the first sink is offset by six per sink so far. That covers the goaway,
torpedo and dive draws. The gunnery and ship-AI draws are missing the same six per sink.
- Sinks in BSM09 come at 155 s, so they cannot explain anything that diverges before then, such
  as the D3A deaths at 42.70/42.75 s under hitq ON.
- In the JM08 shared logs, three have no plane activity after the first sink. There the H-U
  offset has nothing to act on. The H-G gap remains.

**Not live (streams mode):** every other row. Both generators are keyed, so the six draws touch
no other consumer.

## 4. Expected movers with the switch ON

- **Shared mode, per sinking ship:**
  - H-U loses six draws, so every later plane-AI draw shifts.
  - H-G gains six draws at the row 15 flush, so every later gunnery, hit-effect, break-up,
    ship-AI and routed death-mode draw shifts.
  - The leak draw values change, so the redistributed rates change and the sink trajectory and
    kill-depth time move slightly.
  - Expect chaotic divergence after the first sink: the times and the per-mission sink counts
    are in the table above. Nothing before the first sink moves.
- **Streams mode:**
  - Only the six values change, now keyed `(death_mode, ship)` instead of "`<name>#leak`".
  - The rates and the sink trajectory move slightly, and wreck-pose effects follow (the
    shell-on-wreck coupling in docs/CC13_LEAK92.md section 4).
  - No other draw moves.
- **Draw counts:** the total per mission is unchanged, six per sink. They move from H-U to H-G.
- **Log:** no new line. The OFF and ON paths log the same `LeakManager::redistribute_0074ec50`
  row.

## Uncertainty

- Which image thread runs each pilot-bot draw is not fixed: the census shows that the wave jobs
  race onto workers. So "H-U models stream 1" holds only on average. The argument for H-G rests
  on the fan-out running on the main thread and on H-G being the host's main-thread stand-in.
- The plane-activity column is a proxy. It shows the plane AI alive after the first sink, not a
  count of H-U draws. The host has no H-U draw counter.
- The streams-mode key reuses the `death_mode` purpose label, as the other routed units-host
  draws do.
