# The squadron's `land` task (packet `cc9_land_task_reach`)

Addresses: 009B41C0 009B3240 009B2E50 009AFE70 009AF9A0 009AFA50 009B3EB0 009B3900 009B34D0
009B3560 009B3680 009B3770 009B3CF0 009B3C60 009B3750 006C54C0 006C4790 006BD080 006C0B50 006CD240
006CC9F0 006C7960 006C3B10 006C5C40 006C5380 006C3E50 006C46B0 006C45C0 006BED60 007C6760 006C7540
006BEF70 006C0750 006BEE40 006BC960 006BF0D0 006BA620
0099A3DD

Worker cc9-lua8, 2026-09-28. This continues `docs/CONTROLLED_UNIT.md`, "The squadron's
`returntobase` resolution, and the `land` task's shape" (cc9-lua6). The switch is
`kSquadronLandTaskBound` in `include/bsp/game_hosts_units.hpp`. The code is in
`src/game_hosts_units.cpp`: `install_land_task_0099a3dd`, `land_command_still_valid_009b34d0`
and, in the pilot binding, `run_land_task_tick_009b3eb0` with its helpers.

Status words follow AGENTS.md. Everything below is read from the listing (V) unless it says
LABELLED (a substitution this host makes) or unread.

## 1. Which squadrons reach the task (OFF measurement)

The runs use the reference parameters: `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`, lockstep
0.05, an idle player, present interval immediate, `tools/run_game.ps1`.

| row | `returntobase` placements | squadrons resolved | arm |
| --- | --- | --- | --- |
| LOMP10 9200/9000 (main `4ad2a1bcb`, `local\l8_base_lomp10.log`) | 114 | B-25 01, Lightning 01, Warhawk 01, all at 3.80 s | `land at site CB4_AF` |
| USN04 9200/9000 (same build, `local\l8_base_usn04.log`) | 0 | none | - |

- **The time moved.** cc9-lua6 measured 8.65 s on `1749c12d1`; main `4ad2a1bcb` resolves at 3.80 s,
  the first SELLING tick.
- **USN04 has no SELLING group,** so none of its returning Lexington planes gets a `returntobase`
  from the AI. It cannot measure this task.
- **What the host does with them today.** The SELLING tick places `returntobase` on every member
  plane (command 21, `ai_selling_tick`). No task is installed from it: the host installs bot tasks
  only from the Lua pilot bindings. So B-25 01 keeps its `levelbomb` (`00E08F28`) and the two
  fighter squadrons keep `divebomb` (`00E08F20`), all from stage-init `PilotSetTarget`.
- **What the image does.** PLANNER_TASK_CHOICE 13.3 already predicted it: the air group's squadrons
  leave the fight. `007F1940` issues the resolution on the squadron, each member's attack task
  retires when its predicate sees the command change (`0099A4C0`), and `0099A170` installs `land`.
  So on LOMP10 the image flies all three squadrons home from 3.80 s.

## 2. The task, from the listing

### Construction

- **Factory `009B41C0`** (`009B41C0`-`009B4225`): `ECX` = the bot, `EDX` = the target; allocates
  670h and calls `009B3240(bot, target)`.
- **The land arm of `0099A170`** (`0099A3DD`-`0099A41A`): the target is `006BCD20(target, 1)`,
  the site's air-ops block; null refuses. Then `006C4790(block, [bot+50h]+9D4h)` must be true. So
  the task's target is the **block**, not the site entity the command names.
- **`006C4790`** (`006C4790`-`006C47EC`, `RET 4`): false for a null squadron, false when the
  squadron is on the block's list at `+B4h` (`std::list`, sentinel at `+B8h`, value at node `+8h`),
  else true. The list's producer is unread.
- **Constructor `009B3240`** (`009B3240`-`009B3306`, `RET 8`): base `0099C6F0(bot, 3)`, approach
  `009B2E50(approach = this+3F8h, [bot+50h], target)`, vtables `00D1FFA0` / `00D1FF94` /
  `00D1FF90` (at `+4B0h`). The start state is chosen at `009B32AA`-`009B32DD`:
  - `(plane+72Ch)->vtable[38h]` false: `+620h`;
  - else a flight leader (`007B8AD0`): `+4C4h`; else `+500h`.
  - Then the state's `vtable[4]` (its enter) and `009F9980(approach, this)`.
- **`009F9980 BSP_BotApproach_BindToTask`**: approach `+18h` = task+4 (the plan), `+1Ch` =
  task+314h, `+20h` = task+38Ch.
- **Approach `009B2E50`** (`009B2E50`-`009B3028`, `RET 8`) builds, in order: the base `009AFE70`,
  the moveto state at approach `+CCh` (`009C2AC0`, task `+4C4h`), the follow state at `+108h`
  (`009C2980`, task `+500h`), six states at `+1A0h`..`+254h` (vtable writes only), clears
  `+274h`/`+275h`, then `009AF9A0`.
- **`009AF9A0`** registers the eight state names through `00411E70`. The strings are in the image,
  so these are recovered names:

| task offset | approach offset | vtable | name (string) | reached from (rule `009B3CF0`) |
| --- | --- | --- | --- | --- |
| `+4C4h` | `+CCh` | `00D20AEC` (`009C2AC0`) | `moveto (land)` (`00D1FE1C`) | mode 1, flight leader |
| `+500h` | `+108h` | `00D20AB8` (`009C2980`) | `follow (land)` (`00D1FE0C`) | mode 1, wing member |
| `+598h` | `+1A0h` | `00D1FEA4` | `land/line` (`00D1FE00`) | mode 2, wing member |
| `+5B8h` | `+1C0h` | `00D1FEF4` | `land/standby` (`00D1FDF0`) | mode 2 leader, mode 3, or `+66Ch` in abort |
| `+5D8h` | `+1E0h` | `00D1FF14` | `land/begin` (`00D1FDE4`) | mode 4 |
| `+5F8h` | `+200h` | `00D1FF44` | `land/final` (`00D1FDD8`) | from begin when `009B3C00` answers |
| `+620h` | `+228h` | `00D1FF60` | `land/park` (`00D1FDCC`) | `+900h` 4 or 5; from final when `009B3370` answers; not airborne |
| `+64Ch` | `+254h` | `00D1FED4` | `land/abort` (`00D1FDC0`) | from begin or final when mode is not 4; a done line/standby |

- **The approach base `009AFE70`** (`009AFE70`-`009AFFEA`, `RET 8`, args plane and block):
  `009F9CE0(plane, Pilot/Landing/ReferenceSpeed)`, then:
  - `+2Ch` = the block, `+30h` = block `+7Ch` (the owner), `+34h` = `006C0B50(block, plane+9D4h)`;
  - `+38h..+40h` = the owner's position, `+44h` = 0, `+48h` = `+4Ch` = 1.0, `+50h` (the mode) = 1;
  - `+54h` = `U(0.9, 1.1)` × `Pilot/Landing/ApproachAngle`, then `+58h` = `U(0.9, 1.1)` ×
    `ApproachPitch` (two `00BD2F10` draws, `ECX` = 1, bounds `00CE3860` and `00CE6448`);
  - `+5Ch` = class `+18Ch` TravelSpeed × 0.75 (double `00CEC9D8`), `+60h` = 0, `+64h` = 10.0
    (`00CE38B8`), `+A4h` = 0, `+ACh` = 0.5 (`00CE3800`), `+B0h` = −`U(0, 0.5)` (third draw);
  - then `009AFAF0(0)`, the block-frame geometry (`006BCA80`, `006BCC90`); contract unread beyond
    the fields it writes (`+68h..+70h`, `+8Ch..+94h`, `+A8h`).
- **The follow state `009C2980`** draws `+74h` = −`U(0, 0.6)`. The moveto state `009C2AC0` does not
  draw. Its arguments at `009B2EA9`-`009B2EC9` are the target `[approach+30h]` (the block's owner),
  near 100.0 (`00CE3D08`), far 100.0 (`00CE3D08`) and the speed range 800.0 (`00CE3950`).

### The per-tick `009B3EB0` (vtable `+64h`, `009B3EB0`-`009B3F49`, `RET 4`)

1. `+4ACh` = FFh.
2. `009B3900(approach, onGround, dt)`, `onGround` = the state is begin, final, park or abort.
3. When `+424h` (approach `+2Ch`) is non-zero: the rule `009B3CF0(dt)`, then the state's
   `vtable[0Ch](dt)`, then `+2E4h` = `+4ACh`.

### The approach update `009B3900` (`009B3900`-`009B3B34`, `RET 8`)

1. `009B34D0` (below). If `+2Ch` is now 0, return.
2. `009B3915`-`009B39AD`: when the squadron's command target differs from `+30h` and the squadron
   head is airborne, re-issue `land` at that target on the squadron's director (`0071D810(1)`,
   `0071ECF0(00E08FA0, 00465080(target, 0.0))`).
3. `+68h..+70h` = the plane's position.
4. `[+18h]+268h` bit 0 (the plan's flag word): `+60h` += 50.0·dt (double `00CE3938`), capped at
   plane.y + 50 − `+3Ch`, and `+64h` = 0. Otherwise `+64h` += dt; when `+64h` > 6.0 (`00CE6630`) and
   `+60h` > 0, `+60h` −= 60.0·dt (double `00CE3D68`), floored at 0.
5. Return when plane `+904h` is set. With `onGround`: `009AFAF0(1)` and `0099B650(owner)`.
6. The request pacing: while dt < `+B0h`, `+B0h` −= dt. Otherwise `+B0h` = `+B0h` + (`+ACh` − dt)
   and `006C54C0(ECX = +34h, plane, &approach+38h)`.

**Correction to cc9-lua6's section.** `009B34D0` is not the mode writer. It is the site's validity
check (`009B34D0`-`009B3551`): the site stays only while the squadron's current command
(`vtable[174h]`) is `land` (`00E08FA0`) whose target (`vtable[178h]` -> `00521EA0`) is `+30h`, the
block's owner exists with `+5Dh` clear, and `006C4790(block, squadron)` passes. Otherwise `+2Ch`,
`+30h` and `+34h` are cleared. **The mode `+50h` is written by `006C54C0`** through its out record
(`approach+38h` + 18h).

### The landing request `006C54C0` (`006C54C0`-`006C5634`, `RET 8`)

- `sq = plane+9D4h`; null returns.
- Unless `006BF060(X+4, sq)`, `006C0B50(X+4, sq)` queues the squadron at the block (`+98h`,
  20-byte records). A newly queued squadron also moves any launch slot (`+4Ch`, 58h each) that
  launched it to state 4.
- `006BD080(X+4, plane, &rec)` (`006BD080`-`006BD1AA`) looks the plane up in the block's
  **assignment vector** at `+A8h` (14h-byte records: plane, float `+4h`, float `+8h`, bytes
  `+0Ch`/`+0Dh`, mode `+10h`). Found: the out record takes the two floats, a byte and the mode.
- Not found: out `+14h` = 1.0 and **mode = plane `+904h` ? 4 : 1** (`006C5541`-`006C5560`). Then out
  `+0Ch` = whether the airborne members' offsets in the block frame sum to at least `00D7A218`.
- Always: out `+10h` = `006C3E50(X+4, plane)`, out `+0h..+8h` = `006C5380(X, plane, out+0Ch)`.
- **The assignment vector's producer** is the landing sequencer `006CC9F0` (called from
  `006CD240`). It walks the vector with `006C7960` and `006C3F80` and inserts through `006CAA10`.
  `006C3F80` and `006BCFD0` are readers. None of the three is read here.

### The rule `009B3CF0` (`009B3CF0`-`009B3EAA`, `RET 4`)

1. `009B3770`: from moveto or follow, nothing. From line or standby with the state's `+18h`
   done byte, switch to `009AFA50()`; any other state with it, switch to abort.
2. `+900h` 4 or 5 and the state is neither park nor abort: switch to park.
3. begin: mode not 4, abort; mode 4 and `009B3C00(begin)`, final.
4. final: mode not 4, abort; mode 4 and `009B3370()`, park.
5. park: nothing. abort: `+66Ch`, standby; else `+66Dh`, park.
6. moveto, follow, line, standby: mode 1, `009AFA50()`; mode 2, standby for the flight leader
   (`sq+3D0h` == plane) else line; mode 3, standby; mode 4, begin.

- **`009AFA50`**: not airborne, park; flight leader, moveto; else follow.
- **`009B3680`** (`RET 4`): exit the old state (`vtable[8]`), enter the new (`vtable[4]`); nothing
  when it is the same state.

### The cruise profile `009B3C60` (vtable `+54h`, `009B3C60`-`009B3CE5`)

- A flight leader only (`007B8AD0` on `+3FCh`). The block is `[task+404h]+37Ch`, the squadron's.
- `+38Dh` clear: when `+380h` < 0 and `+3A9h` clear, `+394h` = `Pilot/Landing/CruisingAlt` (1400)
  and `+3ADh` = 1; then `+3A9h` = 0. With `+38Dh` set, neither the write nor the clear runs.
- `+38Ch` clear: when `+37Ch` < 0 and `+3AAh` clear, `+398h` = the same CruisingAlt and `+3ADh` = 1;
  then `+3AAh` = 0.
- **Correction to `docs/BOT_TASKS.md`'s `+54h` table.** The land row says the second altitude is
  `contract: unread` and the tail is `0099B740`. The listing writes `+398h` from the same `+514h`
  and ends `POP ESI` / `RET` with no tail call.

### The vtable `00D1FFA0`

`+40h` = `009B3560`, the retire predicate: 2 without a squadron or command, 1 for `land` whose
target is `+428h` (approach `+30h`), else 0. `+4Ch` = `009B3750`: `009BE3E0` on the follow state
when it is current, else `0099B720` (−1.0). `+30h` = `009B3730`: false in park or final, else true.
`+54h` = `009B3C60`, `+64h` = `009B3EB0`.

## 3. What is bound, and what is refused

**ON** (`kSquadronLandTaskBound = true`), for a member plane that receives the SELLING
`returntobase` while its squadron's resolution is `land at site`:
- **The intake** (`install_land_task_0099a3dd`) retires the attack task (torpedo, dive-bomb,
  dogfight or moveto), sets the command class to `land` and the command target to the site's
  owner, makes the four construction draws (`#l54`, `#l58`, `#lb0`, `#l74`) and starts the leader
  in `moveto (land)`, a wing member in `follow (land)`.
- **A repeated placement** for a squadron already flying `land` at the same site installs
  nothing (`009B3560` answers 1), and is counted as `kept`.
- **The per-tick** (`run_land_task_tick_009b3eb0`) runs `009B34D0`, the approach timers, the 0.5 s
  request `006C54C0`, the rule `009B3CF0`, the state's tick and the cruise profile. A plane flying
  `land` runs no attack arm.
- **`moveto (land)`** is `009C18C0` with near 100, far 100 and speed range 800 at the site's owner:
  the host's glide (`move_to_glide_009c18c0`), cruise altitude, pitch command and the mode-2
  heading at the target, as the dive-bomb moveto already runs it.
- **`follow (land)`** is `009C1FD0` through the host's station and follow law.
- **The wingmen-wait term** (`moveto_speed_009c1850`) now counts a member in `follow (land)` as
  following, because the land task's `+4Ch` is `009B3750`.

**REFUSED, counted and logged** (never approximated):
- a resolution that carries an unread input. On LOMP10 that is B-25 01 (class 10h head, the
  deck's `+20h` bit 1 unread), which keeps its `levelbomb`;
- a plane that is not airborne, whose task would start in `land/park`;
- every entry into `land/line`, `land/standby`, `land/begin`, `land/final`, `land/park` or
  `land/abort`.

**LABELLED substitutions:**
- The install runs at the order's delivery, one bot tick early (`docs/SENTITY_INIT_ATTACH_ORDER.md`
  22.7), and the host's fan-out gives the order to each member plane.
- `006C4790`'s list at block `+B4h` is taken as empty, so the land arm always passes it.
- The squadron's current command for `009B34D0` is the host's last `007F16D0` answer for it.
  The owner's `+5Dh` (network remote) is clear in a single-player mission.
- `006BD080` misses every time, because this host has no landing sequencer (`006CC9F0`). That is
  the image's answer for a plane not yet sequenced, and it is why the mode stays 1. The count is
  `empty_assignments`.
- `006C3E50`, `006C5380` and the block-frame sums of `006C54C0` are unread; only the refused states
  read their outputs.
- `plan+268h` is never set in this host, so the approach altitude ramp takes its second arm.
- The cruise profile runs once per think, as the dive profile does, on the squadron's slot. The
  dirty byte `+3ADh` is not modelled.

## 4. Predictions, written before the ON runs

OFF is this tree's build of `d5adbb900`; ON is `pair_export --commit d5adbb900 --flip
kSquadronLandTaskBound=true` (`local/l8_on`). The OFF logs are `local\l8_off_{lomp10,usn01}.log`.

**LOMP10 9200/9000, OFF** (measured): the three heads resolve `land at site CB4_AF` at 3.80 s,
planar 7047.0 m (B-25 01), 7220.7 m (Lightning 01) and 6730.9 m (Warhawk 01) east of CB4_AF
(-3480.4, 49.9, -4136.8). All ten planes then die attacking between 99.45 s and 117.95 s:
11 deaths, 283 hits (224 hull), damage 3799.8, first hit 90.50 s.

**LOMP10 9200/9000, ON: exit 3.**
- **Installs:** 8, at 3.80 s: Lightning 01 and Warhawk 01 in `moveto (land)`, their six wing
  members in `follow (land)`. B-25 01's two planes are REFUSED (`unread input`) and keep bombing.
- **The later placements** count as `kept` for the fighters and `refused unread` for B-25 01.
- **The approach:** the two leaders close from about 7.2 km and 6.7 km. At the host's
  TravelSpeed-based moveto speed they are over CB4_AF (planar distance under 500 m) between about
  70 s and 120 s after the install. They then keep circling the field, because the moveto tick
  steers at the owner's point for the rest of the run. The minimum distance is under 300 m.
- **The mode stays 1:** `empty_assignments` equals `requests` (about 2 per second per plane),
  `refused_states` = 0, and no plane enters `land/begin`. **No plane lands.**
- **The glide** brings the leaders down toward max(100 + 49.9, 100) = 149.9 m.
- **Cruise profile:** writes 0. All three squadrons were given `SquadronSetTravelAlt` and
  `SquadronSetAttackAlt` with force 1, so `+38Dh` and `+38Ch` are set.
- **Gameplay:** the eight fighters leave the fight at 3.80 s, so most or all of them survive.
  Deaths fall from 11 to 3 or fewer (the two B-25s and the one non-plane death, if it did not
  come from the fighters' bombs). Hits, damage and the first hit move.

**USN01 3200/3000, ON: exit 0 or 1.** No SELLING `returntobase` exists, so the land summary reads
`installs=0` and gameplay is identical.

## 5. The pairs and the verdict

OFF is this tree's build of `d5adbb900` (`local\l8_off_*.log`). ON is `local/l8_on`
(`local\l8_on_*.log`). The diffs are `local\l8_diff_{lomp10,usn01}.txt`. The first ON launch died at
renderer init (`D3DERR_NOTAVAILABLE`, `logonui=1`, a locked RDP desktop); the runs below were
taken after the display came back.

| row | pair_diff | what moved |
| --- | --- | --- |
| LOMP10 9200/9000 | exit 3 | deaths 11 -> 3, hits 283 -> 98 (hull 224 -> 79), damage 3799.8 -> 1279.8, shots 1891 -> 446; first hit 90.50 s both; dive-bomb-task releases 3 of 8 -> none |
| USN01 3200/3000 | exit 1 | the land summary line, all zero; gameplay, death rows and the 35 unit rows identical |

**LOMP10, against the predictions:**
- **Installs, held.** Eight at 3.80 s. The two leaders start in `moveto (land)` and the six wing
  members in `follow (land)`. B-25 01 is refused 69 times (`unread input`) and keeps bombing.
  Both its planes die at 99.25 s and 102.30 s (OFF 99.40 s and 102.25 s), so its row is
  RNG-shifted but has the same shape.
- **Later placements, held.** 592 `kept` per fighter squadron. The SELLING tick keeps placing
  the order on the surviving fighters.
- **The mode, held.** `requests` = `empty_assignments` = 7150, `refused_states` = 0 and
  `retired_invalid` = 0. No state change happens beyond the start, and no plane lands.
- **Cruise profile, held.** 8926 calls and 0 writes (force 1 on all three squadrons).
- **Gameplay, held.** The eight fighters survive; the three deaths are the two B-25s and PT 02,
  which dies at 312.40 s (OFF 312.35 s).
- **The approach time, MISSED (spread).** The leaders are first within about 65 m of CB4_AF at
  about 183 s (Lightning 01), not 70-120 s. Two causes, both in code other than this binding:
  - The glide `009C18C0` commands its far-distance altitude (about 1450 m at 7 km) first, so the
    leaders climb to about 1.25 km before descending to 149.9 m near the field.
  - The moveto speed blend holds the leaders near 31.5 m/s for long stretches. That is the
    wingmen-wait term.

  After arrival the leaders circle within about 600 m at 149.9 m, as predicted. Their minimums
  are 0.2 m and 0.4 m at about 366 s.
- **Not predicted: two Lightning wing members drift.** Lightning 01|.-2 and |.-3 end 5.2 km and
  6.0 km from the field, while their leader circles it. Their commanded speed is the per-think
  reseed, TravelSpeed x 1.6 = 173.3 m/s, against a leader at 31.5-120 m/s. The host's follow law
  (`run_follow_tick_009c1fd0`, shared with the dive-bomb and torpedo follow) does not hold a
  station on a slow, circling leader. The Warhawk members end 460-936 m out.
  This is the follow law's behaviour, not the land task's. It is recorded as an open item for
  that code.

**Verdict: `kSquadronLandTaskBound = true`.** The mechanism matches; the miss is the arrival time
and the wingmen's spread. The ON state is the image's first half: the squadrons leave the fight
and fly home. The second half, the landing, waits for the deck's sequencer (section 6).

## 5b. The landing sequencer, read (packet `cc9_landing_sequencer`, cc9-lua9, 2026-09-28)

This section is a read only. Nothing is bound. The routines below produce the assignment vector at
block `+A8h` that `006BD080` looks up; without them every request answers mode 1, as section 3
says. Every address was read from the Ghidra listing; the constants come from the image on disk,
and every constant address is in `.rdata` (`00CE2000`-`00E08000`), so none has a run-time writer.

### The queue at block `+98h` and its tick

- **`006C0B50`** (`006C0B50`-`006C0D1C`), called from `006C54C0` unless `006BF060` finds the squadron.
  Its 14h-byte record is {squadron, block `+80h`, `n`, countdown `+0Ch` = 0.0, interval `+10h` =
  1.0}. `n` is one more than the largest `n` already queued, so the first squadron has `n` = 0.
  The tail moves any launch slot (block `+4Ch`, 58h each) whose `+28h` is the squadron to state 4.
- **`006BEF70`** (`006BEF70`-`006BEFE7`, `RET 4`): the plane's squadron's queue `n`, or -1.
- **`006CD240`** (`006CD240`-`006CD34C`, `__thiscall(block)(float dt)`), called from `006CDC70`.
  For each queue record: countdown -= dt. When it goes below 0, countdown = interval, interval =
  `006CC9F0(squadron, interval)`, and countdown = min(countdown, interval). The first countdown
  is 0.0, so the sequencer runs on the first tick after the request.

### The sequencer `006CC9F0` (`006CC9F0`-`006CCD77`, `__thiscall(block)(squadron, float)`, `RET 8`, returns ST0)

The float argument is not read. `head` is squadron `+3D0h`; the members are the up to five
pointers at squadron `+3D0h`..`+3E0h`, ending at the first null.

1. Walk `+A8h` for the record whose plane is `head`. On a hit: `006C7960(rec, 1, 4, -1.0)`, then
   `006C3F80(rec)`, then read the record's mode `+10h` and float `+4h`, then `done = 006C45C0(head)`.
2. **`done`**: `006C7540(squadron)` erases every record of the squadron. Returns 1.0 (`00D7A24C`).
3. **Hit, not done**: the result is 0.25 (`00CE3868`) when the head's mode is 3 or 4, else 1.0.
   Every other record whose plane's `+9D4h` is this squadron gets `006C7960(rec, 0, headMode,
   headFloat4)` and `006C3F80(rec)`.
4. **Miss**: when `006C46B0(head)` is true, the result is 1.0 and nothing is inserted. Otherwise
   the result is 0.0, and one record per member is appended through `006CAA10` (a vector
   `push_back`, `006CAA10`-`006CAAB0`) at `006CCCDD`-`006CCD27`:
   - `+0h` the member, `+4h` StandbyDist (tuning `+500h`), `+8h` 1.0, `+0Ch` 0;
   - `+0Dh` = (x - holder `+A4h` >= 0), where x is the head's position through the holder's
     inverse matrix at holder `+48h` (`006CCC52`-`006CCC89`);
   - `+10h` the mode: 3 for the head, 1 for every other member.
- **`006C7540`** (`006C7540`-`006C7670`): erases every `+A8h` record whose plane's `+9D4h` is the
  squadron.

### The mode `006C7960` (`__thiscall(block)(rec, bool leaderPass, int headMode, float headDist)`)

- Plane `+904h` set (landed): mode 4, `+4h` = -1.0, `+8h` = 1.0, return.
- Not airborne (`(plane+72Ch)->vtable[38h]`), or `+0Ch` set: mode = leaderPass ? 2 : 1, `+4h` =
  999999.0 (`00CF87D0`), `+8h` = 1.0, return.
- Squadron `+3B0h` clear and not the leader pass:
  - headMode 2: mode 2 when `006C46B0(plane)` is false and `006C5E20(head)` is true, else 1;
  - headMode 1: mode 1;
  - headMode 4: mode 4 when `006BED60()` and `006C3B10(plane)` answer true and `+8h` > 0.0;
  - otherwise, and headMode 3: mode 3.
- The leader pass, or squadron `+3B0h` set (which also sets leaderPass):
  - `006C3B10(plane)` true: mode 4 when `006BED60()` and `+8h` > 0.0, else 3;
  - otherwise, with squadron `+3B0h` clear: mode 3 when `006C5C40(plane)` is true, else
    2 - `006C46B0(plane)`. So mode 2 inside StandbyDist, 1 outside;
  - otherwise (`+3B0h` set): mode 3.
- Then `006C6020(rec)`. After the leader pass it returns; otherwise `+4h` =
  max(`+4h`, headDist + 1.0) (the double `00D7A210`).

### The predicates, all `__thiscall` on the block, all `RET 4`, each with the plane as the argument

- **`006BED60`** (fastcall on the block): block `+1Ch` and `+1Dh` clear, owner `+7Ch` non-null
  with `+5Dh` clear. The deck can take a landing.
- **`006C46B0`**: the plane's horizontal distance to the touchdown point T is above StandbyDist.
  T is holder `+A4h`..`+ACh` through the holder's matrix at holder `+8h`.
- **`006C45C0`**: airborne and that distance is above StandbyDist x 1.2 (the double `00CEC160`). This releases the squadron.
- **`006C3B10`** (`006C3B10`-`006C3E41`): the runway corridor test.
  - A null plane answers false. So does a plane with `+904h` clear whose `+900h` is 2 or 4.
  - A plane that is not airborne answers true.
  - An airborne plane whose parent (`00923810(1)`, entity `+3Ch`) is an AirField (45h) or a
    MotherShip (9) also answers true.
  - Otherwise (x, y, z) = `006BCC90(pos)`, the position in the holder frame relative to T.
    Let z' = z - 1.5 L, where L = `006BA620()`, which is RunwayLength x 0.3 on a MotherShip owner
    and x 0.4 otherwise.
  - The test is false when y > 5.0 - 2 tan(ApproachAngle) z'.
  - e = max(0, x - w) for x > 0, else min(0, x + w), where w = RunwayWidth x 0.5. The test is
    false when |(e, y, z')| - 1.5 L > PosBehind x 1.1.
  - b is the bearing from the plane to T. It becomes the runway heading (holder `+88h`) when
    |e| < 1.0 and z' > -2L.
  - The test is false unless |wrap(h - runwayHeading)| < pi/2, where h is `plane->vtable[50h]()`.
  - It is true when |wrap(h - b)| < `00419010`(0, 30 deg, PosBehind x 1.25, 90 deg; |(e, y, z')| - 1.5 L).
- **`006C5C40`** (`006C5C40`-`006C5E18`): on the landing circle.
  - Null or landed answers false.
  - `side` = (x > 0) of `006BCC90(pos)`. P = `006C5380(plane, side)`, and d = |P - pos| in x and z.
  - The test is false when d > r x 1.6, with r = `006C3E50(plane)`.
  - Otherwise delta = |wrap(bearing(P - pos) - (side ? +pi/2 : -pi/2) - h)|.
  - It is true when delta < `00419010`(r x 0.25, 45 deg, r x 0.8, 20 deg; d).
  - The stack was traced from the listing; the pseudocode's argument order is wrong.
- **`006C5380`** (`__thiscall(holder)(out, plane, bool side)`, `RET 0Ch`): the circle point.
  - It is holder-local (T.x + (side ? r : -r), PosAlt + 20 n, -PosBehind - 60 n), through the
    holder's matrix at `+8h`, with n = `006BEF70(plane)`.
  - A LevelBomber (10h) or LargeReconPlane (16h) adds 250.0 to the `60 n` term.
- **`006C3E50`** (`__thiscall(block)(plane)`, returns ST0): r = f^2 x `007C6760(plane)`.
  - f = `00419010`(RadiusChange[1], 1.0, RadiusChange[2], 2.5; the record count at `+A8h`).
  - A null plane gives 500.0.
- **`007C6760`** (fastcall on the plane): class `+268h` TurnCircleRadius times m.
  - m is CircleMultiplierMax for a LevelBomber.
  - For a DiveBomber, TorpedoBomber or ReconPlane (12h, 11h, 14h), m = Max x 0.4 + Min x 0.6.
  - Otherwise m is CircleMultiplierMin.
- **`006C5E20`**, partial. It is false for a null plane. When `006C3B10` or `006C5C40` answers, it
  is false as well. Otherwise it applies a StandbyDist distance gate to T and a heading test against
  the circle point, using `00419010`(PosBehind, 80 deg, StandbyDist x 0.6, 30 deg; distance). The
  argument roles need the listing.

### The holder at block `+80h`

- **Builder `006C0750`**, reached from `006C0D20`, which allocates C0h bytes.
  - `+4h` is the block.
  - `+98h`..`+A0h` is the local offset. It is zero from `006D3C10`.
  - `+B0h` and `+B4h` are descriptor `+138h` and `+13Ch`.
  - Then it calls `006BEE40(0)` and `006BC960`.
  - `+8Ch` = 0, `+90h` = `+88h`, and `+94h` = 100.0.
- **`006BEE40`**: `+8h`..`+47h` is a copy of the owner's world matrix, at owner `+CCh`.
  - The offset is added to the translation row at `+38h`.
  - `0085DEA0` then builds the inverse at `+48h`.
  - `+88h` = owner `vtable[50h]()`, the runway heading.
- **`006BF0D0`** (from `006D3C10`): holder `+B0h` = RunwayWidth and `+B4h` = RunwayLength, both
  scene keys. Then it calls `006BC960`.
- **`006BC960`**: T = (`+A4h`, `+A8h`, `+ACh`).
  - On a MotherShip owner: (0, 0.5, -0.5 x RunwayLength + 10).
  - Otherwise: (max(0, (RunwayWidth - block `vtable[10h]()`) x 0.5), 0.5, -0.5 x RunwayLength + 35).
  - The block `vtable[10h]` is unread.
- **`006BCC90`** (`RET 8`): the holder-frame position relative to T, through `+48h`.
- **`006BCA40`** (`RET 4`): T in world, through `+8h`.

### Unread

- `006C6020` (`006C6020`-`006C64A4`) is read in part. It sets `+4h`, the path still to fly to T along
  the circle, and `+0Dh`, the side taken from the members' x sum when the mode is below 3.
  Its arc term goes through `00BF9940` with an x87 register argument, so the rest needs the
  listing.
- `006C3F80` (`006C3F80`-`006C45B7`, `__thiscall(block)(rec)`) is read from the pseudocode only.
  The argument roles below must be checked against the listing before binding. It is the spacing
  rule that sets `+8h`, which gates mode 4.
  - When the plane is not airborne, or `+0Ch` is set: `+8h` = 1.0.
  - Otherwise it walks every other airborne record with `+0Ch` clear and `+4h` > 0.
    - A mode-4 record (`own4`) considers only mode-4 records.
    - A wingman below mode 3 on a squadron with `+3B0h` clear considers only its own squadron.
    - Everyone else considers mode 3 and 4 records.
    - The head skips its own squadron's members.
  - For each considered record, g = `+4h`(other) - `+4h`(own). A record behind by more than
    `00E08E50` is ignored, and so is one within that band that loses the tie-break. The
    tie-break compares plane `+9D8h`, then the side `+0Dh`, then the pointers.
  - `00E08E50` is in `.data` and holds 70.0 in the image. Its only xref is this read.
  - For the rest it keeps the smallest `+4h` ahead and the smallest non-negative gap -g, and
    counts them (`k`).
  - With `k` = 0, `+8h` = 1.0 unless own is mode 4. For mode 4 it takes the launch-site object's
    time (`00F876A4` - [block `+3Ch`]`+40h`) and the flight time `+4h` / speed (`vtable[38h]`),
    against FollowDistTime (tuning `+504h`), through `00419010` with 0.75, 0.01, 0.25, 0.4 and
    the site's `vtable[30h]`.
  - With `k` > 0, the gap is turned into seconds (gap / speed) and scaled against
    FollowDistTime x 1.2 through `00419010`:
    - own mode 4 uses 0.7 / 1.1 x, 1.1;
    - otherwise it uses 0.75 / 1.25 x, 2.0;
    - either result is capped by `00419010`(-0.15, 0.01, 0.15, 2.0; the relative slack).
- The block `vtable[10h]`, the scene rows' RunwayWidth and RunwayLength for CB4_AF, and the
  host's substitute for the parent link `+3Ch` of an airborne plane. The proposed substitute is
  null.

### What the LOMP10 row should do once this is bound (from the read, not measured)

- Tuning values from this installation's `scripts/datatables/planeglobals.lua` (mtime 2024-10-29):
  StandbyDist 3200, PosBehind 780, PosAlt 170, ApproachAngle 12 deg, RadiusChange {10, 50},
  CircleMultiplierMin 1.1 and CircleMultiplierMax 1.05.
- **Who gets a record.** The Lightning 01 and Warhawk 01 squadrons circle CB4_AF inside
  3200 m. So on the first `006CD240` pass after their first request, every airborne member gets a
  record: the head at mode 3, the wingmen at mode 1. B-25 01 is refused before its request.
- **When the mode rises past 1.** The sequencer returns 0.0 after an insert, so the next pass is
  one tick later. That pass gives the head 2, 3 or 4:
  - 4 only inside the runway corridor with the deck usable;
  - 3 on the landing circle;
  - otherwise 2.
  Wingmen then take 3 behind a mode-3 head, 2 or 1 behind a mode-2 head, and 4 or 3 behind a
  mode-4 head. Their next request, paced by approach `+ACh`/`+B0h` at about 0.5 s, reads the mode.
- **The first landing state.** The flight leader enters `land/standby` (mode 2 or 3), or
  `land/begin` (mode 4) if it happens to be in the corridor. Wingmen enter `land/line` (mode 2) or
  `land/standby` (mode 3). All three are refused states in this host today, so a binding of the
  sequencer alone moves the refusal counters and leaves the flight paths as they are, unless the
  found arm of `006C54C0` changes approach fields that moveto (land) or follow (land) read.

## 6. Open, in order

1. **The landing sequencer `006CC9F0`** is read in section 5b; it is not bound. Two bodies are
   left before the binding can be exact: `006C6020` (the record's `+4h`, whose arc term needs the
   listing) and `006C3F80` (the record's `+8h`, which gates mode 4; read from the pseudocode,
   its argument roles unverified). The binding then needs the
   holder frame from the owner's pose, `RunwayWidth`/`RunwayLength` and block `vtable[10h]`. After
   that come `land/standby` and `land/line` for the row.
2. **B-25 01's approach bit.** `block+20h` bit 1 for a class 10h/16h head (`0047B850`). Until it is
   read, the B-25 squadron is refused and keeps bombing.
3. **The follow law on a circling leader** (the Lightning members above). This belongs to the
   code that owns `run_follow_law_009bfee0_009bee30`.
4. **`006C4790`'s list at block `+B4h`**: its producer is unread; it is taken as empty.
5. **GetLastCatapulted `00892860`** (cc9-lua7's item) is untouched.

## Coverage

| routine | coverage |
| --- | --- |
| `009B41C0`, `009B3240`, `009B2E50`, `009AFE70` | complete for the fields and draws; `009AFAF0` partial: its block-frame geometry `009AFB46`-`009AFDCC` unread |
| `009B3EB0` | complete |
| `009B3900` | complete except the `onGround` arm (`009AFAF0(1)`, `0099B650`), unreachable in mode 1 |
| `009B34D0`, `009B3560`, `009B3680`, `009AFA50`, `009B3750`, `009B3C60` | complete |
| `009B3770`, `009B3CF0` | complete as a read; bound for moveto and follow, refusing every other state |
| `006C54C0` | partial: the miss arm bound; `006C3E50` and `006C5380` read (section 5b); the frame sums unread |
| `006BD080`, `006C4790` | complete as a read; the vector and list producers unread |
| `006CD240`, `006CC9F0`, `006C7960`, `006C0B50`, `006BEF70`, `006C7540`, `006CAA10` | complete as a read (section 5b); not bound |
| `006C3B10`, `006C5C40`, `006C5380`, `006C3E50`, `007C6760`, `006C46B0`, `006C45C0`, `006BED60` | complete as a read; not bound |
| `006C0750`, `006BEE40`, `006BF0D0`, `006BC960`, `006BCC90`, `006BCA40`, `006BA620` | complete as a read; block `vtable[10h]` and `0085DEA0` unread |
| `006C5E20` | partial: gates read, the heading test's argument roles unread |
| `006C6020` | partial: `006C6038`-`006C60F0` and the tail stores read; the arc term (`00BF9940`) unread |
| `006C3F80` | partial: read from the pseudocode; the `00419010` argument roles and `00E08E50`'s width unverified against the listing |
| the six landing states' bodies | unread |

## ABI

- `009B41C0`: `__fastcall(ECX = bot, EDX = target)`, returns the task.
- `009B3240`: `__thiscall(this, bot, target)`, `RET 8`.
- `009B3EB0`: `__thiscall(this, float dt)`, `RET 4`.
- `009B3900`: `__thiscall(approach, bool onGround, float dt)`, `RET 8`.
- `006C54C0`: `__thiscall(ECX = approach+34h, plane, out)`, `RET 8`.
- `006BD080`: `__thiscall(block, plane, out)`, `RET 8`, returns AL.
- `006C4790`: `__thiscall(block, squadron)`, `RET 4`, returns AL.
- `009B3CF0` and `009B3680`: `__thiscall`, `RET 4`.
- `009B34D0`, `009B3770`, `009AFA50`, `009B3560`, `009B3C60`: `__thiscall`, `RET`.

## Uncertainty

- The state names are the registered strings. The routine names in the ledger are hypotheses.
- The land moveto shares the host's `009C18C0` substitutions: `009F9E40`'s bearing and `009BECD0`'s
  shaping.
