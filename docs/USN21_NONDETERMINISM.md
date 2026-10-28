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

## 5. Load and stall discriminator (cc15-nondet2)

Packet cc15-nondet2, 2026-10-24.

Setup:
- Exe: the kept ga exe, sha256 3cd35037eed8. Each run used its own copy, local/hint2_exe/gA1..gC2.
- Args: the s46_u21_f row, 80000 frames, in the ref env. Every log has its "rng streams: on" line.
- Driver: local/nondet2_arm.ps1. It reads the run's slot and core from run_game.ps1's runner line,
  then:
  - starts the hog, `pwsh -Command while(1){}`, with ProcessorAffinity set to that core. It measured
    about 81% of the core.
  - for arm C, sets every bsp_game process of the run to BelowNormal.
  - kills the hog when the run exits.
- At most two of these runs were alive at once.
- The machine ran 14 of 14 slots with other lanes' runs throughout, and was at 100% CPU for part of
  it. "No added load" therefore still means a fully loaded host.

| run | arm | slot / core | start-end UTC | shots | deaths | hits | end t |
| --- | --- | --- | --- | --- | --- | --- | --- |
| gA1 | A control | slot8 c18 | 12:55-13:42 | 130772 | 514 | 9644 | 4002.73 |
| gA2 | A control | slot3 c8 | 13:42-14:20 | 130772 | 514 | 9644 | 4002.73 |
| gA3 | A control | slot3 c8 | 15:43-17:06 | 130772 | 514 | 9644 | 4002.73 |
| gB1 | B hog | slot13 c28 | 12:31-13:19 | 130772 | 514 | 9644 | 4002.73 |
| gB2 | B hog | slot12 c26 | 13:28-14:17 | 130772 | 514 | 9644 | 4002.73 |
| gB3 | B hog | slot3 c8 | 14:20-15:42 | 130772 | 514 | 9644 | 4002.73 |
| gC1 | C BelowNormal + hog | slot1 c4 | 14:17-18:17 | 130772 | 514 | 9644 | 4002.73 |
| gC2 | D BelowNormal + duty hog | slot12 c26 | 17:36-19:33 | **130396** | **517** | **9239** | 4002.73 |

All end at step 80000 with no mission end. Notes on the arms:

- **Arm C starves rather than stalls.** The game's main thread has base priority 7 (AboveNormal in
  the BelowNormal class) and the hog's is 8. With both pinned to one core, the game ran only on the
  scheduler's starvation boost: 88 s of CPU in its first 58 minutes, where a normal run uses about
  2700 s.
- **gC1 does not cover the divergence window under starvation.** I removed its hog after 244 s of
  game CPU, about the first 5% to 9% of the mission. It then finished at BelowNormal alone. It
  starved through startup and the early mission only, so it says nothing about the step 37000-46000
  window where the USN21 pairs diverge.
- **Arm D replaced C for the second run.** Since full starvation can't finish, gC2 used a
  duty-cycled hog at Normal priority: 200 ms busy, then 200 ms asleep, measured 47% of the core. The
  BelowNormal game therefore lost the core for about 200 ms of every 400 ms for the whole run.
- **No gC3.** The user set C:\Users\sqz269\.bsp\DRAIN at 18:12 UTC ("drain requested by user via
  cc16"), so it was never launched.

Strict comparisons against gA1 with local/nondet_first.py:

- **gA2, gA3, gB1, gB2, gB3 and gC1:** no gameplay line differs. The only differences are the
  startup `fmod_calls` count (line 99), the end-of-mission avoidance and free-bearing refill/empty
  counters, and the `pretranslate` call count. Unlike the earlier off2 rerun/off2r pair, there isn't
  even a transient steering-target wobble.
- **gC2 deviates:** 130396 / 517 / 9239, the same outcome as the earlier on2 and hon runs.
  - The first diverging line is 563951, `ship ai step 45340 SantaFe state=follow mode=navigate`.
    `target=` is 2.3251 vs 2.3221 and `throttle=` is 1.241 vs 1.242.
  - The step 45330 line and every earlier line match.
  - The difference grows over the following steps while the rudder stays saturated, and the runs
    then separate.

Conclusion:
- A bursty stall on the game's core is enough to move USN21 off the 130772 outcome.
- A constant 81% hog at equal priority, and an early-only starvation, are not.
- Six control and hog runs, plus cc9's four, all gave 130772. So 130772 is the normal outcome, and
  the earlier spread came from runs that were stalled the same way, under heavy load.

What the first diverging value goes through:
- It is again the steering target and throttle of a ship in follow/navigate. That is the main-thread
  fixed-step ship AI, fed by the avoidance and free-bearing search; position and hull heading still
  match on that line.
- The fixed-step job waves are not on worker threads in this build. Log line 3490 says
  "fixed-step job waves: actual groups at 00f876c0 and their shared native frame-job owner are not
  attached", so they run inline. The original's 24-bit vs 53-bit job-claiming race
  (docs/CC13_FPU_PRECISION.md) is therefore not the path.
- The only per-run-varying state the logs show is the heap-dependent searcher cache. Its end
  counters differ in every run, including the matching ones.
- A timing-dependent heap layout, from other threads allocating during a stall, would explain why
  stalls change the outcome. That is consistent with these runs but not proven by them.

Wall-clock reads in src. The mission step's time does not come from the clock:

- `GameMissionFrameHost::run_mission_frame_004e4a40` (src/game_hosts_mission_frame.cpp:4534)
  replaces the frame delta with `--mission-frame-seconds` (0.05).
- Frame jitter (src/game_hosts_mission.cpp:2181) is armed only by BSP_FRAME_JITTER, which is unset.
- `scale_frame_delta_004c6e30`'s clamps (src/game_frame_control.cpp:143, 162) act on that fixed
  delta, so they are constant.

The QueryPerformanceCounter, timeGetTime and GetTickCount reads are all outside the step:

| read | where | what it does |
| --- | --- | --- |
| update/initialize/sample frame clock | src/frame_clock.cpp:88-143 | frame clock; its delta is overridden in-mission |
| `query_performance_counter_00be3240` | src/engine_services_cc13.cpp:285 | no caller in src |
| `measure_cpu_clock_0098c940` | src/engine_seg11_35_66_cc13.cpp:1482 | startup hardware info |
| `GameFrameProfiler::query_performance_counter` | src/game_hosts_menu.cpp:118 | profiler only |
| `wait_for_mapping` | src/game_native_data_bootstrap.cpp:432 | GetTickCount64 startup handshake timeout |
| XLive clock sample | src/xlive_manager_runtime.cpp | XLive |
| d3d9_probe, physical_pending_reads_probe | | probes |

Thread pools that exist in the build:

- the frame-job pool (src/native_frame_job_lifetime.cpp:99), with workers at
  THREAD_PRIORITY_HIGHEST;
- the Dyn task manager (src/dyn_task_manager.cpp:90), which has a LIFO dequeue and is typed for
  avoid-zone hull memory;
- the renderer control worker and the XLive IPC thread.

Neither pool's worker count, nor whether either runs during the mission step, is logged. The Dyn
task manager is the one worker path whose types connect to the avoidance search, so it is the first
place to look for a stall-sensitive input. That pointer is unverified.

Save directory: the same SHA-256, size and mtime before (local/nondet2_save_before.txt) and after
(local/nondet2_save_after.txt) the runs. After the last run, Get-Process/CIM showed no hog, no
driver and no bsp_game of this tree left.

## 6. The Dyn task pool, runtime threads and the stack scrub (cc9-nondet1)

Worker cc9-nondet1, 2026-10-27, from main 1c49c8c0d.

### The Dyn task pool is not on the USN21 path

- `dyn_task_manager_run_batch_00c33140` has two callers: the native world step 00C5C540 (Solve, at
  00C5BCCA and 00C5C0E8) and its collision pass 00C57070 (at 00C577EE).
- The mission never runs that step. Every USN21 log's end table has
  `FixedStepFanout::simulate_physics_world 00875e0c UNIMPLEMENTED calls=80000`, because
  `GameFixedStepHost::native_game_` is null (src/game_hosts_fixed_step.cpp). Rigid bodies are
  integrated through 00C41550 and 00C5B1B0 on the main thread.
- Had it run, it still could not reorder results:
  - `run_batch` is a full barrier: the main thread waits on the group's event.
  - Each task gets a fixed, contiguous range of islands.
  - The kernels are transcriptions of the image's assembly.

### Threads that run during a mission

These were listed from the live nd1_c1 process, with start addresses from
NtQueryInformationThread (local/nd1_threads.ps1):

| thread | CPU | start |
| --- | --- | --- |
| main | about 1650 s | bsp_game.exe+4FD170 |
| three FMOD threads | 34 s, 5 s, 2 s | fmodex.dll+1C161 |
| about 60 driver threads | about 19 s, nearly all on one thread | nvd3dum.dll+45A344 |

There are also 64 idle-priority threads that used 0 s of CPU. No Dyn worker, frame-job worker or
renderer control worker does any work.

The mission loads no messages or sounds. The message manager's sound hooks are unreached, and the
end of a run reports `sound after raw singleton drain: started=0`. So FMOD playback state cannot
feed gameplay.

This leaves the main thread's own inputs. The stack is one of them: the message pump, Present
and the driver leave garbage below the stack pointer that depends on timing. A gameplay read of
a stack slot nothing wrote would then depend on stalls.

### BSP_STACK_SCRUB

`BSP_STACK_SCRUB=<hex byte>[,<KiB>]` is a harness-only diagnostic in src/game_hosts.cpp. It is
off when the variable is unset. When set, it fills the given amount of stack below
`GameFrameHost::game_on_move` with the byte before every frame; the default amount is 256 KiB.

Results, both on 1c49c8c0d, plain load:

| run | scrub | shots | deaths | hits |
| --- | --- | --- | --- | --- |
| nd1_c1 | none | 146859 | 520 | 10693 |
| nd1_s0 | 00 | 146859 | 520 | 10693 |

`local/nd1_first.py` finds no gameplay line that differs between the two. The only differences
are the monitor name, `pretranslate` and `present failed` lines.

Zero-filling the stack at the start of each frame therefore does not move USN21. That is weak
evidence against a stale stack read before the first call inside the frame. It does not rule out
garbage written during the frame, for example by FMOD or the driver.

### Runs lost to the renderer under the rdp-tcp session

- **nd1_d1** (BelowNormal plus a 200/200 ms duty hog) crashed at mission frame 27670:
  - c0000005 reading 00000000 at bsp_game.exe+430325, in
    `set_native_renderer_render_state_00b24460`+D5.
  - The minidump is local/nd1_d1_u21.dmp; the map is local/nd1_map_base.map.
- **nd1_c2** hung in `d3d9!CBaseDevice::Reset`, called from bsp_game+2DC040. I killed the process.
- **nd1_c1 and nd1_s0** survived, but 32757 and 41543 of their frames skipped Present.

The lead stopped hog and stall runs on 2026-10-27.

### The searcher bounds are the only state that varies between plain runs

These runs used plain load and the same s46_u21_f row:

| run | exe | shots | deaths | hits | avoidance refills | free-bearing empty |
| --- | --- | --- | --- | --- | --- | --- |
| nd1_sc | 1c49c8c0d, BSP_STACK_SCRUB=cc, 1600x900 window | 146859 | 520 | 10693 | 7235 | |
| nd1_b1 | 8eded9e50 + 05790190b, kSearcherCacheBoundsInitCc13Bound forced ON (local exe) | 146859 | 520 | 10693 | 7551 | 2985071 |
| nd1_b2 | the same exe as b1 | 146859 | 520 | 10693 | 7551 | 2985071 |

The c1, s0 and sc refill counts were 6928, 7099 and 7235.

- sc matches c1 and s0 on gameplay, at a different window size and on a different monitor.
- `local/nd1_first.py` finds that b1 and b2 differ only in the `PlatformLoopCallbacks::pretranslate`
  call count and its registration line. Every other line matches, including the end summaries,
  `fmod_calls` and the avoidance and free-bearing counters.

So the four uninitialised searcher-cache bounds (section 3, docs/AVOIDANCE_REFILL_DETERMINISM.md)
are the only gameplay-side state that changes from run to run, and they are image-faithful. The
message pump count also varies, but it does not reach the simulation.

Every USN21 split seen so far must therefore come through those bounds. A stall or load changes
the heap's history, and with it the leftover values; gameplay depends on them only occasionally.
This report found no host-only order or timing dependence: the Dyn pool is idle, the frame-start
stack does not matter, and the stale-stack reads on the follow path are modelled as constants.

Options for the lead:
- Leave the switch OFF. This is faithful to the image, and USN21 pairs stay noisy.
- Run pair and WINS work with the switch ON as a labelled diagnostic. The switch is a deviation
  from the image, so flipping it is not a faithful fix.

Not yet run, because the lead holds stall runs: whether a duty-hog run with the switch ON still
matches b1. That run would close the loop.

Decision (cc9 lead, 2026-10-28):
- The shipped build stays faithful: kSearcherCacheBoundsInitCc13Bound stays OFF.
- The environment variable `BSP_SEARCHER_BOUNDS_INIT=1` (src/ship_ai_search_storage.cpp) does
  the same zeroing at run time. It is a harness determinism aid, not image behaviour. It is off
  when unset and logs one startup line when active.
- Pairs and WINS runs on noisy rows (USN21 at least) set it on both sides (docs/TOOLING.md,
  section 1, "Noisy rows").

## 7. Hand-off: PC24 and stream switch censuses and pairs (cc9-nondet1, 2026-10-28)

The worker tree is `J:\PROG\battlestations-pacific-decompile-cc9-nondet1`. Everything below runs
one game at a time from `local\`:
- `nd1_census2.ps1 -Rows <list> -ExeTag <dir under local\nd1_exe> -Prefix <p> [-BoundsAid]`
  writes `nd1_<p>_<row>.log` and appends start/end lines to `nd1_<p>sus.txt`.
- `nd1_chain*.ps1` chain those calls and log each step to `nd1_chain.txt`.
- `nd1_pairs.ps1 -Off <p> -On <p> -Rows <list>` prints pair_diff headlines.
- Creating `local\nd1_census.stop` or the DRAIN flag stops a chain before its next run.

Every pair's OFF half is a census log from the same commit as its ON exe.

Done and committed:
- Censuses: 0a9f01d9b (gunnery RNG, avoid zone, dyn hull), 21b615d04 (S4) and c2ff264bd (S5 + S15)
  are landed. b3ed16b1e (hullscope) is on the branch, not yet landed.
- Aid: 66f1f910e (`BSP_SEARCHER_BOUNDS_INIT`, section 6).
- Flips: 08f74b74f (gunnery RNG ON) and 6f1edbdb9 (avoid zone ON). Both were gameplay identical on
  every pair.

S4 kAimLeadBodyAxisCc13Bound is not flipped:
- ijn18 is identical.
- bsm09, usn01, lomp10, usn12, bsm01, esmp08 and usn21 (aided) moved, and the moves are plausible.
- prcp01 goes from completed at 689.96 s to failed at 745.94 s (`usn03.obj_p2_fail`, Enterprise).
  The lead decides whether that is a knife-edge row or an S4 defect.
- Logs: `nd1_s4c_*` (OFF), `nd1_s4on_*`, `nd1_s4a/s4oa_usn21`.

The census on b3ed16b1e (`nd1_c2_*`, usn21 in `nd1_c2a_usn21`) found every switch nonzero:
- S5 track_differs is nonzero on every row except lomp10 (0 reads). The threat reads differ on
  usn02, usn12 and ijn18.
- S15 not_reset is nonzero on bsm09, usn01, lomp10, prcp01, bsm01, jm06, ijn18 and esmp08.
  lomp06, usn02 and usn12 have 0.
- hullscope hull_scope_differs is 1 to 8 shapes per row, 0 on usn01.

Queued when this was written:
- chain5: the 7-switch stream-set joint pair (S1, S3, S10, both stream set 2 switches, S19, S22).
  The ON exe is `sson` and the OFF exe is `cen2`, both sides aided. Rows, in order: usn01,
  bsm09, lomp10, prcp01, usn12, bsm06 `h1_i`, jm08 `s63_j8_v3`. Prefixes `ssoff` / `sson`.
- chain6, then:
  - S5 alone (`s5on`);
  - S15 alone (`s15on`);
  - hullscope + dyn hull jointly (`hullon`).
  Each runs on its nonzero rows, then on usn21 aided (`*oa`). The OFF halves are `nd1_c2_*` and
  `nd1_c2a_usn21`.

Each result is judged by plausibility. Rows that move go to cc9-lua74 after a flip. A flip
commit takes a one-commit lease on the header, then pc24_lint and a full build.