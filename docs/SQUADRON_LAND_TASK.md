# The squadron's `land` task (packet `cc9_land_task_reach`)

Addresses: 009B41C0 009B3240 009B2E50 009AFE70 009AF9A0 009AFA50 009B3EB0 009B3900 009B34D0
009B3560 009B3680 009B3770 009B3CF0 009B3C60 009B3750 006C54C0 006C4790 006BD080 006C0B50 006CD240
006CC9F0 006C7960 006C3B10 006C5C40 006C5380 006C3E50 006C46B0 006C45C0 006BED60 007C6760 006C7540
006BEF70 006C0750 006BEE40 006BC960 006BF0D0 006BA620
0099A3DD 007C07A0 007D83D0 007D88CB 009B1C79 009B0A3B 0074E210 007DC6C5

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

### The pairs and the verdict (cc9-lua11, 2026-09-28): ON, with two outcomes missed

OFF is this tree's build of `8a1936324` (`local\l11_off2_<row>.log`). ON is `local\l11_ln`, the same
commit with `kLandLineStateBound=true` (`local\l11_lnon2_<row>.log`). A first pair on `9cdf5ee0e`
(`l11_off_`, `l11_lnon_`, trace every 200 ticks) matches it: each side against its re-run gives
`pair_diff` 1.

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | unit table and deaths identical; B-25 01's travel 5945 -> 6207 m |
| USN01 3200/3000 | 1, gameplay identical | - |

1. **Entries: held.** There were 16 entries, all by wingmen. Each wingman first entered at the time
   of its OFF refusal:

   | wingman | first entry |
   | --- | --- |
   | Warhawk 01\|.-4 | 73.20 s |
   | Lightning 01\|.-4 | 75.40 s |
   | Warhawk 01\|.-2 and \|.-3 | 77.40 s |
   | Lightning 01\|.-2 | 80.10 s |
   | Lightning 01\|.-3 | 80.30 s |
   | B-25 01\|.-2 | 146.00 s |

2. **Leaving line: held.** Each flip to mode 1 returned the wingman to `follow (land)`, and each
   return to mode 2 re-entered line (two or three entries each). Mode 3 sent Warhawk 01|.-4 to
   standby at 92.20 s, as before.
3. **Flight in line: the mechanism held, and two outcomes differ from the prediction.** The traces
   reproduce the listing's values:
   - At d = 187.3 m the band is (1055.8, 1133.8). That is `H.y + 10 + 0.06 d` and
     `H.y - 10 - d/4`, with `H.y` = 1112.6.
   - At delta = 0.533 rad the threshold is 40.6 m, which is `interp(30 deg, 40, 80 deg, 100)`.
   - The commanded speed is 28.80 m/s (0.9 x 32.0) at spacing 0, and 29.80 m/s at spacing 0.010.
     That gives class+190h = 128.8.
   - B-25 01|.-2 ran the near arm (`far=0`: 53.8 m, inside its 66.4 m threshold) and held its
     head's heading.
   - At 148 s its heading came back wrapped to (-pi, pi], which is the probe's `00438AA0` add.

   The two outcomes that differ:
   - **Head distance.** It closed in the first stint: 187 -> 128 m for Warhawk 01|.-4 and
     225 -> 142 m for Lightning 01|.-4. In later stints it opened, to 425 m and 595 m (Warhawk
     01|.-4 and |.-2 at 88 s). The prediction was "tens to low hundreds".
     - The wingmen fly at 29 m/s because their spacing `+8h` is 0: the sequencer is holding them
       back from the plane ahead (section 5c).
     - The distance stays bounded because every stint ends at a mode flip after 1 to 6 s.
   - **Height.** Every Warhawk and Lightning wingman stayed 110 to 150 m above the band's upper
     edge for its whole stint. This literally meets the height clause written above. **Reclassified
     on review (the lead, 2026-09-28): a spread miss of the vehicle response, not a mechanism
     failure.**
     - The target was the upper edge every time.
     - The band fell with the head, at 100 m/s at first and then 20 to 27 m/s. The wingman
       descended at 21 to 46 m/s.
     - The clause was meant to catch a wrong target or band, and both are correct in every trace.
       The miss happens because the wingman enters 100 m above a head that descends as fast as it
       can.
4. **Held.** LOMP10 moves and USN01 does not.
5. **Held.** `land/begin` (mode 4, from 132.9 s) and the rest stay refused.
- **Verdict: ON.**
  - The entries, the rule arm, and the tick's threshold, band, speed and probe values all match the
    listing.
  - The two missed outcomes follow from the sequencer's spacing and from the head's descent.
  - The height clause was met literally. On review it is reclassified as a spread miss of the vehicle
    response. The state computes the right target, band and speed. The gap is the airframe's descent
    rate under the pitch command `009FB800` and the follow speed law, which is the plane flight
    controller's open item (docs/PLANE_FOLLOW_LAW.md; `pitch_command_009fb800` and the unit+AB0h
    controller). The switch stays ON.

## 5h. `land/begin`, read and bound OFF (packet `cc9_land_begin_state`, cc9-lua11, 2026-09-28)

State vtable `00D1FF14` (task `+5D8h`): enter `009B13E0`, exit `009B0240` (standby's), tick
`009B1D70`. Every body below was read whole from the raw listing. Ghidra's decompile of `009B1420`
drops the x87 glide block.

### The rule and the approach update in begin

- **`009B3EB0`** passes `onGround` = 1 to `009B3900` while the state is begin, final, park or
  abort. So in begin the approach update runs `009AFAF0(1)` and `0099B650(owner)` every tick,
  before the rule. `0099B650` stores the owner pointer at `[approach+18h]+25Ch` and is recorded,
  not modelled.
- **`009B3770`**: begin with the done byte `+18h` set goes to `land/abort`.
- **`009B3CF0`'s begin arm** (`009B3D45`-`009B3DAF`): mode not 4 goes to abort. Mode 4 with
  `009B3C00` true goes to `land/final` (`+5F8h`).
- **The mode arm** sends mode 4 from moveto, follow, line or standby to begin (`009B3E9F`).
- **`009B3C00`** (`009B3C00`-`009B3C56`): true when `A8 < a+24h x PosBehind x 0.4` (`00CE65D0`), or
  when `(plane+72Ch)->vtable[38h]` is false (this host's control mode not 7).
  - `a+24h` is `009F9CE0`'s `max(1, MaxSpd / Pilot/Landing/ReferenceSpeed)`.

### `009AFAF0`, the geometry (`009AFAF0`-`009AFE65`, `__thiscall(approach, bool onGround)`, `RET 4`)

- `+68h..+70h` = the plane position. `006BCA80(&a+98h, t = 0)` writes the path point P at
  `+98h..+A0h` and its heading at `+A4h`.
- `006BCC90` puts the plane in the runway frame less T.
  - When the local z is above 0 (past T): `+8Ch` = x, `+90h` = y, `+94h` = `-max(z, 1)`,
    `A8` = 1.0, and it returns.
- Otherwise `d = P - p`, and `A8 = sqrt(dx^2 + dz^2)`, or 0 at 1e-10 or below.
- **onGround arm** (`009AFC58`-`009AFDC4`), unless plane `+BF8h` is set and `+BF4h` is non-zero
  (that gate is not modelled):
  - A lead time `t = min((A8 + min(0.5 TurnCircleRadius |dh|, 0.18 A8)) / speed, 10)` (`00D7A280`,
    `00D05AA0`, `00CE38B8`).
  - Then `P = 006BCA80(t)` and `A8 = max(len2d(P - p), 1.0)` (`00414C60`).
- **The tail** (`009AFDCC`-`009AFE59`): `+90h = -dy`, the height above P; `b` = the bearing to P;
  `e = 00438B10(A4, b)`; `+8Ch = A8 sin e` (lateral) and `+94h = A8 cos e` (along track).
- **`006BCA80`** (`006BCA80`-`006BCC8A`, `ECX` = holder, `out[4]`, `float t`, `RET 8`):
  - The point is T (holder `+A4h..+ACh`) turned by `holder+8Ch x t` about the holder origin,
    through the holder frame `+8h`, plus `owner->vtable[34h]` velocity x t.
  - `out[3] = 00438AA0(holder+88h, holder+8Ch x t)`.
  - It also clamps holder `+ACh` to at most -20.0 (`00CF180C`), after taking its copy.
  - `holder+8Ch` is written only at construction, as 0.0 (`006C07F5`, `006CACAF`). A sweep of
    `006B9000`-`006CE000` finds no other store. The mother-ship refresh stays refused.
  - So for the airfield holder, P is T in world at any t, and the heading is wrap(`+88h`).
  - The owner velocity is taken as 0 for the static airfield (labelled). The lead time therefore
    changes nothing except the 1.0 floor.

### The tick `009B1D70` and its steer `009B1420`

`009B1D70` (`009B1D70`-`009B1DE3`, `RET 4`) runs these steps:
1. `approach+B4h` = 2 (not modelled).
2. `009B1420(dt)`.
3. `+2B4h = mc + a+4Ch x (a+5Ch - mc)`, with `mc` = `007C4810`, the minimum control speed, and
   `a+5Ch` = 0.75 TravelSpeed.
4. `+2B0h` = 1, `+2D8h` = 1, and `[a+1Ch]+40h` = 0.

`009B1420` (`009B1420`-`009B1D60`, `RET 4`, dt unread) copies tuning `+4E0h..+52Fh` and computes:

1. **Slopes.** `T1 = tan(min(DropAngle/2, ApproachAngle))` and `T2 = tan(0.6 ApproachAngle)`, both
   through `00412E20`.
2. **The lateral gain.**
   - `c1 = interp(80, 0.8, 260, 0, A8)`, softened below 0.8 to `(1 - (0.8 - c1) x 0.3) x c1`.
   - `c2 = interp(5, 0.01, 10, 1, W) x c1`. W is `[unit+DF4h]+9Ch+[00F876B8]*1Ch`, the pilot bot's
     lifetime in seconds: `BSP_PilotBot_Tick` copies `bot+80h` there at `0099B198`, and
     docs/PILOT_BOT_TICK_GATES.md shows the `bot+80h` accumulator.
3. **The lateral correction** (`009B15C0`-`009B179A`):
   - `k = max(A8 tan 16 deg, 4) x min(c2/0.64, 1)`.
   - X is moved toward 0 by k to give xn, stopping at 0.
   - When `xn != 0`: `corr = 1.25 asin(min(|xn| / den, 0.95))`, negated for `xn > 0`, where
     `den = (1.1 - 0.5 interp(ApproachDist, 1, 0.8 PosBehind, 0, A8/a+24h)) x class+26Ch`.
4. **Heading** (`009B17BB`): `+2C0h = 00438AA0(A4, corr)`, mode 2. `+2C8h`, the bank limit, is
   `interp(60, 0.1, 150, 1.5, A8)`.
5. **The glide height** (`009B1842`-`009B1929`), with Z the along-track distance:
   - `m = max(Z/2, 100)` and `zm = Z - m`.
   - `H = 0.5 + zm T2`. When `zm > 100`, `H = 0.5 + 100 T2 + (zm - 100) T1` instead.
   - When `Z > 20` and `G = (Z - 10) T2` is above Y: `H += (G - Y) m / max(Z/4, 20)`.
6. **Pitch.**
   - `P = -atan2(Y - H, m)`.
   - The done byte is set when `P < -max(1.8 DropAngle, 1.0)`.
   - It is also set when `Z < ApproachDist` and `Y - 0.5 > (0.4 RunwayLength x 1.4 + Z) T1 + 2`
     (`006BA620`).
7. **The pitch arm.**
   - When `speed <= lvl x interp(3, 1.1, 6, 2.5, W)`, `Z <= 3 speed` and `Y <= 1.4 class+A0h`
     (Length): `+2BCh` = ApproachPitch with `+2D0h` = 1.
   - Otherwise `+2BCh` = P clamped to [-DropAngle, ClimbAngle] with `+2D0h` = 2.
8. **The hold.** When `c2 > 0`, `007C07A0(plane, (cos, tan P, sin), c2)` arms `007D83D0`'s timed
   direction hold on `unit+AB0h`, with the strength scaled by heading misalignment
   (`interp(15 deg, 1, 60 deg, 0.4)`). This host has no consumer for that hold
   (docs/PLANE_DYN_TIMED_HOLD.md 6), so the call is counted.
9. **Dead code.** The speed block `009B1C7E`-`009B1D52` is a dead store: `009B1D70` rewrites `+2B4h`.
   It uses `009B1300` (`009B1300`-`009B13D9`, `RET`, no stack arguments), which returns
   `max(lvl + owner velocity along the plane's heading, mc)`.

**`class+26Ch`** is `007DB4D0` (`007DB4D0`-`007DB62F`, `RET`), called from
`BSP_PlaneClass_DeriveFlightConstants` at `007C4C1A`.
- `rate = TurnRollSpd sR + c cR + PitchSpd min(e, 0.6) sR`, stored at `+270h`.
  - `sR, cR` = sin, cos of TurnRollLeader (`+260h`).
  - `c = (SlideRatio sR + 0.2 interp(YawTurnRollRange1, 1, YawTurnRollRange2, 0, TurnRollLeader)) x YawSpd`.
  - `e = sR c / (cR PitchSpd)`.
- `class+26Ch = TravelSpeed / rate`.
- When e is above 0.6, the image also rewrites `+260h` as `_CIatan(0.6 PitchSpd / c)` (`00BF8490`).
  The host does not keep that write.
- TurnRollLeader is now loaded (`desc+260h`, `007D28D4`).

### The binding, behind `kLandBeginStateBound` (committed OFF)

- In src/game_hosts_units.cpp:
  - `land_geometry_009afaf0` and `land_path_point_006bca80`;
  - `land_enter_begin_009b13e0`, `land_begin_steer_009b1420` and `run_land_begin_tick_009b1d70`;
  - `land_begin_done_009b3c00` and `class_turn_radius_26c_007db4d0`;
  - the rule's begin and mode-4 arms, and the approach update's onGround arm.
- `land/final` and `land/abort` stay refused and counted. A refused transition leaves the plane in
  begin, flying the begin tick.
- **Substitutions, labelled:**
  - **W** is taken as saturated (at least 10 s). A land task younger than 10 s is counted
    (`young_bot=`).
  - The **speed getter** `vtable[38h]` is the live |v|, as in the follow law.
  - The **owner velocity** is 0 for the static airfield.
- A trace line, `land begin trace`, prints every 10 begin ticks. The `summary landing plane` line
  gains `begin entries=`, `ticks=`, `abort_refused=`, `final_refused=`, `hold_unapplied=`,
  `young_bot=` and `last_a8=`.

### Predictions for LOMP10 9200/9000 and USN01 3200/3000, written before any ON run

The authored tuning comes from this installation's scripts/datatables/planeglobals.lua (mtime
2024-10-29): ApproachDist 210, ApproachPitch 6 deg, ApproachAngle 12 deg, ReferenceSpeed
KMH(140), PosBehind 780.

1. **Entries.** Warhawk 01 enters begin at its first request after its mode 4, at 132.90 s (its OFF
   refusal). The pair is identical up to then. Lightning 01 (141.8 s OFF) and B-25 01 (226.3 s OFF)
   follow, possibly moved by the sequencer's coupling.
2. **Final at once.** On entry A8 is about 800 m. The final threshold `312 x a+24h` is above 1 km for
   every head, because a+24h is at least 1. So `009B3C00` answers true from the first rule tick,
   and `land/final` is refused on every begin tick. The plane stays in begin.
3. **Flight in begin.**
   - The heading command converges on the runway heading 0.2618 as the lateral offset X is taken
     out.
   - The bank limit falls from 1.5 rad to 0.1 as A8 closes from 150 m to 60 m.
   - The commanded speed is 0.75 TravelSpeed at spacing 1.
   - The pitch runs in mode 2 while the plane is fast or far (`Z > 3 speed`), clamped to
     [-DropAngle, ClimbAngle]. The height falls toward the glide height H.
   - A8 falls steadily through the first 10 s in begin.
4. **The done byte** is likely set inside 210 m of T. The heads reach begin from their standby
   circle about 200 m above T, and the done test there allows only about 50 m. `land/abort` is then
   refused and counted.
5. **LOMP10 moves** (exit 3). USN01 has no land task and comes out identical (exit 0 or 1).
- A mechanism failure is any of:
  - no begin entry;
  - a heading command that does not settle toward the runway heading within 10 s;
  - a mode-2 pitch outside [-DropAngle, ClimbAngle];
  - A8 that does not fall during the first 10 s in begin.


### The pairs and the verdict (cc9-lua11, 2026-09-28): OFF until `land/final` is bound

OFF is this tree's build of `8abe8cee0` (`local\l11_bgoff_<row>.log`). ON is `local\l11_bg`, the same
commit with `kLandBeginStateBound=true` (`local\l11_bgon_<row>.log`). A 300-frame USN01 smoke of
the ON build ran clean.

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | deaths identical (0); eight IJN ships gain an engagement range; B-25 01's travel 6207 -> 9107 m |
| USN01 3200/3000 | 1, gameplay identical | - |

1. **Entries: held.** Warhawk 01 entered at 132.90 s (its OFF refusal time), Lightning 01 at
   142.30 s (141.80 s OFF) and B-25 01 at 228.81 s (226.31 s OFF). Two wingmen also reached mode 4
   and entered: Lightning 01|.-4 at 162.41 s and B-25 01|.-2 at 240.51 s.
   - On entry, A8 was 786 to 834 m and the height above T was 168 to 208 m.
2. **Final at once: held (corrected in section 5j: begin lasts until A8 < a+24h x 312, 1.7 s for Warhawk 01).** `009B3C00` answered true from the first rule tick after each entry.
   Warhawk 01 counted 143 refused finals, one for each tick from 133.0 s until its done byte was
   set. **So in the image begin lasts one tick on this row, and final flies the approach.**
3. **Flight: the mechanism held.** These are Warhawk 01's traces, one per second:
   - X fell 19.6 -> 0.7 m in 10 s, and the heading command went 0.159 -> 0.2618.
   - A8 fell 786 -> 17 m in 13 s, and the height fell 208 -> 9 m. The pitch stayed in mode 2,
     inside [-DropAngle, ClimbAngle]; the clamp -0.698 was hit at 153.9 s.
   - Mode 1 (ApproachPitch 0.1047) took over at 144.9 s, once the plane was slow, near and low.
     The bank limit fell 1.5 -> 0.1 between 143.9 and 145.9 s.
   - The commanded speed was 0.75 TravelSpeed.
   - All four clauses were met.
4. **Done byte: missed.** No head arrived high. The done byte was set only after T, at 148.9 s for
   Warhawk 01, when Z had gone negative and the glide height went below ground.
5. **Held.** LOMP10 moves; USN01 does not.
- **After T** (the reason this stays OFF):
  - With final refused, the plane stays in begin past T. The geometry then gives A8 = 1 and
    Z = -max(z, 1).
  - The plane skims on at 7 to 19 m and 58 to 77 m/s along the runway heading, alternating between
    the dive clamp and ApproachPitch, for the rest of the run.
  - It crosses toward the IJN ships, which is why their engagement ranges change.
- **Verdict: OFF.**
  - The begin tick matches the listing clause for clause.
  - On this row, though, the image leaves begin after one tick. Bound alone, begin would stand in
    for final's whole approach and for everything after touchdown.
  - `land/final` (vtable `00D1FF44`, then `009B3370` to park) is read next. Begin will be re-paired
    with it.


## 5i. `006C3F80`'s launch-site arm (packet `cc9_land_begin_state`, cc9-lua11, 2026-09-28)

The k=0 mode-4 arm, `006C42E9`-`006C4405`, was read whole from the listing. It needs the launch
site at block `+3Ch` (docs/AIRFIELD_TAXI.md 2).

- `tp = own +4h / plane->vtable[38h]` (`006C42F6`-`006C430C`).
- `site = max(0, [00F876A4] - site+40h)` (`006C4310`-`006C4334`).
- `+8h = 00419010(0.75 FDT, 0.01, FDT, 1.0, site + tp)`, where FDT = FollowDistTime (`+504h`), 0.75 is
  at `00CEC9D8` and the call is at `006C4389`.
- `+8h` is zeroed when `tp < 0.25 FDT` (`00D7A348`) and either `site < 0.4 FDT` (`00CE65D0`) or
  `site->vtable[30h]` is false.
  - The airfield site's `vtable[30h]` is `006CF3F0`, which returns true.
- **`site+40h`** has two writers:
  - The site constructor `006CF100` stamps `[00F876A4] - 99999.0` (`00CF89D0`) at `006CF166`.
  - `site->vtable[48h]` = `006CE230` stamps the clock.
- Its one caller found is `007CB5F0` at `007CB75D`, through `(unit+BF4h)->+4h->+3Ch`. That is a
  plane flight-state routine that this host does not run.
  - The caller search swept `006B0000`-`00A00000` for a `+3Ch` load followed by a `vtable+48h` call.
- No plane takes off from or lands at CB4_AF on LOMP10 in this host.
- **Binding**, behind `kLandingSiteSpacingBound` (committed OFF):
  - The stamp is the constructor's value at mission start (labelled), so `site` = 99999 s plus the
    mission clock.
  - `+8h` is then exactly 1.0 whenever the arm runs.
  - A new `site=` count sits on the `summary landing sequencer deck` line.
- **Prediction for LOMP10 9200/9000 and USN01:**
  - OFF, every refused k=0 mode-4 record kept its prior value. For the three heads that value was
    1.000 (the `mode 3 -> 4` lines).
  - ON, `site=` counts the same calls that `spacing_mode4_refused=` counts OFF, and every `+8h`
    is 1.0.
  - Both rows come out gameplay identical (exit 0 or 1).
  - A mode-4 record whose prior `+8h` was not 1.0 would move LOMP10. The log would name it.
- **The pair (cc9-lua11, 2026-09-28): ON.** OFF is this tree's build of `87895cada`
  (`local\l11_stoff_<row>.log`). ON is the same commit with `kLandingSiteSpacingBound=true`
  (`local\l11_ston_<row>.log`).
  - `pair_diff` gives 1 (gameplay identical) on both LOMP10 9200/9000 and USN01 3200/3000.
  - The deck line went from `spacing_mode4_refused=227 site=0` to `spacing_mode4_refused=0
    site=227`: the same calls, with no gameplay change.


## 5j. `land/final`, read and bound OFF for its airborne half (packet `cc9_land_final_state`, cc9-lua11, 2026-09-28)

State vtable `00D1FF44` (task `+5F8h`). Every body below was read whole from the listing.

- **Enter `009B1E60`** (`009B1E60`-`009B1E8B`): clears the done byte `+18h`, sets `+1Ch` = 0.0, clears
  the touched latch `+20h`, sets `+24h` = plane `+9F0h` and sets plane `+844h` = 0.
  **Exit `009B1E90`** (`009B1E90`-`009B1E9E`): plane `+844h` = 1. Plane `+9F0h` and `+844h` are
  not modelled.
- **Tick `009B1ED0`** (`009B1ED0`-`009B211F`, `RET 4`):
  1. `approach+B4h` = 0 and `[approach+1Ch]+40h` = 0.
  2. When `(plane+72Ch)->vtable[38h]` is false (the plane is not airborne), `+20h` = 1.
  3. **Airborne** (`+20h` clear, `009B1F1D`-`009B1FE5`): begin's steer `009B1420`, then
     `+2B4h = r + a+4Ch x (s - r)` with `r = 009B1300` and
     `s = 00419010(a+24h x ApproachDist, r, a+24h x PosBehind x 0.4, (r + a+5Ch)/2, A8)`, and
     `+2B0h = 1`, `+2D8h = 1`.
  4. **On the ground** (`009B1FEA`-`009B207A`): the ground-roll controls (`+29Ch`, `+2A0h`, `+2D0h`,
     `+2C4h`, `+2CCh`, `+278h`, `+27Ch`, `+2A8h` = 1.0 on a class-9 owner or 0.2 (`00CE54A0`),
     `+2ACh`, `+2D8h`).
  5. **With ground contact** (plane `+BF8h` and `+BF4h`, and `007B8D70` false): `006BC530` gives a
     height. The done byte is set when that height is above 2 x WheelHeight (class `+1FCh`), or
     when `+94h` is below `[00CFBC84]`.
- **`009B3370`** (`009B3370`-`009B339A`), final to park: `+20h` set and the plane not airborne.
- **The rule's final arm** (`009B3DB2`-`009B3DF7`): mode not 4 goes to abort; mode 4 with `009B3370`
  goes to park. `009B3770` sends final to abort when the done byte is set.

**Bound OFF**, `kLandFinalStateBound`:
- Bound: the enter, the airborne half, the rule's begin -> final transition (`009B3DA7`) and its
  final arm, and the approach update's onGround arm for final.
- Refused and counted: the on-ground half, park and abort. **This host has no touchdown**: nothing
  models plane `+BF8h`/`+BF4h` or `007CA3F0`, and the control mode stays 7 in flight. So the
  on-ground half, park and the ground-contact done tests are unreachable here.
- The owner velocity in `009B1300` is 0 for the static airfield (labelled).

### Predictions for the joint pair (begin and final both ON) on LOMP10 9200/9000 and USN01

OFF is this tree's build with both switches off. ON flips `kLandBeginStateBound` and
`kLandFinalStateBound`.

1. **Begin lasts one tick.** Each head enters begin at its OFF refusal time (Warhawk 01 132.90 s)
   and final at the next rule tick. `begin ticks` equals `begin entries`.
2. **The approach up to T matches the begin-only run**, clause for clause, because it uses the same
   steer. The one difference is the speed command.
   - `a+24h x ApproachDist` is at least 210 m, and on entry A8 is about 800 m.
   - While A8 is above `a+24h x 312` the command is `(r + a+5Ch)/2`. Below `a+24h x 210` it is `r`,
     which is `max(LevelFlight x StallSpd, min control speed)`, about 32 m/s for the fighters.
   - So the planes fly the glide more slowly than in the begin-only run (58-62 m/s), and mode 1
     (ApproachPitch) takes over earlier.
3. **No touchdown.** `touched=0` and `ground_refused=0` on every plane. Past T, final keeps flying
   the steer, as begin did, and the done byte sends it to a refused abort.
4. **LOMP10 moves** (exit 3); USN01 comes out identical.
- A mechanism failure is any of:
  - no final entry;
  - begin lasting more than one tick per entry;
  - a speed command that does not follow the formula at the traced A8;
  - any of the begin-only run's four approach clauses failing.
- **Flip rule for this pair:** both flip only if the mechanism holds **and** no plane flies on past
  T for more than 10 s. Without a touchdown the second test is expected to fail. If it does, both
  stay OFF, and the touchdown (`007CA3F0`, ground contact `+BF4h`) is the missing piece.


### The joint pair and the verdict (cc9-lua11, 2026-09-28): both OFF, the touchdown is missing

OFF is this tree's build of `880219379` (`local\l11_fnoff_<row>.log`). ON is the same commit with
`kLandBeginStateBound=true` and `kLandFinalStateBound=true` (`local\l11_fnon_<row>.log`).

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | deaths identical; the same eight IJN ships gain an engagement range; B-25 01 6207 -> 8414 m |
| USN01 3200/3000 | 1, gameplay identical | - |

1. **Begin lasts one tick: missed. The prediction was wrong; the mechanism is fine.**
   - Begin lasted 12 to 66 ticks. `009B3C00` turns true when A8 falls below `a+24h x 312`.
   - The entry points fix a+24h: Warhawk 01 went to final at A8 = 664.8 m (a+24h = 2.13, so
     MaxSpd is about 83 m/s), Lightning 01 at 713.1 m, and B-25 01 and B-25 01|.-2 at about 555 m.
   - The prediction had assumed a+24h near 4.
   - Begin lasted 1.7 s for Warhawk 01, 1.8 s for Lightning 01, 1.2 s for Lightning 01|.-4, 6.0 s
     for B-25 01 and 6.6 s for B-25 01|.-2.
2. **The approach: held.** Warhawk 01's final traces follow the speed formula:
   - 45.96 m/s at A8 = 664.8, which is `(r + a+5Ch)/2` with a+5Ch = 59.9;
   - interpolated in between (39.36 at 563.2);
   - `r` = 32.00 from A8 = 436 m, below `a+24h x 210` = 447 m.
   - Heading, pitch and height followed the begin-only run.
3. **No touchdown: held.** Every plane shows `touched=0` and `ground_refused=0`. Past T, every
   head in final flew on and set its done byte, which sent it to a refused abort (1926 to 2919
   refusals each).
   - Warhawk 01|.-4 entered begin at 156.70 s. Its mode then fell from 4, so it went to a refused
     abort before reaching final. The image would abort it too.
4. **Held.** LOMP10 moves and USN01 does not.
- **Verdict: both stay OFF.** The mechanism holds. The second half of the flip rule failed as
  expected: every head flies on past T.
  - The missing piece is the touchdown: plane `+BF8h`/`+BF4h` ground contact and `007CA3F0`
    `BSP_Plane_HandleTouchdownOrCrash`, which moves the control mode off 7.
  - With it, final's on-ground half and `land/park` become reachable.
- **Correction to section 5h, item 2.**
  - **Was:** "`009B3C00` answered true from the first rule tick after each entry ... in the image
    begin lasts one tick on this row."
  - **Is:** begin lasts until A8 falls below `a+24h x PosBehind x 0.4`: 1.7 s for Warhawk 01 and
    6.0 s for B-25 01. The 143 refused finals in the begin-only run ran from 134.6 s to the done
    byte at 148.9 s.
  - **Evidence:** the final entries above.
  - 5h's verdict stands: begin covers the first 2 to 6 s of an approach of about 14 s, and final
    flies the rest.

## 5k. The touchdown, read and bound OFF (packet `cc9_plane_touchdown`, cc9-lua12, 2026-09-29)

**Correction to the handoff's framing.** `007CA3F0 BSP_Plane_HandleTouchdownOrCrash` is not how a
flying plane lands. The handler `007CCFA0` sends message kind `C3h` with sub-kind 4 or 5 to
`007CA3F0` only when the message's current state (`msg+20h`) is **not** 7. With current state 7
both sub-kinds go to `007CB5F0` (`007CD02C`, `007CD06D`). Only two routines request 4 or 5:
- `007C1570`, called from `007C3C90` (placement: `006CFA20` in `BSP_AirOpsSite_PlacePlaneOnSpot`,
  and `006D0684`, an unnamed mother-ship site tick);
- `007C71E0`, the touchdown.

`007C71E0` runs on message kind `C5h` (handler jump table `007CD1AC`/`007CD1D8`, arm `007CD0D9`).
`C5h` has one constructor, `00762A60`. Of its two callers, `007CC4B6` is in the free-flight arm
`007CC2F0`.

### Who writes `+BF4h`/`+BF8h` (a census of every store to `+0BF4h`/`+0BF8h` in `.text`)

| site | routine | store |
| --- | --- | --- |
| `007D01A7`, `007D01AD` | `BSP_PlaneUnitInstance_Construct` | null, clear (and `+C02h` clear, `+BFCh` = 1000.0 at `007D01EB`, `+C04h` = 0.0 at `007D0006`) |
| `007D6261`, `007D6267` | `BSP_Plane_ReadPropertyBag` | null, clear |
| `007B8ECA` | `BSP_Plane_SetGroundContactSite` `007B8E80` | `+BF4h` = the holder (observer swap, src/airfield_taxi.cpp) |
| `007C5BFD`, `007C5F2D`, `007C5F49` | `007C5AC0`, the probe | below |
| `007BB2FC` | `007BB2C0` (no direct caller; a virtual) | `+BF8h` = `006BC530(+FCh, &+BFCh)` when `+C02h` and `+BF4h` |

The other three hits (`004EC40F`, `0062E3C0`, `006351E4`) are stack slots or another class.

### `007C5AC0`, the probe (`007C5AC0`-`007C5F5E`, `__thiscall(plane, float step)`, `RET 4`)

Both motion arms run it first (`007CBFC3`, `007CC43B`). **coverage:** complete except the gear
request at `+C1Ch` (`007C5C11`-`007C5EEF`).
1. `+C04h -= min(step, 0.5)` (`00CE3800`); a negative step sets -1.0.
2. When `+C04h < 0` and the unit is present (`+5Ch`) with `+5Dh`, `+60h` and `+5Eh` clear:
   - `holder = 006C0840(+54h, plane, &dist, 0, airborne or state 6)`, then `007B8E80(holder)`;
   - `+C02h = 1000.0 > dist` (`00CE47A0`, double), or with no holder `+BF8h`/`+C02h` clear and
     `+BFCh` = 999.0 (`00CF4888`);
   - `+C04h = (dist - 200.0) x 0.004` (`00CE4D70`, `00CEB160`). Within 200 m of the key the probe
     runs every step; at 1000 m every 3.2 s.
3. Every step (`007C5F0A`): with `+C02h` and `+BF4h`, `+BF8h = 006BC530(pos, &+BFCh)`; otherwise
   `+C02h`/`+BF8h` clear and `+BFCh` = 1000.0 (`00CE3804`).

`006BC530` (`006BC530`-`006BC5CE`, `__thiscall(holder, pos, float*)`, `RET 8`) takes the position
through the holder's inverse `+48h`, **not** less T. It writes the local y and answers
`|x| < float(width x 0.5)` and `|z| < float(length x 0.5)` (the double 0.5 at `00D7A280`). So
`+BF8h` means "over the runway rectangle" and `+BFCh` is the height above the owner's origin.

**006C0840's key**, now read (`006C09FE`-`006C0A9B`), and its output (`006C0AFF`-`006C0B2A`):
- `l = 006BCC90(pos)` with y zeroed.
- An accepting holder (`006BC530`) keys on `|00438B10(holder+88h, plane vtable[50h])|`.
- Any other holder keys on `00427E30(l)`, the length **squared**. Before that, `l.z x 0.3`
  (`00CE3DC8`) when `-1500 < l.z < 800` (`00CF86A8`, `00CE3948`).
- `*dist` is 0.0 for an accepting winner, else `sqrt(key)` (`00BF7030`). With no winner it is not
  written.

### The touchdown test (`007CC440`-`007CC4CA`) and the chain

On a live plane (`+5Dh` clear, or net mode 2), after the probe:

| gate | site |
| --- | --- |
| `+BF8h` set | `007CC440` |
| `classDesc+1FCh WheelHeight >= +BFCh` | `007CC453`-`007CC463` (`JB` skips) |
| `+908h > 10.0` (`00CE38B8`) | `007CC465` |
| the gear channel `(+DECh)+28h` absent, or its value `+2Ch` = 1.0 | `007CC476`-`007CC494` |
| `vtable[34h]` (`007BBB70`, the world velocity) y > -6.0 (`00D05E30`) | `007CC496`-`007CC4B0` |

Then `00762A60` builds `C5h`, and `0077C2A0(plane, msg, 1, 0)` routes it. The arm returns there
and skips the water test. The chain from there:
- **`007C71E0`** (`__fastcall`): `+C49h` clear, `classDesc+198h != 0.0`, `+BF4h` and the gear
  again. The owner is `holder->+4h->+7Ch`. When it is not the scene parent (`00923810(1)`), it
  re-parents the plane (`007D9CE0`, `vtable[ACh]`). A class-9 owner runs the wire block
  (`007DB630`). Then it calls `007C1570(4, 0)`.
- **`007C1570`**: on the authority it builds `C3h` with current 7 and requested 4 (vtable
  `00D03504`, `+20h`/`+21h`).
- **`007CB5F0`**:
  - `0090F6C0(plane, 4 or 0)`, scoring;
  - state 4, `+C04h` = -1.0;
  - with `classDesc+198h != 0`: `+904h = +908h > 5.0` (the landing flag, `00CE3850`) and
    `+C18h` = 3;
  - `007C11E0(0)`;
  - the site `vtable[24h]` `006CED90` (`006CED90`-`006CEDCC`, `RET 4`) appends the plane to the
    occupancy vector `+34h`/`+38h` unless it is there;
  - `vtable[48h]` `006CE230` restamps `+40h`;
  - a human plane over its own class-9 owner gets `0090F6C0(plane, 1)`.

**The gear.** `(+DECh)+28h` is actuator channel B. The constructor `007EABC0` enables it only when
the class has `+5D4h`, which is model data. `007C11E0` drives its target down unless the plane is
on the water or has been airborne more than 5 s. In flight its target is `+C1Ch` (`007C6B6C`-`007C6BC6`),
the request `007C5AC0` computes in mission state 0Dh. It asks for the gear:
- when the plane is not flying;
- when the plane is early in its flight (`+908h <= classDesc+5F8h`);
- inside the approach cone in front of a friendly usable holder (`007C5CF8`-`007C5EE6`).

`007B8D70` (`007B8D70`-`007B8D99`) is the same gear test. It is the one land/final uses before its
done tests (`009B209B`).

**WheelHeight** is written at `007D2A7C` only when WheelHeight and GroundPitch are both authored.
The descriptor comes from `BSP_Memory_AllocZeroed` `00470B80`, so otherwise it is 0.0. This
installation authors WheelHeight 1.52 for the B-25 Mitchell (`VehicleClass[118]`). It authors
neither key for the P-40 (`[135]`) or the P-38 (`[104]`), so they must bring their origin down to
the runway plane (vehicleclasses.lua mtime 2026-05-09, modded).

### The binding, behind `kPlaneTouchdownBound` (committed OFF)

- **Bound:**
  - the probe, with `006C0840`'s key and output: `plane_landing_site_006c0840`,
    `plane_site_probe_007c5ac0`;
  - the touchdown gates and `007C71E0`/`007CB5F0`'s state effects, in `plane_touchdown_007cc440`;
  - the site's occupancy add and its restamp. `006C3F80`'s site arm (5i) now reads the stamp, which
    stays -99999.0 with the switch off;
  - `009AFAF0`'s onGround gate.
- **Not carried, labelled:**
  - the gear channel: both gear gates pass, and `007B8D70` answers true, so land/final's done tests
    never run;
  - `+C49h`, taken clear;
  - the re-parent: a static airfield;
  - `0090F6C0` and `007C11E0(0)`'s actuator targets.
  - Both messages are applied at once. The image drains the loopback queue at row 9 of the same
    fixed step.
  - The probe runs after this host's integration step.
  - A mother-ship holder is refused, as in 5c.
- **After state 4** the ground-roll arm `007CBFA0` does not run in this host, so the plane is held
  where it touched down. A state-6 plane is held the same way.

### Predictions for LOMP10 9200/9000 and USN01 3200/3000 (touchdown ON alone), written before any ON run

OFF is this tree's build. ON flips only `kPlaneTouchdownBound`, so begin and final stay OFF and
their entries are refused.
1. **The probe runs on every plane.** The only holder in either mission is CB4_AF. Every plane of
   party 0 near LOMP10's airfield gets `+BF4h`. The `summary plane touchdown` line lists every
   plane with `probes > 0`.
2. **No touchdown.** Without begin and final, no plane descends to the runway:
   - line flies the pattern at its authored height;
   - the P-40s and P-38s would need `+BFCh <= 0.0`;
   - the B-25s would need `+BFCh <= 1.52`.

   So `touchdowns=0` everywhere. Any `contact_steps` come from pattern passes over the 20 x 400 m
   rectangle, at a `min_height` of tens of metres.
3. **Gameplay identical on both rows** (`pair_diff` 1). The only differences are the new summary
   lines and the landing-deck build line, which the probe may print earlier.
- **Mechanism failure:** any of:
  - a touchdown with a height above the wheel height;
  - a probe count of zero on a plane that flew within 1000 m of CB4_AF;
  - any gameplay move.
- **Flip rule:** flip ON if the mechanism holds and gameplay is identical, since then the switch
  changes nothing. The real test is the joint pair with begin and final (packet
  `cc9_land_begin_final_repair`).


### The pair and the verdict (cc9-lua12, 2026-09-29): gameplay identical, contact unexercised

OFF is this tree's build of `6369a63c4` (`local\l12_tdoff_<row>.log`). ON is the same commit with
`kPlaneTouchdownBound=true` (`local\l12_tdon_<row>.log`). A 300-frame LOMP10 smoke of the ON build
ran first and ended cleanly.

| row | `pair_diff` | touchdown summary |
| --- | --- | --- |
| LOMP10 9200/9000 | 1, gameplay identical | 10 planes, 428 to 1004 probes each, `contact_steps=0`, `touchdowns=0` |
| USN01 3200/3000 | 1, gameplay identical | 7 planes (Airfield2 is the holder; Enterprise is refused), `contact_steps=0`, `touchdowns=0` |

1. **Held.** The probe runs on every flying plane and finds CB4_AF on LOMP10.
2. **Held, and more strongly than predicted.** No plane in the line pattern ever crossed the
   20 x 400 m rectangle, so `contact_steps` is 0 rather than a few high passes.
3. **Held.** Both rows are gameplay identical. The ON log differs only by the summary lines and by
   the deck build line, which the probe now prints earlier (LOMP10: log line 4275 instead of
   5399).

**Verdict: the switch stays OFF for now.** The flip rule is met, but `006BC530`'s contact path
never ran, so this pair does not test it. The joint pair with begin and final (section 5l) flies
the heads over the runway past T. It decides the switch together with begin and final.


## 5l. The joint re-pair of begin, final and the touchdown (packet `cc9_land_begin_final_repair`, cc9-lua12, 2026-09-29)

### Predictions for LOMP10 9200/9000 and USN01 3200/3000, written before any ON run

OFF is `local\l12_tdoff_<row>.log`, with all three switches off. ON flips `kLandBeginStateBound`,
`kLandFinalStateBound` and `kPlaneTouchdownBound`. The base is section 5j's joint pair (begin and
final ON, `local\l11_fnon_lomp10.log` in cc9-lua11). Its final traces put the heads past T at
local heights of 3.4 to 16 m over T:
- Warhawk 01: minimum Y 3.4 m;
- Lightning 01: 3.7 m;
- Lightning 01|.-4: 5.8 m;
- B-25 01: 11.3 m;
- B-25 01|.-2: 10.8 m.

1. **Contact is exercised.** Every head that reaches final (Warhawk 01, Lightning 01,
   Lightning 01|.-4, B-25 01, B-25 01|.-2) shows `contact_steps > 0` once it flies along the
   runway inside 10 m of the centre line.
   - `min_height` is Y + 0.5 at the lowest point over the rectangle: about 4 m for the fighters
     and 11 m for the B-25s.
   - The line-only wingmen show 0.
2. **No touchdown.** `touchdowns=0` everywhere. The P-40s and P-38s need `+BFCh <= 0.0` and
   the B-25s need 1.52, and nothing in 5j's traces comes that low.
3. **Everything else is 5j's joint pair.**
   - With no touchdown nothing restamps the site. `009AFAF0`'s onGround gate changes nothing on
     a static holder: past T the arm is never reached, and before T only the 1.0 floor could move.
   - Begin lasts 1.2 to 6.6 s per head.
   - Final flies on past T, sets its done byte and goes to a refused abort.
   - LOMP10 moves (exit 3, the same moves as 5j: deaths identical, B-25 01 6207 -> 8414 m).
   - USN01 is gameplay identical (exit 1).
- **Mechanism failure:** any of:
  - a touchdown with `+BFCh` above the wheel height;
  - `contact_steps > 0` with a local |x| or |z| outside the rectangle, checked on the first
    contact of Warhawk 01 from the final trace;
  - begin or final departing from 5j.
- **Flip rule (5j's):** the three flip together only if a head touches down **and** no head flies
  on past T for more than 10 s. It is expected to fail on both counts, so all three stay OFF. The
  blocker would then be the descent: the heads float 3.5 to 16 m over the runway, where the image
  needs the wheel height.


### The joint pair and the verdict (cc9-lua12, 2026-09-29): begin and final stay OFF, the touchdown flips ON

OFF is `local\l12_tdoff_<row>.log`. ON is `6ad4b7fcd` with the three switches true
(`local\l12_jon_<row>.log`).

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | 5j's moves exactly: deaths identical, B-25 01 6207.20 -> 8414.00 m, the same eight IJN ships gain an engagement range |
| USN01 3200/3000 | 1, gameplay identical | - |

1. **Contact is exercised: held.** Each head's first contact is at the runway threshold, local
   z = -198.8 to -199.9. The lateral offset |x| is 0.58 to 0.97 m for the fighters and 3.6 m for
   the B-25s. The first-contact heights are 11.9 to 14.4 m. Minimum heights over the rectangle
   (`contact_steps` 218 to 256, i.e. the whole 400 m at about 32 m/s):

   | head | minimum height |
   | --- | --- |
   | Warhawk 01 | 3.96 m |
   | Warhawk 01\|.-4 | 3.89 m |
   | Lightning 01\|.-4 | 6.09 m |
   | Lightning 01 | 7.25 m |
   | B-25 01\|.-2 | 11.88 m |
   | B-25 01 | 12.35 m |

   The prediction said "about 4 m for the fighters". That holds for the Warhawks; the Lightnings
   stayed 2 to 3 m higher. Warhawk 01|.-4 made contact while still in begin (2934 begin ticks,
   no final entry), as in 5j.
2. **No touchdown: held.** `low_steps=0` on every plane: no head came below its wheel height (0.0
   or 1.52).
3. **Everything else matches 5j: held.**
   - Begin lasted 12 to 66 ticks per head, and there were five final entries.
   - The abort refusals are 1926 to 2919 per head, 5j's numbers.
   - The native-table and summary moves are 5j's.
- **Why the heads float.** Over the runway, Warhawk 01's final trace (every 10 ticks) runs:
  - Y = 15.2 and 16.3 m in mode 2, the steer's nose-down arm (pitch -0.28 to -0.32);
  - then 10.2 / 6.6 / 7.7 / 10.6 / 14.0 m in mode 1, where the steer holds ApproachPitch
    +0.1047 rad because Y is below 1.4 x hull length;
  - spd_cmd 32.0 and live speed 30.0 to 34.6 m/s throughout.

  In this host, a +6 degree hold at 30 m/s climbs. For the image's planes to reach their wheel
  height, the same hold has to sink at the commanded speed. This is the airframe response at the
  landing speed that 5g already recorded as a spread miss. It belongs to the flight model
  (`009FB800`, the lift and throttle at the ApproachPitch hold), not to the land task or the
  touchdown.
- **Verdict.**
  - **Begin and final stay OFF.** 5j's rule fails on both counts: no head touches down, and every
    head flies on past T for more than 10 s.
  - **`kPlaneTouchdownBound` flips ON.** Section 5k's rule is met: it is gameplay identical alone
    on both rows, and this pair shows its contact geometry working at the threshold with the
    right lateral offsets. On its own it changes no gameplay until something brings a plane down
    to its wheel height.


## 5m. `land/park` read at its head, `land/abort` read and bound OFF (packets `cc9_land_park_state`, `cc9_land_abort_state`, cc9-lua12, 2026-09-29)

### `land/park`: not reached on LOMP10

Vtable `00D1FF60`:
- **Enter `009B21A0`** (`009B21A0`-`009B21BA`): `+1Ch` = 0.0, `+18h` = 0, `+28h` = 3.0
  (`00CE3854`).
- **Exit `009B21C0`** (`009B21C0`-`009B21C5`): `+18h` = 0.
- **Tick `009B22C0`** (`009B22C0`-`009B2C68`, `RET 4`; ledger name
  `BSP_PlaneBot_ApproachBaseStep`, docs/AIRFIELD_TAXI.md). Its head, `009B22C0`-`009B2313`:
  `[approach+1Ch]+40h` = 0.0. Then, when the plane has ground contact (`+BF8h`, `+BF4h`), a holder
  at `approach+34h`, and the holder's owner absent or with `+5Dh` set, the done byte is set and it
  returns. Otherwise the ground taxi runs: the controls `+2C4h`/`+2CCh` and the `4 <-> 5`
  transitions `007C1680`/`007C16F0`.

The rule sends a plane to park only when `+900h` is 4 or 5 or `009B3370` answers. Both need a
touchdown, which no LOMP10 plane makes (5l). Every output of the tick feeds the ground-roll arm,
which this host does not run. **Park is left unbound.** It follows the descent, the ground roll and
final's on-ground half.

### `land/abort` (vtable `00D1FED4`), read whole from the listing

- **Enter `009B0980`** (`009B0980`-`009B0998`): `+20h` = `+21h` = 0 (task `+66Ch`/`+66Dh`), then
  `006BCD80(approach+2Ch, plane)`. `006BCD80` works under the block lock: it sets `+0Ch` (held) on
  the plane's record in the vector at `block+A8h` (`006BCE32`).
- **Exit `009B09A0`** (`009B09A0`-`009B09B6`): with `approach+2Ch`, `006BCE60(block, plane)`
  clears the same byte (`006BCF12`).
- **Tick `009B09C0`** (`009B09C0`-`009B0F96`, `RET 4`; dt is not read):
  1. Bank 0.0 with `+2CCh` = 1, throttle 1.0 and air brake 0.0 with their bytes, `+2D8h` = 0,
     then `007C07A0(plane, (0, 1, 0), 0.0)`. `+20h` = 0.
  2. **On the ground** (`009B0E74`-`009B0F93`): `+21h` = 1, pitch `class+1ECh x 0.5`, and a
     ground heading. Refused and counted.
  3. **Airborne**: `l = 006BCC90(position)`, the runway frame less T;
     `e = 00438B10(plane heading, holder+88h)`.
     - `l.y < 5.0` (`00CE3850`): no heading command.
     - `5 <= l.y < 25` (`00CE3880`): when `l.z < 200` (`00CE4D70`),
       `|l.x| < holder+B0h` (the whole width) and `|e| < 30 degrees` (`00CEC724`), hold the
       runway heading (`+2C0h` = `holder+88h`, `+2CCh` = 2). Otherwise take the look-ahead turn:
       - the point 100 m ahead (`007BA2E0` x 100, `00D7A220`) goes into `006BCC90`;
       - `+2C0h` = heading + 0.5 when its x >= 0, else heading - 0.5, with `+2CCh` = 2.
     - Both lower arms end at `009B0C5C` with a pitch hold (`+2D0h` = 1):
       `+2BCh = 00419010(007C47F0, 3 degrees (00D0CBA0), class+18Ch, class+1ECh, 007D99C0)`.
     - `l.y >= 25`:
       - above 40 m (`00CE685C`), `+20h` = 1 when `|e| > pi/2`, `l.z > 10` (`00CE38B8`) or
         `l.z < -1200` (`00D1FEF0`);
       - then the look-ahead turn and `009FB800(approach+3Ch, 1.0)`: climb toward the circle
         point's height.
- **The rule's abort arm** (`009B3E08`-`009B3E43`): `+66Ch` goes to standby, else `+66Dh` goes
  to park. Abort is entered from begin or final, on the done byte (`009B3770`) or on mode not 4
  (`009B3D7A`, `009B3DCE`).

So abort is the go-around: full throttle, climb by speed below 25 m, and back to standby once
above 40 m past T. The held byte keeps the plane's record out of the sequencer's spacing
(`006C3F80` skips held records) until it leaves abort.

**Bound OFF, `kLandAbortStateBound`:** the enter, exit, airborne tick, the rule's abort arm and
the four begin/final edges. The on-ground arm and park are refused. This commit also makes the
approach update honour `+904h` (`009B3ABA`: a touchdown ends the update before the geometry and
the request pacing).

### Predictions for the pair on LOMP10 9200/9000 and USN01 3200/3000, written before any ON run

OFF flips `kLandBeginStateBound` and `kLandFinalStateBound` (the touchdown is ON by default). ON
also flips `kLandAbortStateBound`. OFF reproduces `local\l12_jon_*`.
1. **Every head that sets its done byte in final enters abort once at that time.** In 5l that is
   Warhawk 01 at about 158.6 s, 198 m past T at Y 14 m. The other heads take the same route.
2. **Abort is short and ends in standby.**
   - Arm B holds the runway heading at full throttle with a climbing pitch (about 3 degrees at
     the level-flight speed).
   - Above 25 m, `009FB800` climbs toward the circle height.
   - Above 40 m the plane is past T by more than 10 m, so `+20h` is set.
   - Prediction: `to_standby` = `entries`, and each abort lasts under 20 s.
3. **The heads fly the circuit again.** Standby -> line or begin -> final -> abort. Each head gets
   two to four aborts in 450 s. There is still no touchdown (`touchdowns=0`), because the descent
   is unchanged.
4. **LOMP10 moves (exit 3) and USN01 is gameplay identical.** The final refusals of 5l
   (`abort_refused` 1926 to 2919 per head) drop to 0.
- **Mechanism failure:** any of:
  - an abort longer than 60 s;
  - an abort that ends anywhere but standby;
  - an entry from a state other than begin or final;
  - a plane left held after it leaves abort (the sequencer's census).
- **Flip rule:** abort is reachable only with begin and final, which stay OFF (5l). It stays OFF
  whatever the pair shows; the pair records whether its mechanism holds for the day the descent
  is fixed.


### The pair and the verdict (cc9-lua12, 2026-09-29): the mechanism holds, abort stays OFF with begin and final

OFF is `aa541a3e1` with begin and final flipped (`local\l12_aboff_<row>.log`). ON also flips
`kLandAbortStateBound` (`local\l12_abon_<row>.log`). A 300-frame USN01 smoke of the ON build ended
cleanly.

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | B-25 01 8414.00 -> 7360.32 m. The eight IJN ships' engagement ranges go back to the all-OFF values. Deaths are identical (0 on both) |
| USN01 3200/3000 | 1, gameplay identical | - |

Ten aborts, eight from final and two from begin (Warhawk 01|.-4 at 158.71 s, Lightning 01|.-4 at
386.38 s):

| head | aborts | ticks | to standby |
| --- | --- | --- | --- |
| Warhawk 01 | 3 | 131 | 3 |
| Lightning 01 | 3 | 134 | 3 |
| B-25 01 | 1 | 26 | 1 |
| B-25 01\|.-2 | 1 | 56 | 1 |
| Lightning 01\|.-4 | 1 | 60 | 1 |
| Warhawk 01\|.-4 | 1 | 56 | 1 |

1. **One abort at the done time: held.** Warhawk 01 went to abort at 158.21 s, 12.6 m over T and
   185 m past it, against the predicted "about 158.6 s, Y 14".
2. **Short and back to standby: held.** `to_standby` equals `entries` for every head, and the
   longest abort is 60 ticks (3 s). Warhawk 01's first abort:
   - arm B holds the runway heading while z < 200;
   - past 200 m the look-ahead turn takes over (heading 0.30 -> 1.40 rad);
   - the plane climbs from 12.6 to 34 m while the speed rises from 33 to 57 m/s at full throttle;
   - the standby flag is set above 40 m, 4.5 s after entry.
3. **The circuit repeats: partly held.** Warhawk 01 and Lightning 01 fly three circuits each. The
   B-25s and the fourth planes abort once and spend the rest of the run in standby: 14 and 15
   standby entries for the B-25s, from the standby/line mode arm. There is still no touchdown.
4. **Held.** LOMP10 moves, USN01 is gameplay identical, and the 15307 final refusals (`009B3DCE`)
   are gone.
- **No mechanism failure.** Every abort ends in standby within 60 s, and every entry comes from
  begin or final.
- **Verdict: `kLandAbortStateBound` stays OFF.** It is reached only through begin and final, which
  stay OFF until the descent is fixed (5l). When they are re-paired, abort flips with them.


## 5n. Why the heads float over the runway (packet `cc9_landing_descent`, cc9-lua12, 2026-09-29): no host substitution found

A diagnostic line `land final flight` (every 10 final ticks, commit `046998bd5`) was added and one
LOMP10 9200/9000 run made with begin and final ON (`local\l12_diag_lomp10.log`). Warhawk 01 over
the runway, 150.6 to 164.6 s:
- the pitch follows the command: +0.090 to +0.104 at the flare hold of 0.1047;
- q (007D99C0 / StallSpd / LevelFlight) is 0.92 to 1.05;
- the angle of attack is 0.000 to 0.050;
- the throttle cycles 0 to 0.62 around the speed command 32.0;
- vy is -0.5 to +3.4 m/s in the hold.

B-25 01 (flare threshold 1.4 x 17 = 23.8 m) shows the same at q 0.97 to 1.03, climbing at 3.3
to 3.9 m/s. Both climb in the hold.

**The reconstructed image laws give this result.**
- **Lift** (`007DB875`): `(1 + aoa) x min(q, 1)^2 x AccelCheatMul x 9.81` against gravity
  `AccelCheatMul x 9.81`. At q >= 1 lift balances weight at aoa = 0, and the body damping
  (`007DBD37`, YDrag at DragRange 1.0-2.0, `DragFuncPower` 1.8) keeps the velocity on the nose.
  So the path angle equals the pitch.
  - The installation's planeglobals.lua says the same of `LevelFlight`: above that fraction of
    StallSpd the lift is maximal and there is no sink in level flight. Its comment on
    `DragFuncPower` calls it the flare's characteristic.
- **The command** in final is `r = max(007C47F0, 007C4810)`, with `007C47F0 = LevelFlight x
  StallSpd` (32.0 for the P-40 and 37.0 for the B-25). That is exactly q = 1. Both sides of q use
  the same StallSpd, so this holds whatever the class row says.
- **The hold** is ApproachPitch, `tuning+4E0h` copied at `009B1439`-`009B144B` and stored at
  `009B1AB3` when not fast, not far out and Y < 1.4 x `class+A0h` Length (`00D045F0`). At q = 1
  it climbs at the pitch; to sink it needs q below about 0.93.
- **Nothing in final lowers the speed below r.** Past T, `s = r` and `+2B4h = r`. The throttle law
  `0099D300` holds it:
  - `+2B0h` = 1 skips the divisor raise at `0099D924`;
  - `+2B8h` decays to 1.0 at `0099D75C`-`0099D79A`.
- So under these laws the flare climbs until Y reaches the threshold, the glide arm dives back, and
  the plane porpoises. The host reproduces the laws; I found no substitution that turns a descent
  into a climb.

Substitutions checked on this path, none of them systematic:
- `+2B8h` held at 1.0: equal to the image in final, since the raise is skipped.
- The measured speed is `|v|` instead of `007D99C0 / +2B8h`: within 0.1 m/s here.
- **`0099DAC8`'s correction, taken as 0.** Its inputs do have writers, which corrects the host's
  label "no displacement writer anywhere": `unit+B1Ch` is `ctl+6Ch` (`007D87F1`, `007DB382`) and
  `unit+AE0h` is `ctl+30h`, a low-pass copy of a velocity (`007DC792`-`007DC80B`).
  The term is `(forward speed - |smoothed velocity|) x 20.0 (00CE3D88) x dt`. That is a lag term:
  it damps the throttle oscillation but has no sign bias.
- Thrust's TurboMultiplier, `desc+604h`, `unit+0CC8h` and `008E6430` are 1.0 or off: no effect on
  a hold at q = 1.

**Gear and ground effect, checked.** A sweep of 007B0000-007F8000 for loads of `unit+DECh`
followed by `+28h`/`+2Ch` finds only the gear gates (`007B8D70`, `007C7220`, `007CC476`) and
the two setters (`007C619E`, `007C6B6C`), so the gear channel adds no drag in the flight law. The
core law reads `+BFCh` only in the ground mode (`007DB702`), so there is no ground effect. By the
laws read so far, the image's flare climbs as the host's does. That leaves three possibilities:
- the image's heads also porpoise on this installation. With WheelHeight 0.0 for the P-40 and
  P-38 that is plausible;
- the speed in the image's flare falls below q = 1 for a reason outside these laws;
- a law not yet read acts in the flare.

The host cannot settle which without the original running, which the rules forbid.

**The blend does not give the sink (checked in `local\l12_diag_lomp10.log`).** In every mode-1
hold the final trace's speed command is `r` itself:
- Warhawk 01: 32.00 in all 238 held samples;
- Lightning 01: 31.50 in 236;
- B-25 01: 37.00 in 123.

The blend `009B1F2C`-`009B1FDF` only ever raises the command. `a+5Ch` (TravelSpeed x 0.75) is above
`r`: Warhawk 01 entered final at 45.96 = (32.0 + 59.9) / 2, and Lightning 01 at 33.78 and 35.84
on the way in. And the blend reaches `r` below `a+24h x 210` m, before the hold begins. So the
command never falls below q = 1.

**Not bound.** With no host substitution identified there is no switch to commit, and begin, final
and abort stay OFF. The diagnostic line stays: it is read-only and prints only while final is
bound.

### Second pass: the pitch arm in mode 1 and the drag the host supplies (packet `cc9_landing_descent_2`, cc9-lua13, 2026-09-29): nothing in the airframe laws

The two reads the first pass left open are done, from the disk listing (`disasm-raw`), and so are
the other host-supplied inputs of the core law that could put a sink under the flare. **None of
them differs from the host in a way that sinks a plane held at q = 1 and +0.1047 rad.** Nothing is
bound, there is no pair, and begin, final, abort and park stay where 5l and 5m left them.

**The pitch arm in mode 1 (`0099DC9E`-`0099E512`).**
- The flare writes mode 1: `009B1AB3` loads `[esp+58h]`, the first dword of the `tuning+4E0h`
  block that `009B1439`-`009B144B` copied (`REP MOVSD`, 14h dwords, ApproachPitch = DEG(6)), sets
  `+2D0h = 1` at `009B1ABB`, and `009B1B22` stores it to `+2BCh`. No `FCHS` on that path; the
  `FCHS` at `009B1AD3` belongs to the glide arm's clamp to `[-desc+1F0h, desc+1ECh]`.
- In mode 1 there is no slew. `0099DCE0` takes the `else` at `0099DD54`: the measured angle is
  `unit+C64h`, the live pitch, which is what the host passes (`pin.held_pitch`). The mode-2 slew
  `0099DCFA`-`0099DD4E` is bound ON already (`kPlanePitchModeTwoBound`) and is not on the flare path.
- The floor `0099E490`-`0099E512` is `max(PitchTurnMaxPitch - 2.5 x (1 - q), +2BCh)` with
  `PitchTurnMaxPitch` = DEG(6) (`tuning+5C0h`, mirrored at `tuning+88h`). It can only raise the
  target; at q = 1 it equals the flare hold itself. It cannot sink anything.
- The error gain: `0099E604` multiplies the wrapped error by `[esp+28h]`, which `0099D46E`-`0099D4E6`
  sets to `1 / max(unit+340h x [00CE65D0], 1.0)`. `unit+340h` is CheatTurbo, 0 in this host and in
  the installation's rows, so the gain is 1.0: the host's `dt_scale = 1.0` is exact.

**The drag `007D9140` and its call site `007DBA32`-`007DBC76`.**
- `a1` is `sqrt(|ctl+18h|^2)` (`007DBA47`-`007DBA88`), the world speed; the host uses the same.
- `a2`, `a4`, `a5` are the plane's throttle, air brake and pitch control. The first pass took them
  from the command block (`unit+9F0h`) and the latched block (`unit+BC0h`, `unit+BB4h`). My own
  frame count leaves a four-byte doubt about which of the two slots `[esp+3Ch]`/`[esp+40h]` holds
  which block at `007DBB28`/`007DBB33`. Either way the two blocks hold the same values one step
  apart: `007B9770` copies `+9E4h..+9F4h` into `+BB0h..+BC0h` verbatim, with no rate or filter
  (`007B9770`-`007B97C6`). That is not a sink.
- `a3` (the pitch) only feeds the speed floor, whose two endpoints are both `MaxSpd x 0.1` in this
  installation's PlaneGlobals, so its value cannot matter.
- After the call: the result is multiplied by the `007DBB23` pitch ramp (`-0.3 -> r1`, `0.1 -> 1.0`,
  with `r1 = 1.0` until the DeadMeat timer runs), by `[00D7A370]` only when `ctl+44h < 0`
  (`007DBB6A`, flying backwards), and applied along the unit world velocity (`007DBB82`-`007DBBDF`).
  The extra lateral term `007DBBE2`-`007DBC5C` runs only when `ctl+98h > 1.0`; `ctl+98h` is 1.0 from
  the constructor (`007D7F5D`) and I found no other literal-offset writer to `ctl+98h` in
  `007B0000`-`007F8000`. The host's drag is this.
- The drag scales with `(1 - throttle)^2 x GlideRate`, so it only changes the throttle the speed
  hold needs; the hold still settles at `r`, where lift balances weight. Drag cannot move the
  flare below q = 1 while the throttle is below 1.0, and the trace's throttle peaks at 0.62.

**The other inputs to the lift and gravity terms, censused** (byte scans for `MOVSS`/`FST(P)`/`MOV`
stores at the literal offsets, `007B0000`-`007F8000` and `00996000`-`009F7000`; the controller is
`unit+AB0h`, so `ctl+9Ch` would also be `unit+B4Ch`, which has no plane writer):
- `ctl+9Ch`, the multiplier on q in the lift (`007DB8CF`): 1.0 from the constructor (`007D7F65`).
  The other `+9Ch` stores in the range write other objects (the flight sub-object at `ctl+10h`:
  `007D7D51`, `007D7D83`, `007DB726`; a vtable object at `00D05814`: `007B5390`; the unit
  constructor's helper `007BAF90`; the sound element `007EB380`; the pilot-bot blocks). The host's
  `lift_scale = 1.0` holds.
- `ctl+94h`, the second gravity term (`007DB9E7`): set by `007D8180` (arg true or game mode 0/1)
  and cleared by `007DB2C0`. Its size is `interp(1.0 -> 0, 2.5 -> 3.0, DeadMeat timer)`, which is 0
  until the timer passes 1.0. No sink for a healthy plane.
- `ctl+90h`, the lift cap: 99.0 from the constructor, or the 3.0 to 6.0 ramp (`007DB80D`-`007DB86F`);
  at q = 1 the scaled lift is 1.5, under every value it can take.
- `dyn+88h..90h`, the velocity delta `007D8470` adds (`007D8755`-`007D8774`): the sub-object's only
  stores are the constructor (`007D7EFB`-`007D7F1B`) and `007D7A80`; the other hits at those offsets
  are `ctl`-relative (`007DCCD3`, `007DCDAD`, `007DD84F`) or other objects.
- The sink term in `007D8470`'s tail (`007D8CE9`-`007D8E15`, body `vy -= 0.1 x speed`) is gated on
  the latched `unit+904h`. Its stores (22 sites) are the flight-state setters, the touchdown
  `007CB6B0`/`007CB71F`, the spawn chooser, the property bag, and the ground task `009CE2C0`, which
  clears it. It is a ground-contact flag, set only after a touchdown; it cannot act in the flare.

**What the census left out: the steer's own hold.** The flight model has one more input on this
path, and the host counts it instead of applying it: `009B1420` arms the timed direction hold
through `007C07A0` on every steer pass with `c2 > 0` (`land_begin_hold_unapplied` in the land
summary). That is section 5o, and it is the sink.

## 5o. The steer's direction hold is the descent (packet `cc9_landing_descent_2`, cc9-lua13, 2026-09-29)

docs/PLANE_DYN_TIMED_HOLD.md read the hold's setter, commit, decay and yaw-law use, and left the
consumer of the direction `dyn+B4h` unfound. It is in `007D8470`, and it moves the velocity.

### The consumer, `007D88CB`-`007D8C5E` (read from the disk listing)

In `BSP_PlaneDynamics_IntegrateStep` (`__thiscall(dyn, float step, const Matrix* bodyToWorld =
unit+74h, const Matrix* worldToBody = ctl+0B0h)`), after the velocity step, the 0.01 deadband and
the contact terms, and before `007D8C6B` rotates the body velocity `dyn+64h` (ESI, `007D85F7`)
back to world:
- `007D88CB`-`007D88D6`: runs only while `dyn+C0h > 0.0` (`COMISS` against XMM0, which is the
  deadband's zero from `007D878F`-`007D87C2`).
- `007D88E0`-`007D8927`: `speed = |v|` when `|v|^2 > 1e-10` (`00CE3820`), else 0.
- `007D8927`-`007D8938`: `d = 0042D0D0(dyn+B4h, [esp+58h] = worldToBody, 0)`, the direction in the
  body frame.
- `007D8951`-`007D8A0F`: `d / |d|` (the reciprocal only when `|d| > 0`).
- `007D8A13`-`007D8AB6`: `v / |v|`, stored in place.
- `007D8AB9`-`007D8B35`: `b = s d + (1 - s) v` with `s = dyn+C0h`, **not clamped**.
- `007D8B39`-`007D8C12`: `v = (b / |b|) x speed`.
- `007D8C15`-`007D8C5E`: the 0.01 deadband (`00D7A238`) again, per component.

So while the hold has time left, every fixed step turns the velocity toward the hold's direction
by the fraction `s` and keeps the speed. The attitude and the lift do not enter.

### The arm on the landing path

`009B1B1C`-`009B1C79` in `009B1420` (`BSP_BotStateLandBegin_Steer`, run by land/begin's tick and by
land/final's airborne tick), when `c2 > 0` (`[esp+3Ch]`, `c1 x w_gain`, so `A8 < 260` m):
- `lat = -X x 1.25 x interp(1.0 -> 0, 4.0 -> 1, |X|)` (`00CF87C0`, `00CE3D34`);
- `fwd = max(Z, 2|X|)`; `off = pi/2 - atan2(fwd, lat)`, wrapped up to `[0, 2pi)`;
- `hd = 00438AA0(A4, off)`; `ang = pi/2 - hd`, wrapped up;
- `dir = (cos ang, tan P, sin ang)`, with `P = -atan2(Y - H, m)` at `[esp+10h]` (the glide pitch
  from `009B1956`, **not** the ApproachPitch hold);
- `007C07A0(plane, &dir, c2)`.

`007C07A0` (`__thiscall(plane, const float* dir, float seconds)`, `RET 8`) normalises `dir`, and
when `|nz| + |nx| > 0.1` it scales the seconds by `interp(15 deg -> 1.0, 60 deg -> 0.4)` of the
heading error `|00438B10(pi/2 - atan2(nz, nx), vtable[50h])|` (`00D05AA8`, `00D05AAC`,
`00CE7804`). `007D83D0` then stores the direction and the seconds. land/abort's tick clears it with
`007C07A0(plane, (0, 1, 0), 0.0)` at `009B0A3B`. The only other arm is the launch path
(`007C6F50` at `007C705C`, 0.8 s), from `007C713A` and `007CD0CA`; a rel32 and absolute scan finds
no other caller of `007C07A0` or `007D83D0`.

**The commit keeps it in flight.** `007DC6AA`-`007DC6C5` keeps the seconds only when
`(plane+72Ch)->vtable[38h]()` is true. That slot of vtable `00D06130` is `0074E210`
(`BSP_PlaneControlMode_IsFreeFlight`): `unit+900h == 7`, the free-flight mode every landing head is
in. `007D81B0` also clears it when `GGame+1FE4h == 2`, which a single-player host takes as false
(labelled).

**What it does on the flare.** Near the runway `A8 < 80`, so `c1 = 0.8` and `s` is re-armed to
about 0.8 on every steer pass. Every step then turns the velocity 80% of the way onto the glide
path `P` toward `H`, whatever the pitch hold does. With ApproachAngle DEG(12), `t2 = tan(7.2 deg)
= 0.126`, and `H = 0.5 + (Z - 100) x 0.126` for `Z < 200`. At Warhawk 01's first mode-1 sample
(Z 85.5, Y 14.2, `local\l12_diag_lomp10.log` in cc9-lua12), `H = -1.4` and `P = -0.155 rad`, so
the path sinks at about 5 m/s at 32 m/s. That is the descent the ApproachPitch hold cannot give.

### The binding, behind `kPlaneDirectionHoldBound` (committed OFF)

- `src/plane_flight.cpp` `blend_direction_hold_007d88cb`, the consumer above.
- `src/game_hosts_units.cpp`:
  - `arm_direction_hold_007c07a0`, 007C07A0;
  - the steer's arm at `009B1C79`;
  - land/abort's clear at `009B0A3B`;
  - in the free-flight integrator: the `007D81C7` gate, the `007DC6C5` commit
    (`plane_control_mode_900 == 7`), the consumer on the body velocity, then the `007D902F`
    decay;
  - the `0.6 x seconds` term (`007D9AFC`) in `control_authority`.
- Not bound: the launch arm `007C705C`.
- Diagnostics: `summary plane direction hold` per landing plane (arms, blend steps, maximum
  seconds), printed only when the switch is on, and `hold=` in the `land final trace` line.

With begin and final OFF nothing arms the hold, so the switch alone should be gameplay identical.
It is paired jointly with begin, final and abort, where the arm is reached.

### Predictions for LOMP10 9200/9000 and USN01 3200/3000, written before any ON run

OFF is the committed tree (the hold, begin, final and abort all OFF; the touchdown ON). ON flips
`kPlaneDirectionHoldBound`, `kLandBeginStateBound`, `kLandFinalStateBound` and
`kLandAbortStateBound`. The base for the landing numbers is 5m's pair (`local\l12_abon_lomp10.log`,
the same three switches without the hold).
1. **The hold is reached.** Every head that enters begin or final shows `arms > 0` and
   `blend_steps > 0`, with `max_seconds` between 0.3 and 0.8. The line-only wingmen show 0.
2. **The heads descend.** In the `land final trace`, Y falls through the old 4 to 16 m band
   instead of porpoising. There is no mode-2 dive back from above 1.4 x Length after the first
   mode-1 sample.
3. **They touch down.**
   - At least Warhawk 01 and one B-25 head reach `+BFCh <= WheelHeight` inside the rectangle,
     with `touchdowns > 0` in `summary plane touchdown`.
   - The touch-down vy is between -6 and -1 m/s, near or before T (local z -150 to -210).
   - A head that passes T before reaching the ground sinks steeper (`H` falls with `Z`), and may
     hit the vy gate (-6 m/s) and go through the runway: `td_refused_vy > 0` on such a head is
     the image's law, not a mechanism failure.
4. **After the touchdown.**
   - The plane is held at its touch-down point in state 4: the ground roll `007CBFA0` is not run.
   - final's on-ground half and park are refused, with `ground_refused > 0` on that head.
   - Abort is not entered from a landed plane.
5. **Deaths identical** (LOMP10 has none either side).
   - LOMP10 moves (exit 3): the landing heads' positions, and the IJN engagement moves 5l
     recorded.
   - USN01 is gameplay identical (exit 1): no head reaches begin in 3000 frames, as in 5l and 5m.

- **Mechanism failure:** any of:
  - a blend step on a plane not in mode 7;
  - `arms > 0` with `blend_steps = 0`;
  - a head in final that still porpoises 4 to 16 m over the runway for more than 10 s with
    `hold > 0` in its trace;
  - a touchdown outside the rectangle.
- **Flip rule.** The four flip ON together if at least one head touches down and no head in final
  flies on past T for more than 10 s. If the mechanism holds but only some heads land, the hold
  still flips ON (it is inert without begin), and begin, final and abort stay OFF with the reason
  recorded.

### The joint pair and the verdict (cc9-lua13, 2026-09-29): all four flip ON

OFF is `546fe663d` as committed (`local\l13_off_<row>.log`). ON is the same commit exported with the
four switches true (`local\l13_jon`, `bsp_game.exe` SHA-256 prefix `4E458B89288D`,
`local\l13_jon_<row>.log`). Both ran with the reference options on the console session, after a
300-frame USN01 smoke of the committed tree (`local\l13_smoke_usn01.log`).

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | deaths identical (0); the same eight IJN ships gain an engagement range as in 5j, 5l and 5m; B-25 01 6207.20 -> 7026.66 m (landed at 251 s) |
| USN01 3200/3000 | 1, gameplay identical | only the ship avoidance refill counter moves (known noise) |

**Touchdowns** (`plane touchdown:` lines; the local frame is the site's):

| head | first contact | touchdown | height / wheel | vy | local x, z |
| --- | --- | --- | --- | --- | --- |
| Warhawk 01 | 151.20 s, z -199.77, y 7.66 | 153.15 s | -0.164 / 0.00 | -4.06 | 0.9, -141.5 |
| Lightning 01 | 163.16 s, z -199.80, y 7.41 | 165.06 s | -0.084 / 0.00 | -3.99 | 0.7, -142.7 |
| B-25 01 | 250.01 s, z -198.54, y 6.70 | 251.16 s | 1.496 / 1.52 | -4.52 | 1.0, -159.1 |

Warhawk 01's traces show the mechanism. The hold seconds climb from 0.29 at A8 183.7 to 0.80 at A8
53.4. From the first mode-1 sample (Y 14.7 at Z 84.7) the flare hold is still +0.1047, but the
body velocity turns nose-high relative to the path: aoa 0.14 to 0.23, q 0.86 to 0.95. Y falls
14.7 -> 9.8 -> 5.8 -> 1.8 m, and the plane touches down 0.6 s after crossing T (Z -4.4 at 152.60 s). There is no climb
back and no porpoise.

**The predictions, one by one.**
1. **The hold is reached: held for the three heads that landed, missed for the two that aborted.**
   - Warhawk 01, Lightning 01 and B-25 01 show 88 / 87 / 74 arms, 172 / 170 / 144 blend steps,
     and `max_seconds` 0.800.
   - Lightning 01|.-4 and B-25 01|.-2 entered begin and final but aborted at A8 676.7 and 436.9
     m, outside the 260 m arm range. Their 63 and 57 `arms` are land/abort's clears
     (`009B0A3B`, 0.0 s), so `blend_steps = 0` is correct.
   - Each aborted at the moment its leader touched down (Lightning 01|.-4's first abort trace is
     165.41 s against Lightning 01's touchdown at 165.06 s) and went back to standby
     (`to_standby=1`). The landed plane stays on the runway because the ground roll does not run.
   - The line-only wingmen show 0: held.
2. **The heads descend: held.** Nothing porpoises, and nothing dives back from above
   1.4 x Length after the first mode-1 sample.
3. **They touch down: held,** inside the rectangle with vy -4.0 to -4.5, no vy refusals, and one
   low step each. Spread miss: the fighters touched down at local z -141.5 and -142.7, 8 m past
   the predicted -150 to -210.
4. **After the touchdown.**
   - State 4 and held still: held.
   - `ground_refused > 0` on a landed head: **missed**. The land task stops ticking at the
     touchdown: Warhawk 01 has 1494 task ticks, against 4463 for its wingmen, and
     `ground_refused=0`. So final's on-ground half is never reached in this host. Why the pilot
     pass skips a state-4 plane has not been read.
   - No abort from a landed plane: held.
5. **Deaths identical, LOMP10 moves, USN01 gameplay identical: held.**

No mechanism failure: every blend step is on a mode-7 plane (the commit), no steer arm is
without a blend, no final head porpoises with `hold > 0`, and every touchdown is inside the
rectangle.

**Verdict.** The flip rule is met: three heads touch down, and no head in final flies on past T.
`kPlaneDirectionHoldBound`, `kLandBeginStateBound`, `kLandFinalStateBound` and
`kLandAbortStateBound` flip ON together. The misses are recorded above and none of them is in the
mechanism.

## 5p. Begin's W is the pilot bot's age; the saturation is exact (packet `cc9_land_begin_w`, cc9-lua13, 2026-09-29)

`009B155A`-`009B15A9` reads `W = [(unit+DF4h) + 9Ch + [00F876B8] x 1Ch]` and uses it in
`interp(5 -> 0.01, 10 -> 1, W)`, the gain on `c1`. The later speed test uses
`interp(3 -> 1.1, 6 -> 2.5, W)` in the same way. The host takes both at `W >= 10`.

**What W is, from the listing.**
- `bot+84h` is an array of 1Ch-byte records, one per double-buffer index. `+14h` is a counted
  reference and `+18h` is a float.
- `0099A9E0` copies `+18h` and the reference from the previous record into the current one on
  the tick's non-think path (`0099AD2B`-`0099AD65`).
- `0099B181`-`0099B198`, at the end of every think, stores `bot+80h` into the current record's
  `+18h` (`[bot + idx x 1Ch + 9Ch]`, `idx = [00E0B6CC]`). Begin reads the same field through the
  previous index `[00F876B8]`, so W is at most one think old.
- `bot+80h` has two writers in `0099xxxx`:
  - the constructor `0099A880` (`0099A90E`), zeroing it;
  - `0099AD7E`, which adds each think's accumulated dt.

  So W is the pilot bot's age in seconds, as of its last think.
- `0099A880` is called from `007CA2AE`, `007D66E7` and `007D71FE` (rel32 scan), in the plane's
  init (`007C9770`) and property-bag reader (`007D5D20`). So the bot is as old as the plane.

**The substitution is exact on every row measured.** A land task is installed on the plane's
existing bot, so the bot is at least as old as the task. The host counts `young_bot` whenever begin
runs on a task younger than 10 s, and every head in `local\l13_jon_lomp10.log` shows
`young_bot=0`. Where `young_bot` is 0, W >= 10 and both interpolations are saturated, which is what
the host uses. A plane that begins a landing within 10 s of its own spawn would differ. None does on
LOMP10 or USN01. Nothing is bound and there is no pair; the source comment now cites this.

## 5q. The ground roll, stage A: the landed plane thinks, brakes and stops (packet `cc9_plane_ground_roll`, cc9-lua13, 2026-09-29)

**Why a landed plane froze.** The host ran the pilot think only inside the free-flight arm
(`pilot_think_and_commit` from `free_flight_007cc2f0`). Its ground-roll arm only counted. The
image's gate 8 (`0099AE5F`, `0074E230`) admits `unit+900h` 4 to 7, so a landed plane's bot keeps
thinking. That is why Warhawk 01's land task stopped at 1494 ticks in 5o.

**Read for stage A** (disk listing):
- **`007DB6D1`-`007DB73E`, the core law's mode-1 head.**
  - With the second argument 0.0 (the state is Locked), the law is skipped (`007DB6F7`).
  - Otherwise `dyn+98h..A0h = (0, 1, 0)` and `dyn+94h = WheelHeight - BFCh`.
- **`007DBEAA`-`007DC200`, the ground band** (mode 1 only; mode 0 takes the ceiling and mode 2
  the water band at `007DC205`). Frame slots:
  - `[F+24h]` is the latched throttle `unit+BBCh` (`007DB755`);
  - `[F+3Ch]` is the latched block `unit+BB0h` and `[F+40h]` the command block `unit+9E4h`
    (`007DB6CD`, `007DB6E4`, one push earlier). This also settles the doubt 5n's second pass
    left on `007D9140`'s arguments: the first pass's reading stands;
  - `[F+18h]` is the world speed (`007DBA88`, zero at `007DBC61`).

  The band:
  - `brake = classDesc+1E0h WheelBrake x max(unit+BC0h, 0.6 - throttle x 26.0)` (`00CEFF98`,
    `00D06880`);
  - the arrestor wire `ctl+ACh`, only with `unit+904h` set and above 0.1. Its only seeders are
    the constructor, `007DB2C0` and `007DB630`, the class-9 wire block of `007C71E0`;
  - `f = clamp(ctl+68h, WheelFrictionAccel/1, /2)` (`00415620`). `ctl+68h` is `dyn+84h`, the
    previous step's body forward acceleration `(v - dyn+70h) / step` (`007D8F18`, copied at
    `007DC756`);
  - a surface factor from the parent's velocity (`007DC0A2`-`007DC131`): 0.35 -> 1.0 over
    -5.56 -> 0 m/s, and 1.0 -> 1.3 over 1.39 -> 6.94 m/s. It is 1.0 for a static parent;
  - `friction = f x interp(StallSpd x WheelFrictionSpeed/1 -> WheelFriction,
    StallSpd x /2 -> 0, speed)`;
  - `dyn+48h += friction + brake`, `dyn+40h += |2 x ctl+3Ch|`, `dyn+0Ch -= wire`.

  `dyn+40h..48h` is the third resisting fold, which `007D8611` applies as `|q| x step` against
  the motion.
- **`007D87F6`-`007D8838`, `007D8CE9`-`007D8E1B` and `007D8E28`-`007D8F0E` in the integrator.**
  - While below the wheels, body vy is zeroed when `|vz| < 1e-5`.
  - The landed hold-down: with `dyn+C4h` (the latched `unit+904h`) and world vy >= -0.001, above
    3 m/s body vy -= 0.1 x speed, renormalised to the speed.
  - The contact projection: while below the wheels, the world velocity's downward component into
    `(0, 1, 0)` is removed, and the body velocity is rebuilt.
- **`007CBFA0` and `007DCCF0`:** as docs/PLANE_GROUND_OPS.md 3 and 4. Without contact and off a
  path, the ground arm runs the free-flight step. The lift-off request needs
  `BFCh - WheelHeight > 0.1` and world vy > 0.1.
- **Final's on-ground half, `009B1FEA`-`009B207A`**, with `EBX` = 1 (`009B1F0B`) and `EDI` = 0
  (`009B1EDF`):
  - pitch 0.0 active, with `+2D0h` = 0;
  - bank 0.0, with `+2CCh` = 1;
  - throttle 0.0 active;
  - air brake 1.0 when `approach+30h` answers `vtable[5Ch](9)`, else 0.2, active, with
    `+2D8h` = 0;
  - the done tests at `009B2080`, which need `007B8D70` false.

**The binding, behind `kPlaneGroundRollBound` (committed OFF).**
- `ground_roll_007cbfa0` runs, in order:
  - the pilot pass (labelled position: the arm's head, as in free flight);
  - the probe `007C5AC0`;
  - `007DCCF0`, with the core law in mode 1.
- The core law and its integrator are now one method, `run_core_law_007db680(step, ground)`, shared
  with the free-flight arm. Free flight passes `ground = false`, and the result is unchanged there:
  the new fold is zero and the acceleration store is only read by the band.
- `ground_band_007dbeb3` is in `src/plane_flight.cpp`.
- Final's on-ground half is bound under the same switch.
- **Not carried, labelled:**
  - the wire (no class-9 holder in this host);
  - the surface factor (1.0 for the static airfield);
  - the class-9 brake of 1.0;
  - steps 2 to 5 of `007CBFA0`;
  - the lift-off message (counted);
  - the runway steering band `007DA380` and the rate law's ground arm `007DA542`: the planner and
    rate law run their free-flight arms;
  - the ground pose `ctl+80h..8Ch`, `007D9C80` and `007D80C0`;
  - park, which stays refused. That is stage B.
- Diagnostic: `summary plane ground roll` per landing plane.

### Predictions for LOMP10 9200/9000 and USN01 3200/3000, written before any ON run

OFF is the committed tree: 5o's state, with the ground roll OFF. ON flips `kPlaneGroundRollBound`.
1. **The landed heads think again.** Warhawk 01, Lightning 01 and B-25 01 run final's on-ground
   half (`land_final_ground_ticks > 0`, `ground_refused = 0`). The rule asks for park on every think
   (`park_refused > 0`); park stays refused.
2. **They brake and stop on the runway.**
   - `law_steps > 0` and `band_steps > 0`.
   - `stop_t` falls within 6 s of each touchdown, with a roll of 10 to 150 m.
   - The brake is `WheelBrake x 0.6` once the latched throttle is below 0.02, and
     `WheelBrake x 0.2` before that.
   - `hold_down > 0` and `contact > 0`, with no lift-off request (`liftoff_req = 0`).
   - The planes stay on the runway: `free_steps = 0`.
3. **The followers still abort.** A stopped plane still occupies the site: nothing releases the
   occupancy until park and the taxi run. Lightning 01|.-4 and B-25 01|.-2 abort as in 5o.
4. **Everything else is 5o's.** Deaths are identical. LOMP10 moves (exit 3) only in the landed
   planes' positions and whatever reads them. USN01 is gameplay identical (exit 1).
- **Refactor check:** the committed OFF log against 5o's ON log (`local\l13_jon_lomp10.log`, the
  same switches before the refactor) is gameplay identical.
- **Mechanism failure:** any of:
  - a landed head still moving at 32 m/s 10 s after touchdown;
  - a lift-off request;
  - a landed plane sinking below its wheel height by more than 1 m (`min_bfc`);
  - `free_steps > 0` on a stopped plane.
- **Flip rule:** ON if the heads stop on the runway without a mechanism failure. Stage B (park and
  the occupancy release) follows either way.

### The pair and the verdict (cc9-lua13, 2026-09-29): stage A flips ON

OFF is `33b530f29` as committed (`local\l13_goff_<row>.log`). ON is the same commit exported with
`kPlaneGroundRollBound` true (`local\l13_gon`, `local\l13_gon_<row>.log`).

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | deaths identical (0); B-25 01 7026.66 -> 7017.44 m |
| USN01 3200/3000 | 1, gameplay identical | - |
| refactor check: 5o's ON log against this OFF | 1, gameplay identical | only LOMP10 presentation noise moves |

`summary plane ground roll`:

| head | touchdown | stop | roll | WheelBrake / last brake | min BFCh / wheel | hold-down, contact steps |
| --- | --- | --- | --- | --- | --- | --- |
| Warhawk 01 | 153.15 s | 154.00 s | 15.1 m | 80 / 48.0 | -0.164 / 0.00 | 5, 16 |
| Lightning 01 | 165.16 s | 166.26 s | 4.5 m | 80 / 48.0 | -0.029 / 0.00 | 4, 20 |
| B-25 01 | 251.66 s | 254.41 s | 24.4 m | 10 / 6.0 | 1.393 / 1.52 | 0, 1511 |

All three show `law_steps = arm_steps` (3967 to 5937), `free_steps = 0` and `liftoff_req = 0`.
They stay in state 4 with the land task in final (`land_state` `5F8h`, 2144 to 3155 final ticks,
`touched=1`, `ground_refused=0`).

**The predictions, one by one.**
1. **The landed heads think again: held.** Final's on-ground half runs, with `ground_refused = 0`.
   The prediction named the wrong counter for the park request: the rule's first arm
   (`009B3D38`, `+900h` 4 or 5) refuses park before the final arm is reached, so
   `park_refused` stays 0 and the refusal is counted in `land_refused_states`.
2. **They brake and stop on the runway: held.**
   - Each stops within 2.8 s of its touchdown, with no free-flight step and no lift-off request.
   - The brakes are `WheelBrake x 0.6`, as predicted once the throttle is closed.
   - The deepest point is 0.16 m below the wheels.
   - Spread miss: Lightning 01 rolled 4.5 m, under the predicted 10 to 150 m. It touched down at
     20.7 m/s instead of 5o's 32.3 (see below).
3. **The followers still abort: held.** Lightning 01|.-4 and B-25 01|.-2 abort to standby as in 5o.
4. **Deaths identical, USN01 gameplay identical: held.**
   - LOMP10 moves in the landed planes and in **Lightning 01's approach**: its lines are identical
     to OFF up to 160.61 s and differ from 161.61 s.
   - At 161.61 s the pitch is -0.28 against -0.10, with a sideslip of 1.69 m/s. Lightning 01 is
     then 81 m short of T, approaching the runway on which Warhawk 01 now stands at rest, 15 m
     further on than in OFF.
   - OFF holds Warhawk 01 frozen, with its velocity left at 32.4 m/s (the follow trace's
     `leader_spd`); ON rolls it to a stop at 0.0.
   - So the one input that differs is the stopped plane's position and velocity. The term that
     reads it (a neighbour or avoidance term in the planner) was not identified here. Lightning
     01 still lands on the runway 3.5 s later, at 20.7 m/s.

No mechanism failure: nothing still rolls at speed after 10 s, there is no lift-off request, no
plane sinks more than 0.16 m below its wheels, and there are no free-flight steps.

**Verdict.** `kPlaneGroundRollBound` flips ON. Stage B is park's taxi (`009B22C0`-`009B2C68`)
with the site calls `006CF420`/`006CF520`/`006CF5B0`, and whatever empties the site's occupancy
vector so the followers are not aborted.

## 5r. The followers abort because the host drops the landed arm of `006C7960` (packet `cc9_landing_follower_spacing`, cc9-lua14, 2026-09-29)

### Which arm took Lightning 01|.-4: not the spacing arm

The ON log of 5q (`local\l13_gon_lomp10.log` in cc9-lua13) shows the sequence at the leader's
touchdown:

- `plane touchdown: unit=Lightning 01 t=165.16 ... landed904=1`.
- 165.31 s: `Lightning 01 (head) mode 4 -> 2 path=999999.0 spacing=1.000`.
- 165.31 s: `Lightning 01|.-4 mode 4 -> 1 path=1000000.0 spacing=0.000`, and its siblings `.-2` and
  `.-3` go from mode 3 to mode 1 on the same pass.
- 165.41 s: `.-4` enters land/abort.

The follower's mode 1 does not come from its spacing. It comes from `006C7960`'s `headMode == 2`
arm: the head's record was given mode 2 because the host treated the landed head as "not airborne,
leader pass". Warhawk 01's followers at 153.40 s and B-25 01|.-2 at 251.71 s show the same pattern.
The spacing of 0 printed with it is `006C3F80`'s answer after the mode had already dropped to 1.
(The k = 0 site arm would have given `.-4` about 1.0: tp = 708 / 68 = 10.4 s, over 0.25 x 9 s.)

### `006C7960`'s first arm, read from the live decompile

```
if (plane+904h != 0) { rec+10h = 4; rec+4h = [00D7A260] (-1.0); rec+8h = [00D7A24C] (1.0); return; }
if (!(plane+72Ch)->vtable[38h]() || rec+0Ch) { rec+10h = leaderPass + 1; rec+4h = 999999.0; rec+8h = 1.0; return; }
```

- The landed byte `plane+904h` is tested **before** the airborne test, on both passes. A plane that
  has landed keeps mode 4, with a path of -1.0 and a spacing of 1.0.
- The host's version (`landing_mode_006c7960`) carried the comment "+904h (landed) is clear in this
  host". That was true until `kPlaneTouchdownBound` went ON: `007CB5F0`'s port now sets
  `plane_landed_904` at every touchdown.
- Consequences in the image, on the head's touchdown:
  - The head's record says mode 4 and path -1. On the follower pass, `headMode` is 4 and
    `headDist + 1.0` is 0, so the followers take the last arm: mode 4 when the deck is usable, the
    follower is in the corridor and its previous spacing is positive; otherwise mode 3. Nobody drops
    to mode 1.
  - In `006C3F80` the landed head is not counted. It fails the airborne test at
    `006C412C`-`006C413F`, and its path of -1.0 would also fail the `+4h > 0` test at
    `006C4167`-`006C4178`. A follower in mode 4 with no other mode-4 record ahead takes the k = 0
    site arm, over the stamp `006CE230` set at the head's touchdown. This is where the image's own
    spacing rule applies: the follower is zeroed only when tp < 2.25 s and less than 3.6 s have
    passed since the last touchdown (FollowDistTime 9, this installation's tuning).
- The same byte gates two helpers the host also took as clear:
  - The corridor `006C3B10`: the `+900h` 2-or-4 refusal applies only while `+904h` is clear. A
    landed plane (`+900h` 4) falls through to the not-airborne answer, true.
  - The circle test `006C5C40`: a landed plane answers false.
  - The lookup-miss arm of the land task's approach update (`006C5534`-`006C5560`): the mode is
    `+904h ? 4 : 1`.

### Site occupancy (`site+34h`/`+38h`)

- The vector is appended by `006CED90` (vtable `+24h`) at the touchdown.
- Its readers in the airfield site's vtable `00CF89F8` (21 slots, all decompiled and searched for
  `+34h`/`+38h`) are only two:
  - `006CF5B0` (`+34h`, the taxi spot test). It refuses when another plane **with `+904h` clear**
    is within 30 units, so landed planes never block one another.
  - `006CF2A0` (`+10h`), a visitor that walks the vector (the save or serialise pass).
- No slot removes an occupant. The occupancy therefore plays no part in a follower's abort or in its
  landing clearance. The host keeps it append-only without a reader, and that stays correct until
  park's taxi (`006CF5B0`) is bound.

### The binding, behind `kLandingLandedArmBound` (committed OFF)

When the switch is on:

- `landing_mode_006c7960` takes the landed arm first.
- `landing_corridor_006c3b10` skips the 2-or-4 refusal for a landed plane.
- `landing_on_circle_006c5c40` answers false for a landed plane.
- The approach update's lookup-miss arm writes mode 4 for a landed plane.

Nothing clears `plane_landed_904` in this host (the image's clear, at a relaunch, is not read). This
is labelled: a relaunched plane would keep the byte set. No plane relaunches in the reference rows.

### Predictions for LOMP10 9200/9000 and USN01 3200/3000, written before any ON run

OFF is the committed tree. ON flips `kLandingLandedArmBound`.

1. **Heads keep mode 4.** At each touchdown (Warhawk 01 about 153 s, Lightning 01 about 165 s,
   B-25 01 about 252 s) the head logs no `mode 4 -> 2` line; its summary ends with mode 4. Its land
   task is unchanged: the rule's first arm (`+900h` 4) still refuses park, and the roll and stop
   times match OFF.
2. **No follower drops to mode 1 at its head's touchdown.**
   - Lightning 01|.-4 (mode 4, about 708 m out) does not abort at 165.41 s. It lands about 9 to 14 s
     after its head (touchdown at 174 to 180 s), on the runway where Lightning 01 stands.
   - B-25 01|.-2 (mode 4 since 241.5 s) does not abort at 252.01 s. It lands about 5 to 12 s after
     B-25 01 (257 to 264 s).
   - This host has no plane-to-plane contact on the ground, so a follower rolling through a stopped
     head is expected and is not a failure. The image would have taxied the head off in park,
     which stays refused.
3. **The mode-3 followers keep mode 3.** These are Warhawk 01|.-2/.-3/.-4 and Lightning 01|.-2/.-3.
   They fly the approach line and enter mode 4 when they reach the corridor. At least one more
   follower touches down before 450 s. The order and times are not predicted.
4. **Deaths identical (none on LOMP10); USN01 gameplay identical (exit 1)**, with no touchdown in
   that row. LOMP10 moves (exit 3) in the followers' paths and anything that reads them, including
   whatever moved Lightning 01's approach in 5q.
- **Mechanism failure:** any of:
  - a follower in mode 1 or 2 on the pass right after its head's touchdown;
  - a landed plane whose record is not mode 4 / path -1.0;
  - a follower in mode 4 whose spacing is zeroed while tp is at least 2.25 s.
- **Flip rule:** ON if predictions 1 and 2 hold without a mechanism failure. A spread miss on the
  touchdown times is recorded and does not block.

### The pair and the verdict (cc9-lua14, 2026-09-29): the landed arm flips ON

- OFF is `fc783360c` as committed (`local\l14_loff_<row>.log`).
- ON is the same commit exported with `kLandingLandedArmBound` true (`local\l14_lon`,
  `local\l14_lon_<row>.log`).
- A 300-frame USN01 smoke ran first (`local\l14_smoke_usn01.log`). It shows the module directory,
  `present interval immediate` and the final COM release.

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | deaths identical (0); B-25 01 7017.44 -> 7019.11 m; `landed=8064` on the deck line |
| USN01 3200/3000 | 1, gameplay identical | no touchdown in the row |

Touchdowns on CB4_AF in ON (OFF had only the three heads, at 153.15, 165.16 and 251.66 s, and two
aborts):

| plane | touchdown | stop | roll |
| --- | --- | --- | --- |
| Warhawk 01 | 153.15 s | 154.00 s | 15.1 m |
| Lightning 01 | 165.16 s | 166.26 s | 4.5 m |
| Warhawk 01\|.-4 | 179.06 s | 180.16 s | 4.1 m |
| Lightning 01\|.-4 | 189.76 s | 190.76 s | 3.8 m |
| Warhawk 01\|.-2 | 202.01 s | 203.11 s | 4.1 m |
| Lightning 01\|.-2 | 212.21 s | 213.46 s | 3.9 m |
| Lightning 01\|.-3 | 222.81 s | 223.86 s | 3.7 m |
| Warhawk 01\|.-3 | 233.61 s | 234.91 s | 4.2 m |
| B-25 01 | 254.86 s | 257.56 s | 25.2 m |
| B-25 01\|.-2 | 267.41 s | 270.06 s | 40.1 m |

All ten planes of the three squadrons land, about 10 to 12 s apart, with no abort. The deck line
counts mode 4 for 8850 passes, against 261 in OFF.

**The predictions, one by one.**
1. **Heads keep mode 4: held.**
   - No head logs `mode 4 -> 2`.
   - Every landed plane's record ends at `mode=4 path=-1.0 spacing=1.000`.
   - Warhawk 01 and Lightning 01 stop at OFF's times.
   - B-25 01 lands 3.2 s later than in OFF, because the queue ahead of it changed.
2. **No follower drops to mode 1 at its head's touchdown: held. The times are a spread miss.**
   - Lightning 01|.-4 does not abort. It was still in mode 3 at 165.16 s: Warhawk 01|.-4 had taken
     mode 4 at 156.10 s and counted ahead of it. It entered mode 4 at 166.21 s and landed at
     189.76 s, not at the predicted 174 to 180 s.
   - B-25 01|.-2 lands 12.6 s after its head. The prediction was 5 to 12 s.
3. **The mode-3 followers keep mode 3, then land: held.** All five land. The site spacing zeroed
   nobody whose tp was at least 2.25 s.
4. **Deaths identical, USN01 gameplay identical: held.**
   - LOMP10 moves in the followers' flights and in what reads them: the world unit lists, the ship
     AI's autotarget candidates and eight Japanese ships' nearest-target ranges.
   - It also moves in the GUI text and ocean call counts, and in the removal of the abort calls
     (`BotStateLandAbort::tick` 120 -> 0).

No mechanism failure.

**A defect this exposes (not this switch's mechanism).** B-25 01|.-2 stops at 270.06 s on top of
B-25 01. It then creeps sideways at about 0.2 m/s. At 325.94 s it leaves the runway strip, at
local x = -10.00 against a half-width of 10.00 (the diagnostic `plane ground contact lost`, added
here). From then on it takes free-flight steps (`free_steps=1658`) with no floor, and it sinks to
`min_bfc=-1047` by the end of the row.

- The image would not reach this state. Park (refused here) taxis a landed head off the runway
  before the next plane arrives.
- Off the strip, the image's airfield ground surface (`006CF180`) still carries the plane, and this
  host does not model that surface.
- What pushes a stopped plane sideways was not identified. The candidate is the same unidentified
  planner term that reads a neighbour's position (5q verdict 4).

**Verdict.** `kLandingLandedArmBound` flips ON. Park's taxi (packet `cc9_land_park_taxi`) is next.
The sinking of a plane that leaves the strip is recorded in section 6.

## 5s. `land/park`: the taxi to the hangar (packet `cc9_land_park_taxi`, cc9-lua14, 2026-09-29)

### What was read (disk listing, whole)

- **The rule** `009B3CF0`.
  - Head arm `009B3D0B`-`009B3D3B`: `+900h` 4 or 5 while the state is neither park (`+620h`) nor
    abort (`+64Ch`) enters park.
  - In park the rule has no arm (`009B3E02` -> `009B3EA7`).
  - `009B3770`'s arm for "any other state": park's done byte enters abort (`009B37A7`).
  - Abort's `+66Dh` enters park (`009B3E3B`).
- **Enter `009B21A0`**: `+1Ch` = 0.0, `+18h` = 0, `+28h` = 3.0. **Exit `009B21C0`**: `+18h` = 0.
- **The tick `009B22C0`-`009B2C68`** (`__thiscall(park, dt)`, `RET 4`). The ESP offsets were
  tracked from `SUB ESP,3Ch` and the four pushes; the arguments of `00419010` were taken from its
  pushes.
  - `[approach+1Ch]+40h` = 0.0.
  - **Done when attached with the owner gone:** attached means `+BF8h` and `+BF4h`; gone means
    the owner null or `+5Dh` set (`009B22DC`).
  - `approach+B4h` = 0. Bank 0 with `+2CCh` = 1. Pitch 0 active with `+2D0h` = 0.
  - **The two site calls:**
    - `q` = `006CF520`: point 0 of `006D2780`'s entry path, in the airfield frame. Only its z is
      kept.
    - `t` = `006CF420`: the last point of `006D2640`'s exit path, or the plane's **world** position
      when no hangar qualifies.
  - `(px, pz)` = plane `+A4h`/`+ACh`. After `007C71E0`'s re-parent (5k) these are the position in
    the airfield frame.
  - **The carrier flag** is `approach+30h vtable[5Ch](9)`. This section covers the airfield arm
    (flag clear).
  - **Retarget:** when not attached and the target moved more than 10 m (`|task+20h/+24h - t|^2 >
    100`, `00D7A220`), call `007B9000` (`+C01h` = 2), then `007C1680`. Then `task+20h/+24h = t`.
  - `009B21D0`, the done test:
    - it is skipped on the path (`+900h` 5) and when the byte is already set;
    - unless `+28h >= 0` and the plane is attached, it sets done;
    - otherwise it sets done when `t.z < pz` (`009B1E30` is false for an airfield), or when
      `|00438B10(heading, holder+88h)| > pi/2`.
  - `dx = t.x - px`, `dz = t.z - pz`, `v = vtable[38h]` (the speed).
    `base = AirField/MoveSpd` (`+184h`, KMH 35).
  - **The speed request:**
    - `hi = max(base, 007C4830 x 0.6)`, where `007C4830` is StallRangeMax x StallSpd;
    - `spd = 00419010(5 base, base, 10 base, hi, dz)`;
    - beyond `dz > 100` it is `max(spd, 0.85 v)`.
  - **`plane+910h` = 1** when `q.z - pz` is under 20, or under 28 for a class 10h/16h plane
    (`00D1FF80`).
  - **Slow** means `v <= 1.5 base`. When slow:
    - `rate = max(RunwayYawTurnSpdMul x class+1B0h YawSpd, AirField/MinTurnSpd)`;
    - `qd = (1.5 / rate) x RunwayYawTurnSpdLimit/1`;
    - when `dz < qd + 30`, `spd` is set to RunwayYawTurnSpdLimit/1;
    - when also `dz <= qd`, the plane is on the path.
  - **Joining and leaving the path:** `007C16F0` (4 -> 5) or `007C1680` (5 -> 4) whenever
    `+900h == 5` disagrees with that.
  - **The steer vector `(sx, sz)` and the distance `f18`:**
    - Not slow: `(0, dz)`, with `f18 = dz`.
    - Slow and on the path: `(clamp(dx, +-15), dz)`, with `f18 = |dx|` (`00415690`). When
      `|dx| < 3`, `007B96C0` sets `+C00h` = 1 and calls `00951F40(0)`, which detaches the spatial
      node and hides it: the plane is in the hangar.
    - Slow and off the path: `w = (holder+B0h - class+A4h Width) / 2`,
      `e = max(|px| - w, 0)` signed toward the centre line, and `(e, min(dz, max(3, 18 - |e|)))`,
      with `f18 = dz`.
  - **The spot test:** off the path with `+910h` clear, `006CF5B0(plane, 1, t.x, t.z)`. A false
    answer zeroes `spd`.
  - **Heading:** `des = wrap(pi/2 - atan2(sz, sx))`, and the plane's own heading comes from
    `+94h`/`+9Ch` in the same form. `err = 00438B10(des, cur)`.
  - **The speed scale:** `a = min(|sx| x 5 + 30, 80)` degrees and
    `spd *= 00419010(3 deg, 1.0, a, 0.1, |err|)`.
  - **The yaw:**
    - It is computed on the path, or when `f18 > 0.5` and (`spd > 0` or `v > 1.389`):
      `00419010(-YawSpd/2, -1.1, YawSpd/2, 1.1, err)`.
    - It is then shrunk toward 0 by a deadband `max((5 - f18) x 0.2, 0.1)`.
    - Otherwise the yaw is 0.
  - **Outputs:** yaw desired and active with `+2D4h` = 0; speed `+2B4h` with `+2B0h` = 1 and
    `+2D8h` = 1.
- **`006CF5B0`** (`RET 10h`):
  - With the enable byte, `+904h` set and `+910h` clear, a plane 15 to 35 + v m short of the target
    z is refused when an occupant with `+904h` clear (an arrival) is within 30 m of that z.
  - **Every other path ends in `006CE610`.**
- **`006CE610`:** for each occupant `o` with `006CDF70(plane, o)`:
  - it answers false if `006CDF70(o, plane)` is false;
  - otherwise it takes the holder-frame `z' = L/2 - l.z` and `x' = -l.x` (`006BEFF0`), and
    answers false when `o.z' < plane.z' - 1.5`, or when `o.z'` is within 1.5 with `|o.x'| <=
    |plane.x'|`.
- **`006CDF70(a, b)`** (`__stdcall`, `RET 8`) answers "b is in a's way". It is false when a is
  airborne and b has landed. Otherwise:
  - `D = max(h, g x max(f, |v_a| - 4.1667)) + a.Length / 2`, where:
    - `f` = 1.8 when b is a 10h/16h class, else 0;
    - `g` = 5 for a 10h/16h class, else 2;
    - `h` = 12 for a 10h/16h class, else 2.
  - `l` = b in a's frame, and `R = D + b.Length / 2`.
  - `z = l.z`, forced to -1 for a squadron mate with a higher `+9D8h` when `0 <= z < 1`.
  - For b landed, a not, and a off any holder: `R = z + 10` and `x = max(0, |l.x - 10|)`. The zero
    guard reads a denormal double at `00CF8918`, so it never fires.
  - The answer is `z >= 0 && R > z && |x| < b.Width x 0.8 + a.Width / 2`.
- **`007C16F0`/`007C1680`:** `+900h` 4 <-> 5, `+C04h` = -1.0, and with `classDesc+198h`,
  `+904h = +908h > 5` and `+C18h` = 3. `007C11E0(0)` is not carried.
- **The hangars:** `006D5220` kind 1 reads `"Hangar 1".."Hangar 10"` sub-bags.
  - This installation's LOMP10 scene (`universe/scenes/missions/usn/LOMP/10_san_jose.scn`, mtime
    2024-08-09) authors one hangar on CB4_AF:
    - `CB4_AF_Hangar`, with `CB4_AF_entrypath1` and `CB4_AF_exitpath1`;
    - three points each.
  - By hand from the scene frames, the airfield-frame points are:
    - the exit path's last point at about (44.4, -20.6);
    - the entry path's point 0 at about (44.3, -20.0).
  - The runway runs along +z with |z| < 200, and the planes stop at z = -125 to -150.

### The binding, behind `kLandParkStateBound` (committed OFF)

- **Bound:**
  - the rule's three arms, the enter and the exit;
  - the tick's airfield arm, with `009B21D0`;
  - `006CF420`/`006CF520` over the scene's hangars;
  - `006CF5B0`, `006CE610` and `006CDF70`;
  - `007C16F0`/`007C1680`;
  - `+910h`, `+C00h`, `+C01h`.
- **Not carried, labelled:**
  - The carrier arm.
  - `00951F40`'s hide: `+C00h` is set and counted, and the plane stays visible and simulated.
  - Hangar `object+370h > 0` is read as "the hangar unit is not dead".
  - The re-parent: positions and headings go through the airfield frame.
  - Abort's on-ground arm stays refused. It still sets `+21h`, which now leads back to park.
  - The ground steering laws: the runway steering band `007DA380` and the rate law's ground arm
    `007DA542`. The yaw request reaches the free-flight rate law, as in 5q.
- **New lines:**
  - `land park trace` every 20 park ticks;
  - `summary land park` per plane;
  - `air ops hangar` at scene load.

### Predictions for LOMP10 9200/9000 and USN01 3200/3000, written before any ON run

OFF is the committed tree. ON flips `kLandParkStateBound`.

1. **Every landed plane enters park at the think after its touchdown.** That is all ten on CB4_AF,
   with `no_hangar=0`.
2. **The heads no longer stop at 154/166/257 s.**
   - They keep rolling, at up to `hi` while more than about 97 m short of z = -20.6. The request
     then falls to MoveSpd (9.7 m/s), and to RunwayYawTurnSpdLimit/1 (6.9 m/s) within 35 m.
   - They join the path (`joins >= 1`) about 5 m short, near local z = -25.
3. **On the path they turn right toward x = 44 and taxi into the hangar** (`hangar > 0`, `c00=1`),
   each within 60 s of its touchdown.
   - This depends on the yaw request turning a plane on the ground through the free-flight rate
     law. That law is not the image's ground law.
   - If the planes do not turn (they creep straight at the scaled speed of about 0.7 m/s), that is
     recorded as the missing ground steering, not as a misreading of park.
4. **Followers queue behind a plane still on the runway ahead.** `spot_refused > 0` for at least
   one follower. None of them runs through a plane ahead.
5. **B-25 01|.-2 no longer stops on top of B-25 01**, so its 325.94 s contact loss and sink
   (5r) do not happen.
6. **Deaths identical (0); USN01 gameplay identical (exit 1).** LOMP10 moves (exit 3).

- **Mechanism failure:** any of:
  - a plane in park with `no_hangar > 0`;
  - a park <-> abort loop (`from_abort > 5`);
  - a lift-off request or a sink below the wheels by more than 1 m;
  - a plane passing the target z in state 4 (`done > 0`).
- **Flip rule:**
  - ON when 1 and 2 hold and at least the heads reach the path without a mechanism failure.
  - If the turn fails only for lack of the ground steering, park stays OFF and the ground laws
    come next.

### The pair and the verdict (cc9-lua14, 2026-09-29): mechanism failure, park stays OFF

- OFF is `a2551c4a3` as committed (`local\l14_poff_<row>.log`).
- ON is the same commit exported with `kLandParkStateBound` true (`local\l14_pon`,
  `local\l14_pon_<row>.log`).
- **Refactor check:** 5r's ON log against this OFF is gameplay identical (exit 1). Only the new
  `air ops hangar` line moves.

| row | `pair_diff` | note |
| --- | --- | --- |
| LOMP10 9200/9000 | 3 | deaths identical (0) |
| USN01 3200/3000 | 1, gameplay identical | - |

The scene's hangar is found: `air ops hangar: unit=CB4_AF Hangar 1 object=CB4_AF_Hangar
entry=CB4_AF_entrypath1 exit=CB4_AF_exitpath1`. The target and the queue origin come out at
(44.4, -20.6) and z = -20.0, as computed by hand.

**The predictions, one by one.**
1. **Held.** All ten planes enter park at the think after their touchdown, with `no_hangar=0`.
   The logged lengths and widths are:
   - P-40: 10.6 by 12.0;
   - P-38: 11.6 by 12.0;
   - B-25: 17 by 21.
2. **Held for Warhawk 01, failed for the others.**
   - Warhawk 01 rolls on from -141.5, at 27.6 m/s requested. It slows to the 6.94 m/s request
     inside 35 m, sets `+910h` at z = -36.6 and joins the path at z = -27.2 (165.21 s).
   - Lightning 01 enters park with a heading error of 0.236 rad. It gets full right yaw (1.0), yet
     drifts left: x = -1.7, -6.7, then -11.0 at 171.71 s, with the error growing to 0.73.
   - It then leaves the 20 m strip, loses contact, and `009B21D0` sets done.
3. **Mechanism failure.**
   - On the path Warhawk 01 turns very slowly: its error stays at 1.3 to 1.4 rad for 60 s. It
     reaches `|dx| < 3` at 225.11 s at local (41.5, -1.0) (`007B96C0`).
   - It is still simulated, so it drives on (x = 51 at 243 s), leaves the path and falls into the
     loop below.
4. **Queueing held.** `spot_refused` is 2 and 5 for two followers.
5. **Moot.** Every plane leaves the strip.

**The mechanism failure is the park <-> abort loop.**
- Done sends the plane to abort. Abort's tick head requests throttle 1.0, and its on-ground arm
  (refused) sets `+21h`, which sends it back to park. Park is done again at once.
- The planes accumulate 750 to 1400 loops each (`from_abort`). The loops' throttle pulses drive
  them off the airfield at up to 74 m/s: Lightning 01 is at local z = 700 by 191.7 s and at
  22 km by the end.
- There are lift-off requests (111 and 144 on two planes), and one sinks to `min_bfc = -45`.

**Why.** The park reading is not the part that fails. The failure is in what this host lacks
underneath it:
1. **The ground steering.** The yaw request reaches the free-flight rate law, because the runway
   steering band `007DA380` and the rate law's ground arm `007DA542` are not carried (5q). There
   it does not hold a heading on the ground, and on Lightning 01 it turns the wrong way.
2. **Abort's on-ground arm** (`009B0E74`-`009B0F93`: pitch `class+1ECh x 0.5` and a ground
   heading) is refused. Only its throttle 1.0 and `+21h` act.
3. **`00951F40(0)`**, the hangar's detach and hide, is not carried. A plane in the hangar keeps
   driving.

**Verdict.** `kLandParkStateBound` stays OFF, recorded as a mechanism failure. The reading
(the tick, `009B21D0`, the site calls and the scene's hangars) stands and is kept behind the
switch. Next, in order: the ground laws `007DA380`/`007DA542`, abort's on-ground arm, and
`00951F40`'s effect on a hidden plane. Then re-pair park.

## 5t. The ground steering: the rate law in controller mode 1 (packet `cc9_plane_ground_steering`, cc9-lua14, 2026-09-29)

**The tag.** `ctl+FCh` is written by each law's entry, not by a selector
(docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md): `007DCD24` in the ground law `007DCCF0` stores 1.
This host ran the rate law with mode 0 under the ground law too (5q), and that has three
consequences:
- the roll term survives, where `007DA8D9` zeroes it in mode 1;
- the bank-yaw coupling stays on, where `007DA380`'s byte is 0 in mode 1;
- the yaw factor is the free-flight authority.

**`007DA380`'s mode-1 arm, `007DA542`-`007DA6E3`**, read whole from the disk listing. The frame
was tracked through the three `00419010` calls, which are `RET 14h`:

- **The first two outputs:** `*outA` = the authority (`007D9A70`); `*outB` = 1.0.
- **Below the band** (`ctl+6Ch v < [00F8738C]`, tuning `+2ACh`, which holds
  RunwayYawTurnSpdLimit/1 in this build):
  - `r = max(YawSpd x RunwayYawTurnSpdMul, 0.87266 (00D057E0) / YawSpd)`;
  - `r` is halved for a 10h/16h class with `+904h` set (`007DA5D8`);
  - `blend = 00419010(Limit/1, r, Limit, 1.0, v)`;
  - `a = 00419010(0.01, 15.0, 0.1, 6.0, thr) / 3.6` and
    `b = 00419010(0.001, 6.0, 0.1, 1.0, thr) / 3.6`, where `thr` is `unit+9F0h` and 3.6 at
    `00D06588` converts km/h to m/s;
  - `*outB = blend x 00419010(b, 0, a, 1.0, v)`. There is no steering authority below `b`
    (1.7 m/s with the throttle closed) and full authority above `a` (4.2 m/s closed).
- **The byte:** `*outByte` = 0.

`outB` is the yaw factor at `007DA95B` (the turbo path's overwrite at `007DA7ED` needs
`unit+5Dh`).

**Binding, behind `kPlaneGroundSteeringBound` (committed OFF).** Under the ground law
(`run_core_law_007db680(step, true)`), `control_step_007da710` passes:
- controller mode 1;
- `outB` as the second factor;
- no coupling.

The diagnostic is `summary plane ground steering`.

### Predictions for LOMP10 9200/9000 and USN01 3200/3000 (park OFF), written before any ON run

1. **Identical up to Warhawk 01's touchdown (153.15 s).** The switch acts only under the ground
   law. All ten landed planes then show `steps > 0`.
2. **The roll-outs still stop on the runway.**
   - Each stop is within 1 s and 10 m of OFF's stop.
   - No landed plane leaves the strip. There is no `plane ground contact lost` line.
   - B-25 01|.-2's creep stops, and with it the 325.94 s sink of 5r: `min_bfc > -2` for every
     plane.
3. **Deaths identical; USN01 gameplay identical (exit 1).** LOMP10 moves (exit 3) after the
   first touchdown.

- **Mechanism failure:** any of:
  - a landed plane rolling more than 20 m after its OFF stop, or off the strip;
  - a lift-off request;
  - a sink below the wheels of more than 1 m.
- **Flip rule:** ON if predictions 1 and 2 hold.

### The pair and the verdict (cc9-lua14, 2026-09-29): mechanism failure, stays OFF

- OFF is `f187ab8b1` as committed (`local\l14_soff_<row>.log`).
- ON is the same commit exported with `kPlaneGroundSteeringBound` true (`local\l14_son`,
  `local\l14_son_<row>.log`).
- USN01 is gameplay identical (exit 1).
- LOMP10 moves (exit 3), with deaths identical (0).

**The predictions, one by one.**
1. **Held, with a small miss.** All ten landed planes run the mode-1 rate law, with `steps` equal
   to their law steps. Warhawk 01|.-3 and B-25 01 touch down 0.05 s off OFF's times: a stopped
   plane's position reaches the approach planner, as in 5q verdict 4.
2. **Failed.** The heads and three followers stop as before (rolls of 3.8 to 15 m). But five
   stopped planes now lose contact, against one in OFF:
   - B-25 01|.-2 at 309.20 s;
   - Lightning 01|.-2 at 315.35 s;
   - Warhawk 01|.-2 at 324.24 s;
   - Lightning 01|.-3 at 329.39 s;
   - B-25 01 at 365.88 s.

   Each drifts out through the side of the strip at 0.1 to 0.5 m/s. Their final headings are up
   to 0.8 rad off the runway's 0.26: Warhawk 01|.-2 ends at 1.10 and Lightning 01|.-3 at -0.53.
   They then take free-flight steps; B-25 01|.-2 sinks to `min_bfc = -1978`.

**Why.** The mode-1 yaw factor goes to 0 below `b` (1.7 m/s with the throttle closed). So does
the yaw acceleration the axis step uses, since the host's targets take `accel[1] = YawAccel x
-outB`. A plane that stops while it still has a yaw rate keeps that rate indefinitely and turns in
place. The free-flight arm (OFF) damped it through the free-flight authority.

In the image something else must hold a stopped plane's attitude. The candidates are the ground
pose `ctl+80h..8Ch`, `007D9C80` and `007D80C0` (5q, not carried) and the attached branch of
`007D9F60` (`007DA2B1`, the wheel-height lift). Both are unread.

**Verdict.** `kPlaneGroundSteeringBound` stays OFF, recorded as a mechanism failure. The reading
of `007DA542`-`007DA6E3` stands. The ground pose has to come first, then this pair, then park (5s).

## 5u. What stops a stopped plane turning: the rate law's flat floor (packet `cc9_plane_ground_pose`, cc9-lua15, 2026-09-29)

**The answer is in the rate law, not in the pose.** `007DA380` returns one byte (`[ESP+0Bh]` at
the call `007DA728`), which `007DA9EF` loads into `BL`. `BL` is tested four times:
- at `007DA9F3`, the slide term and bank-yaw coupling (5t binds this);
- at `007DABB0`, `007DAD46` and `007DAECE`, the floor of each of the three axis steps.

With `BL` = 0 each axis takes the flat floor `rate = max(rate, XMM5)`. `XMM5` is loaded at
`007DABBC` from `00CE3D30` = `3F19999Ah` = **0.6** (read from the PE on disk). It is not the
sign-guarded `1.5 x |current|` floor of free flight. Two host defects hid this:
- `control_step_007da710` passed the flag `true` for every axis, even with the 5t binding ON;
- `PlaneRotationFactors::idle_floor` held 0.15, a mis-conversion of `3F19999Ah`. Nothing reached
  it before now.

**Mode 1 below `b`.** `outB` = 0, so the yaw target `-(YawSpd x latched x BC4h x outB)` is 0 and
the yaw accel term is 0. The polynomial is then 0 and the floor makes the rate 0.6. The body yaw
rate `ctl+4Ch` therefore decays to 0 at 0.6 rad/s^2. In 5t the host kept the accel-0 rate
unchanged, which is why stopped planes spun. Pitch and roll take the same floor: the roll target
is 0 in mode 1, and pitch goes toward `PitchSpd x outA x latched`.

**The other candidates, read and not the answer:**
- `007D80C0` (`__thiscall(dyn, step)`):
  - step != 0: `dyn+78h = (dyn+44h - dyn+7Ch) / step`, `dyn+7Ch = dyn+44h`;
  - step 0: it resets those two and copies `[00F87574..7C]` into `dyn+54h..68h`.

  That is a finite difference over the velocity. It holds no attitude.
- `007D9C80`: the body-to-world copy of `ctl+3Ch`/`+48h` (the ledger). The host already carries it
  (`body_to_world_007d9c80`).
- The up levelling in `007D9F60` (`007DA11C`-`007DA20C`): `ctl+80h..8Ch` = `(0, cos GroundPitch,
  sin GroundPitch, RunwaySmoothStrength)` from `007DCD97`-`007DCDBD`.
  - It lerps pose row 1 toward world up at `min(+8Ch x step, 1)`. That holds pitch and roll, not
    the heading.
  - Only `007DCCF0` and the water law (`007DD84F`) store `+8Ch`, in the exported `007Cxxxx` and
    `007Dxxxx` functions. The constructor was not checked.
  - Not carried: this host's pose advance has neither the levelling nor the bank-yaw rotation.
- The attached branch of `007D9F60`, `007DA2B1`-`007DA338`, read whole (disk bytes). It needs
  `ctl+FCh == 1`, `unit+BF4h` and a parent `unit+3Ch`:
  - the position goes into the parent's frame (`00414D10` with `parent+CCh`, after `00414DB0`
    when `parent+C8h` is clear);
  - `006BC530(holder, &local, &y)` supplies the local height. Its over-runway return is ignored;
  - `h = y - (class+1FCh WheelHeight + 0.01)` (`00D7A358`, a double);
  - when `h < 0`: `p.y -= h` and `007D7D70(ctl+10h)`, which stores `dyn+98h..A0h = (0, 1, 0)`
    and `dyn+A4h = 1`.

  So it is a wheel-height lift onto a parent's deck. It does not touch the heading. Not carried.

**Binding.** The flag passed to `plane_control_axis_step_007da710` is now the byte, i.e. the
same `coupling` value 5t computes. It is false only under `kPlaneGroundSteeringBound` in the
ground law. OFF behaviour is unchanged, because the switch stays committed OFF. This packet
completes 5t's binding of `007DA542`-`007DA6E3`, so the pair re-pairs `kPlaneGroundSteeringBound`.

The ground-steering summary now also prints `yaw_rate` (`ctl+4Ch`) and the final `x`/`z`.

### Predictions for LOMP10 9200/9000 and USN01 3200/3000 (park OFF), written before any ON run

1. **Identical up to Warhawk 01's touchdown (153.15 s).** USN01 is gameplay identical (exit 1).
   LOMP10 moves (exit 3).
2. **No stopped plane leaves the strip:**
   - no `plane ground contact lost` line from a plane that has stopped;
   - `|yaw_rate| < 0.01` for every landed plane at the end;
   - `min_bfc > -2` for every plane.
3. **Heading error of stopped planes.** Each final heading is within 0.15 rad of the heading at
   its stop. The rolls are within 20 m of OFF's.
4. **B-25 01|.-2 stays on the runway:** no contact loss and no sink. The 5r sink is also in OFF
   (steering OFF), so it may persist if its cause is a push from B-25 01's body. If it does, it is
   recorded as a separate failure. It is not a failure of this mechanism unless the plane's
   heading turns.
5. **Deaths identical** (the per-entity table).

- **Mechanism failure:** any of:
  - a stopped plane whose heading moves more than 0.15 rad after its stop;
  - a nonzero final yaw rate;
  - a contact loss by a plane that is turning.
- **Flip rule:** ON when predictions 1 to 3 and 5 hold. Prediction 4 is recorded either way.

### The pair and the verdict (cc9-lua15, 2026-09-29): steering flips ON, with a recorded miss

- OFF is `106dabf9b` as committed (`local\l15_goff_<row>.log`). It is built on `ea5775f9b`, so
  hull inertia is ON.
- ON is the same commit exported with `kPlaneGroundSteeringBound` true (`local\l15_gon`,
  `local\l15_gon_<row>.log`).
- USN01 is gameplay identical (exit 1).
- LOMP10 moves (exit 3). The death and unit tables are identical.

**The predictions, one by one.**
1. **Held.** All ten touchdown times are identical to OFF, from 153.15 s to 267.41 s. 5t's
   0.05 s misses are gone.
2. **Held for eight planes, missed for the two B-25s.**
   - The eight fighters end with `yaw_rate` 0.00000, their final heading equal to their stop
     heading to four digits, no contact loss and `min_bfc > -0.2`.
   - B-25 01|.-2 loses contact at 309.55 s (OFF 325.94 s). B-25 01 now loses contact too, at
     352.44 s (it does not in OFF). Both sink (`min_bfc` -1717 and -829).
3. **Held while on the ground.** Each loss happens at exactly the stop heading: 0.7267 and
   0.7282, against stop headings 0.7267 and 0.7282. The later heading changes are free flight
   under the ground. Rolls: B-25 01 40.2 m (OFF 25.2), B-25 01|.-2 52.1 m (OFF 40.1), the
   fighters within 1.2 m.
4. **Missed, and it is not this mechanism.**
   - The two B-25s leave through the strip's side (`local x = -10.0`) at 0.2 to 0.5 m/s with the
     heading constant.
   - They stopped about 0.47 rad off the runway's 0.26, so a forward creep along that heading
     exits through the side after about 130 m.
   - They are the only planes with `WheelBrake` 10 (brake 6.0 against the fighters' 48.0).
   - The creep is forward motion that the band's brake does not hold. It is not a rotation.
     Its source is still unidentified (5r).
5. **Held.** Deaths are identical (0 rows either side).

**Verdict.** The mechanism matches: no stopped plane on the ground turns, and every final yaw
rate on the ground is 0. The one miss is the B-25 forward creep, a separate open item that the
switch makes visible for B-25 01. So this is a spread miss with the mechanism matching, and
`kPlaneGroundSteeringBound` flips **ON**. The B-25 creep goes on the open list.

## 6. Open, in order

1. **After the touchdown.** Standby, line, begin, final, abort, the launch-site arm, the
   touchdown and the direction hold are ON (5e, 5g, 5i, 5k, 5o). The heads now land (5o). Open:
   - stage A of the ground roll is ON (5q): a landed plane thinks, brakes and stops. The
     followers no longer abort: they were aborted by the missing landed arm of `006C7960`, not by
     occupancy (5r, ON). All ten LOMP10 planes now land and stop on the runway;
   - stage B: park is read and bound OFF (5s). It failed on what lies under it:
     - the ground steering (5t) is ON since 5u: the rate law's flat floor 0.6 stops a stopped
       plane's yaw rate. The up levelling and `007DA2B1`'s deck lift are read and not carried;
     - the B-25s' forward creep after stopping (5u, prediction 4): the brake of 6.0 does not
       hold them, and both leave the strip;
     - abort's on-ground arm `009B0E74`-`009B0F93` (the park <-> abort loop);
     - `00951F40`'s hide of a plane in the hangar;
   - a landed plane that creeps off the 20 m strip (B-25 01|.-2 at 325.94 s, 5r) loses contact
     and sinks through the ground in free flight: the airfield ground surface (`006CF180`) is not
     modelled, and what pushes a stopped plane sideways is unidentified;
   - `land/park` (vtable `00D1FF60`: enter `009B21A0`, exit `009B21C0`, tick `009B22C0`);
   - the gear channel `(+DECh)+28h` with `+C1Ch` (5k). Scoped by cc9-lua13, 2026-09-29, and
     not bound:
     - loads of `unit+DECh` (`8B ?? EC 0D 00 00`, whole `.text`) are all in `007B0000`-`007E0000`;
     - the only rel32 caller of `BSP_Plane_GearIsDown` `007B8D70` is final's done test
       `009B209B`;
     - the ground task `009CE2C0` uses channel `+44h` (`007B8D10`, `007B8DC0`), not the gear.

     So the gear reaches gameplay only through the two touchdown gates (`007CC476`, `007C71E0`)
     and final's done test. The host passes all three because the channel is not carried. The
     image's gear is requested inside the approach cone (`007C5CF8`-`007C5EE6`). Whether its
     actuator has reached 1.0 by the contact is the one open timing question: its travel rate is
     unread;
   - the direction hold's launch arm `007C705C` (0.8 s at BeginFlying), not bound;
   - a mother-ship holder, refreshed from the moving ship, is still refused.
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

## The explicit `land` command: PilotLand's entry (packet `cc9_pilot_land_native`, cc9-lua16, 2026-09-29)

**The image.**
- `PilotLand` (`008A47B0`) reads the entity (argument 0) and a command target from argument 1
  (`00B677E0`). It issues `0077D600(entity, land 00E08FA0, target, 1)` at `008A4907`, the same
  shape as PilotMoveTo (`src/pilot_order_bindings.cpp`, `pilot_land_008a47b0`).
- Each plane's bot land arm, `0099A3DD`-`0099A41A` (read from disk bytes), takes the site from
  that command target, not from `007F16D0`:
  - `0099A3E9 006BCD20(target, 1)` gives the target's deck, and a null deck returns at `0099A3F2`;
  - `0099A404 006C4790(deck, plane+9D4h)` must pass;
  - then `009B41C0` builds the task.
- `006C4790`, read whole (`006C4790`-`006C47EC`), returns false for a null squadron
  (`006C4797`-`006C4799`). It also returns false when the squadron is on the deck's `+B4h` list;
  as before, that list is taken as empty.

**The host.**
- `install_land_task_0099a3dd` resolved the site only through `rtb_census`, the squadron's
  `007F16D0` answer. Its half after resolution is now `install_land_task_core_009b41c0`.
- The new public entry `GameUnitsHost::land_at_site_0099a3dd(unit, site)`:
  - requires a deck named after the site and a registry squadron for the plane, then places
    `land` / site on the plane's command;
  - installs on every member when `unit` is the squadron (its fused flight leader), and on the
    plane alone when it is a lone wing plane;
  - marks the task `land_explicit_site`.
- `land_command_still_valid_009b34d0` keeps an explicit task while the plane's own command is
  still `land` with that site as target, instead of consulting `rtb_census`.

**Neutrality.** Nothing calls the entry yet. The native binding is the next step. Exports of
`d7b093d01` (before) and `49cb3ae26` (after) were run on LOMP10 9200/9000: pair_diff 1, gameplay
identical, and 10 `land task install` lines on each side (`local\l16_p{base,new}_lomp10l.log`,
cc9-lua16 tree).

**Rows for the binding.**
- **IJN01.** `ijn_1_pearl.lua` in this installation (mtime 2024-08-26), in the B-17 loop:
  GenerateObject `B-17 01` and `B-17 02` (WingCount 2 each), then `PilotLand(unit, Mission.AF2)`.
  `Mission.AF2` is `FindEntity("Airfield 02")`, which the host has as the deck `AirField 02`.
- **LOMP10.** `10_san_jose.lua` (lines 523 and 557 in this installation, mtime 2024-07-13) sends
  bombers whose `ammoType` is 0 and whose `unitcommand` is not `"land"` to `Mission.Airfield`.
  The `unitcommand` read then needs to answer `"land"` for the placed command, or the script
  re-issues it every pass. The same-target keep (`009B3560`) makes that harmless.

## 5v. `land/abort` on the ground: the runway-axis hold (packet `cc9_land_abort_ground_arm`, cc9-lua16, 2026-09-29)

**The listing, read whole: `009B0E74`-`009B0F93`**, then INT3 at `009B0F96`.
- **How the arm is reached.** `009B0A5C JE`, when `(plane+72Ch)->vtable[38h]()` answers 0 (not
  airborne). Registers at that point, from the tick head:
  - `EBX = 1` (`009B09E7`) and `EBP = 0` (`009B0A14`);
  - `EDI` = the state (`009B09D4`) and `ESI` = `&state+4` (`009B09E4`).

  Nothing reassigns them before the branch; every later `MOV` to them is on the airborne path
  after `009B0A64`.
- **The arm, step by step:**
  1. `009B0E76`: `+21h` = 1.
  2. `009B0E7C`-`009B0E91`: `ctl+2BCh = class+1ECh x 0.5` (`00D7A280`, a double), and
     `ctl+2D0h` = 1. This is a pitch hold at half the climb angle.
  3. `009B0E9E`-`009B0EC8`: `e = 00438B10(plane->vtable[50h](), holder+88h)`, the heading error
     to the runway.
  4. `009B0ED9`-`009B0F17`: when `|e| > pi/2` (`00CE3830`, a double), `e` becomes `e - pi` if
     `e > 0` (`00D7A218` = 0.0), else `e + pi` (`00CE3D28`). The plane is held on the runway
     **axis**, in whichever direction it already faces.
  5. `009B0F1B`-`009B0F75`: `00419010(-ys/2, -1.1 (00D06BB0), ys/2, 1.1 (00CE6448), e)`, with
     `ys = class+1B0h`. That is the same yaw law park uses at `009B2AE1`-`009B2C1B` (section 5s).
  6. `009B0F7A`-`009B0F86`: `ctl+284h` = the yaw, `ctl+288h` = 1, and `ctl+2D4h = EBP` = 0.
- **Unchanged.** The head's throttle 1.0, air brake 0.0 and bank 0.0 (`009B09D6`-`009B0A19`)
  still apply on the ground. The rule's abort arm (`009B3E08`) still sends `+21h` to park.

**The binding.** `kLandAbortGroundArmBound`, committed OFF. The arm is refused, as before, when
the deck holder is not found. That holder refusal is a host guard, because the image
dereferences `holder+88h` without a test.

**Reach.** With park OFF (the default), no plane enters abort: the LOMP10 9200/9000 default run
has no `summary land abort` line. So the default build is unchanged by construction. The pair is
therefore taken with `kLandParkStateBound=true` on **both** sides. The flipped switch is the only
difference.

### 5v.1 Predictions, written before the runs (LOMP10 9200/9000, both sides with park ON)

1. **Mechanism.** ON: `ground_refused` goes to 0 and `ground_ticks` equals the OFF side's
   `ground_refused` count, give or take the moved path.
2. **The loop stays.** The arm sets `+21h` exactly as the refusal did, and the head's throttle
   1.0 is untouched. So the park <-> abort loop of 5s (`from_abort`) persists on both sides, with
   hundreds of loops per plane. This packet does not close park.
3. **The heading.** Abort ticks now yaw toward the runway axis instead of leaving the yaw to
   park's last request. So the planes that 5s saw leave the 20 m strip sideways stay nearer the
   axis. Measured on the park trace's local `x`: the largest `|x|` per plane falls on most
   planes, and the throttle pulses carry them **along** the strip, not across it.
4. **Mechanism failure:** a plane whose heading error to the axis grows during abort ticks.

### 5v.2 Measured, and the verdict

The pair is one commit, `f288ab040`, exported twice by `tools/pair_export.py`, both sides with
`kLandParkStateBound=true`. The ON side adds `kLandAbortGroundArmBound=true`. The run was
LOMP10 9200/9000 in the reference environment, after a 300-frame LOMP10 smoke of the ON binary.
The logs are `local\l16_g{off,on}_lomp10l.log` (cc9-lua16 tree), pair_diff 3, and
`local\l16_parkx.py` tabulates the park trace per plane.

1. **Mechanism: held.**
   - The abort entries per plane are identical on both sides (797, 1247, 739, 1001, 902, 1127,
     1059, 946, 1180).
   - ON turns every `ground_refused` into `ground_ticks`, one for one.
2. **The loop stays: held.** Every abort entry is a park <-> abort loop turn, on both sides.
3. **The heading: missed, in a way the listing explains.**
   - ON, nine of the ten planes end with park's heading error at exactly 1.571 (pi/2). Their
     local `z` stays within 72-423 m, while `x` runs to 8.0-10.3 km. OFF, they end at errors of
     1.86-2.27, 3.2-8.4 km out in `z` and 8.7-18.5 km in `x`.
   - So they now drive **across** the runway axis in a straight line, not along it.
   - Warhawk 01, which never loops, is identical on both sides.
4. **Why: the image's own sign.**
   - `00438B10` is `a - b` (`00438B10 FLD [ESP+4]; FSUB [ESP+8]`, then the wrap).
   - The arm's error is `heading - runway`: `vtable[50h]` is `0074E260`, `FLD [ECX+C6Ch]`, the
     same compass heading `007C1900` writes.
   - Park's is `desired - current` (`009B2A1A`, with `desired` pushed first).
   - Both feed the same `00419010(-ys/2, -1.1, ys/2, 1.1, .)` yaw into `+284h`. So the abort
     arm's yaw has the **opposite sense** to park's: it pushes the heading away from the axis
     until `|e|` reaches pi/2, where its reversal at `009B0EF0` makes pi/2 the resting point.
   - In the image, abort on the ground is presumably a single tick before park takes over.
     Under this host's every-tick park <-> abort loop, the arm acts on half the ticks and wins.

**Verdict: `kLandAbortGroundArmBound` stays OFF**, recorded under the prediction's own failure
clause (the heading error grows during abort ticks). The transcription is checked against the
listing, including the sign, and is kept behind the switch. It cannot move a default row: with
park OFF no plane enters abort. It should be re-paired together with park once the loop is
closed. The loop is the thing to fix: park declares done at once.

**Next.** `00951F40`'s hangar hide (`007B96C0`), then why park's done test `009B21D0` fires on
its first tick (5s). The loop, not the arm, is what drives the planes off.

## 5w. Why park is done at once: the contact arm of `009B21D0` (packet `cc9_land_park_done_test`, cc9-lua16, 2026-09-29)

**The census.** Commit `7d66ee047` adds print-only counters that record which arm of `009B21D0`
sets done, shown as `done_why` on the `summary land park` line. The run is LOMP10 9200/9000 with
`kLandParkStateBound=true`, `local\l16_diag_lomp10l.log` (cc9-lua16 tree).
- On all nine looping planes, **every** done comes from the contact arm: `+BF8h` clear. The
  counts are 798, 1247, 740, 1002, 902, 1127, 1059, 946 and 1180, equal to their park entries.
- The timer (`0 > +28h`), deck (`+BF4h`), behind (`t.z < pz`) and heading arms fire 0 times.
- Warhawk 01, which enters park once and never loops, sets none.

**The first park episode is fine.**
- Each plane enters park on the runway: B-25 01 at local (1.0, -161.2), Lightning 01 at
  (0.5, -147.1), Warhawk 01|.-2 at (0.6, -151.7).
- It is not done for tens of ticks: the traces two seconds later still show `done=0`.
- It then turns toward the taxi target (44.4, -20.6). Once `|x|` passes the runway's half width,
  done fires, and from then on every re-entry is done at once.

**Where the target is.**
- In this installation's `10_san_jose.scn` (USN/LOMP, mtime 2024-08-09), the airfield `CB4_AF` is
  at (1072.4, 542.9). Its x axis is (0.9659, 0, -0.2588).
- Its hangar `CB4_AF_Hangar` is at (1124.0, 505.9). In the airfield frame that is about
  (59.4, -22.4), roughly 50 m beside the 20 m by 400 m runway (`RunwayWidth`/`RunwayLength`).
- The target is `CB4_AF_exitpath1`'s last point. `006CF420` indexes it as `count - 1`
  (`006CF472`-`006CF48B`), as the host does.

**Every input was read again from the listing, and each matches the host.**
- **`009B21D0` (`009B21D0`-`009B22B0`):**
  - `009B21F4` makes the timer arm `0 > +28h`;
  - `009B21FE`/`009B220B` test `+BF8h` and `+BF4h`;
  - `009B221F` tests `pz > t.z`, then `009B1E30` and the holder's `vtable[3Ch]`;
  - `009B2286` is the `00438B10` heading test against pi/2.
- **`006BC530`, read whole (`006BC530`-`006BC5CE`).** It is the runway rectangle:
  `|l.x| < holder+B0h x 0.5` and `|l.z| < holder+B4h x 0.5`, with `l` from `004142E0` by
  `holder+48h`. The height goes to the out pointer.
- **`007C5AC0`'s every-step tail (`007C5F0A`-`007C5F57`).** It sets `+BF8h = 006BC530(pos)` while
  `+C02h` and `+BF4h` hold. `pos` is the pose-refreshed position (`007C5B01`).
- **Park's enter (`009B21A0`).** It seeds `+28h` = 3.0 (`00CE3854`), and the airfield arm never
  lowers it.

**What this leaves.**
- On the reading as it stands, a plane taxiing to an off-runway hangar loses `+BF8h` and is done.
  The image would be too, unless a term this host lacks keeps it: park's rule sends done to
  abort (`009B3770`), and abort's on-ground arm sets `+21h` back to park (5v).
- So the loop is **not** a mis-transcribed done test. The candidates, in order:
  1. The probe's position after `007C71E0`'s re-parent to the airfield (5k). `007C5B01` refreshes
     the pose. If the image's pose is parent-relative there, `006BC530` would transform an
     airfield-local point again. This host keeps world positions throughout (the labelled 5k
     substitution).
  2. Whether the ground-roll arm reaches `007C5AC0` at all once `+900h` is 4 or 5 and the plane is
     parented. The host calls it at `007CBFC3` unconditionally.
  3. The park rule's done edge `009B3770` and its conditions, which are not re-read here.
- The counters stay, print-only. `kLandParkStateBound` stays OFF.

## 5x. The park loop: the three reads, and the loop starts after the hangar (packet `cc9_land_park_loop_reads`, cc9-lua17, 2026-09-29)

This packet makes the three reads that 5w left open. It also re-reads the 5w census log
(`local\l16_diag_lomp10l.log`, cc9-lua16 tree, LOMP10 9200/9000 with `kLandParkStateBound=true`).
No code changed.

### (i) The probe position after `007C71E0`'s re-parent: world, so no double transform

- `007C5B01` calls `00414DB0` with `ECX` = the plane, only while `+C8h` is clear.
  - `00414DB0` (`BSP_EntityPose_RefreshWorld`, `00414DB0`-`00414E09`) rebuilds the world matrix at
    `+CCh`: `local +74h x parent(+3Ch)+CCh` through `00413920`, or `+74h` alone for a root. Its
    copy is `004134F0`.
  - It then sets `+C8h` = 1.
- `007C5B10`-`007C5B34` read `+FCh`/`+100h`/`+104h`, the translation row of that world matrix. This
  is the row `00427EB0` returns. The values go to `[ESP+28h..30h]`.
- The tail passes `[ESP+2Ch]` (after one push; the same slot) to `006BC530` at `007C5F28`, with
  `ECX` = `+BF4h`, the holder.
- `007C71E0` re-parents through `vtable+ACh(site)`; before that it calls `00414E10` and
  `007BA020`, which are not re-read here. Whatever they do to the local pose `+74h`, `00414DB0`
  composes `+CCh`/`+FCh` from `+74h` and the parent, so `+FCh` is the world position.
- `006BC530` therefore gets a **world** point and transforms it once by `holder+48h`. The host's
  world-position substitution (5k) matches here, and candidate 1 of 5w is closed.

### (ii) Is `007C5AC0` reached while parented? Yes, on every fixed step

`BSP_PlaneTickElement_FixedStep` (`007CE040`, `ESI` = unit+310h) dispatches at `007CEC30`:
- `(unit+72Ch)->vtable[38h]` true: `007CC2F0`, which calls `007C5AC0` at `007CC43B`.
- Otherwise, with `+900h` 4 or 5 (`007CEC7B`-`007CEC83`): `007CBFA0`, which calls it first
  (`007CBFC3`).

Neither arm tests the parent. The only earlier exits are:
- `unit+5Eh` set (`007CE856`, to `007CF0D3`);
- `unit+520h` set (`007CE86A`, to `007CECF6`); both are clear for an AI plane.

The host's unconditional call at `007CBFC3` matches. Candidate 2 of 5w is closed.

### (iii) The rule's done edge `009B3770`: matches

- `009B3CF0` calls `009B3770` first, on every call (`009B3CF4`).
- `009B3770`:
  - does nothing in moveto (`+4C4h`) or follow (`+500h`);
  - sends a done line or standby (`+598h`/`+5B8h`) to `009AFA50`'s cruise pick;
  - sends **any other** done state to abort (`+64Ch`, `009B37A7`). That includes park.
- The rule then continues in the same call. In abort, `+66Ch` leads to standby (`009B3E1B`) and
  `+66Dh` to park (`009B3E38`, `ECX` = task+620h from `009B3DFA`).
- The host's order is the same. Candidate 3 of 5w is closed.

### The correction: every loop starts 13 to 16 s after the plane's hangar entry

The first `state 620 -> 64C` per plane in the census log, against its hangar line (`007B96C0`):

| plane | hangar (local) | first park done |
| --- | --- | --- |
| Lightning 01 | 186.71 s (41.9, -21.1) | 200.81 s |
| Warhawk 01\|.-4 | 199.21 s | 214.21 s |
| Lightning 01\|.-4 | 211.61 s | 224.81 s |
| Warhawk 01\|.-2 | 224.81 s | 238.41 s |
| Lightning 01\|.-2 | 233.91 s | 249.91 s |
| Warhawk 01\|.-3 | 245.01 s | 261.01 s |
| Lightning 01\|.-3 | 254.81 s | 269.81 s |
| B-25 01 | 275.41 s (41.9, -19.9) | 290.70 s |
| B-25 01\|.-2 | 288.20 s | 302.30 s |
| Warhawk 01 | 172.51 s (41.5, -20.4) | never (ends in state 5 at (49.6, -25.5)) |

So 5w's "done once `|x|` passes the runway half width" is the second episode, not the first. The
first episode taxis on the path (`+900h` = 5) all the way to the hangar. `009B21E4` skips the whole
done test in state 5, and the host does the same (`land_park_done_009b21d0`).

B-25 01's trace (`land park trace`, every 20 ticks):
- **272.5-276.5 s:** it arrives at about 7 m/s and overshoots to (46.9, -20.7).
- **After that:** `spd` = 0.69 (`AirField/MoveSpd` 6.94 x the 0.1 heading-error scale), while
  `v` holds at 0.92.
- **276.5-288.5 s:** `err` goes from -2.9 to -1.8. The yaw is -0.1 to -0.5, cut by the deadband
  `(5 - f18) x 0.2`. The plane drifts to (43.1, -32.6).
- **290.5 s:** `dz` = 12 passes the turn distance `qd`. The path test drops it to state 4
  (`leaves=1`), the contact arm fires because the plane is off the strip, and the loop starts.

### What this leaves (in order)

1. **Does the image's plane hold still at the hangar point?** The commanded speed is never 0 in
   the listing (`00419010` floors at `base`, then the heading scale floors at 0.1). The host's `v`
   stays at 0.92 against a command of 0.69. This is the forward creep of 5u and 6, and its brake
   is the first suspect. If the image's plane settles near the target, `dz` stays under `qd` and
   state 5 holds.
2. **The hangar hide `007B96C0`.** The listing (`007B96C0`-`007B96CF`) does two things:
   - it sets `+C00h` = 1. The only writers are `007B96C2` and the constructor clear at
     `007D0110`. Byte scans for `80`/`38`/`3A`/`84`/`8A`/`0FB6`/`0FBE`/`F6`/`83`/`8B` on
     displacement `C00h` find no reader, and the `C02h` control scan does hit `007BB2C3`.
   - it calls `00951F40(0)`, which does `00710B80` (detach from the spatial index), then
     visibility 0.

   Neither stops the tick. So the hide alone cannot end the loop. Any loop the image has after
   the hide is invisible, because the plane is hidden.
3. After (1), re-pair park together with `kLandAbortGroundArmBound`.

`kLandParkStateBound` stays OFF.

## 5y. The ground creep is gravity through an unlevelled pose (packet `cc9_ground_speed_hold`, cc9-lua17, 2026-09-29)

### The measurement

`BSP_PLANE_GROUND_TRACE=<name prefix>` is a new env-gated diagnostic. It prints `007D8611`'s
body-frame terms every tenth ground-band step. Run: LOMP10 9200/9000, park OFF, current tree,
`local\l17_gt_lomp10l.log` (cc9-lua17 tree).

B-25 01 stops at 256.9 s. From 258 s on it holds exactly:
- body velocity `vb = (-0.235, 0.010, 0.004)`: the motion is **sideways** (body x), not forward;
- driving term `a1c` (`body.pair_1c`) = `(-0.701, -13.584, -5.615)`;
- resisting fold `r04` = `(0.219, -0.010, -0.004)`;
- ground band `r40` = `(0.470, 0, 6.000)`: the lateral `|2 vx|`, and the wheel brake 10 x 0.6;
- latched throttle 0, `thrust` 0, `pitch` 0.3915 rad.

Along z, the brake (6.0) exceeds the driving term (5.6), so the forward motion is clamped at 0.
Along x there is no brake. The lateral term `|2 vx|` only balances `-0.701` at
`|vx| = (0.701 - 0.219) / 2 = 0.24`, which is the measured creep.

`a1c` is gravity (with the DeadMeat/extra terms) expressed in a body frame that the touchdown
left pitched 0.39 rad and slightly rolled: the x share -0.70 is a roll of about 3 degrees. The
park drift of 5x is the same effect. For example Lightning 01 moves at a constant 0.84 m/s along
-z while its heading error stays at -1.4 rad, which is sideways motion.

### What the image has and this host lacks

`007D9F60`'s up levelling, `007DA11C`-`007DA20C`, read whole (disk bytes):

- **The rate.** It runs only for `ctl+8Ch > 0` (`007DA11C` COMISS / `007DA131` JBE). The stores
  to `ctl+8Ch`, by scan of `f3 0f 11 ?? 8c 00 00 00` and `d9 ?? 8c 00 00 00` in `007B`-`007D`:
  - the constructor `007D7F0B`/`007D7F55`;
  - `007D7BC9`;
  - the ground roll law, `007DCDBD`: `[00F87384]` = tuning+2A4h `Dynamics/RunwaySmoothStrength`,
    4.0 in this installation;
  - the free-flight step, `007DCCD3`: the zero from `007DCCD0` XORPS;
  - the water law, `007DD84F`.

  So the levelling is live exactly while the ground law runs.
- **The axis.** `ctl+80h..88h` = `(0, cos GroundPitch, sin GroundPitch)`, with `GroundPitch` =
  desc+200h (`007DCD6C`-`007DCDAD`).
- **The update.**
  - `0042D0D0(out, ctl+80h, M, 0)` puts the axis through the working pose `M` (`[ESP+38h]`, rows
    at `+38h`/`+48h`/`+58h`) in the row-vector convention: `out = v x M`, into world.
  - The delta is `(0 - t.x, 1 - t.y, 0 - t.z)`. The bytes are `DE E2` = FSUBRP `ST(2) = ST(0) -
    ST(2)` at `007DA155`/`007DA165`, and `DE EA` = FSUBP at `007DA171`.
  - The factor is `min(+8Ch x step, 1)` (`007DA17D`-`007DA19C`).
  - Row 1 (`[ESP+48h..50h]`) takes the scaled delta (`007DA1D0`-`007DA208`).
  - Then `0085DAD0` (`007DA20C`) renormalises row 1, removes row 1 from row 2, renormalises row
    2 and sets row 0 = row 1 x row 2. The pose ends pitched by `GroundPitch` with no roll.

`src/plane_advance_pose.cpp` already reconstructs this (`apply_up_levelling_007da14d`,
`level_blend_007da179`). The host's pose advance (`advance_pose_0085e4d0`) never called it
(5u: "not carried").

**Binding.** Under `kPlaneGroundLevellingBound`, `advance_pose_0085e4d0(step, ground)` now:
1. rotates as before;
2. while the core law runs in ground mode, applies the levelling at `RunwaySmoothStrength` with
   the slot's `GroundPitch`. That is desc+200h, written only when `WheelHeight` and
   `GroundPitch` are both authored (`007D29B8`-`007D2AC6`); B-25: 0.017453, fighters: 0.

Labelled:
- the bank-yaw rotation `007DA080`-`007DA117` between the two is still not carried;
- the water law's store is not modelled;
- the image runs this in the `+4h` tick element, which this host folds into the fixed step.

The ground-roll summary gains `level_steps`.

### Predictions, written before any ON run

**Pair A.** LOMP10 9200/9000, park OFF on both sides, `--flip kPlaneGroundLevellingBound=true`:
1. Identical until the first touchdown, because the levelling needs the ground law.
2. B-25 01 and B-25 01|.-2 stop and **stay** on the strip:
   - no `plane ground contact lost` line;
   - `min_bfc` above -2 (OFF: about -950 and -1780);
   - the lateral creep gone (`vb.x` about 0 after the stop).
3. Every landed plane has `level_steps > 0`. The fighters' rolls stay within a few metres.
4. Deaths identical (0 rows).

**Pair B.** LOMP10 9200/9000, `kLandParkStateBound=true` and `kLandAbortGroundArmBound=true` on
both sides, plus the flip:
1. The post-hangar sideways drift of 5x goes away. Planes that reach the hangar stay within a
   few metres of the target in state 5.
2. So the park <-> abort loop stops. Park entries per plane drop from hundreds to single digits,
   and `done_why contact` falls with them.
3. Mechanism test: the drift distance after the hangar entry, from the `land park trace` lines.

### 5y.1 Measured, and the verdict

The binaries are exports of `1a756bc16`:

| export | flips | SHA-256 prefix |
| --- | --- | --- |
| `l17_a0` | none | `5DAD87554945` |
| `l17_a1` | levelling | `82EC6E1805B6` |
| `l17_b0` | park, abort ground arm | `31A6D6F0B5E6` |
| `l17_b1` | park, abort ground arm, levelling | `D8EED8A0C333` |

All runs used n's launch form with `BSP_PLANE_GROUND_TRACE=B-25 01`. A 300-frame smoke of `a1`
ran clean. The logs are `local\l17_<export>_<row>.log`.

**Pair A (park OFF), LOMP10 9200/9000: `pair_diff` 3, every prediction held.**
1. **Identical before the first touchdown.** The early lines that moved are LOMP10's known
   movie-camera and minimap noise.
2. **The B-25s stay on the strip.**
   - `min_bfc` is 1.514 and 1.470 (OFF: -953.9 and -1777.7).
   - There is no `ground contact lost` line (OFF: 306.90 s and 342.14 s at local x = -10.0).
   - Rolls are 24.3 and 22.0 m (OFF: 41.5 and 53.6, creep included).
   - After the stop the trace holds `vb = (0, 0, 0)`, `a1c = (0.000, -14.713, -0.257)`. The
     -0.257 is 14.71 x sin(0.017453), the B-25's `GroundPitch` (vehicleclasses.lua, this
     installation, mtime 2026-05-09).
3. **Every landed plane levels** (`level_steps` = `arm_steps`). The fighters' rolls move by at
   most 1.2 m.
4. **Deaths are identical** (0 rows); the 34-row unit table is identical.

The same pair on the other rows:

| row | `pair_diff` | what moved |
| --- | --- | --- |
| USN04 4700/4500 | 1 | the avoidance refill counter only |
| USN01 3200/3000 | 1 | the avoidance refill counter only |
| JM05 3200/3000 | 1 | the avoidance refill counter only |
| USN13 3200/3000 | 0 | nothing |

**`kPlaneGroundLevellingBound` flips ON.**

**Pair B (park and the abort ground arm ON on both sides), LOMP10 9200/9000: prediction 1 held
for half the planes, and prediction 2 missed.**

| plane | park entries OFF -> ON | end state ON |
| --- | --- | --- |
| B-25 01 | 798 -> 1 | state 5 at (42.5, -25.5) |
| B-25 01\|.-2 | 740 -> 1 | state 5 at (37.8, -20.9) |
| Lightning 01\|.-3 | 902 -> 1 | state 5 at (47.1, -21.1) |
| Lightning 01\|.-4 | 1127 -> 1 | state 5 at (38.6, -25.3) |
| Warhawk 01\|.-3 | 946 -> 1 | state 5 at (37.5, -17.1) |
| Lightning 01 | 1247 -> 547 | loops |
| Lightning 01\|.-2 | 1002 -> 301 | loops |
| Warhawk 01 | 1 -> 785 | loops |
| Warhawk 01\|.-2 | 1059 -> 549 | loops |
| Warhawk 01\|.-4 | 1180 -> 653 | loops |

The drift of 5x is gone: every plane now stays within about 8 m of the hangar point for a
minute or more. The loop that remains is a different mechanism. Warhawk 01's trace from 269 s to
291 s shows it:
- The target sits **behind** the plane (`err` 2.5 to 2.6 rad).
- `v` pulses between 0 and 1.2 m/s against the command of 0.69.
- The commanded yaw is 0.1 to 0.48, cut by the `(5 - f18) x 0.2` deadband.
- The heading hardly moves. In the pulses where `v` is near 0, the rate law's flat floor of
  0.6 (5u) holds the yaw rate at 0.
- The plane crawls 12 m away, `dz` passes `qd`, and the path drops to state 4 at 293.3 s. The
  abort ground arm then drives it off at up to 37 m/s. That is where the planes' `last x` of
  several kilometres comes from, in `b0` as well.

Both the image's rule (the deadband, `009B2AE1`-`009B2C1B`) and the floor were read and match. So
the open question is whether the image's plane can turn round at 0.69 m/s, which the host's
stop-and-go speed hold (the throttle demand arm integrating by `dt`) prevents.

`kLandParkStateBound` and `kLandAbortGroundArmBound` stay OFF. The next items are:
- the stop-and-go at the 0.69 command;
- the abort ground arm's 37 m/s taxi.

The hangar hide (`007B96C0`) was not bound: it only detaches and hides (5x), so it moves
nothing that a log can check.

## 5z. The stop-and-go at the 0.69 command reads as the image's own laws (packet `cc9_park_stop_and_go`, cc9-lua17, 2026-09-29)

**Runs.**
- `local\l17_c0_lomp10l.log`: an export of `126b71042` with park and the abort ground arm
  flipped.
- `local\l17_d_lomp10l.log`: the same flips as a local build, with the trace extended by the
  latched controls, the yaw slot and the body angular rate.

Both are LOMP10 9200/9000 with `BSP_PLANE_GROUND_TRACE=Warhawk 01`.

### The cycle, measured

Warhawk 01 after its hangar entry, sampled every 0.5 s. It repeats with a period of about 3 s:

| t | latched throttle | brake | body vz |
| --- | --- | --- | --- |
| 173.66 | 0.0315 | 0 | 0 |
| 174.66 | 0.110 | 0 | 0.35 |
| 175.16 | 0.126 (the slot's peak) | 0 | 0.71 |
| 175.66 | 0.102 | 0 | 1.06 |
| 176.16 | 0.0315 | 0 | 1.22 |
| 179.16 | 0.0157 | 15.24 | 1.22, then 0 |

The last row is where the throttle falls through 0.023: `brake = WheelBrake 80 x (0.6 - 26 x
0.0157)`.

The terms against the listing:
- **The demand arm (`0099D924`-`0099DC6B`), read again whole from disk bytes.**
  - `err = 2B4h - measured`, less `(vtable[38h]() - 0042B2F0(unit+AE0h)) x [00CE3D88] x dt`.
  - `increment = 00419010(-6.944, -2, 6.944, 2, err)`, x 0.6 when positive, x the slot `[ESP+6Ch]`
    (dt).
  - `demand = seed + increment`, clamped to [-1, 1].
  - Throttle = `max(demand, 0.001)`, air brake = `max(-demand, 0)`.

  This is the host's `pilot_plan_throttle_0099d300`.
- **The dead band.** The increment is skipped only when all four hold: `|err| <= [00D09450]`,
  `measured >= 1.0`, and `0.5 <= 2B4h/|measured| <= 1.5` (`0099DB1F`-`0099DB56`). At a 0.69
  command, `measured < 1.0` forces the increment every think, so the host's
  `dead_band_skips = false` is exact here.
- **Measured speed.** It is `007D99C0(unit+AB0h)` (the body forward speed plus the carrier term)
  divided by `plan+2B8h`, and less the carrier's speed on a class-6 holder (`0099D99E`-
  `0099DA48`). `plan+2B8h` only decays toward 1.0 from above (`0099D75C`-`0099D79A`), and
  `0099D924` raises it only when `+2B0h` is clear, which park sets. The host uses `|world v|`.
  After the levelling (5y), body x is below 0.17 m/s in these cycles, so the difference is small.
  LABELLED, not changed.
- **The brake cliff.** It is the ground band `007DBEB3`-`007DBF0C` (`0.6 - throttle x 26`, 5q),
  unchanged.

So the pulse is the integral demand arm hunting against the ground band's brake cliff at
throttle 0.023. No host substitution on this path produces it.

### Why the plane cannot come round

The yaw slot carries park's desired yaw, 0.37 to 0.47, the deadband having cut it from 1.1
(`009B2AE1`-`009B2C1B`, `f18` = `|dx|` below 3 m). The latched yaw follows it. The body yaw rate
is -0.13 to -0.21 rad/s while `v` is above about 0.5, and 0 in every stop. That follows from:
- the mode-1 yaw factor `outB` (0 below `b` = about 0.3 m/s at throttle 0.1, full above `a` =
  1.67 m/s);
- the flat floor of 5u.

The result is a turning circle of about 7 m, traversed about a third of the time. From 271 s to
291 s the heading turns about 0.65 rad while the bearing to the target (behind, `err` 2.3 to 2.6)
turns with the motion. The plane crawls from (46.1, -21.6) to (42.6, -31.9), `dz` passes `qd`, and
it leaves the path at 293.2 s.

**Reading.** Every law in this loop matches the listing. The host has no divergence left here
that a switch could carry:
- the park tick and its deadband;
- the demand arm;
- the ground band;
- the mode-1 yaw factor and the floor;
- the path test.

After `007B96C0` the image's plane is hidden but still ticking (5x). Nothing found so far ends
park for it:
- state 5 skips the done test (`009B21E4`);
- `+C00h` has no reader, and the SIB-form scans `80/8A/0FB6/38 ?? ?? 00 0C 00 00` are also empty.

So the image may well loop the same way, invisibly. That cannot be settled from the host.

**Open.**
- The one term left unmatched is the measured speed (`007D99C0` forward speed against
  `|world v|`); a switch for it is cheap but would not change the turning circle.
- Is there an image routine that retires a landed, hidden plane (the air-ops holder's occupant
  list `+34h`, the squadron's landing record)? That decides whether flipping park can ever be
  gameplay-neutral.

`kLandParkStateBound` and `kLandAbortGroundArmBound` stay OFF.

## 5aa. What retires a landed plane: the carrier's elevator, never an airfield (packet `cc9_landed_plane_retirement`, cc9-lua17, 2026-09-29)

This is a time-boxed census for the question 5z left open: does anything end park for a landed,
hidden plane? Disk bytes and Ghidra were read only; no code changed.

### The carrier elevator retires and relaunches (mother-ship holders only)

The site class `00CF89F8` (`006CF3C0`/`006CFABC`, AIRFIELD_TAXI.md 2) has a subclass with vtable
`00CF8A58`:
- Its constructor `006CFAF0` stores `00CF8A58` at `006CFB24`, and `006D0470` stores it at
  `006D0490`.
- The constructor reads `MotherShip.ElevatorSpeed` (GAMEPLAY_SETTINGS.md).
- The string `elevator` follows the table at `00CF8AB0`.
- Its slot `+4h` is **`006D0600`-`006D07A4`**: `RET 4` at `006D07A1`, then INT3 padding. Ghidra
  has no function there.

It is `__thiscall(site, float dt)` and does two jobs:

1. **Intake, `006D06A5`-`006D0722`.** It walks the occupant vector `+34h`/`+38h` for a plane
   that meets all four of:
   - `+904h` is set (landed);
   - `site->vtable[3Ch](plane)` is true (`006CFF70`, not read);
   - `vtable[38h]` speed < `[00CF8AAC]` = `3FB1C71Dh` = 1.389 m/s;
   - `007B8D40` is true (the byte at `[plane+DECh]+44h` clear, or the float at `+48h` zero).

   With a candidate, or when `+18h` is set and `006D02F0` (not read) answers false, it calls `006CFFF0(0, plane)`. That builds message
   `00758B90` and routes it through `0077C2A0` with 5. The handler `006D0050` (slot `+50h`,
   `00CF8AA8`) sends a plane with flag 0 to `006FC720`, which:
   - takes the plane onto the platform `+34h`;
   - calls **`007C2090`**, which requests flight state **2** (message `C3h`, new state 2, routed
     with 7) unless the plane is already in 2;
   - starts the lift (`+50h` = 2).
2. **The lift, `006D0729`-`006D07A1`.** At the bottom it calls `007B96C0` (the hide) and
   `006FC250`. After `tuning+510h`, with the platform empty, it calls `006CFFF0(1,
   readyPlane +18h)`, which is the relaunch through `007C5F60`.

So on a carrier a landed plane leaves the land task's world: it goes to flight state 2 and into
the hangar, and the ready plane comes back up.

### An airfield has no such path

- **The airfield site's tick** (slot `+4h` of `00CF89F8` = `006CF980`, read whole to `RET 4` at
  `006CF9DB`) only routes the ready-plane launch (`+18h` -> `0077C2A0` with 5). No slot of the 21
  removes an occupant (AIRFIELD_TAXI.md, "Site occupancy").
- **The hide `007B96C0`** has three callers. The other callers of `00951F40` are the elevator
  platform (`006FC0D0`/`006FC250`/`006FC6B0`/`006FC810`), the spawn-state helper `007BC550`, and
  the Lua `luaMW_SetVisibility` (`008A13D0`). None retires a plane.
- **`+C00h` has no reader.** The byte scans in 5x plus the SIB forms
  `80/8A/0FB6/38 ?? ?? 00 0C 00 00` all come back empty.
- **The state-5 census** (`83 ?? 00 09 00 00 05`) finds these functions, none of them a
  retirement:
  - the done test `009B21D0` and the ground roll;
  - the 4/5 transitions;
  - `0099D300`;
  - the taxi step `009CD540`;
  - `007EFB60`, which promotes the squadron's next member when the leader is landed on the path;
  - `009CF8E0`, the taxi task constructor;
  - `007B83F0`/`007C7430` (message handlers).
- **The taxi task** (`009CFF40`, created at `0099B10B` by the bot tick) needs the current task's
  `vtable[30h]` true. For the land task that is `009B3730`, false in park and final
  (`00D1FFA0`+30h), so a parked plane never gets it.

### Verdict

Nothing retires a landed plane at an airfield. The image's airfield plane, hidden at the hangar
point, keeps running park. So the image most likely loops the way the host does (5z), invisibly.
**Park stays OFF, and the park-loop line of work ends here for now.**

The carrier elevator path is real. It belongs to the mother-ship holders the host refuses, and
it becomes the model if those are ever bound.

### For the lead (Ghidra, read-only here)

Define `006D0600`-`006D07A4` (exclusive, `RET 4` at `006D07A1` then INT3), provisional name
`BSP_AirOpsElevatorSite_Tick`: the mother-ship site's `+4h` slot, which runs the landed-plane
intake and the elevator relaunch.

## 5ab. The land internals PilotLand reaches on IJN01, ranked by reach (packet `cc9_land_internal_records`, cc9-lua17, 2026-09-29)

**Runs.** IJN01 9200/9000 on main `bacecf5de`, in the reference launch form: `local\l17_e_ijn01l.log`
before the relabel, `local\l17_f_ijn01l.log` after it (cc9-lua17 tree). The two are
gameplay-identical (`pair_diff` 1): concrete/unimplemented 1180/524 -> 1183/521, 3 status
changes, no count moved.

All seven records the lead listed are also reached on LOMP10, so none is new with PilotLand:
- `006BD080` 4090 on LOMP10;
- `006C0B50`/`006C5380` 1503 each;
- `009C1850`/`009C18C0` 2511 each;
- `0099A3DD`/`006C4790` 10 each.

| rank | record | IJN01 calls | what it is | action |
| --- | --- | --- | --- | --- |
| 1 | `006BD080` find_landing_assignment | 1017 | The miss arm after `landing_request_006c54c0` ran the lookup against the sequencer's vector. A miss is the image's own answer for a plane the sequencer has not taken (5c). | **relabelled done** when the request ran on a built deck (`land_request_ran`) |
| 2 | `009C1850` set_desired_speed | 539 | `moveto_speed_009c1850`, bound by `kMovetoSpeedBlendBound`. It keeps a labelled stand-in: the unit's own class row for the squadron's `+35Ch` class. | record kept |
| 2 | `009C18C0` glide_slope | 539 | `move_to_glide_009c18c0` plus the cruise altitude. `cin.has_squadron = false` is a substitution. | record kept |
| 2 | `009F9E40` steer_to_point | 539 | The bearing is the host's `pi/2 - atan2` stand-in, not `009F9E40`. | record kept; a real gap |
| 3 | `006C0B50` queue_squadron | 206 | Already performed by `landing_request_006c54c0` (the `LandingDeck::queue` push, 5c). | **relabelled done** when the request ran |
| 3 | `006C5380` landing_point | 206 | Already performed in both arms by `landing_request_006c54c0`: the side sum, `006C3E50` into `land_radius_48`, and `006C5380` into `land_circle_38` (5e). | **relabelled done** when the request ran, with the standby state bound |
| 4 | `0099A3DD` land arm | 4 | The install-time records in `install_land_task_core_009b41c0`. | not read here |
| 4 | `006C4790` squadron_not_excluded | 4 | Same as above. | not read here |

**The larger land records on the same IJN01 run** (not on the lead's list, ranked by calls):

| record | IJN01 calls | note |
| --- | --- | --- |
| `009B1EED` land/final direction_40 | 10809 | **relabelled done**: a store of the value already there (5ac) |
| `009B3D38` BotTaskLand::refused_state | 10207 | park, OFF by design (5aa) |
| `009FABE0` land/standby direction | 2560 | |
| `0099B650` BotApproachLand::set_owner | 908 | |
| `007C07A0` land/begin direction hold | 320 | |
| `009B1DDA` land/begin direction_40 | 310 | |

After park, `009B1EED` and `009FABE0` are the biggest live gaps in the land states.

## 5ac. land/final's `direction_40` is a store of the value already there (packet `cc9_land_final_direction_40`, cc9-lua18, 2026-09-29)

`009B1EDF`-`009B1EF2`, in `BSP_BotStateLandFinal_Tick` (`009B1ED0`-`009B211F`, `RET 4`), read from
the listing: `EDI` = 0 (`009B1EDF`), `approach+B4h` = `EDI` (`009B1EE1`), then `XMM0` (zeroed by the
first instruction, `009B1ED0`) goes to `[approach+1Ch]+40h` (`009B1EED`). The store is
unconditional; nothing else in the tick touches `approach+1Ch`.

**What the field is.** `approach+1Ch` is the task's auto-strafe gun controller (`task+314h`,
docs/DOGFIGHT_GUN.md section 1), and `+40h` is its strafe cone. Its reader is `009FC7C0`
(`009FC8B5`, the target cone `max(+2Ch, +40h x 1.25)`, and `009FCCA4`, the steer gate
`1 - cos(lead, +68h) < +40h`), which the PilotBot update runs after the task arm. `009FC7C0`
stores `XMM0` = 0.0 to `+40h` at `009FCE69` on its tail (every path reaches `009FCE26` or
`009FCE49` with `XMM0` zeroed at `009FCE07`, `009FCE23` or `009FCDA6`), so the cone is a one-tick
input.

**Why the store is a no-op.** Two facts, both from the listing:
1. The only switch into land/final is `009B3DA7` (`PUSH task+5F8h; CALL 009B3680`, in
   `BSP_BotTaskLand_StateRule`, after `009B3C00` on land/begin). The other disp32 `+5F8h`
   references in `00996000`-`009F7000` are membership compares (`009B347E`, `009B36D0`,
   `009B3DB2`, `009B3ECE`) or unrelated stack slots.
2. land/begin's tick stores 0.0 to the same field every tick (`009B1DDA`), and `009FC7C0`
   stores only 0.0 there. So the field is 0.0 when final is entered, and final keeps it 0.0.

Uncertainty: a writer of the cone outside the task states (through a second pointer to
`task+354h`) is not excluded by census; the disp32 `+354h` hits in `00990000`-`00A10000` belong
to other classes' timers. No such writer is documented.

**The host.** It has no gun controller outside the dogfight arm, so the store has no object.
`kLandFinalDirection40Bound` relabels the record as performed (`owner_.done`) instead of
recorded. No state is written.

**Predictions (written before the pairs).** On IJN01 9200/9000 and LOMP10 9200/9000, OFF against
ON on the same tree:
- `pair_diff` 1 (gameplay identical), or 3 only through the host-call table.
- `BotStateLandFinal::direction_40 009b1eed` leaves the UNIMPLEMENTED list; the same call count
  (10809 on IJN01 at lua17's `l17_f`, give or take the gunnery13 merge) appears as done.
- The per-entity death table, the touchdown lines and the land/final entry lines do not move.

**Not changed:** land/begin's `009B1DDA` (310 calls). Its store is not a no-op in the image on
begin's first tick after standby or line, whose cone is `tuning+66Ch` (`Angle_Prepare`); it only
matters where the gun controller runs, which this host does not do for the land task. The same
holds for standby's and line's `009FABE0` direction records: they are inputs to a gun controller
that the host runs only in dogfight.

**Pairs (OFF `fdc4709f3` tree build, ON its export with the flip; `local\l18_a0_*`, `local\l18_a1_*`).**

| row | pair_diff | the record | other |
| --- | --- | --- | --- |
| IJN01 9200/9000 | 1, gameplay identical | 10809 calls, UNIMPLEMENTED -> concrete | the ship avoidance refill counter (known noise) |
| LOMP10 9200/9000 | 1, gameplay identical | 26213 calls, UNIMPLEMENTED -> concrete | the LOMP10 movie-camera presentation lines only |

The predictions held. **Verdict: `kLandFinalDirection40Bound` ON.**

## 5ad. The return-to-base site key (packet `cc9_rtb_site_key`, cc9-lua19, 2026-09-29)

GAMEPLAY_GAP_RANKING (refresh) #1. `kReturnToBaseSiteKeyBound` (`src/game_hosts_units.cpp`, in
`GameUnitsHost::Impl`) is committed **OFF**.

### The gap

`record_return_to_base_007f16d0` asks `006C0840` for the nearest site the way `007F16D0` does, but
it gave every candidate `key_known = false`. With two or more candidates past the filters,
`nearest_landing_site_006c0840` then flags `site-key-unread`, and `install_land_task_0099a3dd`
refuses every land task. On main `31de7f88a` (`local\l19_main_jm05*.log`):
- JM05 3000: 16 squadrons, 1680 refusals, 0 installs;
- JM05 9000: 25 squadrons, 5493 refusals, 0 installs.

The candidate list has `sites=6 passed=4`: the two US airfields and USS Lexington and Yorktown.
The plane-side caller `plane_landing_site_006c0840` already computed the key, but only for
airfields, because `landing_deck_006c0750` refuses a mother-ship holder.

### The image (read for this packet)

- **The key**, `006C09FE-006C0A9B`, is as `plane_landing_site_006c0840` documents it. An accepting
  holder (`006BC530`, head over the runway) keys on `|00438B10(holder+88h, head vtable[50h])|`.
  Any other holder keys on the squared holder-local offset from T (`006BCC90`), with y zeroed and
  z x 0.3 inside (-1500, 800).
- **The carrier holder** is built by `007593D0` (`007593F1-00759493`):
  - `006C0D20(&class+814h, class+820h, class+824h)`, and then `006C0750`;
  - `+98h..+A0h` is the offset, `+B0h` = class `+820h`, `+B4h` = class `+824h`.
- **`00759590` (MMothership class reader):** `+820h` is `RunwayWidth` and `+824h` is `RunwayLength`.
- **`00759120` (MMothership model bind), `00759237-00759265`:** `+814h..+81Ch` is the first point of
  the model's last `("runwaycenter", 0)` Aux group, looked up through `00718000`. This
  installation's LexingtonCV, yorktown and Zuikaku models each carry a `runwaycenter` item.
- **The refresh, `00758E80`** (carrier update, vtable entry at `00D015EC`): it calls
  `00811AB0(dt)`, then `006BEE40` on `[unit+EF8h]`.
- **`006BEE40`** (`006BEE40-006BEEBC`, `RET 4`; its float argument is unused):
  - copies the owner's `+CCh` world matrix to `+8h`, first running `00414DB0` when `+C8h` is
    clear;
  - adds `0042D0D0(+98h, frame, 0)`, the offset through the rotation rows without normalising, to
    the translation;
  - builds the inverse at `+48h` (`0085DEA0`);
  - stores owner `vtable[50h]` at `+88h`. For a carrier that is `006DFD60`,
    `FLD [unit+1050h]`: `hull_heading_1050`.
- **`006BC960`'s MotherShip arm** (`IsKindOf(9)`): T = (0, 0.5, float(float(-B4h x 0.5) + 10.0)).
  `00CE3DC0` is the double 10.0.

### The binding

With the switch ON, `landing_site_key_006c09fe` gives each record-path candidate its key:
- an airfield through `landing_deck_006c0750`;
- a carrier through `carrier_holder_frame_006bee40`, which is the frame above, recomputed at each
  resolution from the owner's current `world`.

A class without a `runwaycenter` point keeps `key_known = false`. The census line is `summary
squadron returntobase site keys computed=... carrier=... missing=... bound=...`.

LABELLED:
- The x87 sums are taken in double.
- The host frame is the last published pose, where the image's is the last `00758E80` update.
- The carrier holder serves only the key. Landing on a carrier stays refused (ranking #3).

### Predictions, written before any ON run

- **OFF** is gameplay-identical to main on every row: the switch only guards the new code, and the
  summary adds one line.
- **JM05 3000 and 9000, ON:**
  - No resolution carries `site-key-unread`, and `unread=` is 0 in every land-task row.
  - The squadrons still parked at their airfield at 2.55 s move to `ground` refusals ("not
    airborne").
  - The airborne carrier squadrons resolve to their nearest US site and install land tasks
    (installs > 0).
  - A squadron whose winner is a carrier installs a task whose deck is then refused by the
    sequencer, so it does not land (ranking #3).
  - Plane paths, deaths and damage move (exit 3).
- **The other rows are gameplay-identical** (exit 0/1). USN04, USN13 and IJN01 resolve no
  return-to-base on main, and LOMP10 has one site, where the key never decides.

### 5ad.1 Measured (pairs on `b8c3b26c7`)

**The pair.** OFF is `local\l19_rtb0` (SHA-256 prefix `BD8901D4B3C6`). ON is `local\l19_rtb1`
(`EAA5AD182E06`) with the flip. Both use reference p's launch form, and the logs are
`local\l19_rtb{0,1}_<row>.log`.

| row | pair_diff | what moved |
| --- | --- | --- |
| USN04 3000 | 1, gameplay identical | nothing |
| LOMP10 3000 | 1, gameplay identical | nothing |
| IJN01 3000 | 1, gameplay identical | nothing |
| JM05 3000 | 3 | release counts only. Deaths, hits and damage are identical; installs 0 -> 108 |
| JM05 9000 | 3 | deaths 29 -> 3, damage 32265 -> 17337, torpedo drops 8 -> 0; installs 0 -> 117 |

**The mechanism held.**
- Every resolution now carries a key: 15084 keys at 9000 frames, 10056 of them carrier keys, and
  none missing.
- `site-key-unread` is gone, and `refused=` is 0.
- The carrier frame read `runwaycenter` for Lexington (-0.231, 17.401, 0.002), Yorktown and
  Zuikaku.
- Each squadron resolves to its nearest US site. USS Yorktown_sqn06 (planar 21.6 m) resolves to
  its own carrier, where it was the airfield before.

**The spread missed.**
- The predicted `ground` refusals did not happen: the JM05 squadrons are airborne at their first
  placement.
- **The consequence is larger than predicted.** The placements come from the AI SELLING tick
  (`00A11FF0`, `kSellingTickBound` ON). The Sell think (`00A22800`) gives SELLING to the US carrier
  groups (Lexington and Yorktown, 6 members each). Each command tick then sends `returntobase` to
  every squadron of an air group; the image's own gates `+361h` / `+3B0h` are clear, as
  CONTROLLED_UNIT establishes.
- From 6.45 s, the freshly launched strike squadrons get `land` at their own carrier. The
  carrier deck is refused (ranking #3), so they circle over it. None of the JM05 9000 air strikes
  happens: the 24 plane deaths and 8 drops of OFF are gone.
- **What OFF had been doing.** The `returntobase` command was on each director too, but the
  install was refused. So each plane kept the torpedo task that the script's `PilotSetTarget` had
  given it, and flew a task its own command no longer backed.

### 5ad.2 Verdict: kept OFF

- The key is faithful, and it is what makes the land intake consistent with the command.
- Flipping it would remove JM05's air strikes on the strength of an upstream question this packet
  did not read: does the image's Sell think really give SELLING to a carrier group that holds
  squadrons it has just launched? That covers the group membership through the coordinator's
  splits and the `00A2C660` air-member test.
- That question belongs to the AI lane. Until it is answered, and until ranking #3 lets a carrier
  land its planes, `kReturnToBaseSiteKeyBound` stays OFF.
- It can flip once the SELLING assignment is shown to be the image's, with the pair above as its
  evidence.

## 5ae. Carrier landing decks, part 1: the holder and the run length (packet `cc9_carrier_landing_deck`, cc9-lua19, 2026-09-29)

GAMEPLAY_GAP_RANKING (refresh) #3. `kCarrierLandingDeckBound` (`src/game_hosts_units.cpp`,
`GameUnitsHost::Impl`) is committed **OFF**.

### The image (read for this packet)

- **The holder.** `007593D0` builds the carrier's holder: `006C0D20` -> `006C0750` with the
  `runwaycenter` offset (class `+814h`) and `RunwayWidth` / `RunwayLength` (class `+820h` /
  `+824h`).
  - `00758E80` re-frames it on every carrier update through `006BEE40`. 5ad has the frame, the
    inverse, `+88h` = `unit+1050h`, and the T of `006BC960`'s MotherShip arm, (0, 0.5, -L/2 + 10).
- **The run length**, `006BA620` (`006BA620-006BA65A`, `RET`): `RunwayLength` x 0.3 (`00CE3DC8`)
  when the owner answers `IsKindOf(9)` (`006BA62F-006BA63D`), otherwise x 0.4 (`00CE65D0`). Each is
  stored through a float.
- **The corridor**, `006C3B10`. Its `IsKindOf(9)` at `006C3B98` tests the **plane's** scene parent
  (`00923810(1)`): a plane already parented to an airfield or carrier answers true.
  - The corridor's geometry has no MotherShip arm of its own; it reads the holder and `006BA620`.
  - The parent test belongs to part 2 (touchdown attach, `007C71E0`), and stays labelled.
- **The other `IsKindOf(9)` sites** found by `local\l19_kind9.py`:
  - `006BA5F1`: the runway half width, `B0h` whole on a carrier and x 0.5 otherwise. No caller
    was found by xref or dword scan, so it is left alone.
  - `006BCD39`: `006BCD20`, the block lookup, where a carrier's block is at `+1188h`.
  - `006C08FE`: `006C0840`'s own-site arm.
  - `007B3C1D`, `007C6EBD`, `007C72D3`, `007CB7B7` and `007CC264`: the plane's parent, touchdown
    and ground-roll arms (part 2).
  - `009B1E40`, `009B202F` and `009B23E4`: the land/final and approach arms (part 2).

### The binding (part 1)

With the switch ON:
- `landing_deck_006c0750` builds a mother-ship deck through `carrier_holder_frame_006bee40`
  (5ad), marks it `mother_ship`, and re-frames it from the carrier's current pose at every later
  lookup.
- `landing_run_length_006ba620` answers x 0.3 on a mother-ship deck.
- Everything downstream runs on the holder as it does for an airfield: the sequencer, corridor,
  circle, standby, touchdown probe and landed arm.

LABELLED:
- The re-frame happens at a lookup, not at the carrier's update.
- A plane that touches down is held at its world touchdown point (the airfield substitution),
  where the image re-parents it to the moving deck (`007C71E0`, part 2).

The census line is `summary carrier landing decks built=... refreshes=...`.

### Predictions, written before any ON run

**Reach.** Without `kReturnToBaseSiteKeyBound`, no reference row gives a carrier a land task. So
both sides of the pair flip `kReturnToBaseSiteKeyBound=true`, and the pair measures this switch on
top of it.
- JM05 at 3000 and 9000 frames.
- USN04, USN13 and LOMP10 at 3000 frames as controls. They resolve no carrier land task.

**OFF.** The same as 5ad's ON side: land tasks to the carriers, whose decks are refused, so the
planes circle over them.

**ON, JM05:**
- `carrier landing decks built` = 2 (Lexington and Yorktown; the Japanese carriers take no US
  task).
- The two decks' landing-sequencer rows show inserts and passes greater than 0, and the queue
  modes move off 0.
- Planes fly the corridor to the carrier's T.
- Some touch down on the deck: the 5k probe answers over the carrier's runway within the 0.3 L
  corridor.
  - A plane that touches down is held at its world touchdown point while the carrier steams on.
  - It then sits behind or beside the ship, which is part 2's gap.
- Plane paths move (exit 3).
- Deaths should stay near 5ad's ON side (3), since the strikes were already recalled there.

**Controls.** USN04, USN13 and LOMP10 are gameplay-identical (exit 0/1).

### 5ae.1 Measured (pairs on `cc00545ef`)

**The pair.** Both sides flip `kReturnToBaseSiteKeyBound=true`.
- OFF is `local\l19_cd0`, SHA-256 prefix `8E54C57F5727`.
- ON adds `kCarrierLandingDeckBound=true`: `local\l19_cd1`, prefix `80846B49F94F`.
- The logs are `local\l19_cd{0,1}_<row>.log`, and every run is clean.

| row | pair_diff | what moved |
| --- | --- | --- |
| USN04 3000 | 1, gameplay identical | nothing |
| USN13 3000 | 1, gameplay identical | nothing |
| LOMP10 3000 | 1, gameplay identical | nothing |
| JM05 3000 | 3 | plane paths |
| JM05 9000 | 3 | 18 unit rows (plane paths); plane water contacts 3 -> 4 |

On JM05 9000, deaths (3, the same victims), hits and damage are all identical.

**The mechanism held.**
- Carrier holders were built and re-framed 486039 times. For Lexington: RunwayWidth 40,
  RunwayLength 279, T local (0, 0.5, -129.5), heading 1.5708.
- The US carrier decks now run the sequencer:

  | deck | inserts | passes | hits | landed-arm hits | modes 1 / 2 / 3 / 4 |
  | --- | --- | --- | --- | --- | --- |
  | USS Lexington | 15 | 7395 | 6569 | 1461 | 3063 / 794 / 13133 / 2690 |
  | USS Yorktown | 6 | 3533 | 2339 | 4044 | 540 / 282 / 1787 / 4396 |

- **13 carrier planes touch down on their deck** (touchdowns 15 -> 28), from USS Lexington_sqn03,
  sqn05 and sqn07 and USS Yorktown_sqn04 and sqn06. For example, Yorktown_sqn04 touches down at
  218.56 s, deck local (-0.9, -53.2), vy -5.04.

**Spread misses.**
- Four decks were built, not two: Shokaku and Zuikaku too. The plane-side site probe
  (`007C5AC0` -> `006C0840`) looks up every deck.
- **The held plane leaves its deck, as predicted.** The landed plane is held at its world
  touchdown point. Yorktown_sqn04's contact is lost at 234.26 s at deck local z = 124.49 against a
  half length of 124: the carrier has steamed out from under it. The one extra water contact
  belongs to this.

### 5ae.2 Verdict: kept OFF

- Part 1's mechanism is the image's, and carriers now recover planes.
- It leaves a landed plane hanging where the deck was. The re-parent to the moving deck
  (`007C71E0` at `007C72D3`) and the ground roll on it (`007CC264`) are part 2.
- It is also measured only on top of `kReturnToBaseSiteKeyBound`, which is itself OFF pending the
  SELLING question (5ad.2).
- So `kCarrierLandingDeckBound` stays OFF until part 2 carries the plane with the deck.

**Part 2, the queue.** Each is an `IsKindOf(9)` site from `local\l19_kind9.py`:
- `007C71E0` (the touchdown attach, re-parent at `007C72D3`);
- `007CB5F0` (`007CB7B7`) and the ground roll `007CBFA0` (`007CC264`);
- `007B3C1D` and `007C6EBD` (the plane's parent and its observed-unit callback);
- `009B1E30` (`009B1E40`), land/final `009B1ED0` (`009B202F`) and the approach base step
  `009B22C0` (`009B23E4`);
- the elevator retirement of 5aa.

## 5af. Handoff: carrier landing decks, part 2 (cc9-lua19, 2026-09-29, stamped 22:42 UTC)

**Where the three switches stand.** All are OFF on main `57b499145`.

| switch | section | state | what gates it |
| --- | --- | --- | --- |
| `kReturnToBaseSiteKeyBound` | 5ad | OFF | only carrier decks part 2, below. The SELLING question of 5ad.2 is answered: cc9-ships17, SHIP_AI_OPEN_ITEMS 59, found the JM05 recall of the fresh strike squadrons faithful |
| `kCarrierLandingDeckBound` | 5ae | OFF | part 2 |
| `kMoveToArrivalEndCommandBound` | PILOT_MOVETO_TASK, "The arrival ends the command" | **ON** | nothing; it is stage-only |

**Part 2, to do.** Carry a landed plane on the moving deck, then flip both switches together.
- Pair them on JM05 3000 and 9000, with USN04, USN13 and LOMP10 as controls.
- Take the OFF side from `kReturnToBaseSiteKeyBound=true` alone, as in 5ae.1: that isolates the
  deck.
- Also take a pair with both switches against neither: that is the flip's own evidence.

The part 2 sites are `IsKindOf(9)` tests, from `local\l19_kind9.py <lo> <hi>` in the cc9-lua19
tree. They were found by linear capstone, so check each one's containing function.

| site | routine | what it gates |
| --- | --- | --- |
| `007C72D3` | `007C71E0` `BSP_Plane_TouchdownAttachToSite` | re-parenting to the site. The host labels it "not carried (a static airfield)", and that stand-in is what leaves the plane hanging |
| `007CB7B7` | `007CB5F0` `BSP_Plane_OnTouchdownFromFlight` | the touchdown's authority arm |
| `007CC264` | `007CBFA0` `BSP_Plane_GroundRollStep` | the ground roll, which this host does not run after state 4 |
| `007B3C1D` | `007B3C00` | unread |
| `007C6EBD` | `007C6E90` `BSP_Plane_ObservedUnitCallback` | unread |
| `009B1E40` | `009B1E30` | land/final's enter side |
| `009B202F` | `009B1ED0` `BSP_BotStateLandFinal_Tick` | land/final |
| `009B23E4` | `009B22C0` `BSP_PlaneBot_ApproachBaseStep` | the approach step. The host has the airfield arm only (see the comment near `kLandingLandedArmBound`) |
| `006C3B98` | `006C3B10`, the corridor | the plane's parent test. It answers once the re-parent exists |

Also part of the carrier path:
- **5aa, the carrier elevator.** It retires and relaunches a landed plane (mother-ship holders
  only).
- **`006BA5E0`.** The runway half width: `B0h` whole on a carrier, x 0.5 otherwise. No caller has
  been found by xref or dword scan yet. Find one before part 2 uses the carrier width.

**Measured facts to keep.**
- The carrier holder and its re-frame: 5ad and 5ae. The runwaycenter points are:
  - Lexington (-0.231, 17.401, 0.002);
  - Yorktown (0.019, 15.401, 1.952);
  - Zuikaku / Shokaku (-0.231, 17.291, 0.002).
- The four JM05 holders build, and the US decks sequence and land 13 planes (5ae.1).
- The failure to fix: Yorktown_sqn04 touches down at 218.56 s (deck local (-0.9, -53.2)) and
  loses contact at 234.26 s at local z 124.49, because it is held at its world point.

## 5ag. Carrier landing decks, part 2: the deck carries the landed plane (packet `cc9_carrier_landing_deck_part2`, cc9-lua20, 2026-09-29)

`kCarrierDeckParentBound` (`src/game_hosts_units.cpp`, `GameUnitsHost::Impl`) is committed
**OFF**. It can only be reached through `kCarrierLandingDeckBound`, because a mother-ship deck must
exist first.

### The image (read for this packet)

- **The re-parent**, `007C71E0` (`007C71E0`-`007C742F`, `__fastcall(plane)`, `RET`). After the
  gates the host already models (`+C49h`, `classDesc+198h`, `+BF4h`, the gear), the holder's owner
  `(+BF4h)+4 -> +7Ch` is compared with the plane's scene parent `00923810(1)` (`unit+3Ch`,
  `007C725F`). When they differ:
  - `007D9CE0(owner)` runs on the controller `unit+AB0h` (`007C7277`). The velocity `ctl+18h`
    becomes `R_owner^-1 (v - owner vtable[34h])`: the owner-relative velocity in owner axes.
    `ctl+24h` is turned the same way, and `ctl+30h` copies `ctl+18h`.
  - `007BA020(00414E10(owner))` runs on `unit+74h` (`007C7287`): the local pose becomes the owner's
    inverse times the world pose. `vtable[ACh](owner)` (`007C72B1`) sets the parent, and
    `007D9C10` rebuilds the body frame (`007C72C5`).
  - `007C72D3`: when the owner answers `IsKindOf(9)` (a ship), the **arrestor seed** runs. Its
    three gates:
    - `|(ctl+3Ch, ctl+44h)|` (`00414C60`) > 3.3333 (`00D05B48`);
    - `|00438B10(plane vtable[50h], 006BCA80(holder, t 0).w)|` < 1.0472 (`00D05AAC`, 60 degrees);
    - the holder-local-less-T z (`006BCC90`) < `006BA620()` x 2.8 (`00D05A38`).

    When all three pass, it calls `007DB630(ctl, clamp(z - 2.0, 1.0, 16.0) x 0.5)` (`00D7A308`,
    `00D7A24C`, `00CE6454`, `00415620`, `00D7A280`).
  - `007DB630` (`__thiscall(ctl, float)`, `RET 4`) sets `ctl+ACh = min(arg, tuning+51Ch
    MaxWireRope)`.
  - Then `007C1570(4, 0)` runs as before.
- **The wire band**, `007DB680` at `007DBEEE`-`007DC088`.
  - The frame: ESP there is the frame base - 4. So `[ESP+13h]` is `[base+0Fh]`, which is
    `unit+904h` as stored at `007DB6DA`, and `[ESP+5Ch]` is the step.
  - `+904h` clear: `ctl+ACh = 0` (`007DC085`).
  - `ctl+ACh <= 0.1` (`00D7A3A0`): left alone.
  - Otherwise, the reference heading is `006049F0(unit)+88h` (the holder heading) when the plane
    has contact and a holder, and the plane's own `vtable[50h]` otherwise.
  - When `|00438B10(heading, ref)|` < 60 degrees and the body `|(vx, vz)|` > 6.9444 (`00D06878`):
    - `ctl+ACh += |(vx, vz)| x step x tuning+518h WireRope`;
    - then `min(tuning+51Ch)` (`00415510`);
    - then, with a contact holder, `x 00419010(160, 1.4, 250, 1.0, holder+B4h)`.

    Otherwise `ctl+ACh = 0`.
  - **The consumer**, `007DC1A7`-`007DC1C0` (read for this packet): when `ctl+ACh > 0`,
    `dyn+0Ch -= ctl+ACh`. That is a deceleration on the body-forward damping accumulator, which
    the integrator applies only against the motion.
- **The surface factor**, `007DC0A2`-`007DC131`. With a scene parent, take the parent's velocity in
  the plane's body frame (`vtable[34h]`, `0042D0D0` through `00414E10`); call its z component `v`.
  The wheel friction is scaled by:
  - `00419010(-5.5556, 0.35, 0, 1.0, v)` when `v < 0` (`00D06870`, `00CF6560`);
  - `00419010(1.3889, 1.0, 6.9444, 1.3, v)` otherwise (`00CF8AAC`, `00D0686C`, `00CEB4B4`).
- **`007CC264`**, in the ground roll `007CBFA0`. A state-4 plane that lost contact while its
  holder's owner is a ship sends the takeoff message (`00762A00` -> `0077C2A0`), so a plane that
  rolls off the deck edge flies. The takeoff `007C7110` is not bound, so this is counted, not sent.
- **Read and left alone:**
  - `007CB7B7` (`007CB5F0`) gates only the scoring call `0090F6C0(plane, 1)` (own-side carrier,
    kind 17h, the player's own squadron).
  - `007B3C1D` is inside `007B3C00`, a kind classifier with no plane state: 6, then 9 or Dh -> 0;
    Bh or Ah -> 1; else 2.
  - `007C6EBD` (`007C6E90`) re-runs the site probe `007C5AC0(-1.0)` when an observed 45h or 9 unit
    changes.
  - `009B23E4` (`009B22C0`) is the carrier arm of the taxi/park step. It belongs with the elevator
    (5aa).

### The binding

With the switch ON, at a touchdown on a mother-ship deck:
- the plane is parented to the carrier (`unit+3Ch`);
- its velocity becomes carrier-relative (`007D9CE0`);
- its position and pose rows are kept in carrier-local form;
- the arrestor seed runs (`007C72D3`, `007DB630`).

On every ground-roll step of a parented plane:
- The world pose is rebuilt from the carrier's current world matrix and the stored local pose
  before the arm runs, and the local pose is re-taken after it. The image integrates `unit+74h`
  under the parent; this host integrates world rows, so it re-expresses them around the step.
- The velocity stays carrier-relative and turns with the carrier.
- The wire band, its consumer and the surface factor run on it.

LABELLED:
- The carrier's velocity is its `motion.linear_velocity` (vtable[34h]).
- Only a mother-ship deck re-parents. An airfield's re-parent is to a static owner and changes
  nothing in this host.
- A parented plane's `plane_world_velocity` holds the owner-relative velocity in world axes, as the
  image's `ctl+18h` holds it in owner axes. A reader that wants a world velocity gets the relative
  one.
- The deck-edge takeoff (`007CC264`) is counted, not sent.

The census line is `summary carrier deck parent ...`.

### Predictions, written before any ON run

The pairs are JM05 at 3000 and 9000 frames, and USN04, USN13 and LOMP10 at 3000. The OFF side flips
`kReturnToBaseSiteKeyBound` and `kCarrierLandingDeckBound`; the ON side adds
`kCarrierDeckParentBound`.

- **Controls** (USN04, USN13, LOMP10): gameplay identical (exit 0/1). No mother-ship deck gets a
  touchdown, and every term is gated on a parented plane.
- **JM05:**
  - Every carrier touchdown re-parents. `parented` equals the carrier touchdowns: 13 on 9000 in
    5ae.1, if the order holds.
  - Most touchdowns seed the wire, since the plane is aligned and fast.
  - The wire stops each plane within tens of metres, so a landed plane stays on its deck:
    - no `plane ground contact lost` line for a carrier plane, and Yorktown_sqn04's loss at
      234.26 s goes away;
    - plane water contacts return to 3 (the fourth in 5ae.1 was that plane);
    - `deck-edge takeoff requests` = 0.
  - Deaths stay 3, with the same victims.
  - Plane paths after the first carrier touchdown move (exit 3).
- **Both switches against neither** (the flip evidence): JM05 moves (recall and recovery). The
  controls stay gameplay identical.

### 5ag.1 Measured (pairs on `7d8d5a86b`)

**The exports.** All three are `tools/pair_export.py --commit 7d8d5a86b`:
- `local\l20_n0`, no flip, SHA-256 prefix `FA29EA35EAAC`;
- `local\l20_p0`, `kReturnToBaseSiteKeyBound` and `kCarrierLandingDeckBound`, `47C4CCB593A4`;
- `local\l20_p1`, those two plus `kCarrierDeckParentBound`, `B09D7B509A8D`.

The runs use reference p's launch form with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`
(`local\l20_runs.ps1`); the logs are `local\l20_{n0,p0,p1}_<row>.log`. Every log is clean:
`present interval immediate`, the export's module directory, `frames_presented` = frames - 1 and
the final COM release.

The environment: a first batch at 16:20 local (Pacific) died at renderer init (`hr 0x8876086A`,
`logonui=1`, the session locked), and one run stopped at frame 4346. Those runs were discarded.
After a clean 300-frame smoke at 16:41, every failed row was re-run.

**The deck pair** (p0 -> p1, this switch alone):

| row | pair_diff | what moved |
| --- | --- | --- |
| USN04 3000 | 1, gameplay identical | nothing |
| USN13 3000 | 1, gameplay identical | nothing |
| LOMP10 3000 | 1, gameplay identical | nothing |
| JM05 3000 | 1, gameplay identical | nothing: no carrier touchdown happens within 3000 frames |
| JM05 9000 | 3 | plane paths: 7 unit rows (nearest distances only); deaths (3, the same victims), hits, damage and shots are identical |

**The mechanism held.**
- `summary carrier deck parent parented=13 wire_seeded=13 carry_steps=32923 wire_steps=87
  wire_resets=13 wire_max=94.01 stops=13 edge_takeoff_requests=0`.
- **Every carrier touchdown re-parents and seeds the wire, and every plane stops on its deck:**
  - On Yorktown, 6 planes touch down at local z -47 to -51 (z-less-T 61 to 65, so the seed is
    8.0). They stop within 0.5 s at z -38 to -42.
  - On Lexington, 7 planes touch down at z -139 (z-less-T -9.4, so the seed is 0.5). They stop
    at z -134 to -136, inside the half length 139.5.
- **OFF lost contact 13 times and ON never does.** On p0, the 6 Yorktown planes slide off at
  local z 124.1 to 124.5 (the named Yorktown_sqn04 failure, again at 234.26 s). The 7 Lexington
  planes lose contact at the stern edge (z -139.5 to -140.3) at touchdown, with |v| about 34.
- The landed planes ride their carriers. Their ground-steering end points move from about
  (-8900, -9400) to (-10600, -11000) with Yorktown.

**The spread missed.** Plane water contacts are 4 on both sides, not 3. The fourth is USS
Lexington_sqn07|.-2, on both sides. On this base the slid-off Yorktown planes never reached a
water contact (5ae.1's fourth contact came from a different base).

**Side finding for the AI lane: Yorktown steams astern.** The relative velocity at every Yorktown
touchdown is larger than the world velocity:
- world (19.9, 18.8) and relative (27.5, 26.0), so the carrier velocity is (-7.6, -7.2), 10.4 m/s
  at heading 226.6 degrees;
- the carrier's own runway heading is 0.815 rad (46.7 degrees).

So Yorktown moves backwards at 10.4 m/s while it recovers aircraft. OFF's slide-off over the
*bow* edge is the same fact. Lexington steams ahead: carrier velocity (12.3, 11.3), 16.7 m/s along
its heading.

**The flip pair** (n0 -> p1, all three switches against none):

| row | pair_diff | what moved |
| --- | --- | --- |
| USN04 3000 | 1, gameplay identical | summary lines only: 2 carrier decks build, and the deck-spawned planes probe their deck (contact steps 0 -> 8) |
| USN13 3000 | 1, gameplay identical | summary lines only: 9 carrier decks build |
| LOMP10 3000 | 1, gameplay identical | presentation lines |
| JM05 3000 | 3 | torpedo-task releases 0 of 12 and dive-bomb 0 of 6 -> none installed; one plane water contact; 43 unit rows. Deaths 0, hits and damage are identical |
| JM05 9000 | 3 | deaths 28 -> 3; 24 plane deaths -> 0; Shokaku no longer sinks, Kuma-class 01 now does; torpedo drops 8 -> 0; water contacts 21 -> 4; units 425 -> 407 |

This is 5ad.1's recall (deaths 29 -> 3 on `b8c3b26c7`). The recalled strikes are now recovered
aboard, not left circling.

### 5ag.2 Verdict: all three ON

- **`kCarrierDeckParentBound`.** The mechanism is the image's and matched the prediction on every
  counted item. The one miss is a spread miss (the water-contact count), explained above. It flips
  ON.
- **`kCarrierLandingDeckBound`.** Part 2 was its gate (5ae.2): landed planes now stay on the
  moving deck. It flips ON.
- **`kReturnToBaseSiteKeyBound`.** Its two gates are closed:
  - SELLING: SHIP_AI_OPEN_ITEMS 59 found the JM05 recall faithful;
  - the carrier deck: 5ae plus this section.

  It flips ON. JM05's reference row moves as the flip pair shows. That is the faithful recall of
  the freshly launched strikes, recorded here for reference q's successor.
- **Left open:**
  - the carrier elevator retirement and relaunch (5aa; the taxi/park carrier arm `009B23E4`);
  - the deck-edge takeoff `007CC264`, which needs the takeoff `007C7110`;
  - the scoring call at `007CB7B7`;
  - the host's use of the relative velocity by readers that expect a world velocity (labelled).

## 5ah. The carrier elevator, read whole, and what it needs before it can bind (packet `cc9_carrier_elevator`, cc9-lua20, 2026-09-29)

This packet read the image only; no code changed. It completes 5aa's reading of the mother-ship
site, answers `006BA5E0`, and names the chain a binding would need.

### `006BA5E0` has no caller

`006BA5E0` (`006BA5E0`-`006BA615`, `RET` at `006BA614`) answers the runway half width: `holder+B0h` whole when the
owner answers `IsKindOf(9)`, x 0.5 otherwise. `local\l20_rel32.py` scans every section of the PE
on disk for an `E8`/`E9` rel32 or an absolute dword landing on it, and finds none.

The scan is live: on the same pass it finds `006BA620`'s five callers (`006C3BD2`, `006C3C80`,
`006C3D52`, `007C73B0`, `009B19B5`) and the vtable slot `00CF8A5C` for `006D0600`. A
register-indirect call through a computed address is not excluded. **Recorded as caller-less;
nothing to bind.**

### The mother-ship site (vtable `00CF8A58`, constructor `006CFAF0`)

**The vtable.** Slots against the airfield site's `00CF89F8`:

| slot | elevator site | airfield site | role |
| --- | --- | --- | --- |
| `+4h` | `006D0600` `BSP_AirOpsElevatorSite_Tick` | `006CF980` | the site tick, from `006CDC70` (`block+3Ch`) |
| `+8h` | `006CFE40` | `006CEBE0` | the platform tick below. No caller was found: the 36 `FF 5x 08` calls in `.text` include none that loads its object from `[reg+3Ch]`, and the `mov reg,[reg+8]` / `call reg` form was not scanned |
| `+2Ch` | `006D00E0` | `006CF420` | park's taxi target: `class+808h..+810h` with z - `classDesc+158h` (`RET 8`) |
| `+34h` | `006D0390` | `006CF5B0` | park's spot test: `006CE610`, then a lane test against `class+810h` and the constant `[00E08FC8]` = 14.0 (`.data`, on disk) |
| `+3Ch` | `006CFF70` | | the intake test: `006CFE90`, the plane's nose point (`classDesc+158h` through `unit+74h`) to the lift's x/z, below 3.0 (`00D7A2B0`) - plane `vtable[38h]` |
| `+44h` | `006D0120` | `006CF520` | park's queue origin: `class+808h..+810h` |
| `+50h` | `006D0050` | | the message handler, below |

**The lift point.** `class+808h..+810h` is written by `00759120` at `007591B2`-`007591C2`: the
first point of the model's Aux group `liftexitpoint`, index 2 (`push 2` at `00759167`; the
string is at `00D018CC`). `runwaycenter` is the same call with index 0. The point is in the
carrier's model frame.

**The platform** (site `+44h`, constructor `006FC2A0`, set up by `006FC380` at `006CFD42`):
- `P+0Ch..+14h` is the lift point;
- `P+44h` is `MotherShip.ElevatorSpeed` (settings `+4D8h`), and `P+48h` the depth
  `MotherShip.ElevatorDepth` (`+4DCh`);
- `P+1Ch` = depth x 0.1 (`00D7A3A0`); the position `P+18h` starts at -`P+1Ch`;
- `P+50h` is the mode: 0 still, 1 up, 2 down. `P+4Ch` is a 1.0 multiplier.

**The platform tick**, `006FC480` (from slot `+8h`):
- A dead platform plane (`+5Dh`) is released.
- Mode 2: `P+18h += speed x dt x P+4Ch`. At `P+18h >= depth + P+1Ch` it clamps and goes to mode 0.
- Mode 1: `P+18h -= ...` until below `-P+1Ch`, then mode 0.
- `006FC0D0` then carries the plane: `unit+A4h..+ACh` = lift point + offset (`P+38h..+40h`),
  with y lowered by `clamp(P+18h, 0, depth)`, and `00951F40(1)`.

**The site tick**, `006D0600` (`006D0600`-`006D07A4`), when dt != 0, an owner (`+B8h`) exists and
the platform is still (`+94h` = `P+50h` == 0):
- **Lift at the top** (`P+18h < 0`):
  - release a platform plane (`006FC250`); if it is alive, request ground state 4 (`007C3C90(0)`);
  - then, with the carrier block's owner alive, scan the occupants `+34h`/`+38h` for a plane
    with `+904h` set, the slot `+3Ch` intake true, `vtable[38h]` < 1.389 (`00CF8AAC`), and
    `007B8D40` true (the gear channel);
  - a candidate, or a ready plane `+18h` with `006D02F0` false, gives `006CFFF0(0, plane)`.
    That is message `00758B90` routed with 5, whose handler `006D0050` with flag 0 calls:
    - `006FC720(plane)` for a plane: it takes the plane (offset = its local position less the lift
      point), requests **flight state 2** (`007C2090`: message `C3h`, routed with 7, unless it is
      already 2), and starts down (mode 2);
    - `006FC640` with no plane: an empty platform goes down.
- **Lift at the bottom** (`P+18h > depth`) with mode 0:
  - a platform plane is hidden (`007B96C0`) and released (`006FC250`);
  - otherwise `+9Ch += dt`;
  - past `tuning+510h`, with no platform plane, `006CFFF0(1, ready +18h)` relaunches: handler flag
    1 -> `BSP_Plane_PlaceOnLaunchSpotLocked` and `006FC810` (up with the plane), or `006FC6B0`
    (empty, up).

### What a binding needs first

**The lift takes only a plane parked on it.** The intake needs the plane's nose within 3.0 m of
the lift point, less its speed. The landed plane stops near the stern (5ag.1: local z -38 to
-42 on Yorktown, -134 on Lexington) and never gets there by itself.

**The taxi is land/park.** Park's tick `009B22C0` has a carrier arm, `bVar4` = `009B23E4`'s
`IsKindOf(9)` on the site owner. It uses the elevator site's slots `+44h`/`+2Ch`/`+34h` above,
the move speed `tuning+4E8h`, and no path join (`009B271A` never goes to state 5). Its clock
`park+28h` runs down while the plane is slow (< 1.0) and within 10 m (`00CE38B8`), and it feeds
the done test `009B21D0`, whose class-9 test is `009B1E30` (`approach+30h` `IsKindOf(9)`). It
steers with the same yaw law as the airfield arm.

**`kLandParkStateBound` is OFF** for the park <-> abort loop (5s; 5aa found the image's airfield
most likely loops the same way, invisibly). So no plane reaches park in any reference row, and an
elevator binding is unreachable until park runs for carriers.

**The relaunch side has no feed.** The ready plane `+18h` comes from the ready-plane pull
`006C6540` and the stock regeneration, neither reconstructed (`air_operations.hpp`). The relaunch
arm would be a record.

**The state-2 request.** `007C2090` sends `C3h` with new state 2. The host's C3h handling and the
land task's rule for `+900h` 2 were not read in this packet.

### Proposed order (for the lead)

1. Bind park's carrier arm (`009B23E4` and the `bVar4` branches, the `+2Ch`/`+44h`/`+34h` slots
   above, `006CE610`) and the elevator site (`006D0600`, `006FC480`, `006FC720`, `007C2090`,
   hide). Both are gated behind park.
2. Pair them with `kLandParkStateBound=true` on both sides: JM05 9000 lands 13 carrier planes
   (5ag.1).
3. Measure whether the elevator ends the carrier park <-> abort loop (state 2 and the hide).
   This is the question that decides whether park can flip for carriers.

A separate packet is needed to read the host's `C3h` state-2 path and the rule for `+900h` 2 first.

## 5ai. Handoff (cc9-lua20, 2026-09-30, stamped 02:11 UTC)

**What cc9-lua20 landed.** Each row is in main through the lead's merges.

| packet | switch | state | section |
| --- | --- | --- | --- |
| `cc9_carrier_landing_deck_part2` | `kCarrierDeckParentBound` (with `kCarrierLandingDeckBound` and `kReturnToBaseSiteKeyBound`) | **ON** | 5ag |
| `cc9_carrier_elevator` (read only) | none | - | 5ah |
| `cc9_moveto_target_speed` (ranking #8) | `kMoveToTargetSpeedOverrideBound` | **ON**, stage-identical | PILOT_MOVETO_TASK, "The target-speed override" |
| `cc9_follow_trail_arm` (ranking #14) | `kFollowTrailArmBound` | **ON**, stage-only | PILOT_MOVETO_TASK, "The follow trail arm" |
| `cc9_plane_wanderer` | `kPlaneWandererBound` | **ON**, every plane row moves | docs/PLANE_WANDERER.md |

**The queue, in the lead's order (2026-09-30):**

1. **Followers sit 100-600 m off their stations (a likely real gap).** The wanderer pair measured
   the 009BFD70 distance each follow tick, with the diagnostic
   `summary follow station error n=... mean=... max=...` (both sides).

   On the OFF logs `local\l20_w0_<row>.log` (cc9-lua20 tree, the no-flip export of `7e5fe18c1`):

   | row | follow ticks | mean (m) | max (m) |
   | --- | --- | --- | --- |
   | USN04 3000 | 12674 | 142.3 | 349.3 |
   | E2 (USN04 9000) | 16462 | 143.1 | 349.3 |
   | JM05 9000 | 39453 | 623.3 | 2158.2 |
   | JM08 3000 | 6534 | 112.8 | 246.5 |
   | LOMP10 3000 | 7 | 251.8 | 274.6 |

   With the wanderer ON (`l20_w1`) the means are 163.3 / 160.1 / 553.4 / 108.8 / 96.7.

   Formation flight, escort cover and every strike approach hang on this.

   - **The host side.** `GameUnitsHost::Impl::place_wing_member_on_station_007f23a0`
     (`src/game_hosts_units.cpp`) computes the station and, with `apply_position` true, teleports
     the member onto it.
     - Its callers are the moveto follow tick `run_moveto_follow_tick_009c1fd0` and
       `Impl::run_follow_tick_009c1fd0` (torpedo and land follow), both with
       `apply_position = false`, and a once-placement at the `true` call site.
     - The flying law is `run_follow_law_009bfee0_009bee30` under `kPlaneFollowLawBound` (ON). It
       holds the `+85h` latch (within GoodPositionDist and GoodPositionDir) and runs the hold arm
       `run_follow_hold_arm_009bee56` only inside the latch. Outside it, the approach is the host's.
   - **The image side.** It is the follow tick `009C1FD0`:
     - the station `009BFD70` (reconstructed);
     - `009BFEE0`, of which docs/BOMBER_AFTER_TASK.md reads only arm A, the hold; arm B, the
       approach to the station, is about 1500 instructions and unread;
     - `009BEE30` (`009BEE30`-`009BFD67`), about a thousand instructions.

     docs/PLANE_FORMATION.md section 6 names the placement a hole. The numbers above say the
     approach does not converge: a member outside the latch never gets the image's approach arm.
   - **Start by reading:** docs/BOMBER_AFTER_TASK.md sections 5-6, docs/PLANE_FORMATION.md
     sections 5-6, docs/PLANE_FOLLOW_LAW.md. Then take `009BFEE0` arm B with `--asm`
     (register-passed x87, as `007BE060` was). Measure with the same diagnostic.
2. **The wanderer uncertainty.** Does `006D1FC0` copy the published pose `unit+74h` back into
   `unit+674h` between fixed steps? docs/PLANE_WANDERER.md section 4 reads `+0h` as an extra
   velocity because of that loop (docs/PLANE_ADVANCE_POSE.md). The tuning comments call it a
   deviation that returns to position.
   - If the commit does not feed back, `007D8230`'s term is a sub-frame render offset, and
     `kPlaneWandererBound` goes back OFF.
   - **Read** `006D1FC0` and its call order against `007C6500` and `007CE040`.
3. **Ranking #7:** the squadron `+348h` command block, `approach+6Ch`, the moveto circle radius.
4. **Ranking #15:** the airfield destruction slot `006D40F0`. It is parked until a row destroys a
   hangar.
5. **5ah (a)(b)(c):** the carrier elevator chain.
   - (a) the host's C3h state-2 path and the rule for `+900h` 2;
   - (b) park's carrier arm `009B23E4` and the elevator site `006D0600` / `006FC480` / `006FC720`,
     committed OFF;
   - (c) the pair with `kLandParkStateBound=true` on both sides of JM05 9000.
6. **Low priority:** a unit SetParty in `src/game_hosts_units.cpp` with the event-6 group removal
   (SHIP_AI 63; no reference row reaches it in-window).

**Tools in the cc9-lua20 tree** (`local\`), copy what you need with your own prefix:
- `l20_runs.ps1` / `l20_wait.ps1` / `l20_until.ps1`: the launch rows, the foreground wait, and a
  wall-clock wait;
- `l20_rel32.py <addr...>`: every E8/E9 rel32 and absolute dword to an address, over the PE on disk;
- `l20_disp.py <disp...>`: every `.text` operand with a displacement. Check decodes by hand: a
  misdecode can mask a MOVSS/FLD;
- `l20_tuneread.py <off...>`: tuning-block readers after `CALL 0042E740`.

**Environment notes.**
- Runs failed at renderer init at 16:20 local on 2026-09-29, with `logonui=1`. They were clean
  again at 16:41.
- From about 17:27 local the window request became 1600x900 instead of 640x480 (the lead routed it
  to the reference worker).

## 5aj. Handoff (cc9-lua21, 2026-09-30, stamped 05:19 UTC)

**What cc9-lua21 landed.** Each row is in main through the lead's merges; the last two are
merging.

| packet | switch | state | section |
| --- | --- | --- | --- |
| `cc9_follow_approach_arm` (5ai item 1, read) | `kFollowPhaseABound` | **ON** on fidelity | PLANE_FOLLOW_PHASE_A 8-8.8 |
| `cc9_follow_error_split` | none (diagnostic) | both sides | PLANE_FOLLOW_PHASE_A 8.9 |
| `cc9_follow_turbo` (5ai item 1, the fix) | `kFollowTurboBound` | **ON**: station error 154 -> 22 m on USN04 | PLANE_FOLLOW_PHASE_A 9-9.5 |
| `cc9_wanderer_feedback` (5ai item 2) | `kPlaneWandererBound` | stays **ON** | PLANE_WANDERER 9 |
| `cc9_weapon_facts_order` | `kAiWeaponFactsAtAttachBound` (in `src/game_hosts_gunnery.cpp`) | **ON** | WEAPON_FACTS_ORDER |

**The queue, in the lead's order:**

1. **The recon publication `00806B10` (PRIORITY).** cc9-ships20 found it in SHIP_AI 74 (main
   `20edc0e8c`).
   - `Recon::publish_slot_table` is unimplemented (`src/game_hosts_lua.cpp` about line 2262).
     `recon[p][rel][cat]` stays empty, so every `luaGetShipsAround*` query returns nil. JM08's
     invasion trigger never fires, although Allied ships reach 42.8 m.
   - `00806B10` (`00806B10`-`00806CCD`, `RET 4`, `__thiscall(ReconSlot*, LuaInstance*)`) is
     called only from `008079B0` `BSP_Recon_ServicePeriodicRefresh`. The ledger says it is read
     only to `00806BAE`. `00805D90` is its fill partner.
   - Read both whole, then bind OFF with predictions. Use ships20's env-gated `BSP_ORIGIN_DIAG`:
     `recon[PARTY_ALLIED].own` entries should become non-zero, and JM08 36200/36000 should start
     the invasion near frame 10800.
   - Pair on JM08 36000 plus the usual rows, flip by verdict, and list every scripted mission
     that moves.
2. **Ranking #7:** the squadron `+348h` command block, `approach+6Ch`, the moveto circle radius
   (docs/GAMEPLAY_GAP_RANKING.md).
3. **Ranking #15:** the airfield destruction slot `006D40F0`. It is parked until a row destroys a
   hangar.
4. **5ah (a)(b)(c):** the carrier elevator chain. Unchanged from 5ai item 5.
5. **Low priority:**
   - unit SetParty with the event-6 group removal (SHIP_AI 63);
   - renaming the withdrawn `unit_lacks_follow_target` to `unit_is_flight_leader`. `unit+9D8h` is
     the squadron member slot (AA_LETHALITY_AUDIT 12). Uses are in `dive_bomb_task.hpp` (about 298
     and 1435), `torpedo_release_orders.hpp` (about 146 and 198) and `game_hosts_units.cpp`
     (about 3835 and 14432). Check the polarity at each use, and put it in a commit that already
     touches those files.

**Open from this worker's packets:**
- JM05 9000's follow error is 390 m even with turbo: |cross| 209 m, and 40% of ticks beside
  the station. That is a steering or station problem, not speed.
- The follow exit's immediate turbo clear (`009BDE40`) is one think late in the host.

**Tools in the cc9-lua21 tree** (`local\`). Copy what you need with your own prefix.
- `l21_emu.py` + `l21_harness.py`: an x86/x87/SSE interpreter that runs the image's own bytes on
  fake objects. A read of an unset field stops the run and names it.
- `l21_sym.py`: a concolic SSA trace.
- `l21_blocks.py` / `l21_raw.py`: block-level symbolic listing / listing with the exact x87 depth
  (`l21_depth.json`).
- `l21_paths.py`: random-input path census with coverage.
- `l21_probe*.{cpp,py,ps1}`: checks a C++ transcription against the image through a probe exe
  (link with `/FORCE:UNRESOLVED`).
- `l21_disp.py`: the displacement scan.
- `l21_dword.py`: absolute-dword references, which find vtable slots.
- `l21_rd.py`: reads dwords at an address.
- `l21_runs.ps1` / `l21_wait.ps1`: the launch rows and the foreground wait.
- `l21_along.py`: aggregates the per-400-tick `follow law` rows.

**Environment:** runs were clean all session. The window is 1600x900 and a 3000-frame row takes
about 20-40 s.

## 5ak. Handoff (cc9-lua22, 2026-09-30, stamped 07:29 UTC)

**What cc9-lua22 landed.** Each row is in main through the lead's merges; the last one is
merging.

| packet | switch | state | section |
| --- | --- | --- | --- |
| `cc9_recon_publication` (5aj item 1) | `kReconPublishBound` (`game_hosts_lua.hpp`) | **ON**: JM08's invasion starts at frame 14005; USN02, JM05 and JM05 long move; 15 rows identical | RECON_PUBLICATION 1-4 |
| USN02 idle failure (lead's check) | none | authored: arcade Type 93 at 170.444 m/s (locally modified `bulletclasses.lua`) sinks Houston at 20.85 s | RECON_PUBLICATION 5 |
| `cc9_moveto_command_range` (ranking #7) | `kMoveToCommandRangeBound` | **ON**, stage-only: twelve rows identical; no row issues a ranged `PilotMoveToRange` | PILOT_MOVETO_TASK, last section |
| `cc9_follow_cross_track` (5aj low priority) | none (diagnostic) | JM05's residual is the land task's holding pattern (99.6% of follow ticks), not the cruise law | PLANE_FOLLOW_PHASE_A 10 |

**The queue:**
1. **Ranking #15**, the airfield destruction slot `006D40F0`: still parked until a row destroys a
   hangar.
2. **5ah (a)(b)(c), the carrier elevator chain.** Unchanged from 5aj item 4 and 5ai item 5:
   - (a) the host's C3h state-2 path and the rule for `+900h` 2;
   - (b) park's carrier arm `009B23E4` and the elevator site `006D0600` / `006FC480` /
     `006FC720`, committed OFF;
   - (c) the pair with `kLandParkStateBound=true` on both sides of JM05 9000.
3. **The land task's holding speed (new, from PLANE_FOLLOW_PHASE_A 10).** JM05's follow error is
   in `follow (land)`. Members sit behind leaders held near 31.5 m/s by the moveto blend's
   wingmen-wait term, and 23% of ticks are below 40 m/s. Check that term and the land follow's
   station against the image before touching the cruise follow law.
4. **Low priority:**
   - unit SetParty with the event-6 group removal (SHIP_AI 63);
   - renaming the withdrawn `unit_lacks_follow_target` to `unit_is_flight_leader` (5aj item 5,
     unchanged). Check the polarity at each use.
   - the one-think-late turbo clear (`009BDE40`).
   - Untested at run time: the ranged moveto orbit. `usn_19_coralus.lua`'s `moviefisher` (3750 /
     3900) and LOMP06's seaplanes (500 / 1000) are the authored uses. LOMP06 9000 and USN04 36000
     do not reach them.

**Recon notes for the next worker:**
- The publication runs once per host recon pass. It publishes a party only when its content
  changed; that stands in for the `+25h` dirty byte.
- A class's category comes from `bsp::recon_publish_category_for_class` (`src/recon_slot_lists.cpp`,
  read from the image's `+170h` vtables).
- The rows that moved belong to reference T.

**Tools in the cc9-lua22 tree** (`local\`). Copy what you need with your own prefix.
- `l22_runs.ps1` / `l22_wait.ps1`: the reference rows plus `jm08x` (JM08 36200/36000), `lomp06l`
  and `usn04x`, with `-Diag` for `BSP_ORIGIN_DIAG`.
- `l22_scripts.py`: each log's mission script and its recon reads.
- `l22_grep.py`: a regex count over this installation's scripts.
- `l22_fl.py`: per-member `follow law` rows.
- `l22_cat.py` / `l22_pair.py`: the `+170h` vtable slot-0 census.
- `l22_disp.py`: the displacement scan.
- `l22_leasewait.ps1`: a foreground wait for a shared file's lease.

## 5al. The land holding speed: no host difference; the follow ticks are the sequencer's mode-1 phases (packet `cc9_land_holding_speed`, cc9-lua23, 2026-09-30)

The question (5ak item 3, PLANE_FOLLOW_PHASE_A 10): JM05's follow error lives in `follow (land)`.
Is the leader held near 31.5 m/s by the moveto blend's wingmen-wait term, and does the land
follow's station differ from the image? **Answer: the host matches the image on both, and the
premise does not hold. Nothing is bound.**

### The image, read for this packet

- **The land moveto's speed slot is `009C1850`.** `moveto (land)` has vtable `00D20AEC`
  (constructor `009C2AC0` at `009B2EC9`); its `+1Ch` dword is `009C1850`, the slot `009C18C0`
  calls at `009C198A`-`009C1999`. `009C1850` computes `009BECD0([approach+0Ch]+3A0h,
  007C47F0(), sep)`. The land approach's head is `009F9CE0`, which sets `+0Ch` = `unit+9D4h`, the
  squadron. So the land leader gets the same blend as the dive-bomb moveto. The host's
  `run_land_moveto_tick_009c18c0` already calls `moveto_speed_009c1850` there.
- **`009C18C0`'s target** is `[state+2Ch]`'s position (`+FCh`..`+104h`, `009C18EC`-`009C1913`).
  For the land task that is `[approach+30h]`, the block's owner (the carrier). The host uses the
  site slot's position.
- **`follow (land)` is the shared follow state.** `009C2980` (`RET 8`) writes vtable `00D20AB8`,
  whose tick is `009C1FD0`. The 100.0 that `009B2ECE`-`009B2ED5` pushes is not read by the body.
  The live decompile shows one parameter; the listing sweep found no load of the second stack
  slot. The approach constructor `009B2E50` rewrites only the six later states' vtables. So the
  station is `009BFD70`'s, the one section 8.1's oracle matched.
- **The wingmen value, per member.** The publisher is `007BCC20`:
  - it answers -1 through `007BCC6F` when the plane is dead (`+5Eh`), remote (`+5Dh`), or has no
    squadron, `+9D0h` or bot;
  - it answers 0.0 when `(plane+72Ch)->vtable[38h]` is false;
  - otherwise it tail-jumps to `00999AE0`. That walks the bot's tasks (`+58h`, count `+5Ch`) and
    tail-jumps to `vtable[4Ch]` of the first task whose `vtable[34h]` answers false. With none,
    it returns -1.
  - The land task's vtable `00D1FFA0` has `+34h` = `0099B700` (false) and `+4Ch` = `009B3750`.
  - `009BE3E0` reads the plane's `+FCh`..`+104h` less the follow state's `+30h`..`+38h`, the
    leader's `vtable[50h]` heading, and the `+85h` latch.
  - The host's `follow_wait_value_009be3e0` takes the same inputs.

### Measured (diagnostic `summary land moveto speed`, commit `ffb2a7aca`)

Logs are `local\l23_d1_jm05{,l}.log`, reference launch form. JM05 9000's follow rows are
identical to cc9-lua22's `l22_fct2_jm05l.log`.

| row | calls | a | b | want | wingmen | own v | sep | at b | wingmen < 1 | sep < 3000 | v < 40 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| JM05 3000 | 2440 | 103.5 | 34.1 | 83.5 | 0.690 | 52.7 | 3343 | 0 | 1056 | 41 | 831 |
| JM05 9000 | 8035 | 100.9 | 34.5 | 81.3 | 0.682 | 50.8 | 3403 | 11 | 3810 | 47 | 2840 |

- **The land leader is not held at the floor.** It sits at b in 11 of 8035 calls. The wingmen
  term does pull the command down, to about 81 of 101 m/s.
- **Its low speed is not commanded.** 35% of the calls are below 40 m/s while the leader wants
  about 81 m/s at almost full throttle (`follow speed ceiling`: `thr_leader` 0.906). The speed is
  lost in flight. The climb to the glide's far altitude (section 5) is the likely cause; this is
  not measured here.
- **The 31.5 m/s rows are the members' own commands**, not the leaders':
  - the follow law's ahead-of-station arm (`follow law ... spd=31.5` with `along` > 0);
  - `land/standby`'s speed (for example `USS Lexington_sqn03|.-3 state=5B8 spd=31.50` at 25.65 s).

**Where the follow ticks come from** (`local\l23_states.py`, from the `state A -> B` lines):

| row | member-s in follow (land) | head in `land/standby` | head in `moveto (land)` | head 2<->3 flips |
| --- | --- | --- | --- | --- |
| JM05 3000 | 1197 | 710 | 488 | 61 (16 heads) |
| JM05 9000 | 3373 | 1771 | 1601 | 120 (19 heads) |

- The members are not in a steady cruise formation. They re-enter `follow (land)` whenever the
  sequencer gives them mode 1 (5b, `006C7960`):
  - **The head flies `moveto (land)` from outside StandbyDist** (head mode 1).
  - **The head is in `land/standby` but off its circle** (head mode 2). Its `006C5C40` heading
    test fails, and a member with head mode 2 gets mode 1 unless `006C5E20` answers.
- One member of Lexington_sqn03 switches between modes 3 and 1 every 2 to 5 s from 35 s on
  (22 changes by 113 s).
- Each switch changes the member's state, `land/standby` <-> `follow (land)`. So the station error
  is re-seeded each time, which explains the large error while following a turning head.

### Verdict

- No switch. The speed chain and the follow station are the image's. PLANE_FOLLOW_PHASE_A 10's
  "leaders held near 31.5 m/s by the wingmen-wait term" is corrected: the leaders are not held,
  and 31.5 m/s is the members' own command.
- **Open, for the queue** (neither is read yet):
  1. **The head's 2<->3 chatter.** Is it the image's? 006C5C40 picks the circle point by the
     side of the runway axis (`x > 0` in the holder frame). The radius `006C3E50` grows with the
     record count (`r` 1100 -> 2558 m on Lexington_sqn03 between 5.75 s and 75 s). A head circling
     a point at more than the offset `T.x + r` crosses the axis, the side flips, and the
     heading test fails. This is a hypothesis: check `land/standby`'s circle steer and when
     006C5C40 fails, before anything else.
  2. **The moveto leader's speed loss** (35% of calls below 40 m/s at full throttle). Check the
     glide's commanded altitude against the leader's climb.

## 5am. The head's mode 2<->3 chatter: the image's 20-degree gate, not a side flip (packet `cc9_land_head_chatter`, cc9-lua23, 2026-09-30)

The question (5al open item 1): on JM05 the land heads' sequencer mode flips between 3 (on the
landing circle) and 2 (inside StandbyDist, off it). Each flip moves the wing members between
`land/standby` and `follow (land)`. Is the flip a host defect? **No difference from the image was
found, and nothing is bound.** 5al's hypothesis, that the circle side flips across the runway
axis, is refuted.

### `006C5C40` re-read from the listing

The body is `006C5C40`-`006C5E18`. Each step matches `landing_on_circle_006c5c40`:

1. **Side.** Side = `006BCC90(pos).x > 0` (`006C5C8F` `COMISS` against `00D7A218`, `JA`), held
   at `[esp+30h]`.
2. **Circle point.** P = `006C5380(plane, side)`, and d = `00414C60` (the 2D length) of P - pos.
3. **Distance gate.** False when d > r x 1.6 (double `00CE3D48`), with r = `006C3E50(plane)`.
4. **Offset.** +pi/2 (`00CE3C64`, `3FC90FDB`) for side 1, and -pi/2 (`00CE3CCC`) otherwise.
5. **Angle.** delta = |`00438B10`(`00414EB0`(P - pos) - off, `vtable[50h]()`)|. The heading is
   staged as `00438B10`'s second stack argument before `00414EB0` is called (`006C5D48`-`006C5D51`).
6. **Threshold.** `00419010`(r x 0.25 (double `00D7A348`), 45 deg (`00CEB5A8`), r x 0.8 (double
   `00CE3D40`), 20 deg (`00CE398C`, `3EB2B8C3`); d). The stack order was traced from the pushes at
   `006C5D9E`-`006C5DF3`.
7. **Result.** True when the threshold is greater than delta (`006C5E01` `FCOMIP`, `JBE` to false).

### Measured (diagnostic `summary land head circle`, commit `524d449ff`)

The diagnostic runs at `006C7960`'s leader pass for each head in `land/standby`. Logs are
`local\l23_d2_jm05{,l}.log`.

| row | samples | on | beyond 1.6 r | heading fail (mean delta) | side differs from +44h | own-point heading < 90 deg | d / r | point moved | 3 -> not 3 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| JM05 3000 | 4972 | 4267 | 115 | 590 (1.058) | 1 | 4835 | 0.983 | 83.1 m | 48 |
| JM05 9000 | 8665 | 6907 | 159 | 1599 (1.334) | 1 | 8306 | 0.959 | 259.0 m | 96 |

The columns:
- **Side differs from +44h:** the side `006C5C40` takes from the head's position against the side
  the standby steer `009FBB20` flies.
- **Own-point heading < 90 deg:** the same test against the steer's own point `+38h`.
- **Point moved:** P now against `+38h`.

What the numbers say:
- **The side never flips** (1 sample in 8665), and the heads circle the right way round
  (96-97% within 90 deg of the tangent of their own circle).
- **Every traced exit is the heading gate** at d beyond 0.8 r, where the threshold is 20 deg.
  Examples of the delta at an exit: 0.35-0.55 rad, with d / r between 0.63 and 1.36.
- **Carrier heads make 47 of the 60 traced exits on JM05 9000.** Lexington_sqn03 alone has 11.
  The 13 airfield exits (F4F, SB2C and the airfield squadrons) come in two groups: four within
  20 s of entering standby, and nine between 80 s and 175 s. The latter are likely re-entries or
  r changes, but they are not split here.
- **Two image mechanisms keep a carrier head off its circle:**
  - **The circle point rides the carrier.** `00758E80` re-frames the holder on every carrier
    update (5ae). P sits PosBehind (780 m) aft and r abeam, so a carrier turn swings P by
    hundreds of metres.
  - **r follows the deck's record count.** `006C3E50`'s f runs over RadiusChange 10 -> 50 records
    (1.0 -> 2.5, squared). Lexington holds 24 records on JM05 3000, so r = 1100 x 2.33 = 2558 m.
    Each insert or release on the deck moves every head's r at once. At 64.35 s Yorktown_sqn08's
    r went from 3014 m to 1296 m, and P moved 2462 m.
- The circle is only 1.1 TurnCircleRadius at f = 1 (CircleMultiplierMin), so a head has little
  turn margin to recover within 20 deg.

### Verdict

- No switch. The chatter is the image's own gate, fed by the carrier's motion and the deck's
  record count.
- One host input is the head's circle tracking, which depends on the flight laws (the heading
  response and the standby speed command, `+2B4h` up to TravelSpeed x 1.6 at spacing 1.0). It is
  not compared here: this host has no measurement of the original at run time.
- Left for the queue:
  - 5al item 2, the moveto leader's climb speed loss.
  - Whether JM05's carrier record counts (Lexington 24) match what the image would hold once the
    carrier elevator (5ah) removes landed planes. That is part of the elevator chain.

## 5an. The carrier elevator, step (a): the state-2 request and what `+900h` 2 does to the land task (packet `cc9_elevator_state2_read`, cc9-lua23, 2026-09-30)

This is 5ah's step (a). It was read from the disk listing only, and no code changed.

### The request `007C2090` (`007C2090`-`007C2126`, `__thiscall(plane)`, plain `RET`)

- **It returns without a message** in either of two cases:
  - `+900h` is already 2 (`007C20B0`);
  - the game's `[00E188A8]+1FE4h` is 2 (`007C20BE`). That is the network mode; it is not 2 in
    a single-player mission.
- **Otherwise it builds message `C3h`** (`0075B430(C3h)`, vtable `00D03504`):
  - `+20h` = the current `+900h` (the byte);
  - `+21h` = 2.
- **It routes the message** through `0077C2A0(plane, msg, 7, 0)`.

### The handler `007CCFA0` (`BSP_Plane_HandleMessage`)

- **Kind dispatch.** The kind byte `+10h` less `7Ah` indexes the byte table `007CD1D8`. For
  `C3h` that is `007CD221` = 1, and the dword table `007CD1AC` gives `007CCFEF`.
- **State dispatch.** `007CCFEF` takes the requested state `+21h` - 1 into the table `007CD22C`,
  whose entries 1..7 are:

  | requested state | handler address | what it does |
  | --- | --- | --- |
  | 1 | `007CD00A` | calls `007CC820` |
  | 2 | `007CD01B` | calls `007CC7A0` |
  | 3 | `007CD0A8` | default |
  | 4 | `007CD02C` | the touchdown pair of 5k |
  | 5 | `007CD06D` | the touchdown pair of 5k |
  | 6 | `007CD090` | not read here |
  | 7 | `007CD0A1` | not read here |

  The state-2 arm does not test the current state.

### `007CC7A0` `BSP_Plane_EnterFlightStateTwo` (`007CC7A0`-`007CC810`, `__fastcall(plane)`, `RET`)

It does five things, in order:
1. **Network mode only** (`game+1FE4h` == 2, not single player): `007C78A0(2, +904h)`.
2. **Throttle.** `+9F0h` = 0.0 (`007CC7CC`), unconditionally.
3. **The state write**, when `+900h` is not already 2:
   - `+900h` = 2 (`007CC7E2`);
   - `+C04h` = -1.0 (`00D7A260`), so the site probe `007C5AC0` runs on the next step;
   - `007C11E0(0)` (`BSP_Plane_NotifyFlightState`).
4. **Controlled-plane clear.** With no pilot at `+9D8h`, the byte `game+193Ch` is cleared.
5. It returns.

### What state 2 does to the land task and to the motion

- **The land task stops thinking.** Its rule `009B3CF0` has no arm for `+900h` 2; its only
  `+900h` arm is 4 or 5 -> park. The bot tick's gate 8 (`0099AE5F`, `0074E230`) admits only
  `+900h` 4 to 7 (docs/PILOT_BOT_TICK_GATES.md). So from the state-2 write on, `0099ACD0` bails:
  - neither the rule nor the park tick runs again;
  - the park <-> abort loop ends, which answers 5ah's question 3 from the image side.
- **No motion arm runs.** The fixed step's dispatch `007CE040` takes free flight only when
  `(+72Ch)->vtable[38h]` answers (`+900h` 7), the ground roll for 4 or 5, and the surface for 6.
  For 2 it takes none (`007CECA0`). The plane moves only as the platform carries it: `006FC0D0`
  writes `unit+A4h..+ACh`. At the bottom it is hidden (`007B96C0`) and released (`006FC250`).
- **Other `+900h == 2` readers.** A byte census of `CMP dword [reg+900h],2` (`83 B8..BF 00 09 00 00
  02`) finds two sites:
  - `0074E222`, the predicate `0074E220`, which has no rel32 caller and no dword reference;
  - `007CC7C2`, above.
  Register compares (`CMP [reg+900h],EAX`) are not covered. The corridor's 2-or-4 refusal is in
  `006C3B10` (5b).
- **The sequencer keeps the record.**
  - `006C7960` answers mode 4 for a `+904h` plane before any other test (5b, 5r).
  - `006C45C0`, the squadron release, needs the head airborne. A squadron whose head has landed
    and gone below is never released by it.
  - So the image, too, keeps landed-and-stowed planes on the deck's `+A8h` vector unless another
    path erases them (not found here). 5am's question, whether the elevator lowers the record
    count, leans to no. The count drops only through a release, and the release needs an
    airborne head.

### The host today

- The host never writes `+900h` 2 (the census of `plane_control_mode_900` stores finds 4, 5, 6 and
  7 only).
- Its motion dispatch (`select_motion_arm_007ce040`, `src/plane_flight.cpp`) already answers
  `None` for 2.
- Its pilot pass runs at the head of the ground-roll arm, so a state-2 plane would neither think
  nor move, as in the image.
- A binding of step (b) needs these pieces:
  - `007C2090` as a direct state write (single player, no message queue): `+900h` = 2,
    `+9F0h` = 0, `+C04h` = -1.0, and the notify;
  - the platform carry `006FC0D0`;
  - the hide and release.

### Next: step (b)

Park's carrier arm `009B23E4` (the `bVar4` branches, the site slots `+2Ch`/`+44h`/`+34h`, and
`006CE610`), and the elevator site `006D0600` / `006FC480` / `006FC720` with the state write
above. All of it goes behind `kLandParkStateBound`, committed OFF.

## 5ao. The carrier elevator, step (b): park's carrier arm and the lift, bound OFF (packet `cc9_carrier_elevator`, cc9-lua23, 2026-09-30)

Read from the disk listing whole: park's tick `009B22C0` for its carrier flag, the done test
`009B21D0`, the mother-ship site's slots and the platform. This section adds to 5ah.

### Park's carrier arm (`009B22C0`, the flag at `[esp+13h]`)

The flag is set at `009B23EA` when `approach+30h vtable[5Ch](9)` answers true (`009B23E4`).
Where it is set, the tick differs from the airfield arm (5s) as follows.

- **Targets.**
  - The site calls go to the mother-ship vtable `00CF8A58`:
    - `+44h` is `006D0120`: q = the lift point, `class+808h..+810h` of the carrier;
    - `+2Ch` is `006D00E0`: t = the same point, with z less `planeDesc+158h`.
  - Both points are in the carrier's model frame, the frame of plane `+A4h..+ACh` under the
    carrier.
  - `planeDesc+158h` is written at `007D473A` as (model `+3Ch` + model `+30h`) x 0.5. That is
    most likely the model box's centre along z.
- **No retarget.** `009B23EF` jumps over the retarget block (`007B9000`/`007C1680`).
- **Speed.**
  - base = `Pilot/Landing/ParkVelocity` (tuning `+4E8h`, KTS 40 in this installation's
    planeglobals.lua) instead of AirField/MoveSpd (`009B24C7`).
  - The `00419010` ramp, the 0.85 v floor and the `+910h` queue flag are all jumped over
    (`009B24E7` -> `009B260C`).
  - At `009B260C`-`009B2631`, `park+28h -= dt` while the plane is slower than 1.0 and within 10 m
    of t in z. This is the clock 5ah names.
- **No path.** `009B265F` skips the path test, so `bl` stays 0 and a plane on the path leaves it.
- **Steer when slow** (`009B27F6`-`009B2851`): sx = dx, sz = min(dz, max(1.0, 12.0 - |dx|))
  (double `00CE42D0` = 12.0), and f18 stays dz.
- **Lane test.** It calls site `+34h` = `006D0390` with the enable byte clear (`009B28F9` `SETE`).
  - `006D0390` answers false when `006CE610` does.
  - Otherwise, with Lz = `class+810h` and pz = plane `+ACh`, it answers true outside the lane
    Lz - (19 + 2 max(0, v - 1)) < pz < Lz - 12. The constants are 14.0 at `[00E08FC8]`, the
    doubles 2.0 (`00D7A308`) and 5.0 (`00D7A370`), and 1.0 (`00D7A210`).
  - Inside the lane, `006D0150` decides. It is true when the platform is still (`+94h`), at the
    top (0 > `site+5Ch`), and no other occupant's `006CFE90` distance is under 14.0.
- **Speed clamp** (`009B2923`-`009B298D`):
  - behind the target (0 > f18): spd = 0;
  - otherwise c = max(0.5, f18 x 0.5); spd below 0.1 becomes 0.1, else min(spd, c).
- **Done test** (`009B2228`): past the target in z, `009B1E30` (the class-9 owner) sends the plane
  to site `vtable[3Ch]` = `006CFF70`. A plane that answers there is not done.
  - `006CFF70`: (3.0 (double `00D7A2B0`) - speed) > `006CFE90`.
  - `006CFE90`: the plane's point (0, 0, `planeDesc+158h`) through `unit+74h`, then its x/z
    distance to the lift (`site+50h`/`+58h`).

### The lift (mother-ship site `+4h` `006D0600`, `+8h` `006CFE40` -> `006FC480`)

The platform sits at `site+44h`. The reading in 5ah holds; these are the details it lacked:
- **The platform's fields.**
  - `+0Ch..+14h` the lift point, `+18h` the depth, `+1Ch` = depth x 0.1 (`006FC3D0`, double
    `00D7A3A0`);
  - `+34h` the platform plane, `+38h..+40h` its offset from the lift;
  - `+44h` ElevatorSpeed, `+48h` ElevatorDepth, `+4Ch` 1.0, `+50h` the mode.
  - The values are 4.0 and 7.0 in this installation's shipglobals.lua, `ShipGlobals.MotherShip`
    (mtime 2024-07-13).
- **`006FC480`, the platform tick.**
  - Mode 1: `+18h -= speed x dt x +4Ch`, stopping at -`+1Ch`.
  - Mode 2: `+18h += ...`, stopping at depth + `+1Ch`.
  - Then `006FC0D0` carries the plane: local position = lift + offset, with y less
    clamp(`+18h`, 0, depth); then `00951F40(1)`.
  - A still platform resets `+4Ch` to 1.0.
- **`006FC720`, the take.** It returns at once for the plane already on the platform. Otherwise:
  mode 0, `+18h` = -`+1Ch`, the old plane dropped; the offset = the plane's local position less
  the lift; `007C2090` (the state-2 request, 5an); then down (mode 2).
- **`006FC250`, the release.** It carries, drops the plane, and calls `00951F40(0)` when the
  platform is still and at the bottom.
- **`006D0600`, the site tick.** At the top it scans the occupants for a candidate plane with:
  - `+904h` set;
  - `006CFF70` true;
  - speed < 1.389 (`00CF8AAC`);
  - `007B8D40` true.
  It takes the last such candidate. At the bottom it hides the plane (`007B96C0`) and releases
  it; after LiftDelay (0.5) with the platform empty, it sends the lift back up (`006FC6B0`).

### The binding, behind `kCarrierElevatorBound` (committed OFF)

It is meaningful only with `kLandParkStateBound` on.

**Bound:**
- the carrier arm of the park tick;
- the done test's intake exemption;
- `006D0390`/`006D0150`, `006CFF70`/`006CFE90`;
- the site tick, the platform tick, the take, the carry and the release;
- `007CC7A0` as a direct state write: `+9F0h` = 0, `+900h` = 2, `+C04h` = -1.0.

The lift point comes from the carrier model's (`liftexitpoint`, 2) Aux group (`00759167`), and
ElevatorSpeed/Depth from `ShipGlobals.MotherShip`.

**LABELLED:**
- `planeDesc+158h` is 0.0: the plane's origin stands in for the box centre.
- The ready plane `+18h` is never set, because the relaunch feed is not reconstructed. So the
  relaunch arm sends only the empty lift back up (counted as `unfed_relaunch`).
- `007B8D40` (the gear channel) answers true, because there is no `+DECh` block.
- `00951F40`'s node detach and show are not carried. A stowed plane keeps its slot, stays in
  state 2 and is no longer moved or thought for.
- `site+1Ch`'s `006CEDD0` is taken clear.
- The two site slots run from the landing queue's caller, before the queue.
- `007C11E0(0)` is not carried, as at the 4 <-> 5 transitions.

**New lines:**
- `carrier lift class ...` and `carrier elevator <carrier>: lift=... speed=... depth=...`;
- `  carrier elevator ...: takes <plane>` and `... stows <plane>`;
- `summary carrier elevator <carrier>: ...`.

### Predictions, written before any ON run

Both sides run with `kLandParkStateBound=true`. OFF is that alone; ON adds
`kCarrierElevatorBound=true`. The rows are JM05 9000 and JM05 3000, with USN04 4500 and LOMP10
9000 as controls.

1. **JM05 9000:**
   - Both US carriers (Lexington, Yorktown) read a lift point and build their lift.
   - OFF: every carrier-landed plane that enters park is refused at `006CF520` (a carrier
     deck has no authored hangar). It stays in park, on deck, near the stern (5ag.1: local z
     -38 to -42 on Yorktown, -134 on Lexington).
   - ON: those planes taxi forward toward the lift at up to ParkVelocity (20.6 m/s) and slow
     to max(0.5, dz/2) near it.
   - The first plane on each deck reaches the lift and is taken (`takes`, state 2) within 60 s
     of its park entry, and is stowed about 2 s later (7.7 m at 4 m/s).
   - Later planes queue: the lane test stops them 12 to 19 m aft of the lift while it is busy.
   - `intakes` >= 1 on each deck with a park entry, and `stowed` = `intakes` or one fewer.
2. **The park <-> abort loop falls on carrier planes.** The `from_abort` counts of the stowed
   planes stop growing at their stow time, because state 2 stops the bot (5an).
3. **The sequencer records stay** (5an): the deck record counts are unchanged by the stows.
4. **JM05 gameplay:** deaths identical, and the diff exit is 3 (positions moved).
   - A stowed plane is still a unit this host can target, which is labelled.
   - A moved death would be a plane on deck (OFF) that is below (ON). It is recorded per
     entity.
5. **Controls.**
   - LOMP10 9000 has no carrier deck, so it is gameplay identical (exit 0 or 1).
   - USN04 4500: its Lexington lands no plane in 4500 frames (5ag), so exit 0 or 1.

**Mechanism failure**, any of:
- a carrier plane in park that never comes within 12 m of the lift while the lift is free;
- a take with the plane more than 3 m from the lift;
- a stowed plane that moves again;
- a plane running off the deck (contact lost) on the way to the lift.

**Flip rule:** ON when 1 and 2 hold with no mechanism failure. The flip is of
`kCarrierElevatorBound` only; park itself stays OFF for the airfield loop (5aa).

### 5ao.1 Measured (pairs on `13fab4abe`), and the verdict: ON, staged behind park

- **OFF** is `local\l23_eoff` (`kLandParkStateBound=true`).
- **ON** is `local\l23_eon`, which adds `kCarrierElevatorBound=true`.
- The logs are `local\l23_e{off,on}_<row>.log` and the diffs are `local\l23_ediff_<row>.txt`.
- Everything was run in the reference launch form.

| row | `pair_diff` | note |
| --- | --- | --- |
| JM05 9000 | 3 | deaths identical (5 rows); positions moved; the torpedo-task line "0 of 2" is absent ON |
| JM05 3000 | 1 | the lift is unexercised (the first carrier park entry is at 178.76 s); only the new lines |
| USN04 4700/4500 | 1 | gameplay identical |
| LOMP10 9200/9000 | 0 | identical |

**The lifts.** Each carrier's (`liftexitpoint`, 2) point was read. Lexington's is
(-0.831, 17.396, 39.359) and Yorktown's (0.069, 15.366, 80.959); Shokaku and Zuikaku share
(0.069, 17.296, 42.709). All four lifts built with speed 4.0 and depth 7.0.

**The predictions:**
1. **Held.**
   - OFF: all 8 carrier park entries are refused at `006CF520` (3456 refusal calls). The planes
     stay where they stopped.
   - ON: each carrier plane taxis from local z of about -138 at up to 20.58 m/s. It slows
     under the clamp (7.3 m/s at z = 24.7, 2.1 m/s at z = 35) and is taken 13.3 to 13.9 s after its
     park entry (the four planes whose entries are traced).
     - The take offsets are (-0.07..0.12, -0.3..-0.7, -1.1..-2.6), all inside 3 m.
     - Each plane is stowed 2.15 s later.
   - Lexington takes and stows 8 of 8: Lexington_sqn03 and its two wingmen, Yorktown_sqn04
     and its two wingmen, and Yorktown_sqn08 and its first wingman.
   - The later planes queue: `spot_refused` is 11 to 19 on them. After each stow the empty lift
     comes back up (`empty_up` = 8).
   - Yorktown's own lift takes nothing: every carrier landing on this row is on Lexington's
     deck.
2. **Moot, then held.** No carrier plane looped park <-> abort on the OFF side either (the
   refusal keeps it in park), so there was no count to fall. ON, every stowed plane stays in
   state 2 with `from_abort` = 0 and does not move again (last z 36.6 to 38.2).
   - The airfield planes' loops are identical on both sides. For example, F4F Wildcat 01 has 220
     and MainAirfieldEntity 01_sqn01|.-2 has 672; that loop is 5aa's and is untouched here.
3. **Held as read, and it moves the count up.** The records stay after a stow (5an). Because the
   deck is cleared, more planes land, so Lexington's final `+A8h` count rises from 18 (OFF) to 24
   (ON), with landed arm hits 2273 -> 4344.
   - OFF, the wingmen of a refused head never land. They re-install (Lexington_sqn03|.-2/-3 at
     303.30 s) and keep circling.
4. **Held.** Deaths are identical (5 rows), and the exit is 3.
5. **Held.** LOMP10 is exit 0 and USN04 exit 1.

**No mechanism failure.** Every take is within 3 m, no stowed plane moves, no carrier plane
loses deck contact, and none reaches `done`. Two planes are still taxiing at the end of the run:
Yorktown_sqn06 (entered at about 442 s) and Yorktown_sqn10|.-2.

**Verdict: `kCarrierElevatorBound = true`.**
- It is staged: with `kLandParkStateBound` OFF (5aa's airfield loop), no reference row reaches
  it, and only the new `carrier lift`/`carrier elevator` lines appear.
- On a carrier, park now ends as the image's does: on the lift, below, in state 2.
- Step (c) is therefore answered: the elevator ends the carrier's park <-> abort question. The
  airfield half of park is what still holds park OFF.

## 5ap. Handoff (cc9-lua23, 2026-09-30, stamped 08:56 UTC)

**What cc9-lua23 landed** (all in main through the lead's merges; the last is merge `15b678442`):

| packet | switch | state | section |
| --- | --- | --- | --- |
| `cc9_land_holding_speed` (5ak item 3) | none (diagnostic) | the 31.5 m/s leader premise is refuted; the follow ticks are the sequencer's mode-1 phases | 5al |
| `cc9_land_head_chatter` | none (diagnostic) | the head's 2<->3 flips are the image's 20-degree gate, fed by the carrier's motion and the deck's record count | 5am |
| `cc9_elevator_state2_read` (5ah a) | none | `007C2090` -> `007CCFA0` -> `007CC7A0`; state 2 stops the bot (gate 8), so the land task stops | 5an |
| `cc9_carrier_elevator` (5ah b, c) | `kCarrierElevatorBound` | **ON, staged behind park**: JM05 9000 with park on stows 8 of 8 carrier planes | 5ao, 5ao.1 |

**The queue:**
1. **Park's airfield loop.** This is the only thing holding `kLandParkStateBound` OFF: the
   airfield planes loop park <-> abort after the hangar. On JM05 9000 with park on, for example,
   F4F Wildcat 01 loops 220 times and MainAirfieldEntity 01_sqn01|.-2 672 times, identically with
   and without the elevator.
   - **The question.** What does the image do to a parked airfield plane that ends the loop? This
     would be the airfield counterpart of state 2 below deck. 5x-5aa closed the laws (done test,
     probe, rule, deadband, rate floor) and found no retirement.
   - **A lead not yet read.** The hangar hide `007B96C0` -> `00951F40(0)` calls
     `00710B80 BSP_UnitPartInstance_DetachSpatialNode` on `unit+360h`, which removes the node
     from `BSP_SpatialIndex_GetSingleton` (`0042E630`) through `0098A500`
     (`BSP_SpatialIndex_DetachNode`, docs/SPATIAL_INDEX.md). Check whether the detached plane
     keeps any of the following. If it loses any of them, the image's hidden plane cannot drift
     off the strip, and the loop never starts:
     - its ground contact or terrain height (the probe `007C5AC0` uses the holder, but the
       ground roll's surface query may use the spatial index);
     - its collision;
     - its fixed step.
   - **Then decide.** Bind that effect OFF, pair LOMP10 9200/9000 and JM05 9000 with park on and
     off, and flip park ON if the loop ends.
   - 5ao.1's OFF and ON logs (`local\l23_e{off,on}_jm05l.log`, cc9-lua23 tree) already hold the
     park-on airfield loop counts for JM05 9000.
2. **The relaunch feed.** It gives the lift something to bring up. The ready plane `site+18h`
   comes from the ready-plane pull `006C6540` and the stock regeneration, neither reconstructed
   (`air_operations.hpp`). The elevator's relaunch arm is counted as `unfed_relaunch` (8 on JM05
   9000). The handler for flag 1 with a plane is `BSP_Plane_PlaceOnLaunchSpotLocked` plus
   `006FC810`, and the top release is `007C3C90(0)`.
3. **`planeDesc+158h`.** It is taken as 0.0 in the lift's nose distance and taxi target.
   `007D473A` writes (model `+3Ch` + model `+30h`) x 0.5; model is `classDesc+50h`, and its box
   layout is not read. Every take on JM05 was within 3 m, so this is not urgent.
4. **#15** stays parked until a row destroys a hangar.
5. **Low priority:**
   - SetParty with the event-6 group removal (SHIP_AI 63);
   - the `unit_lacks_follow_target` -> `unit_is_flight_leader` rename;
   - the one-think-late turbo clear (`009BDE40`);
   - the moveto leader's climb speed loss (5al item 2).

**Notes:**
- A stowed carrier plane keeps its slot and its sequencer record. It is targetable in this host
  (labelled; the image detaches its node).
- On JM05 9000, every carrier landing is on Lexington's deck; Yorktown's lift takes nothing.
- The diagnostics `summary land moveto speed` and `summary land head circle` stay, print-only.

**Tools in the cc9-lua23 tree** (`local\`):
- `l23_runs.ps1` (the reference rows, as in cc9-lua22's);
- `l23_states.py` (land state occupancy from the `state A -> B` lines);
- `l23_lft.py` (the `land follow trace` rows);
- `l23_edit3.py` and `l23_edit4.py` (the elevator binding, a model for anchor-checked edits).

## 5aq. The hangar hide's detach moves nothing: the lead of 5ap item 1 is refuted (packet `cc9_park_hide_detach`, cc9-lua24, 2026-09-30)

5ap item 1 asked whether the hangar hide's spatial detach takes away the hidden plane's ground
contact, terrain height, collision or fixed step. Ghidra was read only; no code changed, no run.

### What the hide does

- `007B96C0`-`007B96CF`: `+C00h` = 1, then `00951F40(0)` (5x).
- `00951F40`-`00951F7E` (`__thiscall(unit, char show)`, `RET 4`), listing:
  - it does nothing unless `unit+4A4h` (the scene node) is set;
  - `ECX` = `unit+360h` (the unit part instance), then `00710B80` for 0 or `00710AD0` for 1;
  - `00B6DA70(node+4A4h, 0.0 or 1.0, 0)`, `BSP_SceneNode_SetVisibilityFactor`, which only stores
    the float at scene node `+ACh` and recurses into the children (`00B6DA70`-`00B6DAAB`).
- `00710B80`-`00710BA2` (`__fastcall(part)`, `RET`): if `part+184h` is set,
  `BSP_SpatialIndex_DetachNode(0042E630(), part)` (`0098A500`), then `part+184h` = 0.

`unit+360h` is the plane's collision node (docs/COLLISION_SHAPES.md row D, `[owner+360h]`).

### Who reads the spatial index

The 25 callers of `0042E630` and the callers of the four queries (`0098ADD0` segment,
`0098B130` nearest unit on segment, `0098B370` sweep, `0098C630` sphere):

| consumer | kind |
| --- | --- |
| `BSP_Projectile_TraceSegmentAndImpact` `0084BF00`, `0084B6B0` | shell hits |
| `BSP_Explosion_GatherHitRecords` `00904470` | blast hits |
| `BSP_LineOfFirePredicate_Blocked` `0072CDD0` | line of fire |
| `BSP_Aim_ResolveRayToWorldPoint` `00957740`, `00957BD0` (gun aim) | aim rays |
| `00904400` (callers `007C2450`, `00864680` gunnery visibility, `009F1BC0` ship AI approach), `009043A0` (HUD pick) | segment tests |
| `BSP_CameraMover_KeepAboveWater` `0042F0C0`, `0042EF90`, `0043B9C0`, `00452BD0` (binoculars), `00547480`, `0060ABD0`, `008949D0`, `00894C00` | camera and HUD; `0042EF90`, `0060ABD0`, `008949D0` and `00894C00` have no direct caller and were not read further |
| `006D3B10`, `006D3C10` (airfield read and destructor), `00883BB0`/`00880CD0` (landscape), `0092AAE0` (ship collision install), `00712C80` (part destroy) | attach and detach |
| `00875BB0` -> `0098BDB0` | the per-step refresh of registered roots |

`007C2450`, the only plane-segment caller, is reached from `006082D0`/`00609BD0` (HUD segment
24), not from any tick.

None of them is on a plane's motion path:
- **Ground contact and height.** The probe `007C5AC0` asks the landing site (`006C0840`) and the
  holder's runway test (`006BC530`); neither touches the index (5k, 5x (i)).
- **Collision.** The index is a hit broadphase (shells, blasts, line of fire, aim). No fixed-step
  routine queries it for body contacts.
- **Fixed step.** `BSP_PlaneTickElement_FixedStep` (`007CE040`) exits early only on `unit+5Eh` and
  `unit+520h` (5x (ii)). The refresh `0098BDB0` only walks registered nodes, and a detached node is
  simply not refreshed. The plane's own pose is refreshed lazily by its readers (`00414DB0` on
  `+C8h` clear, for example at `007C5B01`).
- **Readers of `part+184h`.** A scan of `80 ?? 84 01 00 00 00` finds 15 sites. Only `00710AD0`
  and `00710B80` test it on a part. Two others were checked and are other structures: `0099ADD0`
  (`PilotBot_Tick`, `[bot+50h]` = the unit) and `007EE304` (`[esi+3D0h]`). The remaining hits are
  menu, weapon director, gun bot and ship AI code.
- **The visibility factor.** Loads of `unit+4A4h` (`8B ?? A4 04 00 00`) in the plane (`007B`-
  `007D`), bot (`0099`-`009B`) and fan-out (`0087`) ranges are all effects and message handlers
  (`007CA647`, `007D0A00`, `007D0B80`, `007D766F`), with no motion reader.

### Verdict

The image's hidden plane keeps its ground contact, its runway height and its fixed step. It
loses only its shell, blast and line-of-fire targetability and its drawing. Nothing in the hide
can stop it drifting, so this lead does not end the loop. There is no difference to bind, and
`kLandParkStateBound` stays OFF. This agrees with 5aa and 5z: as far as the listing shows, the
image's airfield plane loops invisibly the way the host's does.

What the hide does change is labelled in the host already (a stowed or hangared plane stays
targetable here; 5ap notes). A binding for that would be a targeting change, not a park change.

## 5ar. Park as a fidelity verdict: the image loop accepted (packet `cc9_park_verdict`, cc9-lua24, 2026-09-30)

5z, 5aa and 5aq read the image three ways and found nothing that retires a hidden airfield plane.
The lead's decision: the invisible park <-> abort loop is the image's own behaviour as far as the
listing shows. Keeping park OFF leaves the rebuild less faithful: carrier planes never reach the
elevator, and airfield planes never taxi to the hangar. This packet pairs `kLandParkStateBound`
ON against OFF (the elevator is already ON behind it) and flips park unless the loop visibly
touches gameplay.

**Build.** Both sides are exports of main `a6680e8bf`: `local\l24_poff` (no flips) and
`local\l24_pon` (`kLandParkStateBound=true`). The rows are run in the reference launch form with
`BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`.

**Rows.** These are the landing rows JM05 3200/3000, JM05 9200/9000, LOMP10 3200/3000, LOMP10
9200/9000 and USN04 4700/4500, plus two controls, USN01 3200/3000 and BSM01 3200/3000.

### Predictions, written before any run

1. **Deaths.** The per-entity death table is identical on every row. The looping planes are
   airfield planes of the player's side on JM05 and LOMP10. In this host they stay targetable (the
   image's hidden planes are not; 5aq), so a death that differs would be an enemy shooting a
   plane that the image has hidden. That would be the one interaction that counts against the
   flip.
2. **JM05 9000.** The counts match 5ao.1's ON side:
   - F4F Wildcat 01 has `from_abort` 220, and MainAirfieldEntity 01_sqn01|.-2 has 672;
   - Lexington stows 8 of 8;
   - `pair_diff` returns 3 (positions and the land-task counters move).
3. **LOMP10 9000.** The planes of 5x loop after their hangar entries: Lightning 01, Warhawk
   01|.-2/-3/-4, Lightning 01|.-2/-3/-4, and B-25 01 and 01|.-2. `pair_diff` returns 3.
4. **JM05 3000 and LOMP10 3000.** 1 or 3. The first park entries are late (JM05's first carrier
   park entry is at 178.76 s), so few loops.
5. **USN04 4500.** 1 (5ao.1 was gameplay identical with park on at the OFF side).
6. **Controls.** USN01 and BSM01 return 0 or 1.
7. **Wandering.** A looping airfield plane ends far from its field (5ao.1: Wildcat 01 at local
   (-2671, 1368)). If a looping plane comes into range of enemy AA or a ship, it shows up as extra
   `shots` or hit records against a plane. Those would be host-only interactions (the image's
   plane is hidden), recorded but not a mechanism failure of park.

### 5ar.1 Measured, and the verdict: park ON, the image loop accepted

The logs are `local\l24_p{off,on}_<row>.log` and the diffs `local\l24_pdiff_<row>.txt` (cc9-lua24
tree). A 300-frame smoke of the OFF build exited 0.

| row | `pair_diff` | death rows | note |
| --- | --- | --- | --- |
| JM05 9200/9000 | 3 | identical (18) | unit table: 12 `nearest` moves; deaths, hits (772), shots (953), damage and water contacts identical |
| JM05 3200/3000 | 1 | identical (12) | 7 airfield planes and none of the carrier planes enter park; no loop before 3000 frames |
| LOMP10 9200/9000 | 1 | identical (9) | no plane reaches park on this base (no `land park` summary on either side) |
| LOMP10 3200/3000 | 1 | identical (7) | as above |
| USN04 4700/4500 | 1 | identical (48) | |
| USN01 3200/3000 (control) | 1 | identical (17) | |
| BSM01 3200/3000 (control) | 0 | | |

**The predictions:**
1. **Held.** Every death table is identical.
2. **Held.** On JM05 9000, F4F Wildcat 01 has `from_abort` 220 and MainAirfieldEntity
   01_sqn01|.-2 has 672. Lexington stows 8 of 8 (Lexington_sqn03 x3, Yorktown_sqn04 x3,
   Yorktown_sqn08 x2, all in state 2). `pair_diff` returned 3.
3. **Failed as a premise.** LOMP10's landing traffic of 5x (2026-09-29) no longer reaches park
   on the current base: no plane enters it on either side. The LOMP10 rows therefore say nothing
   about the loop.
4. **Held.** JM05 3000 and LOMP10 3000 both returned 1.
5. **Held.** USN04 returned 1.
6. **Held.** USN01 returned 1 and BSM01 returned 0.
7. **Measured.** On JM05 9000, six airfield planes loop. They end far from their field:

   | plane | `from_abort` | last local (x, z) |
   | --- | --- | --- |
   | F4F Wildcat 01 | 220 | (-2671, 1368) |
   | SB2C Helldiver 02 | 360 | (-3181, 2816) |
   | MainAirfieldEntity 01_sqn01\|.-2 | 672 | (6397, 7473) |
   | MainAirfieldEntity 01_sqn01\|.-3 | 617 | (5940, 6733) |
   | SecondaryAirfieldEntity 01_sqn02\|.-2 | 276 | (-2602, 2608) |
   | SecondaryAirfieldEntity 01_sqn02\|.-3 | 751 | (-7432, 8310) |

   The other airfield planes hold in state 5 at their hangar points. Their only interactions
   outside the land task are small moves in counters:
   - `Gunnery::score_candidate_00863990` 277683 -> 281923;
   - ship AI autotarget `mean_candidates` 6.18 -> 6.20;
   - recon publication `excluded` (plane classes met in the recon triples) 14360 -> 14325;
   - recon level table `writes` 1367 -> 1370.

   Shots, hit records, damage, deaths and plane water contacts are identical. These moves come
   from hidden planes (the looping ones, and the stowed carrier planes) staying targetable and
   visible in this host. The image detaches them from the hit index and hides them (5aq). That
   is the existing labelled substitution, not a park mechanism.

**No mechanism failure.**
- On a carrier, park ends on the lift (5ao.1).
- On an airfield it loops invisibly as the image's listing reads (5z, 5aa, 5aq).
- The loop touches no outcome on these rows.

**Verdict: `kLandParkStateBound = true`, labelled "image loop accepted, SQUADRON_LAND_TASK
5aq".** This also puts the elevator (5ao) into the reference rows.

**For reference U.** JM05 9000 moves (exit 3, the positions and the land-task, park and elevator
counters); JM05 3000 moves at exit 1 (native table and summary counters only). The other rows are
unchanged by this switch.

**Open (not park).** The host keeps hidden planes (`+C00h`, and state 2 below deck)
targetable and visible. The image does not: the `00710B80` detach removes them from the shell,
blast, line-of-fire and aim queries, and the scene node hides them. A binding that drops `+C00h`
planes from the gunnery candidates and the recon triples belongs to the gunnery and recon lanes.

## 5as. The hide's detach, bound: a hidden plane leaves the hit index (packet `cc9_hit_index_detach`, cc9-lua24, 2026-09-30)

5aq read what the hide does: `00951F40(0)` -> `00710B80` -> `0098A500` removes the collision part
`[unit+360h]` from the spatial index, and `00B6DA70` stops drawing the plane. With park ON
(5ar), the host now reaches the hide on JM05 9000 (six looping airfield planes, eight stowed
carrier planes). This packet binds the index half.

### What is bound, behind `kHitIndexDetachBound` (`src/game_hosts_gunnery.cpp`, committed OFF)

- **The flag.** `GameUnitsHost::unit_hit_node_detached`, slot field `plane_hit_node_detached`:
  - set where the host carries `00951F40(0)`: the park hangar hide (`009B28B8` -> `007B96C0`) and
    the elevator's bottom release `006FC250` (`006FC279`-`006FC28F`);
  - cleared by the carry `006FC0D0`, which is `00951F40(1)` -> `00710AD0`. The spawn-state helper
    `007BC550` and `luaMW_SetVisibility` (`008A13D0`) are the other `00951F40` callers and are not
    carried.
- **The queries that skip a detached unit** (the index's consumers, 5aq):
  - `SegmentBinding::shape_count`, the host's `0098ADD0` segment sweep: the shell sweep of
    `0084BF00`, the player seat's aim rays and the picks;
  - the blast gather of `apply_impact_blast` (`00904470` -> `0098C630`);
  - the line of fire's unit half (`0072CDD0` with `0098B130`).
- **Not bound, and why.**
  - **The AA and gun candidates.** They come from the side's recon list `[recon+DE8h]`
    (docs/AA_TARGETING.md 2), not from the index. The only index query on the AA path is
    `00864680`'s visibility segment, and that is kind `44h`, the landscape. So the image's guns may
    still pick a hidden plane; they just cannot hit it.
  - **The recon triples.** The sensor pass `008073C0` walks class buckets of present units
    (docs/RECON_SENSOR_PASS.md), not the index. Nothing read shows the hide removing a unit from
    those buckets.
  - **The drawing** (`00B6DA70`, scene node `+ACh`). It is presentation only.
- **Counter.** `summary mission gunnery hit index detach bound=%d offers=%llu` counts the offers
  of a detached unit on both sides. They are skipped only when the switch is ON.

### Predictions, written before any ON run

1. **JM05 9000** (park ON on both sides): `pair_diff` returns 0 or 1.
   - On the park-ON run of 5ar.1, the hits, shots and damage equal the park-OFF run's. So no shell
     or blast reached a hidden plane, and skipping them changes nothing.
   - `offers` is non-zero: the sweeps meet detached planes' boxes.
   - This differs from the lead's expectation. The candidate and recon counters of 5ar.1 stay
     where they are, because neither path runs through the index (above).
2. **JM05 3000.** 1: the hangar entries exist (5ar.1: seven airfield planes in park), so
   `offers` may be non-zero, but no hit moves.
3. **USN04 4500, LOMP10 9000 and the controls USN01 and BSM01.** 0 or 1, with `offers` = 0 (no
   plane reaches the hide).
4. **A mechanism failure would be** a detached plane taking a hit or a blast record ON, or any
   death row changing without a hidden plane in it.

### 5as.1 Measured (pairs on `ad0380150`), and the verdict: ON

- **OFF** is `local\l24_hoff` and **ON** is `--flip kHitIndexDetachBound=true`, `local\l24_hon`.
  Both have park ON (5ar).
- The logs are `local\l24_h{off,on}_<row>.log` and the diffs `local\l24_hdiff_<row>.txt` (cc9-lua24
  tree).
- The 300-frame smoke of the OFF build exited 0.

| row | `pair_diff` | death rows | `offers` | moved |
| --- | --- | --- | --- | --- |
| JM05 9200/9000 | 1 | identical (18) | 1258 | `line_of_fire_tests` 27484 -> 27398 |
| JM05 3200/3000 | 1 | identical (12) | 18 | `line_of_fire_tests` 24825 -> 24823 |
| USN04 4700/4500 | 1 | identical (48) | 0 | the bound line only |
| LOMP10 9200/9000 | 1 | identical (9) | 0 | the bound line only |
| USN01 3200/3000 (control) | 1 | identical (17) | 0 | the bound line only |
| BSM01 3200/3000 (control) | 1 | identical (0) | 0 | the bound line only |

The ship AI `free` refill counter also moved on some rows; it is the known same-binary noise.

**The predictions:**
1. **Held.** JM05 9000 returned 1, and `offers` = 1258. The line of fire's box tests fall by 86,
   which is the detached planes no longer being tested. Hits, shots, damage and the per-entity
   death table are identical. As predicted, the candidate and recon counters of 5ar.1 do not move.
2. **Held.** JM05 3000 returned 1, with `offers` = 18.
3. **Held.** The other rows show `offers` = 0.
4. **No mechanism failure.** No detached plane took a hit or a blast record on either side.

**Verdict: `kHitIndexDetachBound = true`.** The flip itself is the lead's to apply, because
`src/game_hosts_gunnery.cpp` is leased to cc9-gunnery18. It is U material, after T's base.

## 5at. The relaunch feed, read: it is the squadron launch itself (packet `cc9_relaunch_feed_read`, cc9-lua24, 2026-09-30)

5ap item 2 asked what fills the elevator's ready plane `site+18h`. The answer is not the stowed
planes. Every carrier and airfield launch goes through it, so the "relaunch" is the launch path.
Ghidra and the disk bytes were read only; no code changed.

### The chain in the image

1. **The launch task** (vtable `00D08AE4`: `007F1EE0` dtor, `0071C470`, `0071C480`, `007F1F00`
   tick, `0071C4A0`).
   - Its constructor is `007F1DE0` (`__thiscall(task, element, squadron)`). It chains the base
     task constructor, registers through `00876020`, clears `+1Ch`, and puts the squadron under
     the observer pair at `task+20h` (`007F0F80`), so `task+34h` is the squadron.
   - Its one caller is `007F53FF`, in the squadron's pass C `007F4BA0`
     (`BSP_PlaneSquadron_SEntityInitSlotA4`), at `007F5390`-`007F5404`. It runs when the home
     base `squadron+404h` is set, the holder kind `[+C0h]+4h` is not 2, and the base answers
     `IsKindOf(9)` (a mother ship) or `IsKindOf(45h)` (an airfield). It allocates 38h bytes and
     passes `base+310h` and the squadron.
2. **The tick `007F1F00`** (`007F1F00`-`007F1FDB`, `__thiscall(task, float dt)`, `RET 4`):
   - `block` = `[task+4]+28h` + `1188h` for kind 9, or + `72Ch` for kind 45h (`007F1F14`-
     `007F1F58`).
   - With a squadron (`+34h`), a block and the block's owner `+7Ch` alive:
     - `+1Ch -= dt`, and it waits while that is above 0;
     - then it asks the site `[block+3Ch]->vtable[18h]()` whether it is ready;
     - when ready, `+1Ch` = `tuning+190h` (`AirField/PlaneSendInterval`, 2.0 in this
       installation) and it calls `007EF010(squadron)`.
   - When `007EF010` answers 1 (no member left to send), it unregisters the squadron and the
     task ends through its own `vtable[10h]`.
3. **`007EF010`** (`007EF010`-`007EF090`, `__fastcall(squadron)`, `RET`):
   - it takes the first member `[+3D0h + i*4]` (i < `+3CCh`) with `+5Ch` clear and `+900h` == 1;
   - it calls that member's `vtable[10h]`;
   - it takes the block of `squadron+404h` (`006BCD20`). If the block's owner `+7Ch` is null or
     dead (`+5Dh`), it kills the member (`00926D90(1)`). Otherwise it calls
     `006CF190(block+3Ch site, member)`, which stores the member in the observed slot
     `site+18h` (`006CF190`-`006CF1C0`, `RET 4`).
   - It returns 0 when a member was sent, and 1 when none was left.
4. **The site's readiness**, `vtable[18h]`:
   - airfield `00CF89F8`: `006CDF60` (`006CDF60`-`006CDF69`), `site+18h == 0`;
   - mother ship `00CF8A58`: `006CFF40` (`006CFF40`-`006CFF61`), true when all three hold:
     - `+94h` (the platform mode `P+50h`) is 0;
     - `0.0 > +5Ch` (`P+18h`, the lift at the top);
     - `+18h` is 0.
5. **The deck.**
   - **Airfield.** The site tick `006CF980` routes `+18h` (message through `0077C2A0` with 5)
     to `006CF9F0`: `007C5F60` places and locks the plane (state 2) on the launch spot, then
     `007C3C90(plane, 1)` requests state 5 and `+18h` is cleared (AIRFIELD_TAXI 7).
   - **Carrier**, `006D0600` re-read (`006D0600`-`006D07A4`):
     - At the top with the platform empty and no landed candidate, a ready plane
       (`+18h`, with `006D02F0` false) sends the **empty** platform down: `006CFFF0(0, 0)`, then
       `006FC640` (`006D0707`-`006D071D`, `EBP` = 0).
     - At the bottom, after `tuning+510h`, `006CFFF0(1, +18h)` places the ready plane on the
       platform (`BSP_Plane_PlaceOnLaunchSpotLocked`) and brings it up (`006FC810`).
     - At the top again (`006D065E`-`006D0684`):
       - `006FC250` releases the platform plane;
       - `+18h` is unregistered and cleared;
       - `007C3C90(plane, 0)` requests state 4.
6. **After that**, the plane in state 4 or 5 on the deck or strip needs the bot's taxi and
   takeoff and the lift-off `007C7110 BSP_Plane_BeginFlying` (state 7). None of these is bound in
   this host (CONTROLLED_UNIT, the takeoff items).

`007C3C90` is `BSP_Plane_RequestGroundState(flag)`: state `4 + (flag != 0)`, through `007C1570`.

### What the host does today

`GameScriptOrdersHost::create_air_ops_squadron_006c5050` (`src/game_hosts_script_orders.cpp`)
creates the whole squadron **airborne**, 150 m (`kAirOpsSquadronLaunchAltitude`) over the home
base's origin, all members at once, in state 7 with `+908h` = 3600. Its own comment labels this
as a contract: "the taxi and catapult paths; none of that is reconstructed". So no host plane is
ever in state 1, no launch task exists, and `site+18h` has no writer. Stowed carrier planes are
never relaunched, and that is not a relaunch gap. In the image a stowed plane (state 2, below
deck) is not state 1, so `007EF010` would not pick it either. Nothing read so far brings a stowed
plane back.

### Why this is not bound in this packet

Routing carrier launches through the lift in isolation would leave every launched plane locked on
the deck in state 4. The chain needs these pieces, in this order:
1. **The launched squadron's members in state 1, in the hangar.** `006C5050` -> `007F4580`: the
   state each member starts in is not read. The launch task only sends `+900h` == 1 members with
   `+5Ch` clear.
2. **The launch task** (`007F1DE0`, `007F1F00`, `007EF010`, `006CF190`, the two readiness slots):
   one member per `PlaneSendInterval`, gated by the site.
3. **The deck arms.**
   - Airfield: `006CF980` -> `006CF9F0` -> `007C5F60` (the pose already exists in
     `src/airfield_taxi.cpp`), then state 5.
   - Carrier: the lift cycle above, then state 4.
4. **The takeoff.** The state 4/5 bot taxi to the runway, the takeoff roll, and `007C7110`.
   Without it, pieces 1-3 strand every launch.

Each piece can be bound OFF and paired on its own only from piece 4 backwards: 4 is reachable
today only by a plane already on the ground. A switch for 1-3 alone would take launched
squadrons out of the air on JM05, USN04 and IJN01. That would be a mechanism failure by
construction, so no OFF commit was made for it here.

### For the lead

- Ghidra names, with bounds verified by `disasm-raw` (`RET`, then `INT3`):
  - `007F1DE0` `BSP_SquadronLaunchTask_Construct` (already a Ghidra function; its bounds were not re-checked here);
  - `007F1F00`-`007F1FDB` `BSP_SquadronLaunchTask_Tick`;
  - `007EF010`-`007EF090` `BSP_PlaneSquadron_SendNextHangarMember`;
  - `006CFF40`-`006CFF61` `BSP_AirOpsElevatorSite_IsReadyForPlane`;
  - `006CDF60`-`006CDF69` `BSP_AirOpsSite_IsReadyForPlane`.

  `006CFF40` and `006CDF60` have no Ghidra function.
- **Proposed packets**, in order:
  1. `cc9_plane_takeoff_read`: the state 4/5 taxi-out, the takeoff roll and `007C7110`, read
     from an authored ground start if any reference row has one.
  2. `cc9_launch_member_state`: `007F4580`'s member state for a launched squadron.
  3. `cc9_launch_task` (the task and the site readiness).
  4. `cc9_launch_deck_arms` (the airfield placement, then the lift cycle), paired only once
     1-3 hold.
- This changes launch timing on every row with an air-ops launch (USN04 launches from six decks, docs/USN04_STRIKE_CLASS.md; which other rows launch was not censused). Members
  would leave one per 2.0 s after the site is ready, plus the lift's `tuning+510h` wait and its
  travel of `depth / ElevatorSpeed` each way on a carrier.

## 5au. The takeoff task mapped, and which rows launch from a base (packet `cc9_plane_takeoff_read`, part 1, cc9-lua24, 2026-09-30)

This is the first half of the takeoff read that 5at asked for. It maps the task that takes a
plane from the ground into the air, and it censuses the rows that would reach it. Ghidra and the
disk bytes were read only; no code changed. The large state step `009CE2C0` is left for part 2.

### Which reference rows launch squadrons from a base

Source: reference T's logs in the cc9-gunnery18 tree (`local\g18_t_pool_<row>.log`), the lines
`summary air ops tick ... squadrons_created=` and `summary mission airops gates`, plus the
`<base>_sqnNN` names each created squadron carries.

| row | squadrons created | bases |
| --- | --- | --- |
| USN04 4700/4500 | 4 | Lexington-class01, Yorktown-class01 (carriers) |
| USN04 9200/9000 (E2) | 4 | the same |
| JM05 3200/3000 | 10 | MainAirfieldEntity 01, SecondaryAirfieldEntity 01 (airfields); USS Lexington, USS Yorktown (carriers) |
| JM05 9200/9000 | 13 | the same |
| USN13 3200/3000 | 9 | Enterprise, Essex, Intrepid, Cabot, Cowpens, Monterey, Yorktown; also Hill and Wood (`LaunchSquadron` 9 calls, `IsReadyToSendPlanes` 0) |
| every other row | 0 | |

The other rows are IJN01, JM06, JM08, JM08 long, LOMP06, LOMP10, LOMP10 long, USN01, USN02, USN12,
USNOS and USNOS long.

`Hill` and `Wood` are not carriers by name. Whether their squadrons go through a base site (an
airfield or mother-ship block), or through something else such as `MCatapult`
(AIR_OPERATIONS 3b), is not read. The reach of a base-launch binding is therefore USN04, E2,
JM05, JM05 long and USN13.

### The takeoff task (kind `0Dh`, `takeoff` at `00D21290`)

- **Install.** The pilot bot's tick installs it at `0099B0BE`-`0099B118` when all of these hold:
  - the plane is not in free flight: `(plane+72Ch)->vtable[38h]()` is false;
  - the current task answers `vtable[38h]` true;
  - the plane is on the ground or water: `0042A7E0` (`+900h` is 4 or 5) or `+900h` == 6;
  - the current task answers `vtable[30h]` true.

  It then calls `009CFF40(bot, 0)` (`009CFF40`-`009CFFA6`, `operator new(4D4h)`, then `009CF8E0`)
  and pushes the result with `00999F50`.
  - For the land task, `vtable[30h]` is `009B3730`, false in park and final (5aa). So a landed
    plane never gets a takeoff task from these states.
- **Constructor `009CF8E0`** (`__thiscall(task, bot, char flag)`): the base task with kind
  `0Dh`, then the four states (`009CF710`), then vtables `00D21228`, `00D2121C` (the state machine
  at `+3F8h`) and `00D21218` (`+434h`). The first state `+310h` is:
  - `+4B8h` **parking**, when the plane is landed (`+904h`) and on the path (`+900h` == 5);
  - otherwise `+448h` **prepare**, when `flag` is set;
  - otherwise `+470h` **Takeoff**, when `plane+AA0h <= 0.0` (`00D7A218`);
  - otherwise `+490h` **SlowTakeoff**.
- **The states** (`009CF710` registers them under their names in the bot state registry):

  | state | offset | vtable | enter | exit | step |
  | --- | --- | --- | --- | --- | --- |
  | `takeoff/prepare` | `+448h` | `00D2116C` | `009CDD50` | `009CDD00` | `009CDE50` |
  | `takeoff/Takeoff` | `+470h` | `00D211A4` | `009CE270` | `009CE290` (`RET`) | `009CE2C0` |
  | `takeoff/SlowTakeoff` | `+490h` | `00D21188` | `009CE1D0` | `009CE150` | `009CE160` |
  | `takeoff/parking` | `+4B8h` | `00D21150` | `009CD520` | `009CD530` | `009CD540` (the taxi step of AIRFIELD_TAXI 6) |

- **The tick `009CFD70`** (`BSP_TakeoffTask_Tick`, vtable `+64h`): `+430h` = `FFh`, then
  `009CFA80` (the altitude floor), then the rule `009CFC70`, then the current state's step, then
  `+2E4h` = `+430h`.
- **The rule `009CFC70`** (`__fastcall(task)`):
  - **Done** (`0099B690`) when there is no plane, or when all of these hold:
    - the plane is in free flight;
    - `+908h` > 5.0 (`00CE3850`);
    - and either the height `+100h` is above `task+424h`, or the speed `vtable[38h]` is above
      `BSP_PlaneClass_MinControlSpeed`.
  - `prepare` -> `SlowTakeoff` once the prepare state's byte `+19h` is set.
  - `SlowTakeoff` -> `Takeoff` once `009CFB60` answers true. That happens at once when
    `plane+AA0h <= 0.0`; otherwise when the plane has moved more than 60 m from where the state
    began (squared distance against `00CE3D70` = 3600.0).
- **SlowTakeoff.**
  - The enter `009CE1D0` stores the start position and draws a heading offset through
    `00BD2F10` between -0.3 and 0.3 (`00D06888`, `00CE69C8`).
  - The step `009CE160`-`009CE1CB` (`RET 4`) holds neutral controls, steers to that offset, and
    requests speed 20.0 (`00CE3930`).
- **Takeoff.**
  - The enter `009CE270`-`009CE28F` sets `+18h` = `[00D7A260]` and `+1Ch` = 0, then tail-jumps
    to `007C17D0`, which tests `+904h` and the flight state against the session role; it was not
    read further.
  - The step `009CE2C0`-`009CF6F8` (exclusive, final `RET 4` at `009CF6F5`) is about 5 KB of
    x87-heavy code: 93 calls, 13 of them `00419010` ramps and 7 of them tuning reads. It
    includes:
    - `007C1680` (5 -> 4) at `009CF35F`;
    - `007B9000` at `009CEAC4`;
    - `006BEFF0` at `009CEBC1` and `006BC890` at `009CED37` (the site and runway);
    - `007C4810`/`007C4830`, the takeoff lengths of AIRFIELD_TAXI and `0x8720` of the host.

    This is part 2.
- **The lift-off** stays as PLANE_GROUND_OPS 5 has it:
  - the ground-roll arm `007CBFA0` sends `C6h` when the height over the contact passes 0.1 with
    `+ACCh` > 0.1, or when contact is lost over a class-9 deck;
  - `007CCFA0` sub-kind 7 calls `007C7110 BSP_Plane_BeginFlying` (state 7, and the site's
    `vtable[28h](plane)`).

### The host today

- No takeoff task exists.
- The ground-roll arm and its law run (5q-5u).
- The `C6h` request is counted, not sent: the ground-roll summary's `liftoff_req`, and the deck's
  `edge_takeoff_requests`.
- `begin_flying_007c7110` exists as a pure reconstruction in `src/plane_ground_ops.cpp` and is
  not bound.
- Launched squadrons spawn airborne (5at).

### The plan: one switch group, flipped only end to end

The group is `kBaseLaunchChainBound`, one switch for all five pieces, committed OFF, each piece
paired as a record while it is built:
1. `007F4580`: launched members start in state 1 in the hangar (next part).
2. The launch task `007F1DE0`/`007F1F00`/`007EF010`/`006CF190` and the two readiness slots (5at).
3. The deck arms: the airfield placement `006CF980`/`006CF9F0`/`007C5F60` and the carrier lift
   cycle (5at).
4. The takeoff task: install gate, rule, states, and the `Takeoff` step (part 2).
5. The lift-off: send `C6h` and bind `007C7110`.

**Flip criterion.** On JM05 3000 (airfields and carriers) every launched member reaches state 7
and flies its squadron's task. The per-entity death table is diffed, and the launch timing is
recorded for reference U. USN04, E2 and USN13 follow. Until then the switch stays OFF and
launched squadrons keep spawning airborne.

## 5av. The Takeoff step read, and how launched members start Inside (packet `cc9_plane_takeoff_read`, part 2, cc9-lua24, 2026-09-30)

This part reads the `takeoff/Takeoff` step and the squadron's side of the launch. Ghidra and the
disk bytes were read only; no code changed.

**Sources.**
- The Takeoff step: a scripted listing (`disasm-raw 009CE2C0`, 1333 lines, kept as
  `local\l24_ce2c0_full.asm`) against the Ghidra pseudocode (`local\l24_ce2c0.c`). The listing
  decides wherever the pseudocode's flag bytes are mangled (`CONCAT13`).
- Constants: read from this installation's executable on disk (`local\l24_consts.py`).

### `009CE2C0 BSP_TakeoffStateTakeoff_Step` (`__thiscall(state, float dt)`, `RET 4`)

`EBP` is the state, `ESI` the plane `[[state+4]+4]`, and the control block `[[state+4]+18h]`
(PILOT_CONTROLS: slot 0 throttle `+278h`, slot 1 `+284h`, slot 4 `+2A8h`, the pitch target
`+2BCh`, the speed request `+2B4h`). There are no SEH state stores; ESP is tracked through the
`SUB ESP,14h` blocks that precede each five-float `00419010` call. `00419010` is
`BSP_Math_InterpolateClamped(x0, y0, x1, y1, x)` and cleans its own 14h.

**A. Setup** (`009CE2D7`-`009CE3E5`).
- A landed flag `+904h` is cleared.
- `H` = the contact holder `+BF4h`, or 0 in free flight (`(+72Ch)->vtable[38h]` true). `v` is
  `vtable[38h]` (speed).
- `carrier` = `(H+4)+7Ch` answers `IsKindOf(9)`.
- With `H`: `m = H+B4h x 0.5 - |plane.xz - owner.xz|` (`00D7A280` = 0.5, `0042B2F0`); otherwise
  `m = 150.0` (`00CE3808`).

**B. The obstacle factor `f`** (`009CE3E9`-`009CE56E`).
- It walks the list at `[[00E188A8]+19CCh]+64h` (node `+4` next, `+8` unit), skipping the
  holder's owner.
- For each unit: `00816410(unit, out, plane+FCh)`, then the result is taken into the plane's
  frame (`+110h`, rebuilt through `00B63D50` when `+10Ch` is clear).
- When local `z` lies in (5.0, 100.0) and `|x|` < 50.0: `f = max(f, interp(100, 0, 40, 1, z))`.

**C. The pitch target `P`** (`009CE574`-`009CE81D`).
- `P0` = 0.05 (`00CE7638`), or 0.1 (`00D7A2F0`) when `classDesc+198h MinWaterSpd` == 0.
- `T` = `classDesc+1ECh` (the takeoff pitch, as in 5v).
- `s` = `v / 007C4830(classDesc)`, where `007C4830` is `BSP_PlaneClass_StallRangeSpeed`.

| case | `P` |
| --- | --- |
| free flight | `interp(2.0, P0, 6.0, T, f x 5.0 + plane+908h)` |
| ground, class `10h` or `16h` | `max(interp(1.0, P0, 1.5, T, s), interp(150, P0, 50, T, f x 150 + m), interp(0, -2T, 7.0, T, state+1Ch))` |
| ground, other classes | `max(interp(1.4, P0, 1.8, T, s), interp(80, P0, 30, T, m))` |

**D. The water arm** (`009CE823`-`009CE930`), only with `H` and `MinWaterSpd` != 0.
- With the owner dead: at plane height `+100h` < `classDesc+A4h x |sin(+C68h)| x 0.5 + 2.5`, it
  calls `007B9010` and returns.
- Then, with `+BF8h` clear or `+BF4h` null, `(H+4)+3Ch->vtable[1Ch]()` true gives
  `BSP_PilotPlan_SetDirectThrottle(-1.0)` and returns.
- In free flight, `P` is floored at `+C64h - 0.0523599` (3 degrees).

**E. The low land plane** (`009CE968`-`009CEA3E`, checked on the listing).
- It applies when `MinWaterSpd` == 0 and the plane's world `y` (`+100h`) is below 5.0.
- It sets `state+4 -> +38h` = 3 and writes:
  - throttle 1.0, direct (`+278h`, `+27Ch` = 1, `+2D8h` = 0);
  - slot 4 = 0;
  - yaw slot `+284h` = 0 (active);
  - `+2C4h` = 0 (mode 1);
  - pitch target `+2BCh` = `P` (mode `+2D0h` = 1);

  and returns. So a land plane below 5 m takes off straight ahead, at full throttle, pitching
  up by the ramps in C.
- Whether the host's airfield runways sit below 5 m is not checked here. The carrier decks do
  not (the lift points of 5ao.1 are at 15-17 m).
- `state+38h` is next set to `FFh` when all three hold, else 0 (`009CEA41`-`009CEA96`):
  - the plane is in free flight;
  - `v` > `007C47F0(classDesc)`;
  - `+908h` > 3.0.

**F. No holder** (`009CF66B` on): throttle 1.0, pitch `P`, yaw 0, return.

**G. With a holder: the deck or elevated runway** (`009CEA99`-`009CF6F5`).
- **The site tests.**
  - `007B9000` runs when the owner is dead and `(+BF8h clear or +BF4h null)`.
  - The pitch target `P` is written.
  - The plane is taken into the owner's frame (`lx`, `ly`, `lz`).
  - The site's lane test `vtable[34h](plane, !carrier && no contact, lx, lz)` (`006CF5B0` on an
    airfield) sets a refused flag `[esp+13h]`.
- **The runway direction.** `006BEFF0(H, out, plane+FCh)` gives the direction.
  - On a carrier with `dx` > 0 it takes `dx' = max(dx - 3.0, 0)`.
  - The runway heading is `pi/2 - atan2(dz, dx')` and the plane's `pi/2 - atan2(+9Ch, +94h)`;
    `err` = `00438B10` (wrapped difference), `|err|` at `[esp+18h]`.
- **The lateral tolerance `tol`.**
  - `tol` = `max(v / 007C4810(classDesc) x 8.0, 2.5)`.
  - Past the runway end (`006BC890`'s `z + 40.0`, else 40.0), it is capped by
    `(lz - end) x 0.25`.
  - It is also capped by `H+B0h x 0.5 - 1.0`.
  - With `|err|` < 0.8 it is further capped by `(H+B0h - (classDesc+A4h - 2.0)) x 0.5` for a
    class 10h/16h plane, and by `(H+B0h - (classDesc+A4h + 1.0)) x 0.5` otherwise
    (`009CEDB9`-`009CEE45`).
- **What follows** (`009CEE4B`-`009CF6F5`) is not reduced to closed form here:
  - the lateral excess;
  - a bound of 3.0 on a carrier or 15.0 otherwise (`00CE3854`, `00CE5380`), and the random
    factor `state+18h` (`00BD2F10(0.5, 1.0)` when negative);
  - the yaw command `+284h` = `interp(-k, -1, k, 1, err')` with `k` from `tuning+188h`,
    `+2A8h`, `+2ACh`, `+2B0h` and `classDesc+1B0h YawSpd`;
  - then `007B8D10` / `007B8DC0`;
  - `BSP_Plane_FlightStateFiveToFour` (`009CF35F`) when a path plane is lined up, with
    `state+1Ch += lz`;
  - the throttle `+278h`, from `BSP_Unit_CanDropOrdnance(0)` (`00CE3868` or `00CE81A0` x
    `(1.4 - +B18h)`), the `state+1Ch` ramp, and the floor test against `00CE65D0`;
  - or, when the lane is refused, throttle -1.0 with slot 4 = 1.0 (the brake);
  - or the speed request `+2B4h` from `tuning+2A8h`/`+184h` ramps against `|err|`.

  This part is the deck taxi and roll. It is the one the carrier launch rows need, and it goes
  to the bind packet, which transcribes it from the listing (lines 490-1332 of
  `l24_ce2c0_full.asm`).

### How a launched member starts Inside

`006C5050`'s bag carries `State` 1 on the normal path (AIROPS_LAUNCH_START 3).

The squadron's pass C, `007F4BA0`, for the kind-1 holder (`007F4C0B`-`007F4DB5`):
1. It reads `State` (`00CF8818`, default 7), and `flag = State < 2`.
2. It reads `HomeBase` (`00CF8820`) and `SpawnPoint` (`00CE56B8`). A `SpawnPoint` whose entity
   has an air-ops block also sets `flag` and becomes the base.
3. It calls `007F1C00 BSP_PlaneSquadron_SetHomeAirBase(base, flag)`.
   - In a campaign session (`[00E188A8]+1FE4h` == 0) that is the alternate push (`006CC7B0`,
     the slot queue).
   - With `flag` it then does `+408h` = 1 and calls `007ED6E0`. In a non-campaign session it always
     takes the ready-plane push and `007ED6E0`.
4. `007ED6E0` (`__fastcall(squadron)`) sets `+408h` = 1 and calls `007C2130` on every member.
   `007C2130` (`007C2130`-`007C21C3`, `RET`) requests flight state **1** through a `C3h` message
   routed with 7, the state-1 twin of `007C2090`. It skips a member already in 1 and a client.
   - The handler `007CC820 BSP_Plane_EnterFlightStateOne` sets `+900h` = 1, stamps `+C04h`,
     calls `007C11E0(0)`, and re-parents the plane to the base (`vtable[ACh](squadron+404h)`).
5. `007F2920`, the in-air or water placement through `007C6340`, is **skipped** when `+408h` is
   set (`007F4DA9`).
6. The launch task is built at the end of pass C (5at).

So the image's launched squadron sits Inside at its base, and the launch task sends its members
out one at a time. The host instead places the squadron in the air at 150 m (`create_air_ops_squadron_006c5050`).

### What the bind needs (for the group `kBaseLaunchChainBound`)

1. The bag's `State` 1 honoured:
   - `SetHomeAirBase` with the flag;
   - `007ED6E0` -> `007C2130` -> `007CC820` (state 1, re-parent);
   - no airborne placement.
2. The launch task and readiness (5at). Member `vtable[10h]` (the activation before
   `006CF190`) is still unread.
3. The deck arms (5at).
4. The takeoff task:
   - the install gate (5au);
   - the rule and the four states (5au);
   - the Takeoff step: A-F above as written, G transcribed from the listing.
5. The lift-off `C6h` and `007C7110`.

**Unread:**
- `007B9000`, `007B9010`, `007B8D10`, `007B8DC0`, `006BEFF0`, `006BC890`, `00816410`;
- the list at `game+19CCh+64h`;
- the prepare state (`009CDD50`/`009CDE50`);
- `007C17D0`.

## 5aw. Handoff (cc9-lua24, 2026-09-30, stamped 10:50 UTC)

**What cc9-lua24 landed** (all merged into main by the lead):

| packet | switch | state | section |
| --- | --- | --- | --- |
| `cc9_park_hide_detach` | none | the hide's spatial detach moves nothing; the lead is refuted | 5aq |
| `cc9_park_verdict` | `kLandParkStateBound` | **ON**, image loop accepted (death tables identical on seven rows) | 5ar |
| `cc9_hit_index_detach` | `kHitIndexDetachBound` (gunnery) | verdict ON; cc9-gunnery18 applies the one-line flip | 5as |
| `cc9_relaunch_feed_read` | none | `site+18h` is the squadron launch itself, not a relaunch | 5at |
| `cc9_plane_takeoff_read` | none | the takeoff task mapped (part 1) and the Takeoff step plus the Inside start read (part 2) | 5au, 5av |

**The queue:**

1. **The base launch chain, one switch group `kBaseLaunchChainBound`** (committed OFF). Build it
   in this order, each piece OFF and paired as a record while it grows:
   1. **Launched members start Inside** (5av):
      - the bag's `State` 1 -> `007F1C00 SetHomeAirBase(base, flag)` -> `+408h` = 1;
      - `007ED6E0` -> `007C2130` -> `007CC820`: state 1, re-parented to the base;
      - no airborne placement (`007F2920` skipped).

      Today `create_air_ops_squadron_006c5050` (`src/game_hosts_script_orders.cpp`) places the
      squadron airborne at 150 m.
   2. **The launch task and site readiness** (5at): `007F1DE0`/`007F1F00` (`PlaneSendInterval`
      2.0 s), `007EF010`, `006CF190`, and the readiness slots `006CDF60` (airfield) and `006CFF40`
      (mother ship).
   3. **The deck arms** (5at):
      - the airfield placement `006CF980` -> `006CF9F0` -> `007C5F60` (the pose already exists in
        `src/airfield_taxi.cpp`), then state 5;
      - the carrier lift cycle in `006D0600`: the empty platform goes down, waits `tuning+510h`,
        and `006CFFF0(1)` brings the plane up; then release, clear `+18h`, and `007C3C90(0)` gives
        state 4. `kCarrierElevatorBound` already carries the lift itself.
   4. **The takeoff task**:
      - the install gate `0099B0BE`-`0099B118` (5au);
      - `009CFF40`/`009CF8E0`, the four states, the tick `009CFD70` and the rule `009CFC70` (5au);
      - the SlowTakeoff state (5au);
      - the Takeoff step `009CE2C0` (5av: A-F as written, G transcribed from the listing).
   5. **The lift-off**: send `C6h` (today counted as `liftoff_req` and `edge_takeoff_requests`)
      and bind `007C7110 BSP_Plane_BeginFlying` (`begin_flying_007c7110` in
      `src/plane_ground_ops.cpp` is a pure reconstruction).

   - **Flip criterion.** On JM05 3000 (two airfields and two carriers), every launched member
     reaches state 7 and flies its squadron's task. Then diff the per-entity death table and record
     the launch timing for the next reference. USN04, E2, JM05 long and USN13 follow; they are the
     only rows that launch from a base (5au census).
   - **Timing to expect:**
     - one member per 2.0 s once the site is ready;
     - on a carrier, add the lift's `tuning+510h` wait and its travel (`depth / ElevatorSpeed`)
       each way.
2. **Open reads the chain still needs:**
   - member `vtable[10h]`, the activation `007EF010` calls before `006CF190` (it presumably sets
     `+5Ch`);
   - the G tail of `009CE2C0` (the deck taxi and roll, listing lines 490-1332 of
     `local\l24_ce2c0_full.asm` in the cc9-lua24 tree): the lateral excess, the yaw law, 5 -> 4 at
     `009CF35F`, the throttle and brake, and the speed request;
   - whether the host's airfield runways sit below 5 m world `y`. If they do, the Takeoff step's
     arm E applies there (straight ahead, full throttle);
   - also unread: `007B9000`, `007B9010`, `007B8D10`, `007B8DC0`, `006BEFF0`, `006BC890`,
     `00816410`, the unit list at `game+19CCh+64h`, the prepare state `009CDD50`/`009CDE50`, and
     `007C17D0`.
3. **`planeDesc+158h`** (5ap item 3): the model box, `007D473A`. Not urgent; every JM05 take was
   within 3 m.
4. **#15** stays parked until a row destroys a hangar.
5. **Low priority** (5ap):
   - SetParty with the event-6 group removal;
   - the `unit_lacks_follow_target` -> `unit_is_flight_leader` rename;
   - the one-think-late turbo clear `009BDE40`;
   - the moveto leader's climb speed loss.

**Also open for other lanes:** the host keeps a hidden or stowed plane in the gunnery candidates
and the recon triples. The image does too (5as), so this is not a divergence. It is recorded only
because 5ar.1's counters moved.

**Tools in the cc9-lua24 tree** (`local\`):
- `l24_runs.ps1`: the reference rows, as in cc9-lua23's;
- `l24_consts.py`: floats and doubles from the executable on disk;
- `l24_grep.py`: a line filter for spilled outputs;
- `l24_ce2c0_full.asm` and `l24_ce2c0.c`: the Takeoff step listing and pseudocode.

## 5ax. Base launch chain, piece 1: the Inside start (packet `cc9_base_launch_inside_start`, cc9-lua25, 2026-09-30)

The first piece of the switch group `kBaseLaunchChainBound` (`include/bsp/game_hosts_units.hpp`),
committed **OFF**. With the switch on, an air-ops launch no longer makes its squadron airborne; its
members sit Inside their base in state 1. Nothing sends them out yet (piece 2), so with this piece
alone every base launch strands. That is expected, and the switch stays off until piece 5.

### Read for this piece (Ghidra and the disk listing, read only)

- **`006C5050` (only caller `006C7490`)**: `State` = `flag ? 7 : 1` and `VelocitySI` =
  `flag ? classDesc+18Ch : 0`. The bag's matrix is the owner's `+CCh` (16 dwords). Only with the flag
  is the random height `BSP_Random_UniformFloatRange(00CE3D08, 00CE3AE8) + tuning+210h` added to its
  y. `flag` is `006C7490`'s second argument:
  - `006CC733` (the host's one launch path, `006CC690`) pushes 0;
  - `006CD7CA` (`006CD6C0`) pushes 0;
  - `006CA8F7` (`006CA8E0`) pushes 1 under a byte argument. This airborne launch path is not
    reconstructed here.
- **`007F1C00`** (listing `007F1C00`-`007F1C93`): with a base, `006BCD20` gives its block.
  - In this campaign session (`[00E188A8]+1FE4h` == 0) it calls `006CC7B0` (`007F1C69`), then with
    the flag `+408h = flag` (`007F1C78`) and `007ED6E0` (`007F1C7E`).
  - Otherwise it calls `006CC760` and `007ED6E0` unconditionally.
- **`007ED6E0`** (`007ED6E0`-`007ED718`): `+408h = 1`, then `007C2130` on each `+3D0h[i]`,
  `i < +3CCh`.
- **`007C2130`** (`007C2130`-`007C21C3`, `RET`, INT3 after): it skips `+900h == 1` and a client
  (`+1FE4h == 2`). Otherwise it sends `C3h` (current `+900h`, requested 1, vtable `00D03504`) through
  `0077C2A0(msg, 7, 0)`.
- **`007CC820`** (`RET`):
  - `007C78A0` only on a client;
  - when `+900h != 1`: `+900h = 1`, `+C04h = [00D7A260]` (-1.0), `007C11E0(0)`;
  - then `vtable[ACh]` with `(+9D4h)->+404h` (the squadron's home base) or, without a squadron,
    `(+BF4h)->+4h->+7Ch`.

**Correction to `include/bsp/air_operations.hpp` (`launch_in_progress`) and
`src/air_operations.cpp` (`006CC690`'s comment).** Both say `006C7490` and `006C5050` never write
`block+38h`. They do: `006C532C`-`006C5348` store the new squadron at `block+38h` (the observed slot
of the pair at `block+24h`, `[EDI+14h]` with `EDI = block+24h`) whenever the flag is 0. So every
flag-0 launch sets the readiness brake itself:
- `006BF620` (IsReadyToSendPlanes) refuses;
- `006CC690` queues;
- `006C5050` refuses the next launch at `006C5078`.

This lasts until the pair's slot is cleared. What clears it (the squadron's death through the
observer, or the launch task's end) is **unread**. It belongs to piece 2 and changes the launch
cadence of every row that launches from a base. No code changed for it here.

### The host with the switch on

- `create_air_ops_squadron_006c5050` makes the squadron at the base's own origin, without the 150 m.
  SUBSTITUTION, labelled: the axes stay the identity. It records the bag's `HomeBase` and `State` 1
  on the squadron record.
- The squadron's pass C (`on_squadron_pass_c_initial_command`), before the command test, takes
  `007F1C00`'s flag arm:
  - it sets `home_launch_408`;
  - for every member, `plane_enter_state_one_007cc820`: state 1, `+C04h` = -1.0, the base as the
    deck parent with the local pose captured;
  - it zeroes `+908h` and the velocity that the host's creation seed wrote for `007C6340`, which
    `007F4DA9` skips here.
- A plane in state 1 selects no arm (`007CEC30`). The host carries its pose with the base each
  step. LABELLED: this stands in for the scene hierarchy's derived pose.
- The squadron's initial command (`moveto` home base) is still issued as before. A state-1 member
  does not think (`0099ACD0`'s gate 8 admits 4..7).

### Predictions (with the switch on; recorded before the smoke)

- JM05, USN04, E2, JM05 long, USN13: every launched squadron logs `base launch inside start`. Its
  members stay in state 1 at the base for the whole run, and no launched plane reaches state 7.
  Mechanism failure by construction until piece 2.
- Rows without a base launch: no `base launch inside start` line, and gameplay is identical.
- Off (committed): identical to the parent commit (0 or 1 from `pair_diff`).

### Record (commit `41ef06574`, 2026-09-30)

- **Smoke, off (the committed build), USN01 300 frames:** 299 frames were presented, the run exited
  0, and it made no launch.
- **JM05 3000, switch on** (`pair_export --flip kBaseLaunchChainBound=true`, `local\l25_p1on`,
  `bsp_game.exe` SHA-256 `C3193E925355`):
  - 10 `base launch inside start` lines:
    - `MainAirfieldEntity 01` at (2801.7, 13.3, 6342.2);
    - `SecondaryAirfieldEntity 01` at (-1469.9, 3.2, -845.4);
    - four each from USS Lexington and USS Yorktown, at y -0.3 and -0.4.
  - All 30 members stayed Inside to the end: `summary base launch inside start: squadrons=10
    entries=30 no_base=0 carry_steps=86010 inside_now=30`.
  - The same ten launches (classes 135, 135, 101, 101, 112 x4, 108 x2) as the off reference
    `l24_poff_jm05`. The host still has no `block+38h` brake, so the cadence did not change.
  - As predicted: the mechanism holds, and every launch strands until piece 2.
  - Cosmetic: `007CC820` logs through `record` (UNIMPLEMENTED in the host table) because the
    `007C11E0(0)` step is not carried. The next edit of the file will change it to `done`.

### Open reads this piece settled or added

- **Settled: member `vtable[10h]`**, the call `007EF010` makes before `006CF190`, is `0042E950
  BSP_UnitInstance_NamePointerOrFallback` in all nine plane entity vtables (`00D05F20`, `00D06638`,
  `00D1A000`, `00D19D28`, `00D06920`, `00D00070`, `00D0BA80`, `00D00308`, `00D1A2D8`, slot `+10h`,
  read from the disk image). It is a pure getter, so it activates nothing. It does not set `+5Ch`.
- **Added: `+5Ch` of an Inside member.**
  - `007EF010` sends only a member with `+5Ch` **clear** and `+900h` == 1 (`007EF028`-`007EF035`).
  - `+5Ch` is the scene-node enable byte: `00922F30 BSP_SceneNode_Enable` sets it and recurses
    through the children at `+48h`/`+44h`. The world walk `00904C00` and the liveness test
    `006D1EF0` gate on it.
  - So a member Inside the base is a disabled entity: not updated and not live. The host keeps it
    active (`state->active` = 1). SUBSTITUTION, labelled.
  - What leaves `+5Ch` clear on a flag-0 member, and what enables it when the member is sent, is
    unread:
    - `006C7528` enables the squadron itself (`00922F30(squadron, 0)`), before its members exist
      (they are made in the next InitAll's pass A);
    - `00924F90`, the re-parent `vtable[ACh]`, does not touch `+5Ch`.

    This is piece 2's first read, together with the `block+38h` clear.
- **Added: the launch brake.** Once `block+38h` is set by `006C5050`, the deck refuses the next
  launch. The host never sets `block+38h`, so the carriers launch as fast as the mission script's
  gates admit: four each on JM05 in 3000 frames.
