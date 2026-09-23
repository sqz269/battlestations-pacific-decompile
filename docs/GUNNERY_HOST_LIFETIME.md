# The gunnery host's lifetime

Packet `cc8_gunnery_host`, 2026-09-19. Addresses `00810DD0` (the per-unit gun object's creation
block) and `00864BD0` (the attach onto the unit's tick element).

## 1. The defect

`GameUnitsHost::create_units` ended with, unconditionally:

```cpp
host.gunnery = std::make_unique<GameGunneryHost>(host.log, *this, host.lua);
host.gunnery->set_ship_ai(host.ship_ai);
host.gunnery->attach_00864bd0();
```

`create_units` is not a load-time routine. It has three producers:

| producer | file |
| --- | --- |
| the initial scene pass | `src/game_hosts_mission_frame.cpp:1406` |
| the air-ops squadron launch | `src/game_hosts_script_orders.cpp:405` |
| the `SpawnNew` queue | `src/game_hosts_script_orders.cpp:535` |

So every spawn batch destroyed the gunnery host and built a new one. It is the third instance of
one bug class in that same function: `register_units` reassigning the weapon directors
(`docs/SHIP_AI_DRIVE_GATE.md` section 5, packet `cc8_ship_drive`) and the AI coordinator rebuilt
per batch (the guard now on the lines just below, packets `cc8_ship_follow` / `cc8_ship_screen`).

The image builds the gun object once per unit: `00810DD0`'s creation block puts a 558h-byte
object at `unit+6DCh` and attaches it to the unit's tick element `unit+310h` through vtable +4h
(`00864BD0`). Nothing in it is per-batch.

## 2. What the host owned, and therefore lost

### 2a. The simulation effects, which matter more than the reporting

This was not only a census defect. Rebuilding the host re-ran the load-time seeding over units
that were already fighting:

- **`build_guns` ran `state.health = state.max_health` over every unit, every time.** Every
  living unit was fully healed once per batch - 12 times in USN04's 4800 mission frames. A ship
  therefore never had to carry its accumulated damage, and the mission's controlled carrier
  survived a mission it should have lost. Section 6 measures this: `Lexington-class01` took 6291
  of 8000 and lived unguarded, takes the full 8000 and sinks guarded.
- **`attach_passes` re-primed the throttle accumulator, the category state, the bridge countdown
  and the fire cache** of every unit, which is the load-time seed for the two-second weapon
  director think.
- **The ship-AI host's raw `const GameGunneryHost*` dangled for one line per batch**, between
  the `make_unique` that destroyed the old host and the `set_ship_ai` on the next line that
  rebound it.

### 2b. What the census lost

Everything in `GameGunnerySummary` (`bomb_drops`, `queued_hits`, `hit_records`, `deaths`,
`kill_credits`, `total_damage`, `first_hit`), the `bomb_impacts` vector, every in-flight
projectile, swimming torpedo and falling bomb, every queued hit, `clock_seconds`, `step_index`,
and the whole `unit_state` vector.

`first_hit` and the `t=` of every `torpedo trace` line are **host-relative**, not mission time,
because they come off `Impl::clock_seconds`, which started again at zero with each new host. The
right way to compare a pre-fix `first_hit` with a post-fix one is to add the host-construction
time to the old value: on USN04 at 4800 frames that is 13.35 + 106.55 = 119.90, the post-fix
figure exactly.

By contrast `summary mission dive-bomb task` and `summary mission world` live on the units host,
which survives, and were always mission totals. That asymmetry is what made the artefact look
harmless, exactly as the units host's own `joins=25` did in the coordinator case.

## 3. Confirmation from existing logs (no run needed)

`J:\PROG\battlestations-pacific-decompile-cc8-dive-goaway\local\goaway_after.log` (4800 mission
frames) and `goaway_long.log` (9000, same binary). `create_units` prints one
`world units: N created instance(s)` line per call, which is the census used here.

| | short (4800 frames, 240 s) | long (9000 frames, 450 s) |
| --- | --- | --- |
| `create_units` calls | 13 | 25 |
| last call at | line 20390, world frame 2103, t = 105.15 s | line 71796, world frame 7620, t = 381 s |
| summary window | 135 s | 69 s |
| `bomb_drops` | 19 | 0 |
| `queued_hits` / `hit_records` | 77 | 272 |
| `deaths` / `kill_credits` | 9 | 5 |
| `total_damage` | 10188.4 | 3596.8 |
| `first_hit` | 17.15 s (host) = mission 122.3 s | 15.10 s (host) = mission 396.1 s |

The long run's spawn timeline, from the 25 `world units:` lines and the world frame preceding
each: 1 initial, 4 `SpawnNew` during `luaStageInit`, 4 air-ops launches at t = 27, 27, 30, 30 s,
4 `SpawnNew` at t ≈ 105 s, **3 single-unit creations at t = 345 s**, and **9 air-ops launches at
t = 366, 366, 369, 369, 372, 372, 375, 378, 381 s**.

This corrects the premise the packet was dispatched on. The rebuild that happened after frame
4800 in the long run was **not** a `SpawnNew` batch: both runs contain exactly the same 8
`SpawnNew` batches, all fulfilled before frame 2103. The post-4800 rebuilds are the air-ops
launch producer and the single-unit creations.

### The counter reset, caught in the log

`gunnery: bomb drop N` prints only while `summary.bomb_drops <= 6`, so the printed lines are the
first six **per host**. The long log's eight lines are:

```
29780: gunnery: bomb drop 1 by D3A Val #3.1 ...
...
31177: gunnery: bomb drop 6 by D3A Val #3.1|.-2 ...
68110: gunnery: bomb drop 1 by D3A Val #3.1|.-3 ...      <- after the t=345 s rebuilds
70633: gunnery: bomb drop 1 by D3A Val #3.1|.-2 ...      <- after the t=366..372 s rebuilds
```

The counter restarting at 1 twice is the host being thrown away, directly in the log. The short
log holds the first six of those lines and no more, and its summary says `bomb_drops=19`: 6
printed, 19 counted, both by the host built at t = 105.15 s.

### Rounds in flight at a boundary

In this pair no torpedo was airborne across a boundary: the long run's last `torpedo trace` is
`exit=expired` at host t = 174.46 s (line 54726, mission ≈ 279.6 s), before the t = 345 s
rebuild, and there are no traces at all after line 71801. Bombs did cross: the drops counted by
the host that died at t = 345 s included rounds still falling, and the `bomb_drops=0` of the
final host is the visible end of that.

### Why the two summaries are not comparable

Neither is a mission total. The short run reports the 135 s since its last batch and the long run
the 69 s since its last batch, which is why the longer run reports *less* of everything except
`queued_hits`. **Every mission total in this repository taken on a multi-batch mission is a
since-the-last-batch count**, including the dated reference rows in `docs/GAME_EXECUTABLE.md`.

## 4. The fix

`create_units` now constructs the host once and, on later batches, calls the one new public
method `GameGunneryHost::register_new_units_00864bd0()`, which runs `build_guns` and
`attach_passes` over `[built_units, units.count())` only. `GameUnitsHost` only ever appends to
`slots`, and it holds them as `unique_ptr`, so both the indices and the `GameUnitSlot` addresses
already built are stable across a batch; `unit_state.resize` keeps the states behind them.
`flatten_class_tables` writes `VehicleClass[id].BSPGun` into the Lua state, which outlives the
call, so only the new units' class ids are flattened. `build_rank_table` is built from
compiled-in lists and is not re-run. `resolve_recon_inputs` already self-heals on a unit-count
change and needed no edit.

The ship-AI host's `const GameGunneryHost*`, set through `bind_gunnery`, is now stable for the
mission; before the fix it dangled between the `make_unique` and the `set_ship_ai` on the next
line, once per batch.

## 5. Predictions, written before the runs

Same binary, one line toggled (the `if (host.gunnery == nullptr)` guard), identical parameters.

**(i) USN04, `--frames 5000 --mission-frames 4800`.** Against the unguarded build:

1. `first_hit` falls a long way and changes meaning: it becomes mission-absolute, so it should
   land near the first shell that connects anywhere in the mission rather than at 17.15 s after
   t = 105 s. Predicted well under 17 s.
2. `queued_hits`, `hit_records`, `attributions`, `total_damage` all rise, because the hits of the
   first 105 s are now counted. `total_damage` predicted to roughly double from 10188.4.
3. `bomb_drops` rises from 19 only if bombs fell before t = 105 s. The printed lines say the
   first drop of the mission was after the last batch, so this one may barely move — a null here
   is a real result, not a failure.
4. `deaths` rises above 9, and this one is **simulation, not reporting**: the per-batch heal is
   gone, so a ship that was worn down across several batches now keeps the damage and sinks.
5. The simulation therefore diverges, and earlier than the packet brief expected. It will be
   identical only up to the first boundary at which some unit was carrying damage — the t = 27 s
   air-ops launch at the earliest, t = 105 s at the latest — and not merely where a vanished
   round would have landed. `world frame` / `dive probe` / `plane` lines are expected to differ
   after that point.

**(ii) USN01, `--frames 3200 --mission-frames 3000`.** If USN01 calls `create_units` once, this
is the null control and must be bit-identical on every census line. Predicted identical.

**(iii) USN04, `--frames 9200 --mission-frames 9000`.** The summary must become a superset of the
4800-frame one: `bomb_drops` >= the 4800-frame figure, `first_hit` equal to it. Against the
unguarded long run it should rise enormously (from `bomb_drops=0`, `total_damage=3596.8`).

## 6. Measurements

All runs from base `3e7625be0` through `./tools/run_game.ps1`. The "off" build differs from the
"on" build by one token: the guard reads `if (true)` instead of `if (host.gunnery == nullptr)`,
which reproduces the old path exactly (a fresh host each batch has `built_units = 0`, so its
`attach_00864bd0()` builds every unit, as before).

The `goaway_*.log` figures in section 3 are from a different binary in another worktree. They are
evidence for the premise and are **not** used as a before/after pair.

### Window 1: the fix is doing what it says

`local\gh_on_usn04.log`, USN04 `--frames 5000 --press-start-frame 30 --mission-frames 4800
--mission-frame-seconds 0.05`. 13 `create_units` calls produced 1 construction and 12
`gunnery: spawn batch registered unit(s) X..Y on the existing host` lines, the first four adding
units 21..23, 24..26, 27..29 and 30..32 at 12 new guns each (418 -> 454 guns).

`bomb_drops=23 bomb_impacts=20`, `torpedo_drop drops=12`, `queued_hits=143 dispatched=143
hit_records=143 hull=106 attributions=143 deaths=14 kill_credits=14 total_damage=14607.3
first_hit=119.90 s`, `dive-bomb task: aircraft=15 releases=23`.

### Window 2: USN01 is a real null control

`local\gh_on_usn01.log`, USN01 `--frames 3200 --mission-frames 3000`. USN01 makes **one**
`create_units` call and zero registrations, so the guard never fires and the fixed build cannot
differ from the unguarded one. `bomb_drops=0 bomb_impacts=0`, `queued_hits=38 dispatched=38
hit_records=38 hull=34 attributions=38 deaths=1 kill_credits=1 total_damage=2914.7
first_hit=62.55 s`. Prediction (ii) stands or falls on the off-run being identical to this.

### Window 3: the 9000-frame superset check

`local\gh_on_usn04_long.log`, USN04 `--frames 9200 --mission-frames 9000`. 25 `create_units`
calls, 1 construction, 24 registrations. `bomb_drops=30 bomb_impacts=30`, `torpedo_drop
drops=12`, `queued_hits=171 dispatched=171 hit_records=171 hull=133 attributions=171 deaths=15
kill_credits=15 total_damage=15447.8 first_hit=119.90 s`, `dive-bomb task: aircraft=24
releases=30`.

This is prediction (iii), and it holds on every column:

| | 4800 frames | 9000 frames | superset? |
| --- | --- | --- | --- |
| `bomb_drops` | 23 | 30 | yes |
| `bomb_impacts` | 20 | 30 | yes |
| `queued_hits` | 143 | 171 | yes |
| `deaths` | 14 | 15 | yes |
| `total_damage` | 14607.3 | 15447.8 | yes |
| `first_hit` | 119.90 s | **119.90 s** | equal, as required |

Unguarded, the same comparison ran the wrong way on every column (`bomb_drops` 19 -> 0,
`total_damage` 10188.4 -> 3596.8, `first_hit` 17.15 -> 15.10 with both host-relative). The
longer run now reports strictly more of everything and the first hit of the mission is the same
event at the same time, which is what a mission total has to do.

### Window 4: the 4500-frame reference row, fixed binary

`local\gh_on_usn04_ref.log`, USN04 `--frames 4700 --mission-frames 4500`, the exact command line
`docs/GAME_EXECUTABLE.md` cites. 13 `create_units` calls. `bomb_drops=18 bomb_impacts=18`,
`torpedo_drop drops=12`, `queued_hits=90 dispatched=90 hit_records=90 hull=67 attributions=90
deaths=8 kill_credits=8 total_damage=10022.4 first_hit=119.90 s`, `dive-bomb task: aircraft=15
releases=18`.

### Window 5: the controlled pair

`local\gh_off_usn04.log` (guard `if (true)`) against `local\gh_on_usn04.log`, one build apart by
that token, USN04 `--frames 5000 --mission-frames 4800`, base `3e7625be0`. The off run makes the
same 13 `create_units` calls and 0 registrations.

| column | OFF (per batch) | ON (per mission) |
| --- | --- | --- |
| `bomb_drops` | 23 | 23 |
| `bomb_impacts` | 20 | 20 |
| `torpedo_drop drops` | 12 | 12 |
| `queued_hits` / `hit_records` | 151 | 143 |
| `hull` | 121 | 106 |
| `deaths` / `kill_credits` | 14 | 14 |
| `total_damage` | 14042.2 | 14607.3 |
| `first_hit` | 13.35 s | 119.90 s |
| `dive-bomb task releases` | 23 | 23 |

**The host-construction offset is 106.55 s**, measured rather than assumed: `13.35 + 106.55 =
119.90`, and the same offset appears on three sinkings below. The last batch therefore built its
host at mission 106.55 s, and the off run's whole summary is the 133.5 s after it.

Several columns do not move, and each null has a reason: USN04 drops its first bomb *after* the
last batch, so `bomb_drops`, `bomb_impacts` and `releases` were already whole; `torpedo_drop
drops` likewise. `queued_hits` falls (151 -> 143) even though the window grew, and `hull` with
it, because the battle itself changed - see below.

### Where the two simulations diverge

Census lines (`world frame`, `torpedo trace`, `dive probe`, `plane `): 6944 in the off run, 7324
in the on run. The first textual difference is at census index 2545 and is **only** the `t=`
field of a torpedo line whose position, velocity, `from_y` and `life` are identical - the clock
relabel, not physics.

Ignoring `t=`, the runs are identical for **6274** census lines and first diverge at world frame
4496, mission **224.80 s**:

```
OFF: torpedo trace 10 exit=entity_impact hit=Fletcher-class02 at=(-12779.5,0.00,-12795.8) life=5.00
ON:  torpedo trace 10 ... keeps swimming; traces 11 and 12 continue
```

`summary mission world` moves with it: `total_path` 52358.07 -> 52341.64, with `walked`,
`updated`, `motion_ticks` and `moved` identical.

### The sinkings, by unit

31 of 33 unit rows differ; 10 sinkings changed. Three of those are the clock relabel alone (same
killer, same damage, offset 106.55 s): `B5N Kate #4.1` 13.35 -> 119.90, `B5N Kate #4.1|.-3`
13.45 -> 120.00, `D3A Val #1.1|.-2` 49.50 -> 156.05. The other seven are real:

| unit | OFF `taken`/`health`/`sunk_at`/killer | ON | what changed |
| --- | --- | --- | --- |
| `Lexington-class01` | 6291 / 1709 / lived | 8000 / 0 / **230.26 s** / `B5N Kate #6.1\|` | the controlled carrier now SINKS |
| `movieval` | 211 / 9 / lived | 220 / 0 / 227.91 s / `Northampton-cl` | now dies |
| `Fletcher-class02` | 2500 / 0 / 119.70 s / `B5N Kate #6.1\|` | 1002 / 1498 / lived | now survives |
| `movieval\|.-3` | 220 / 0 / 110.70 s / `Fletcher-class` | 87 / 133 / lived | now survives |
| `D3A Val #1.1` | 69.55 s / `Northampton-cl` | 161.11 s / `Lexington-clas` | offset 91.56, killer changed |
| `B5N Kate #2.1\|.-3` | 65.00 s / `Northampton-cl` | 167.01 s / `Northampton-cl` | offset 102.01, so 4.5 s earlier |
| `D3A Val #1.1\|.-3` | 96.55 s / `Fletcher-class` | 203.46 s / `Fletcher-class` | offset 106.91, so 0.36 s later |

Two units that died now live and two that lived now die, which is exactly why `deaths` is 14
either way. **The headline null hides four flipped outcomes**, and the most consequential is that
the mission's controlled carrier sinks. The cause is not a vanished round: it is
`state.health = state.max_health`, which `build_guns` ran over every unit on every batch, so
`Lexington-class01` was fully healed 12 times and never had to carry its 8000 points of damage at
once.

### The null control, measured

`local\gh_off_usn01.log` against `local\gh_on_usn01.log`: **7140 census lines each, zero
differences**, and every gunnery column identical (`queued_hits=38 deaths=1 total_damage=2914.7
first_hit=62.55 s`). USN01 makes one `create_units` call, so the guard cannot fire. Prediction
(ii) holds exactly.

## 7. Prediction scorecard

Written before the runs in section 5, scored against the controlled pair only.

| # | prediction | outcome |
| --- | --- | --- |
| 1 | `first_hit` falls well under 17 s | **WRONG.** It rose to 119.90 s. I conflated the host clock with mission time: `first_hit` becomes mission-absolute, and the first hit of the whole mission genuinely is at 119.90 s. The right statement is that the same event is relabelled by the 106.55 s offset. |
| 2 | hits and `total_damage` rise, damage "roughly doubling" | **HALF WRONG.** `total_damage` rose only 4 % (14042.2 -> 14607.3) and `queued_hits` *fell* (151 -> 143). Nothing was hit before the last batch that the off run missed; what changed was which ships died. |
| 3 | `bomb_drops` may barely move; a null is a real result | **RIGHT, and it is an exact null** (23 -> 23), because USN04's first drop is after the last batch. |
| 4 | `deaths` rises above the off run's | **WRONG as stated** (14 -> 14). The per-unit table shows why: two deaths gained, two lost. The prediction was right about the mechanism (the per-batch heal) and wrong about the aggregate. |
| 5 | divergence starts at the first boundary where a unit carries damage | **PARTLY RIGHT.** The physics is identical for 6274 census lines and first diverges at mission 224.80 s, far later than the t = 27 s or t = 105 s boundaries I named. The re-primed throttle did *not* perturb the run on its own. |
| ii | USN01 bit-identical | **RIGHT**, 7140 census lines, zero differences. |
| iii | the 9000-frame summary is a superset of the 4800-frame one | **RIGHT** on every column, with `first_hit` equal. |

Two claims I retract outright: that the summary difference between the two `goaway` logs was
evidence about *this* change (those are a different binary in another worktree, and the
controlled pair shows much smaller column movement), and my interim reading that the first hit
came 2.4 s earlier because a vanished round now lands - it is the same hit at the same time,
relabelled.

## 8. The other unit-creation paths (packet cc9_gunnery_host)

2026-09-22, on main `1cc9d3dea`. The fix in section 4 covered the `create_units` batch path. This
section audits every other path that can add units to a running mission. For each one it asks
whether the path constructs or resets the gunnery host. It also asks whether the path re-runs any
per-unit seed over units that already exist. The machine-readable table is
`reports/gunnery_host_audit.json`. This is a host-source audit, and no native routine's contract
was in doubt, so no Ghidra reads were needed.

| path | call chain | host | existing units re-touched | verdict |
| --- | --- | --- | --- | --- |
| initial scene pass | `game_hosts_mission_frame.cpp:1368` `release_units`, `:1397` new units host, `:1406` `create_units`, guard at `game_hosts_units.cpp:4001` | built once | no | correct |
| air-ops squadron launch | `game_hosts_script_orders.cpp:405` `create_units`, `game_hosts_units.cpp:4006` `register_new_units_00864bd0` | kept | **yes: every unit's ordnance mask** | **fixed here** |
| `SpawnNew` / `GenerateObject`, the single-unit path | `game_hosts_script_orders.cpp:492` `create_unit_from_scene_record_0046db4b`, `:535` `create_units` | kept | same mask re-store | fixed by the same change, and not reached by USN01 or USN04 |
| commands host `register_units` | `game_hosts_units.cpp:3945` hands every slot over on every batch, `game_hosts_commands.cpp:1099` | no | no: directors and speed blocks are `resize`d since `cc8_ship_drive` | clean |
| ship-AI `register_units` | `game_hosts_mission_frame.cpp:1456`, load step only | binds once at `:1457` | no | clean, but see below |
| squadron member resolve | `game_hosts_units.cpp:3958` and the per-frame air-ops update at `game_hosts_script_orders.cpp:462` | no | re-derives member indices by name, and gets the same answer because units are never removed | clean |
| AI coordinator | `game_hosts_units.cpp:4026`, guarded | no | no | clean since `cc8_ship_follow` |
| mission reload | `game_hosts_mission.cpp:1637` new frame host, then `release_units` | destroyed together with its units | not applicable: the units are new | correct per-mission contract |
| checkpoint | `game_hosts_mission.cpp:938` and `:943` are unimplemented stubs | no | no | no path exists |

### The fourth re-seed: the ordnance mask

`build_guns` ended by re-storing every unit's ordnance mask, starting at index 0. The comment
said this was harmless: an earlier unit's aggregate cannot change, because `guns` only gains rows
for new units. The aggregate indeed cannot change, but the stored mask can. `release_ordnance_drop`
clears the dropping bomber's kind 2Bh torpedo bit (packet `cc8_torpedo_breakoff`), and the next
spawn batch put the bit back. So every spent torpedo bomber was re-armed at the next batch. It is
the same class of defect as the health, the directors and the coordinator. The unit table's
consumers of that bit include the approach-arm guard at `game_hosts_units.cpp:7610` and the
`007B93F0` binding at `:5091`.

The fix stores masks for the new units only. It still counts, but leaves alone, any
already-built unit whose stored mask differs from the recomputed aggregate. Setting
`BSP_GUNNERY_RESTORE_ALL_ORDNANCE=1` restores the old store, so both behaviours run from one
binary. `summary mission gunnery ordnance_rearms=N mode=...` reports the count.

**Which missions reach it.** At 4500 or 4800 frames USN04 creates all twelve of its later batches
before its first torpedo drop, so it cannot reach the path. Its reference row on the fixed build
reads `ordnance_rearms=0` and matches `local\ref_usn04.log` on every simulation line. USN04 at
9000 frames drops 12 torpedoes and then creates 12 more batches, and that is the run measured.

**The same-binary pair**, USN04 with `--frames 9200 --press-start-frame 30 --menu-select USN04
--mission-frames 9000 --mission-frame-seconds 0.05`, differing only in the environment variable:

| | fixed (`local\ord_new_usn04_9000.log`) | old store (`local\ord_all_usn04_9000.log`) |
| --- | --- | --- |
| `ordnance_rearms` | 144: each of 12 batches finds the same 12 spent bombers and leaves them | 12: the first batch after the drops re-arms all 12 |
| torpedo `drops` / `loadout_cleared` | 12 / 12 | 12 / 12 |
| `total_damage` / `deaths` / `queued_hits` | 17543.1 / 13 / 174 | 17543.1 / 13 / 174 |
| `bomb_drops` / dive-bomb `releases` | 30 / 30 | 30 / 30 |
| per-unit table, 57 rows | | 0 rows differ |
| torpedo task `blocked_arm_009d49a0` | 32862 | 20274 |
| approach `replans`, per spent B5N | 95 to 124 | 200 to 228 |
| scan `runs`, per spent B5N | 10 to 13 | 24 to 27 |

So the re-arm was live but did not reach combat in this window. A re-armed bomber passes the
approach-arm guard again, so it re-plans and re-scans about twice as often. It never releases a
second torpedo, and not one hit, sinking or damage figure moves. The effect is behavioural churn
in 12 spent aircraft, and the fix removes it. Whether a longer run would convert that churn into
a second drop is not measured. `release_ordnance_drop` itself does not test the
mask; only the task gates do.

**One counter varies between runs.** `ship avoidance search refills` differs between two runs
with identical parameters on which this path never fires. The fixed build's 4500-frame run reads
257 and the reference reads 265, and everything else matches. The 557 against 539 in the pair
therefore belongs to that variation, not to this change.

### What the audit leaves open

- **Spawned units get no ship-AI controller.** The ship-AI host's `register_units` clears and
  re-assigns its controllers over the unit count at load, and nothing calls it again. That is
  correct as far as re-seeding goes, but a ship created by `SpawnNew` mid-mission has no
  controller row. It is a coverage question for the ship-AI packets, not a re-seed.
- **A stale comment.** `game_hosts_script_orders.cpp:373-376` still says `create_units`
  constructs a fresh coordinator on each batch, which stopped being true with `cc8_ship_follow`.
  That file is outside this packet's lease.
- **One path is unmeasured.** The `SpawnNew` path is fixed by construction but not reached by
  either reference mission, as its own comment at `:508` already says.
