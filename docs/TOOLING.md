# Pair and worktree tooling

Addresses: none (workspace tooling, packet `cc9_tooling_1`)

Tools that replace steps every worker used to do by hand. None of them touches Ghidra, a ledger
or a reconstructed body.

| tool | usage |
| --- | --- |
| `tools/pair_diff.py` | `python tools/pair_diff.py local\X_OFF.log local\X_ON.log [--json out.json] [--limit N]` |

## 1. `tools/pair_diff.py`: the same-tree pair comparison

It reads two `bsp_game` logs of one pair (the same run parameters, one switch flipped) and prints
a report in order of importance:

1. **Run parameters.** The `bsp_game milestone` header (log path masked) and the `frame jitter`
   line. A difference is printed first as `RUN PARAMETERS DIFFER`. A log from before the jitter
   option has no jitter line and counts as `frame jitter off`.
2. **Clock offset.** `t=` of the first `gunnery step N` line both logs have, ON minus OFF, with
   the `first_hit` delta beside it. The docs have so far judged the offset from the first hit
   ("both first hits land within 0.1 s", docs/SHIP_AI_TAILS.md). The gunnery step is used instead
   because it is the mission clock at a fixed step count, so a real change in the first hit
   cannot hide an offset. The offset is subtracted from every time column (`t`,
   `first_damage`, `sunk_at`) before those are compared. A non-zero offset is never "identical".
3. **Gameplay rows.** Deaths, hit records, hull hits, damage (`summary mission gunnery damage`),
   shots (the last `gunnery step` line), first hit, torpedo-task and dive-bomb-task releases (of
   aircraft), torpedo drops, plane water contacts (the `Plane::water_contact*` native calls), the
   controlled unit's distance moved, units, the mission end or failure time, and the native
   table's concrete / unimplemented header.
4. **Per-entity tables, row by row.** `death row` lines keyed by victim, `plane death mode` lines
   keyed by unit, and the end-of-run unit table (`unit side guns cats ... sunk_at killed_by`).
   Each reports rows only OFF, only ON, and the changed fields. `death row` lines need
   `BSP_DEATH_TABLE=1`; the report notes when a side has none.
5. **The whole native table, both ways.** Rows after `host methods N concrete, M unimplemented`,
   keyed by name and address: added, removed, status changed (UNIMPLEMENTED / concrete), calls
   moved.
6. **Every `summary` line,** keyed by its leading words. A key that repeats is extended by its
   first field's name, and, if it still repeats, by the whole first field (`mission gunnery
   barrel device=202`), so one extra line does not shift every later key.
7. **Every other line,** compared as a multiset of masked lines. The `host X [addr] status`
   first-call lines are left out because the native table carries the same status with counts.

**Exit code.** 0: identical apart from noise. 1: something differs, but every gameplay row and
per-entity row is identical (the usual result for a switch whose own rows change). 3: a gameplay
or per-entity row moved. 2: usage error, or a log without a native table (an unfinished run).
`--json` writes every row the report truncates.

### The noise list

Each item is masked (or, for the one native row, its count ignored) before anything is compared.

| id | what | why it varies |
| --- | --- | --- |
| heap-pointers | every 8-digit hex value, `0x` or bare | heap objects land inside the image's own address range too (`0x005C2F28` in one run), so no range test separates them. Native-table addresses are parsed before masking |
| log-path | `log=` in the milestone header | each run has its own log name |
| module-directory | `module directory ...` | an OFF and an ON build sit in different directories |
| harness-slot | `harness: single-instance mutex name suffixed with instance tag ...` | `tools/run_game.ps1` hands out slots 0-2 (docs/COORDINATION.md, three slots) |
| harness-affinity | `harness: main thread pinned to processor N` | the slot's core (2, 4, 6) |
| thread-ids | `worker=` and `render_thread=` on the renderer lines | OS thread ids |
| press-start-blink | `color=(...)` on `text press_start_Text` | the title screen's blink runs on the wall clock before the mission (docs/GAME_EXECUTABLE.md, `--frame-jitter` measured table) |
| prewindow-fmod-calls | `fmod_calls=` on `sound startup before window:` | FMOD polls before the window exists, a wall-clock count (131 against 132 in the UC3 pair, docs/AVOID_ZONE_REGISTRY.md) |
| avoidance-refills | `refills=` on `ship avoidance search:` | differs between identical runs of one binary (257 against 265, 2026-09-22) |
| pretranslate-count | calls of the `PlatformLoopCallbacks::pretranslate` native row | window messages (focus, paint) depend on the desktop; printed as `(noise)` |

### Validation

Every pair below is a pair of record in a worker tree, read in place. The tool's rows are the
recorded verdict's rows.

| pair (tree) | recorded verdict | tool |
| --- | --- | --- |
| `seg_off_e2` / `seg_on_e2` (cc9-gunnery2) | docs/PLAYER_GUN_SEAT.md 6.4: only `PlayerGunSeat::segment_query` UNIMPLEMENTED -> concrete (18,158 calls) and the seat line's own values; 836 / 51 both | exit 1. Native: that one status change. Summary: `casts 0 -> 17997, bound 0 -> 1`. Death rows, plane death modes and 81 unit rows identical; other lines 0 / 0 |
| `gf_off_usn02` / `gf_on_usn02` (cc9-side-ai) | docs/CONSTRUCT_WORLD.md 14: +2 rows (the two `Session::` records), four bound rows concrete, `fixed step body` concrete 81,000 -> 117,000 and records 54,000 -> 18,000; 22 deaths, 440 hits both | exit 1. Native 1464 -> 1466: those two added, those four changed. Summary: exactly that `fixed step body` change. 22 death rows identical |
| `UC3_OFF_USN02` / `UC3_ON_USN02` (cc9-plane-release) | docs/AVOID_ZONE_REGISTRY.md, the pairs measured: natives identical on USN02, gameplay identical, only noise (refills, slots, `fmod_calls` 131 / 132) besides the bound line | exit 1. Native: no change. Summary: `mission units contracts: bound 0 -> 1`. Noise masked. Notes the pair has no death rows |
| `CS_OFF_9000` / `CS_ON_9000` (cc9-circle-steer) | docs/PILOT_MOVETO_TASK.md, E2 9000: deaths 43 -> 51, hits 796 -> 841, releases 5/6 -> 5/5, Lexington 4284.80 -> 5833.61, units 92 -> 81, steer UNIMPLEMENTED 13,929 gone and `BotStateMoveToCircle::tick` concrete 3,762 | exit 3 with exactly those rows. Death rows: the eight escort Zeros (#1.2, #2.2, #4.2, #8.2 and wingmen) only ON. Unit table: the eleven phase-2 units only OFF |
| `FJ_J1_9000` / `FJ_J2_9000` (cc9-circle-steer) | docs/GAME_EXECUTABLE.md `--frame-jitter` measured: identical but for the title-screen alpha | exit 0 |
| `FP_OFF_9000` / `FJ_LOCK_9000` (cc9-circle-steer) | same section: identical but for `frame jitter off` and the alpha | exit 0 |

Clock offsets were zero in every pair above. The subtraction of a non-zero offset has not been
exercised on a real pair.
