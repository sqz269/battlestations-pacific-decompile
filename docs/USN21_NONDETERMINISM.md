# USN21 run-to-run nondeterminism (cc15-nondet)

Packet cc15-nondet, worker cc15-hint2, 2026-10-24. This page only characterises the effect. cc9 owns
the root cause, and nothing here changes code.

All runs used the ref env: BSP_DEATH_TABLE=1, BSP_GUNNERY_RNG_STREAMS=1, BSP_FRAME_JITTER unset. Every
log has its "rng streams: on" line.

The USN21 row is 80000 mission frames with helm plan local/s46_u21_f.txt, launched through
tools/run_game.ps1 slots.

The save directory (Documents\Battlestations-Pacific: options.txt, save\0\player, quick, valid) had
the same SHA-256, size and mtime before the first run and after the last run.

## 1. Every USN21 run (from the manifests)

- "Head" is the HEAD of the host tree at launch; the exe sha256 identifies the build.
- No run reached the mission end: Mission.EndMission was never true in any of them.
- "Overlaps" counts the other bsp_game runs, from any lane's manifests, whose run window intersected
  this one.

| log | exe sha256 | head | start-end UTC | tag / core | deaths | shots | hit records | overlaps |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| on | f4829b8643bf | 1cf2447d7 | 10-19 19:38-20:27 | slot12 c26 | 514 | 130772 | 9644 | 85 |
| off | ec7124a80f79 | 1cf2447d7 | 10-19 19:41-20:23 | slot5 c12 | 514 | 130772 | 9644 | 75 |
| off2 (first) | 2c4c8035bc4e | 64163508f | 10-19 20:43-21:16 | slot2 c6 | 514 | 127936 | 9373 | 29 |
| on2 | c339513b50be | 64163508f | 10-19 20:51-21:23 | slot13 c28 | 517 | 130396 | 9239 | 14 |
| gb | 9bd208a409e8 | 77cc21b65 | 10-24 02:43-03:27 | slot7 c16 | 514 | 130772 | 9644 | 100 |
| hon | 06986be85e63 | 77cc21b65 | 10-24 02:58-03:36 | slot12 c26 | 517 | 130396 | 9239 | 104 |
| goff | 17eed00ebf72 | 77cc21b65 | 10-24 03:03-03:42 | slot5 c12 | 514 | 130772 | 9644 | 103 |
| ga | 3cd35037eed8 | 77cc21b65 | 10-24 03:15-03:58 | slot10 c22 | 516 | 130430 | 9176 | 108 |
| gm | 3b1babc541d6 | 77cc21b65 | 10-24 03:35-04:13 | slot2 c6 | 514 | 130772 | 9644 | 112 |
| hoff | 1bc2cc38e226 | 77cc21b65 | 10-24 03:36-04:25 | slot8 c18 | 515 | 131757 | 12187 | 129 |
| off2 (rerun) | 2c4c8035bc4e | 77cc21b65 | 10-24 04:39-05:28 | slot10 c22 | 514 | 130772 | 9644 | 151 |
| gar | 3cd35037eed8 | 77cc21b65 | 10-24 05:34-06:20 | slot7 c16 | 514 | 127936 | 9373 | 82 |
| hoffr | 1bc2cc38e226 | 77cc21b65 | 10-24 05:37-06:25 | slot4 c10 | 514 | 130772 | 9644 | 83 |
| gbr | 9bd208a409e8 | 77cc21b65 | 10-24 05:38-06:26 | slot0 c2 | 514 | 130772 | 9644 | 86 |
| off2r | 2c4c8035bc4e | 77cc21b65 | 10-24 05:44-06:32 | slot9 c20 | 514 | 130772 | 9644 | 93 |

Every run overlapped other bsp_game runs; overlap was never absent. Some of the overlapping runs were
USN21 runs in other lanes.

### Which runs matched

The runs fall into four outcome clusters plus two singletons:

- **514 / 130772 / 9644:** on, off, gb, goff, gm, off2 (rerun), hoffr, gbr, off2r. This cluster mixes
  six different exes, both hint switch states, 10-19 and 10-24 runs, and slots 0 to 12.
- **517 / 130396 / 9239:** on2 and hon. Both are hint-ON builds with the warning-manager skip.
- **514 / 127936 / 9373:** off2 (first) and gar. These are different exes, run five days apart.
- **Singletons:** ga (516 / 130430 / 9176) and hoff (515 / 131757 / 12187).

The same exe gives different outcomes:

| exe | runs and shots |
| --- | --- |
| 2c4c8035bc4e | 127936 on 10-19, then 130772 twice on 10-24 |
| 3cd35037eed8 | ga 130430, gar 127936 |
| 1bc2cc38e226 | hoff 131757, hoffr 130772 |

Neither the slot, the core nor the overlap count predicts the cluster. Pairs that matched include
gb/gbr (slots 7 and 0, 100 and 86 overlaps). Pairs that did not match include ga/gar (slots 10 and 7).

## 2. Wider than USN21? USN04 on one exe

The exe is current main 505de7041, built in this tree, sha256 51a15c3ac0a1. Copies were staged as
m1, m2 and m3. USN04 ran 4800 mission frames in the ref env.

| run | start-end UTC (10-24) | tag / core | concurrency |
| --- | --- | --- | --- |
| m1 | 06:55:53-06:56:15 | slot10 c22 | alone (among my runs) |
| m2s | 06:56:25-06:56:47 | slot10 c22 | alone (it queued behind m1 for the slot) |
| m2 | 06:57:41-06:57:58 | slot3 c8 | concurrent with m3 |
| m3 | 06:57:42-06:58:04 | slot8 c18 | concurrent with m2 |

All six pairs among the four runs gave the same result:

- pair_diff exited 0.
- local/hint2_strict.py found 71 event lines on each side and 0 differing indices.
- A full-log index diff, with pointer-sized hex masked, found nothing beyond the differences listed
  below.

All of the differences are presentation or known noise:

- the instance tag, the core and the module directory;
- the press-start text alpha (0.94, 0.89 or 0.86; wall-clock);
- one extra `host PlatformLoopCallbacks::pretranslate` registration in m2. It shifts later indices by
  one line, and the host-method count goes 1135 to 1134.
- `ship avoidance search refills`: 289, 304 and 308;
- `free bearing refills`: 537 and 538.

The clear_37c column of the end table is identical in all four runs.

So USN04 is deterministic on one exe, whether runs are concurrent or alone.

## 3. Leads checked from the logs

### x87 precision

All 14 USN21 logs report the same precision:

- "x87 precision before Direct3D: 53-bit (double)"
- "x87 precision at the fixed simulation step: 24-bit (single) (_MCW_PC = 0x00020000)"

The m1 to m3 USN04 logs report the same. The sampled precision does not separate the clusters. It is
sampled once per run, so a later per-thread flip would not show in these logs.

### The first divergence, by strict index

local/nondet_first.py compares the full logs line by line in index order:

- It masks pointer-sized hex, the run identity lines and the colour alpha.
- It resyncs over the one-line pretranslate insertion.

Results by pair:

| pair (same exe) | outcome | first diverging line |
| --- | --- | --- |
| ga / gar | 130430 / 127936 | line 511789, `ship ai step 41370 Vincennes` |
| hoff / hoffr | 131757 / 130772 | line 459264, `ship ai step 37890 Iowa` |
| off2 (rerun) / off2r | both 130772 | line 470392, `ship ai step 38860 Iowa` |
| gb / gbr | both 130772 | none before the end-of-mission summaries |

The ga / gar divergence:

- Only `target=` and `rudder=` differ: 2.7267 vs 2.7239, and -0.675 vs -0.651.
- Hull heading and position are identical on that line.
- At step 41430 the target crosses the wrap: -2.9352 vs 2.1935. The rudder goes -1.000 vs +1.000 and
  the run separates.

The hoff / hoffr divergence:

- `rudder=` differs, -0.357 vs -0.356. Target, hull and position are identical.
- From step 37900 the target differs as well: 2.2246 vs 2.2261.

The off2 (rerun) / off2r divergence:

- The target differs by 0.001 at steps 38860 and 38880. The rudder is saturated at -1.000.
- It does not propagate: everything else is identical up to the summaries.

In every pair the first gameplay-visible difference is the steering target or rudder of a ship in
`state=follow mode=navigate`. Positions and hull headings still match on that line. That is the value
the avoidance and free-bearing search feeds.

### clear_37c and the searcher cache

Neither family is logged per step. Before the first divergence they appear only as follows:

- clear_37c appears in the end-of-mission table header, around line 905000 and later.
- The searcher cache appears as `host ShipAi::refresh_avoid_zone_searchers` registration lines and
  in the end summaries.

Whether those cells move before the first divergence therefore can't be shown from these logs. A
per-step dump of the searcher bounds would be needed. The end summaries do differ in every pair,
including the two that matched on gameplay:

- avoidance refills: 8209 vs 7818;
- free-bearing `empty`: 2896257 vs 2873069;
- corner_fwd and corner_back.

That fits the cache's uninitialised heap bounds from docs/AVOIDANCE_REFILL_DETERMINISM.md. On USN21
those bounds evidently can reach the steering target: in a ship that sits on a wrap or decision
boundary, the perturbation can flip the steer instead of healing. The earlier statement "gameplay is
unaffected" therefore holds on USN04 but not on USN21.

This is a candidate, not a proof. cc9's dangling stack slot in run_pilot_move_to_range is an
independent candidate: a dead-slot write would also show up first as a steering difference.

## 4. USN21 on the bisect2 fix

The lead dropped this step: cc9's bisect2 lane runs the fixed and unfixed builds five times each, so
those results are recorded there, not here.
