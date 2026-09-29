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

## 5c. The sequencer, finished and bound OFF (packet `cc9_landing_sequencer`, cc9-lua10, 2026-09-28)

Every body below was read from the Ghidra listing, and each `00419010` call was traced through its pushes.
Constants were read from the image on disk.

### Corrections to section 5b

- **`006C5E20` answers TRUE when `006C3B10` or `006C5C40` answers.** Both `JNZ` go to `006C6010`, which
  returns `AL = 1`. Section 5b said false. Otherwise the plane's horizontal distance to T above StandbyDist
  answers false. Inside it, the plane takes `side = (x >= 0)` of `006BCC90(pos)`, P = `006C5380(side)`, and
  delta = |wrap(bearing(P - pos) - h)|, h = `plane->vtable[50h]()`. The answer is
  delta < `00419010`(PosBehind, 80 deg `00CF8858`, StandbyDist x 0.6 `00CEFF98`, 30 deg `00CEC724`; |P - pos|).
- **`006C3F80` always counts a wingman's own head.** At `006C41A6`-`006C41B3`, a plane that is not its
  squadron's head, on a squadron with `+3B0h` clear, jumps to the count at `006C4234` whenever the other
  record's plane is its own head (`[ESP+3Ch]`), whatever the gap.
- **The tie-break** (`006C41CB`-`006C4217`, for |g| <= 70): when the two `+9D8h` differ, the record counts
  when own `+9D8h` > other `+9D8h` (`SETG`). When they are equal, it counts when the sides `+0Dh` differ and
  own `+0Dh` is set, or when the sides agree and own pointer < other pointer (`SETL`).
- **The block `vtable[10h]`** is `006CA5A0` in the airfield's block vtable `00CF8BA0` (the block is the
  embedded base at MAirfield `+72Ch`, installed by `006D1C20`). It returns 35.0 (`00CE4D90`). So an
  airfield's T is (max(0, (RunwayWidth - 35) x 0.5), 0.5, -0.5 RunwayLength + 35).
- **RunwayWidth/RunwayLength.** `006D3C10` first builds the holder from the class's `+138h`/`+13Ch`
  (`006D0B80` reads the class keys), then, for a scene record of kind 1, calls `006BF0D0` with the scene
  keys. `008F2260` returns null for an absent key and `006D3C10` dereferences it, so every kind-1 airfield
  authors both keys.
- **`0085DEA0`** is the rigid inverse: it copies the matrix, transposes the 3x3, and sets the translation to
  -(t through `0042D0D0` with the transposed rotation, no normalise).
- **`006C54C0`'s found test** (`006C550A`) is the copied word at out `+0h`, which `006BD080` fills from
  `plane+174h`, not its return byte. The found arm copies `rec+8h` to approach `+4Ch`, the mode to
  approach `+50h` and `rec+0Dh` to approach `+44h` (out `+0Ch`). `rec+4h` is not copied.

### `006C6020`, the record's path `+4h`, now read whole

- Plane `+904h` set: `+4h` = -1.0. Not airborne, or `+0Ch` set: `+4h` = 999999.0.
- The side `+0Dh`: for mode 3 or 4 it is (x >= 0) of `006BCC90(pos)`. Otherwise it is (sum > 0), where the
  sum runs over the x of every airborne member of the plane's squadron.
- D = |pos - T| in x and z, with T through the holder's matrix `+8h`. Mode 4: `+4h` = D.
- Otherwise, with r = `006C3E50(plane)`, P = `006C5380(side)` and dP = |pos - P| in x and z:
  - H1 = wrap(runwayHeading + (side ? -pi/2 : +pi/2)), constants `00CE3CCC`/`00CE3C64`;
  - B = bearing(pos - P) (`00414EB0`), a = acos(min(1, r / max(1, dP))) (`00415510`, then `00BF9940`);
  - `00BF9940` is `_CIacos`: the Lua binding `math_acos` (`00A617C0`) calls it straight after loading its
    argument, and the argument here is already clamped to [0, 1];
  - side set: B' = wrap(B + a), theta = wrap(H1 - B'); side clear: B' = wrap(B - a), theta = wrap(B' - H1);
  - when theta < -5 deg (`00CF885C`) and `006C3B10(plane)` is false, theta += 2 pi (`00CE3828`);
  - path = min(r, max(1, dP)) x theta + PosBehind (tuning `+4F0h`), plus |pos - Q| when max(1, dP) > r,
    where Q = P + r (`006BC0C0`(B'));
  - `+4h` = max(D, path).

### `006C3F80`, the spacing `+8h`, now read whole

- Not airborne, or `+0Ch` set: `+8h` = 1.0.
- The walk skips null, self, not airborne, `+0Ch` set, `+4h` <= 0, and, for a head, its own squadron.
  A mode-4 plane considers only mode-4 records. A wingman below mode 3 on a squadron with `+3B0h` clear
  considers only its own squadron. Everyone else considers mode 3 and 4 records.
- g = other `+4h` - own `+4h`. Counted: g < -70 (`00E08E50`, in `.data`, 70.0, its only xref is this
  read), the wingman's own head, or |g| <= 70 with the tie-break above. For each counted record it keeps
  the least `+4h` (A) and the gap G = max(0, min(G, -g)), and counts k.
- k = 0: `+8h` = 1.0, except for mode 4, which reads the launch-site object (below).
- k > 0: F = FollowDistTime (`+504h`) x 1.2 and t = G / speed (`plane->vtable[38h]`).
  - Mode 4: s = 0 when t < 0.4 F, else `00419010`(0.7 F, 0.01, 1.1 F, 1.1; t).
  - Otherwise: s = 0 when t < 0.5 F, else `00419010`(0.75 F, 0.01, 1.25 F, 2.0; t).
  - `+8h` = min(s, `00419010`(-0.15, 0.01, 0.15, 2.0; (own `+4h` - (A + speed x k x F)) / own `+4h`)).
- k = 0 and mode 4: `+8h` = `00419010`(0.75 FDT, 0.01, FDT, 1.0; site + own `+4h` / speed), where
  site = max(0, `00F876A4` - [block `+3Ch`]`+40h`). It is then zeroed unless own `+4h` / speed >= 0.25 FDT, or
  site >= 0.4 FDT and the site's `vtable[30h]` answers true. The launch-site object is not in this host, so
  **this arm is REFUSED**: `+8h` keeps its value and the refusal is counted.

### The binding, behind `kLandingSequencerBound` (committed OFF)

- The landing request `006C54C0` queues the plane's squadron at its deck (`006BF060`, `006C0B50`) and runs
  `006BD080` against the assignment vector. When the plane is found, it copies the mode, `+8h` and the side.
- The queue tick `006CD240` runs once per fixed step for every deck with a queue, from
  `GameScriptOrdersHost::run_air_ops_update_006cdc70`, straight after `006C0DA0`. That is its place in
  `006CDC70`'s order, after `006C77E0`, `006C64B0` and `006C6540`, none of which this host runs.
- The sequencer `006CC9F0`, `006C7960`, `006C6020`, `006C3F80`, `006C7540`, the predicates and the holder
  frame are bound for an **airfield owner only**. A mother-ship deck is refused and counted, because its
  holder is refreshed from the moving ship, and that refresh (`006BEE40`'s callers) is unread.
- LABELLED substitutions:
  - Airborne (`(plane+72Ch)->vtable[38h]`) is `+900h == 7`, as the rest of the land task reads it.
  - Plane `+904h` (landed) is clear: no plane lands in this host.
  - The record's `+0Ch` is never set: none of the routines read here writes it after the insert.
  - Squadron `+3B0h` is clear: every store to it is a clear (docs/CONTROLLED_UNIT.md).
  - The parent link `00923810(1)` of an airborne plane is null, so `006C3B10` never answers from it.
  - `+3D0h` is the registry's member list in array order without never-created entries; `+9D8h` is
    `member_spawn_index`.
  - The plane's `+174h` word is taken as non-zero, so a found record is always copied.
  - `006C0B50`'s tail (launch slots whose `+28h` is the squadron go to state 4) is counted, not applied:
    this host's slot squadron ids are not the registry's.

### Predictions for LOMP10 9000 and USN01 3200, written before any ON run

Measured on a pair of this tree's build, OFF against ON:
1. **Inserts.** Exactly one insert each for Lightning 01 and Warhawk 01. Each comes on the first queue pass
   after that head's horizontal distance to T drops to 3200 m or below. Warhawk 01 should insert between 50 s
   and 80 s, and Lightning 01 between 75 s and 105 s; both heads were near 3200 m from the airfield's centre
   at 64 s and 84 s in cc9-lua8's LOMP10 ON log. Every airborne member gets a record: the head at mode 3 and
   the wingmen at mode 1. B-25 01 gets none, because it is refused before its request.
2. **The next pass**, one tick later: each head's mode becomes 2, 3 or 4.
3. **First landing states.** Each head's next request makes the rule refuse `land/standby` (modes 2 or 3)
   or `land/begin` (mode 4). Each wingman's makes it refuse `land/line` (mode 2) or `land/standby` (mode 3),
   or it stays in `follow (land)` at mode 1.
4. **Flight paths do not move.** A refused state keeps the plane in `moveto (land)` or `follow (land)`, and no
   bound state reads approach `+44h` or `+4Ch`. So positions, deaths and the death table match OFF, and only
   the sequencer counters and the refusal counts move. The expected `pair_diff` exit is 1.
5. **No release.** Each head circles inside 3840 m (1.2 x StandbyDist) of T, so `006C45C0` never fires.
6. **USN01** has no land task, and should come out identical (exit 0, or 1 on known noise).

A mechanism failure is any of these: no insert, a head not at mode 3 on insert, a wingman not at mode 1, a
mode outside 1 to 4, or a moved flight path. Any of them keeps the switch OFF, and the result is recorded.

### The pairs and the verdict (cc9-lua10, 2026-09-28): ON

The OFF runs used a clean export of `cce28dd60` (`local\l10_base`). The ON runs used the same commit
with `kLandingSequencerBound=true` (`local\l10_ls`). Both used reference j's parameters.

| row | frames | `pair_diff` | inserts | releases | lookups / found | modes 1 / 2 / 3 / 4 | k=0 mode-4 refusals |
| --- | --- | --- | --- | --- | --- | --- | --- |
| LOMP10 | 9200/9000 | 1, gameplay identical | 2 | 0 | 7150 / 5932 | 1617 / 1267 / 512 / 12 | 12 |
| USN01 | 3200/3000 | 1, gameplay identical | no deck built | - | - | - | - |

1. **Inserts: held.** Warhawk 01 inserted at 70.80 s, with its head 3124.6 m from T. Lightning 01
   inserted at 87.80 s, at 3195.4 m. Each got 4 records, the head at mode 3 and the wingmen at
   mode 1. B-25 01 got none.
2. **The next pass: held.** One tick later (70.85 s and 87.85 s) both heads went to mode 2. Their
   wingmen took mode 2 behind them, then dropped back to 1 once they were outside StandbyDist.
3. **First landing states: held.** Every entry is refused: `refused_states` is 13995, keyed first
   at `009B3E6D` (mode 2). Warhawk 01's head also reached mode 3 (from 142.90 s) and mode 4
   (12 passes).
4. **Flight paths: held.** Deaths, hits, damage, shots and the unit table are identical. The
   native table adds `006CD240` and moves only the `006C0B50`/`006C5380` records, because the
   found arm returns before them. The rest is the known noise.
5. **No release: held.** `releases` is 0.
6. **USN01: held** (exit 1).

- **Refused and reached:** 006C3F80's k=0 mode-4 arm, which reads the launch-site object at block
  `+3Ch`. It ran 12 times on LOMP10, and each time `+8h` kept its prior value. No state reads mode
  4 yet: `land/begin` is refused. The arm must be read before `land/begin` is bound.
- **Seen in the run, not predicted:** the wingmen's spacing `+8h` is 0.0. A wingman far behind
  its head has t = G / speed below 0.5 F, so s = 0. It only gates mode 4.
- **Verdict: ON.** Every mechanism clause matched, and gameplay is identical on both rows.

## 5d. B-25 01's approach bit, block `+20h` (packet `cc9_landing_approach_bit`, cc9-lua10, 2026-09-28)

`006C0840` refuses a deck for a class 10h/16h head unless bit 1 of the dword at block `+20h` is set
(`006C09D2 MOV EAX,[EAX+20h]`, `SHR EAX,1`). The producer has been read:
- **`006CAC00`**, the block's base constructor, stores `[ESI+20h] = 3` at `006CAC3D`, setting bits 0
  and 1. Its two callers are the airfield constructor (`006D1C59`, `ECX = airfield+72Ch`, from
  `006D1C4C LEA EDI,[ESI+72Ch]`) and the mother-ship constructor (`00758589`).
- **`00758550`**, the mother-ship constructor, then stores 1 into `+11A8h`, which is its block
  (`+1188h`) plus 20h. So a carrier's block holds 1, with bit 1 clear, and refuses level bombers and
  large recon planes. An airfield's block keeps 3 and accepts them.
- **No other literal store** to `+20h` was found. A grep of the exported pseudocode of `006BA000`-
  `006D5000` for `+ 0x20) =` finds only `006D1550`'s float field, which is not the block. `006CADD0`
  reads `[ESI+20h]` twice but never writes it. A store through a pre-offset base register would not
  show in this search.

**Bound OFF**, `kLandingApproachBitBound`. `record_return_to_base_007f16d0` gives each candidate deck
bit 1 = (the owner is an airfield). The resolution no longer carries `approach-bit-20-unread`.
Before, it gave every deck the bit and flagged the resolution unread whenever the head needed it.

**Predictions for LOMP10 9200/9000 and USN01 3200/3000, written before the ON run:**
1. **B-25 01 installs `land` at CB4_AF** on its `returntobase` at 3.80 s, in place of the refusal.
   The head starts `moveto (land)`, the wingmen `follow (land)`. The squadron retires its
   `levelbomb`.
2. **The sequencer queues it** with n = 2, as the third squadron. It inserts records when the head
   comes within 3200 m of T. Its circle point adds 250 m to the `60 n` term.
3. **LOMP10 moves** (exit 3). B-25 01 no longer bombs, so any damage or deaths it caused OFF go
   away. USN01 has no class 10h/16h head returning to base, so it is identical (exit 0 or 1).
- A mechanism failure is B-25 01 still refused, or a install with no queue entry.

### The pair and the verdict (cc9-lua10, 2026-09-28): ON

OFF is this tree's build of `441c5cdef`, which carries the same code as `5abf6e808`
(`local\l10_b0_<row>.log`). ON is `local\l10_ab`, a flip of `5abf6e808` (`local\l10_abon_<row>.log`).

| row | `pair_diff` | deaths | damage | shots | first hit |
| --- | --- | --- | --- | --- | --- |
| LOMP10 9200/9000 | 3 | 3 -> 1 | 1304.4 -> 324.4 | 446 -> 4 | 90.50 s -> 306.55 s |
| USN01 3200/3000 | 1, gameplay identical | 5 -> 5 | - | - | - |

1. **Held.** B-25 01 and `|.-2` install `land` at CB4_AF at 3.80 s: `moveto (land)` and
   `follow (land)`, with no refusal.
2. **Held, with one detail missed.** The squadron is queued and inserts 2 records at 86.80 s, 3139 m
   from T. Its queue n is 0, not the predicted 2: B-25 01 requests first, so it queues first. That
   moves Lightning 01 and Warhawk 01 to n = 1 and 2, which shifts their circle points by 20 m up
   and 60 m back per n.
3. **Held.** LOMP10 moves. Both B-25s that died OFF (shot down on their bombing run) now survive.
   Hits, shots and damage collapse, because the AA fire at them is gone. USN01 is gameplay-identical.
- **Verdict: ON.**

## 5e. The first landing state, `land/standby` (packet `cc9_land_standby_state`, cc9-lua10, 2026-09-28)

State vtable `00D1FEF4` (task `+5B8h`, approach `+1C0h`). All three bodies were read whole from the
listing.

- **Enter `009B0230`** and **exit `009B0240`** (`__thiscall(state)`, `RET`):
  - enter: `+18h` (the done byte) = 0 and `+1Ch` = 0.0;
  - exit: `+18h` = 0.
  - The tick never sets the done byte, so standby is left only through the rule's mode arm.
- **Tick `009B0FE0`** (`009B0FE0`-`009B1243`, `__thiscall(state)(float dt)`, `RET 4`; dt is not read).
  With `a` = the approach (`[state+4]`):
  1. `d = 009FBB20(&(a+38h, a+40h), r = a+48h, side = a+44h, 0)` (`009B1019`). This is the circle
     steer the moveto circle state already runs: heading command `+2C0h` and mode 2 toward the
     landing circle, returning |dist - r|.
  2. `f = max(0, float(d - 100.0))` (double `00D7A220`).
     `h = max(float(float(DropAngle(class+1F0h) x f) x 0.5), a+60h)`.
     `A = float(h + a+3Ch)`.
  3. The target altitude is `max(min(A, posY), a+3Ch)` (`009B10C1`-`009B10FB`). The plane only
     ever descends, and never below the circle point's height.
  4. `k = (posY - target) / (65.0` (`00D1FF10`), or `40.0` (`00CE685C`) for class 10h/16h`)`,
     clamped to [0.8, 1.6] (`00CE3D40`/`00CE3D48`; the stored values are `00CE74F8`/`00D06BB4`).
     `009FB800(target, k)` then commands the pitch (`009B11BF`).
  5. `+2B4h = lvl + a+4Ch x (class+190h - lvl)`, with lvl = `007C47F0`, class+190h =
     TravelSpeed x NewTravelSpeedMul, and a+4Ch = the record's spacing `+8h`. Also `+2B0h` = 0 and
     `+2D8h` = 1.
  6. Direction `+40h` = tuning `+66Ch`, then `009FABE0(009A1A20(0099B630()))`. This is the direction
     object this host does not model; it is a record, as in the moveto states.
- **The inputs come from `006C54C0`'s common tail** (`006C55FF`-`006C562A`), run on both arms:
  approach `+48h` = `006C3E50(plane)` (r), and `+38h..+40h` = `006C5380(side = a+44h)` (the circle
  point). On the miss arm, `+44h` is (sum of the airborne members' x in the block frame >= 0.0)
  (`006C55DC`-`006C55FC`). Both are bound with this state.
- **The rule from standby** (`009B3CF0`), bound together with it:
  - `009B3770` does nothing, since the done byte is never set.
  - `+900h` 4 or 5 sends the plane to park, which stays refused.
  - The mode arm: mode 1 goes to `009AFA50` (leader `moveto (land)`, wing `follow (land)`); mode 2
    goes to standby for the flight leader and to `land/line` (refused) otherwise; mode 3 goes to
    standby; mode 4 goes to `land/begin` (refused).
  - A refused transition leaves the plane in its current state.

**Bound OFF**, `kLandStandbyStateBound` (`GameUnitsHost::Impl`). It needs `kLandingSequencerBound`.

**Predictions for LOMP10 9200/9000 and USN01 3200/3000, against this tree's build with the switch
off. The OFF sequencer history comes from `local\l10_h0_lomp10.log`.**
1. **Entries.** The flight leaders enter `land/standby` at their first request after their head's
   mode 2: Warhawk 01 just after 70.85 s, Lightning 01 just after 72.85 s and B-25 01 just after
   84.85 s, each within 0.5 s. Up to 70.85 s the pair is identical.
2. **Wingmen.** They stay in `follow (land)` while their mode is 1 or 2 (`land/line` is refused).
   They enter standby when their mode reaches 3. OFF, that first happens to Warhawk 01's wingmen at
   120.9 s; ON the time will move with the heads' new paths.
3. **Flight in standby.** A head flies the landing circle: its distance to its circle point
   settles toward r, with f = 1.0 for 10 records or fewer, so r = TurnCircleRadius x
   CircleMultiplierMin 1.1. It descends toward the circle point's height: the airfield's frame
   height plus PosAlt 170 m plus 20 m per queue index (`006C5380` builds the local y without T). Its commanded speed is TravelSpeed x NewTravelSpeedMul at spacing 1.0.
4. **LOMP10 moves** (exit 3). USN01 has no land task and comes out identical (exit 0 or 1).
5. `land/line`, `land/begin`, `land/park` and `land/abort` stay refused and counted.
- A mechanism failure is any of: no standby entry, a head whose circle distance diverges, or a
  state entered other than standby, moveto or follow.

### The pair and the verdict (cc9-lua10, 2026-09-28): ON

OFF is this tree's build of `ac4f1d6fe` (`local\l10_s0_<row>.log`). ON is `local\l10_sb`, the same
commit with `kLandStandbyStateBound=true` (`local\l10_sbon_<row>.log`).

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | deaths identical (1); paths moved, B-25 01's travel 6691 -> 5945 m |
| USN01 3200/3000 | 1, gameplay identical | - |

1. **Entries: held.** Warhawk 01 entered at 70.90 s, Lightning 01 at 73.30 s and B-25 01 at
   85.30 s. Each came at its first request after its head's mode 2.
2. **Wingmen: held.** Every wingman's `land/line` (mode 2) is refused. They enter standby at mode 3,
   the first being Warhawk 01's `|.-4` at 92.20 s (120.9 s OFF).
3. **Flight: held.**

   | head | r | settled circle distance | circle-point height | height reached |
   | --- | --- | --- | --- | --- |
   | Warhawk 01 | 1100 | 1070 to 1082 | 259.9 | 258 |
   | Lightning 01 | 1100 | 1070 to 1082 | 239.9 | 238 |
   | B-25 01 | 1890 (TurnCircleRadius x 1.05, a level bomber) | about 1858 | 219.9 | 218 |

   - The three circle points sit at queue n = 2, 1 and 0.
   - The commanded speed follows the spacing `+8h`, from 32 to 159 m/s.
   - B-25 01 left and re-entered standby 11 times as its mode fell to 1 and came back.
4. **Held.** LOMP10 moves; USN01 does not.
5. **Held.** `land/line` and the rest stay refused.
- **Verdict: ON.** `land/line` (wing members at mode 2) is the next state.

## 5f. `land/line`, read in part (packet `cc9_land_line_state`, cc9-lua10, 2026-09-28)

State vtable `00D1FEA4` (task `+598h`, approach `+1A0h`). Nothing is bound yet; the rule still
refuses the entry at `009B3E6D` (mode 2, not the flight leader).

- **Enter `009B02E0`**: `+18h` = 0 and `+1Ch` = 0.0. **Exit `009B02F0`**: `+18h` = 0. Both match
  standby's pair.
- **Tick `009B0300`** (`009B0300`-`009B08F5`, `RET 4`, then INT3). Ghidra has **no function**
  here: read it with `python tools/bsp.py disasm-raw 009B0300 --length 3000`, whose line 398 is the
  `RET 4`. Its calls are:
  - `00414DB0` x6, `00BF701A` (`_CIatan2`) x2, `00BF7030` (`sqrt`);
  - `00438B10`, `00438AA0`, `00419010`, `007F0280` (unread), `009FB800`, `007C47F0`;
  - `0042E740`, `0099B630`, `009FABE0`.
- **Read so far** (`009B0300`-`009B04D9`). With `a` = the approach, `p` = the plane (`a+4`) and
  `H` = the squadron's head (`[a+0Ch]+3D0h`):
  - v = H.pos - p.pos, in x and z;
  - m = wrap0(pi/2 - H->vtable[50h]()), where wrap0 adds 2pi once when the value is negative;
  - Q = H.pos + 100.0 (`00D7A220`) x (cos m, sin m): a point 100 m ahead of the head along its
    heading;
  - bearings through `_CIatan2` in the heading convention, pi/2 - atan2(z, x), wrapped to [0, 2pi):
    b1 to the head, b2 to Q;
  - delta = |wrap(b1 - p->vtable[50h]())|, stored at `[ESP+14h]`.
- **Unread:** `009B04D9`-`009B08F5`, which covers the `00419010` term, `007F0280`, the heading add
  `00438AA0`, the pitch (`009FB800`), the speed (`007C47F0`) and the direction (`009FABE0`).
- **What a pair needs:** LOMP10's wing members reach mode 2 from about 73 s (5e).

## 5g. `land/line`, read whole and bound OFF (packet `cc9_land_line_state`, cc9-lua11, 2026-09-28)

This completes section 5f. The tick `009B0300`-`009B08F5` was read whole from
`python tools/bsp.py disasm-raw 009B0300 --length 1530` (`RET 4` at `009B08F2`, INT3 from
`009B08F5`). The stack frame is `SUB ESP,58h` plus three pushes, and dt at `[ESP+68h]` is never
read. `a` = the approach (`[state+4]`), `p` = the plane (`[a+4]`), and `H` = `[[a+0Ch]+3D0h]`, the
squadron's member 0. The rule sends that member to standby at mode 2 (`009B3E6D`), so `H` is the
flight leader and never the plane itself.

### The tick, in order

1. **Geometry** (`009B0338`-`009B048A`, section 5f): `v = H - p` in x and z; `Q` = 100 m
   (`00D7A220`) ahead of `H` along its heading; `b1` = the bearing to `H`, `b2` = the bearing to `Q`,
   each `pi/2 - _CIatan2(z, x)` wrapped once into [0, 2pi) (`00CE3830`, `00CE3828`).
2. **Offset and distance** (`009B0496`-`009B0521`): `delta = |00438B10(b1, p->vtable[50h]())|`
   (the abs is `-0.0 - x`, `00D7A208`). `dist = sqrt(vx^2 + vz^2)`, or 0 when the square is at most
   1e-10 (`00CE3820`).
3. **The threshold** (`009B0527`-`009B058C`, `00419010`, RET 14h): below 80 deg (`00CF8858`) it is
   `interp(30 deg, 40, 80 deg, 100, delta)` (`00CEC724`, `00CE685C`, `00CE3D08`); otherwise
   `interp(80 deg, 100, 150 deg, 500, delta)` (`00D1FED0`, `00CE397C`). So it is 40 m dead astern of
   the head's bearing line, 100 m abeam, and 500 m from 150 deg.
4. **Far or near** (`009B0591`-`009B05CE`): `dist > thr` gives heading `b2` and gain 1.0
   (`00D7A24C`). Otherwise the heading is `H->vtable[50h]()` and the gain is 0.01 (`00D7A238`).
5. **The squadron probe** (`009B05D6`-`009B06D9`): `007F0280` in mode 0 with `ECX = [a+0Ch]`,
   the unit `p`, extents (70, 50, 80) (`00CE77B4`, `00CEB4D4`, `00CE5444`), weights 0, `RET 18h`.
   When `|out_a.x| > 0.05` (`00D7A270`), `t = -out_a.x * out_b.y * out_b.z` gets a dead zone of
   0.03 rescaled by 0.97 (`00CEB690`, `00D1FEC0`, `00D1FEC8`). The heading becomes
   `00438AA0(heading, t * pi/3)` (`00D03DD0`). This is the attack run's product form
   (docs/ATTACKRUN_SQUADRON_PROBE.md) with a smaller box.
6. **Heading write** (`009B06DD`-`009B06F0`): `[a+18h]+2C0h` = the heading, `+2CCh` = 2.
7. **Altitude band** (`009B070A`-`009B0815`): `floor = a+60h + a+3Ch`,
   `upper = H.y + 10 + min(0.06 dist, 50)` and `lower = max(floor, H.y) - 10 - min(dist/4, 50)`
   (`00CF0AC0`, `00CE3938`, `00CE3DC0`, `00D7A348`). The target is `lower` when `p.y < lower`,
   else `upper` when `p.y > upper`, else `p.y`. Then `009FB800(target, 1.0)` (`009B082D`).
8. **Speed** (`009B0832`-`009B08B6`): `+2B4h = 0.9 lvl + min(a+4Ch, gain) x (class+190h - 0.9 lvl)`,
   with `lvl = 007C47F0` and 0.9 from `00D7A390`; also `+2B0h = 0` and `+2D8h = 1`.
9. **Direction** (`009B08BC`-`009B08E7`): `[a+1Ch]+40h` = tuning `+66Ch`, then
   `009FABE0(heading, 0099B630())`. No `009A1A20` here, unlike standby. It is recorded, not
   modelled, as in standby.

In short, a line wingman chases a point 100 m ahead of its leader, or holds the leader's heading
once inside the threshold, keeps within a height band around the leader, and yields to its
squadron mates inside a 70 x 50 x 80 m box.

### The rule from line (`009B3CF0`, `009B3770`)

- `009B3770` treats line like standby: it goes to `009AFA50` only when the done byte `+18h` is set.
  The line tick never writes `+18h`, so nothing happens.
- The rule then applies the same mode arm as for moveto, follow and standby. Mode 1 goes to
  `009AFA50`, mode 2 goes to standby for the leader and to line otherwise (`009B3E81`), mode 3 goes
  to standby, and mode 4 goes to `land/begin` (still refused).

### The binding, behind `kLandLineStateBound` (committed OFF)

- `land_enter_line_009b02e0`, `run_land_line_tick_009b0300` and the rule's line arm, in
  src/game_hosts_units.cpp. It needs `kLandStandbyStateBound`.
- The probe is the host's `nf_probe_squadron_007f0280` (`kNearFieldProbeBound`, ON), with that
  binding's reach prefilter.
- A new trace line, `land line trace`, prints every 10 line ticks (1 s at the 10 Hz think). The `summary landing plane` line
  gains `line entries=`, `ticks=` and `last_head_d=`.
- The x87 order is kept: every float store in the listing is a float cast in the host.

### Predictions for LOMP10 9200/9000 and USN01 3200/3000, written before any ON run

OFF is this tree's build with the switch off. The OFF history quoted here comes from cc9-lua10's
standby-ON log `local\l10_sbon_lomp10.log`, which is the same behaviour.

1. **Entries.** Each wingman enters `land/line` at its first request with mode 2. The first is
   Warhawk 01|.-4 at 72.90 to 73.20 s. Up to that point the pair is identical. Later entries move,
   because the line path feeds the sequencer's per-member modes. OFF, the next are Lightning 01|.-4
   at about 75 s, Warhawk 01|.-2 and |.-3 at about 77 s, and Lightning 01|.-2 and |.-3 at about 80 s.
   B-25 01|.-2 first reaches mode 2 at 146.0 s OFF.
2. **Leaving line.** OFF, the wingmen's modes flip between 1 and 2 every 1 to 5 s. ON, each flip
   to mode 1 sends the wingman back to `follow (land)`, and each return to mode 2 re-enters line.
   Mode 3 sends it to standby, which OFF happens at 92.2 to 94.4 s.
3. **Flight in line.** The wingmen carry spacing 0 at mode 2, so they fly at 0.9 x LevelFlight x
   StallSpd, slower than their leader, which flies at its spacing 1.0 speed.
   - The head distance stays bounded in the tens to low hundreds of metres. It is mostly `far=1`,
     chasing the point ahead of the leader.
   - The wingman's height stays inside the band around the leader's height.
4. **LOMP10 moves** (exit 3). USN01 has no land task and comes out identical (exit 0 or 1).
5. `land/begin`, `land/park` and `land/abort` stay refused and counted.
- A mechanism failure is any of: no line entry; a flight leader in line; a wingman whose head
  distance grows without bound while in line; or a height outside the band for more than a few
  seconds after entry.

## 6. Open, in order

1. **The landing states the row now enters.** The sequencer is bound and ON (section 5c). LOMP10's
   heads reach modes 2, 3 and 4, and the wingmen reach modes 1 and 2. So `land/standby` and
   `land/line` come next, then `land/begin`, which also needs `006C3F80`'s launch-site arm (block
   `+3Ch`, `+40h` and `vtable[30h]`). A mother-ship holder, refreshed from the moving ship, is
   still refused.
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
| `006CD240`, `006CC9F0`, `006C7960`, `006C0B50`, `006BEF70`, `006C7540`, `006CAA10` | complete; bound (section 5c); the `006C0B50` slot tail is counted, not applied |
| `006C3B10`, `006C5C40`, `006C5380`, `006C3E50`, `007C6760`, `006C46B0`, `006C45C0`, `006BED60` | complete; bound |
| `006C0750`, `006BEE40`, `006BF0D0`, `006BC960`, `006BCC90`, `006BCA40`, `006BA620`, `0085DEA0`, `006CA5A0` | complete; bound for an airfield owner |
| `006C5E20` | complete (section 5c); bound |
| `006C6020` | complete (section 5c); bound |
| `006C3F80` | complete (section 5c); bound except the k=0 mode-4 arm (launch-site object), refused |
| `009B0230`, `009B0240`, `009B0FE0` (standby); `009B02E0`, `009B02F0`, `009B0300` (line) | complete; bound (sections 5e, 5g) |
| the begin, final, park and abort states' bodies | unread |

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
