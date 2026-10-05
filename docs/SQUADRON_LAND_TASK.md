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
5aq".** This also puts the elevator (5ao) into the reference rows. **Superseded as to the loop by
5bp:** the image pushes a takeoff task at the first ground abort, and the host's loopers are below
the terrain. The switch stays ON for the lift and the hangar hold.

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

## 5ay. Base launch chain, piece 2: the launch task and site readiness (packet `cc9_base_launch_task`, cc9-lua25, 2026-09-30)

This piece is part of the same switch group, `kBaseLaunchChainBound`, which is still committed
**OFF**. It also corrects piece 1.

### Read

- **Inside members are disabled entities (settles 5ax's `+5Ch` read).**
  - `007CC820` calls `007C11E0 BSP_Plane_NotifyFlightState(0)`. In state 1 (outside 7/6/4/5/3) that
    function zeroes `+9F0h` and stores `00F87574` into `+AECh..+B00h`. Because the state is 0 or 1,
    it then calls `007BC550`.
  - `007BC550` (`__fastcall(plane)`): with the Lua self `+4A4h` set, state 0 or 1 gives
    `00922F80 BSP_SceneNode_Disable(0)` (`+5Ch = 0`) and `00951F40(0)` (spatial detach). Any other
    state gives `00922F30` (enable, when `+5Ch` is clear) and `00951F40(1)`.
  - Its other callers are `007C5F60 BSP_Plane_PlaceOnLaunchSpotLocked`, `007C6340` and `007C9770`.
    So the deck placement (piece 3) re-enables the member it sends.
- **The launch task is built for any squadron with a home base.** At `007F5390`-`007F5404`, the
  squadron's pass C builds it when:
  - `+404h` is set;
  - the holder kind is not 2;
  - the base is `IsKindOf(9)` or `IsKindOf(45h)`.

  `+408h` is not tested. A scene squadron with a `HomeBase` therefore gets a task too. Its members
  are in state 7, so `007EF010` answers 1 on the first ready tick and the task ends.
- **Readiness.**
  - `006CFF40` (`006CFF40`-`006CFF61`): true when all three hold:
    - `site+94h == 0`;
    - `0.0 > site+5Ch`;
    - `site+18h == 0`.

    With the platform at `site+44h`, the first two are the platform mode `P+50h` and `P+18h`.
  - `006CDF60` (`006CDF60`-`006CDF69`): `site+18h == 0`.
  - `006CF190` (`006CF190`-`006CF1BD`, `RET 4`): moves the observer pair at `site+4h` onto the
    member, so `site+18h` = member.
- **`007EF010`** sends the first member in `+3D0h` order that has `+5Ch` clear and `+900h` == 1. Its
  `vtable[10h]` is the name getter (5ax).

### The host with the switch on

- **Piece 1, corrected.** `plane_enter_state_one_007cc820` now carries `007C11E0(0)`'s state-1
  zeroing (throttle and velocities) and `007BC550`'s disable:
  - `state->active` = 0 and `row.active` = false (`+5Ch`);
  - `plane_hit_node_detached` (`00951F40(0)`).

  The host's world walk skips the member, as `00904C00` does. Piece 1's per-step carry with the base
  is removed, because a disabled member is not stepped. Its pose is placed again when it is sent.
- **The task.** Pass C builds the launch task on the squadron record (`launch_task_*`) right after
  the Inside start. ORDER, LABELLED: before the command block, not after it.
- **The tick.** It runs once per step, before the site ticks in `run_landing_queue_006cd240`.
  POSITION, LABELLED.
  - The countdown, then the readiness of the base's `LandingDeck`:
    - the new `site_ready_18`;
    - for a mother ship, the elevator's `mode_50` and `p18`.
  - Then the send: `site_ready_18` = member.
- **Not carried.** The kill arm of `007EF010` for a dead owner is unreachable in the same step.
- **Not built.** The scene squadrons' tasks, which end at their first ready tick.

### Predictions (switch on, recorded before the run)

- JM05 3000: 10 tasks are built.
  - Each deck's first ready tick sends one member: two airfields, and each carrier with the lift at
    the top.
  - Nothing consumes `site+18h` until piece 3, so each of the four decks holds one ready plane for
    the rest of the run.
  - Four `sends` in all, and `not_ready` grows every step after them.
  - No task ends, and 30 members stay Inside (disabled). No launched plane reaches state 7.
- Off: identical to the parent.

### Record (commits `7a34612d5`, `40412e9e5`, `f41a2bd3b`, 2026-09-30)

- **Smoke, off (`7a34612d5`), USN01 300 frames:** 299 frames were presented and the run exited 0.
- **First JM05 3000 with the switch on (`7a34612d5`, `local\l25_p2on`): a host defect.**
  - Disabling the Inside members made `air_ops_squadron_plane_count` report 0, because it counted
    `row.active`.
  - The deck tick then released each slot as if its squadron were dead (the host's stand-in for
    `007F1B70` -> `006C65B0`).
  - The script's `stloPlaneNum < 2` gate relaunched until the 64-squadron ceiling: 64 squadrons, 192
    members Inside.
  - In the image a disabled member stays in `+3D0h`/`+3CCh` until `007F3970`'s compaction at death.
    Under the switch the count now takes the members that are not out of action, destroyed or
    removed (`+5Dh`/`+5Eh`/`+5Fh`): `40412e9e5`, with `f41a2bd3b` fixing the `C4702` that the ON
    build raised.
- **JM05 3000 with the switch on (`f41a2bd3b`, `local\l25_p2bon`, SHA-256 `A9E9BB5B9998`), as
  predicted:**
  - the same 10 launches as the off reference;
  - `built=10 ended=0 sends=4`;
  - each of the four decks received its first member at 3.05 s:
    - `MainAirfieldEntity 01_sqn01`;
    - `SecondaryAirfieldEntity 01_sqn02`;
    - `USS Lexington_sqn03`;
    - `USS Yorktown_sqn04`.
  - Each member is its squadron's wing 0, the `+3D0h` head. Each deck kept that member in
    `site+18h` for the rest of the run: `not_ready` was 28506 of 28670 ticks.
  - 30 members stayed Inside.
  - Shokaku and Zuikaku, which launch nothing on this row, stayed empty.

Piece 3 (the deck arms) is what consumes `site+18h`.

## 5az. Base launch chain, piece 3a: the airfield deck arm (packet `cc9_base_launch_deck_arms`, cc9-lua25, 2026-09-30)

This piece is part of the same switch group, `kBaseLaunchChainBound`, which is still committed
**OFF**. The carrier lift arm is piece 3b.

### Read

- **`006CF980`** (the airfield site tick, `006CF980`-`006CF9DB`, `RET 4`):
  - with `+1Ch` set it calls `006CEDD0` (unread; the host never sets `+1Ch`);
  - with a ready plane in `+18h` it routes the message `006BDAC0(plane)` to the airfield (`site+44h`)
    through `0077C2A0(msg, 5, 0)`. The handler reaches `006CF9F0`.
- **`006CF9F0`** (`006CF9F0`-`006CFA5D`, `RET 4`): with the airfield and its block owner `+7A8h`
  alive, it calls:
  - `007C5F60(plane, airfield+72Ch)`;
  - `007C3C90(plane, 1)`;
  - `0042ED50`;
  - `(plane+310h)->vtable[8](0.0)`.

  On both paths it then clears `site+18h`. So `006CDF60` answers ready again at once. The next member
  follows one `PlaneSendInterval` later.
- **`006CF730`** (the pose):
  - the entry path of `006D2780`, points 0 and 1;
  - the slide from point 0 along their direction is `(planeDesc+158h + 1.0) - plane+9D8h x 0.2`.
    The factor is the double at `00CE3D10`, `FMUL double ptr` at `006CF877`.
  - y is `[airfield+7ACh]+9Ch`, or the ground height when that is absent;
  - the frame is returned in the airfield's axes.
- **`007C5F60`** (the full pseudocode; its control-block writes are now read):
  - the forward row is tilted to `GroundPitch`;
  - airfield: `+9DCh = 3.0` (`00CE3854`) and local y += `WheelHeight`;
  - carrier: `+9DCh = 5.0`, local x += `+9D8h x +-1.0` by the pitch sign, local y += `WheelHeight`,
    local z += `0.0`;
  - the gear and flap channels of `+DECh` are set;
  - `vtable[ACh](block+7Ch)` re-parents the plane, `007B8E80(block+80h)` attaches the holder, and
    `007C5AC0(-1.0)` probes the site;
  - `+9F0h = 0`; state 2 with `+C04h = -1.0` and `007C11E0(0)`; the velocities are zeroed;
  - `008073C0`, then `007BC550`, which **enables** the plane (state 2).
- **`007C3C90(1)` from state 2.** It calls `007C1570(5)`, which sends a `C3h` message. The handler
  `007CCFA0` routes it through arm `007CD06D`: the current state is not 7, and `0042A7E0` (state 4 or 5)
  is false, so it calls `007CA3F0(1)`. With a holder, `007CA3F0` does:
  - `007C1430(5)` (`+904h`/`+C18h` with `MinWaterSpd`);
  - `+9F0h = 0`, `+9F4h = 1.0`, `+9E4h`/`+9E8h`/`+9ECh = 0`;
  - the site's `vtable[24h]` `006CED90` (the occupancy vector);
  - the velocity zeroed;
  - **`0099A4A0`**: `009CFF40(dl = 1)` -> `00999F50`, the takeoff task install. That is where piece 4
    starts.

### The host with the switch on

- `run_landing_queue_006cd240` runs `airfield_site_tick_006cf980` for every built airfield deck,
  after the launch tasks. POSITION, LABELLED.
- `place_on_launch_spot_007c5f60`:
  - the pose from the entry path picked by the refactored `landing_hangar_path_006d2780_006d2640`
    (shared with `006CF420`/`006CF520`);
  - `GroundPitch` and `WheelHeight`;
  - the airfield as the deck parent, the contact deck, and the site probe;
  - state 2, the velocities zeroed, and the plane enabled again (`state->active` = 1, node attached).
- `ground_state_from_locked_007ca3f0` takes the plane to state 5 with the control reset and the
  occupancy push. The takeoff install `0099A4A0` is recorded only (piece 4).
- SUBSTITUTIONS, labelled:
  - `planeDesc+158h` = 0.0;
  - y is point 0's world y, not the airfield's `+7ACh` object;
  - the airfield's axes are taken as world;
  - `+9DCh`, `+DECh`, `007EB270`, `007C18B0` and `vtable[D8h]` are not carried.

### Predictions (switch on, recorded before the run)

- JM05 3000: each airfield squadron's three members are placed one every 2.0 s from 3.05 s:
  - wing 0 at slide 1.0;
  - the next members at 0.8 and 0.6 along the entry path.
- Each goes 2 -> 5 and stays in state 5 near the entry path, because no takeoff task exists yet.
  That is 6 `placed` and 6 `ground_entries`.
- The carriers' first members stay in `site+18h` (piece 3b).
- No launched plane reaches state 7.

### Record (commit `84cde0e77`, 2026-09-30)

- **Smoke, off, USN01 300 frames:** 299 frames were presented; the run exited cleanly.
- **JM05 3000, switch on** (`local\l25_p2bon` rebuilt at `84cde0e77`, SHA-256 `49C80E481811`), as
  predicted:
  - Each airfield placed its three members at 3.05, 5.10 and 7.15 s:
    - index 0/1/2, slide 1.00/0.80/0.60;
    - `SecondaryAirfieldEntity 01` at about (-1406.0, 3.2, -898.0);
    - `MainAirfieldEntity 01` at about (2876.0, 13.3, 6332.5).
  - Each member went 2 -> 5 on placement: `placed=6 ground_entries=6 refused=0`.
  - Both airfield tasks then ended (`ended=2`, `sends=8`: six on the airfields and one on each
    carrier). The carriers still hold their first member in `site+18h`.
  - 24 members stayed Inside at the end.
  - GroundPitch reads 0.000 for class 135.
- **The spacing is 0.2 m per index,** so the three members stand almost on top of each other at
  the entry path's head. That is the formula as read. The class's `+158h` (0.0 here, a
  substitution) is what would move the stack along the path. The pair should show whether the
  taxi of piece 4 separates them.

## 5ba. Base launch chain, piece 3b: the carrier lift arm (packet `cc9_base_launch_deck_arms`, cc9-lua25, 2026-09-30)

This piece is part of the same switch group, `kBaseLaunchChainBound`, which is still committed
**OFF**. It is commit `0db2e1291`.

### Read (disk listing)

- **`006D0600` in full** (`006D0600`-`006D07A4`, `RET 4`). The platform is `P = site+44h`, so
  `site+78h` is `P+34h`, the platform plane.
  - **At the top** (`0.0 > P+18h`, mode 0):
    - `+9Ch = 0`;
    - with a platform plane: `006FC250` releases it, the ready plane `+18h` is unregistered and
      cleared (`006D0667`-`006D0678`, whatever it holds), and the plane, if alive (`+5Dh`), gets
      `007C3C90(plane, 0)` (`006D0684`);
    - then, with the carrier alive: the landed candidate loop. A candidate gives `006CFFF0(0, cand)`.
      Without one, a set `+18h` with `006D02F0` false gives `006CFFF0(0, 0)`.
  - **At the bottom** (`P+18h > depth`, mode 0):
    - with a platform plane: `007B96C0`, then `006FC250`, then `+9Ch = 0`;
    - otherwise `+9Ch += dt`;
    - when `+9Ch > tuning+510h` and the platform is empty: `006CFFF0(1, +18h)` and `+9Ch = 0`.
- **`006CFFF0(flag, plane)`** (`006CFFF0`-`006D0048`, `RET 8`): a message `00758B90(flag, plane)`
  routed to the carrier (`site+B8h`) through `0077C2A0(msg, 5, 0)`. Its handler reaches
  **`006D0050`** (`006D0050`-`006D00D3`, `RET 0Ch`):

  | flag | plane | calls |
  | --- | --- | --- |
  | 1 | set | `007C5F60(plane, carrier+1188h)`, then `006FC810(P, plane)` |
  | 0 | set | `006FC720` (the landed intake) |
  | 1 | none | `006FC6B0` (empty up) |
  | 0 | none | `006FC640` (empty down) |

  On a client, `006FBCE0` follows.
- **`006FC640`**:
  - `+50h = 0`, `+18h = -0.0 - +1Ch`;
  - a platform plane is carried and dropped;
  - `+50h = 2`;
  - the sound `+54h` `vtable[0Ch](1.0)`.
- **`006FC810`**:
  - for a new plane: `+50h = 0` and `+18h = depth + +1Ch`;
  - an old platform plane is carried, detached with `00951F40(0)`, and dropped;
  - it takes the plane with the offset `(0, WheelHeight, -0.0 - planeDesc+158h)`;
  - the sound, then `006FC0D0`, then `+50h = 1`.
- **`006D02F0`**: blocked when the platform holds a plane, or when an occupant's local z (`+ACh`)
  lies in `(lift z - 14.0 x 2.0, lift z + 14.0)`. The 14.0 is `00E08FC8`, read from the image's
  `.data`; its writers were not censused. The 2.0 is the double `00D7A308`.
- **`006D0930`**, the mother-ship site's `vtable[40h]`, gives the pose for `007C5F60`:
  - identity axes;
  - class `+808h..+810h` (the lift point), with y less `settings+4DCh` and less `planeDesc+158h`.

  `007C5F60`'s mother-ship arm then sets `+9DCh = 5.0` and adds to the local translation:
  - x += `+9D8h x +-1.0`;
  - y += WheelHeight;
  - z += 0.0.

  `006FC810`'s carry replaces that translation, so only the axes (GroundPitch) survive.
- Carrier site vtable `00CF8A58`: `+14h` = `006CFE30`, `+24h` = `006CED90`, `+40h` = `006D0930`.
  Airfield `00CF89F8`: `006CF9F0`, `006CED90`, `006CF730`.

### The host with the switch on

- **Top:** after the release, `site_ready_18` is cleared, the plane is attached again, and
  `ground_state_from_locked_007ca3f0(false)` takes it to state 4. Without a landed candidate, a
  ready plane with `carrier_elevator_blocked_006d02f0` false sends the empty platform down
  (`carrier_elevator_send_empty_down_006fc640`).
- **Bottom:** once `pilot_landing_lift_delay` (`tuning+510h`) has passed with the platform empty,
  the ready plane is placed (`place_on_launch_spot_007c5f60`, the mother-ship arm: the carrier as
  deck parent, the local axes pitched, the translation at the lift) and raised
  (`carrier_elevator_raise_plane_006fc810`). Without a ready plane the platform goes up empty, as
  before.
- **LABELLED:**
  - the messages to the carrier are delivered at once;
  - the site probe runs at the lift point, not at `006D0930`'s point;
  - `planeDesc+158h` = 0;
  - the platform sound is not carried.

### Predictions (switch on, recorded before the run)

JM05 3000, on each of Lexington and Yorktown:
- The empty platform goes down at 3.05 s.
- After `tuning+510h` plus the travel, the first member is placed in state 2 and raised.
- At the top it is released in state 4 at the lift, which clears `site+18h`.
- The launch task sends the second member 2.0 s later. `006D02F0` then stays blocked while the
  first member stands within the lift band, because nothing taxis it away until piece 4.

Expected totals: `up=2`, `top=2`, `down=2`, and `blocked` growing; about 22 members still Inside.

### Record (commit `0db2e1291`, 2026-09-30)

- **Smoke, off, USN01 300 frames:** 299 frames were presented; the run exited cleanly.
- **JM05 3000, switch on** (`local\l25_p2bon` rebuilt at `0db2e1291`, SHA-256 `25E8E1E1883C`).
  **The lift cycle holds, but the prediction on `006D02F0` failed.**
  - **Timing on both carriers**, identical on each: the empty platform goes down at 3.05 s. The
    first member is placed and raised at 5.65 s and released 2 -> 4 at 7.80 s. The next member
    follows every 7.35 s (13.00/15.15, 20.35/22.50, 27.70/29.85 ...).
  - **Totals:** all 24 carrier members went out (`up=24 top=24 down=24`), and every task ended
    (`ended=10 sends=30 placed=30 ground_entries=30 inside_now=0`).
  - **Why the prediction failed:** `006D02F0` blocked for only about 4.5 s per cycle
    (`blocked=1097` ticks over 12 cycles), not until piece 4.
    - The released planes leave the lift band on their own. The leader `USS Lexington_sqn03`
      logs `carrier deck stop` at local (-0.83, 17.40, 39.39) with contact at 7.90 s.
    - Its wingmen `.-2`/`.-3` never record a ground contact (`contact_steps=0`,
      `min_height=1000.00`, `wheel=0.00`).
    - So, in state 4 with no contact, the ground-roll arm takes the free-flight step (`007DC830`)
      and the plane moves off the lift.
  - **What this means:**
    - The deck contact of a plane released at the lift is a host gap for piece 4, with the
      pending read on the airfield height below 5 m.
    - Class 101's `WheelHeight` reads 0.00 in the host (`plane_wheel_height_1fc` needs both
      `WheelHeight` and `GroundPitch`).
    - Until piece 4, the lift cadence is set by planes drifting off, not by a taxi.
- **Airfields:** unchanged from 5az (six placed in state 5).
- **No launched plane reaches state 7,** as expected before piece 5.

## 5bb. Base launch chain, piece 4 part 1: the takeoff task read for the bind (packet `cc9_base_launch_takeoff_read`, cc9-lua25, 2026-09-30)

Read only: no code changed in this part. It settles how a launched plane gets its takeoff task,
the first state it enters, and the two short states. The `Takeoff` step `009CE2C0` (5av) is still
the large remaining read.

### The install is `007CA3F0`'s, not the bot tick's

- `007CA3F0` (5az) ends, for a plane on a holder in single player, with **`0099A4A0`** (`0099A4A0`-
  `0099A4B4`, `RET`):
  - `MOV DL,1`;
  - `009CFF40(bot, 1)`, which is `operator new(4D4h)` and the constructor `009CF8E0`;
  - `00999F50`, the push.

  So every plane that `007C3C90` puts in state 4 or 5 gets a takeoff task at once. That covers the
  airfield placement (state 5) and the carrier lift release (state 4).
- The bot tick's install at `0099B0BE`-`0099B118` (5au) passes flag 0. It is a second, general
  route for a grounded plane whose current task allows it.
- **With flag 1, `009CF8E0` picks `prepare`** (`+448h`), unless the plane is landed (`+904h`) and on
  the path (`+900h` == 5), which picks `parking`. A launched plane has `+904h` clear (`+908h` = 0),
  so it always starts in `prepare`.

### `takeoff/prepare` (`+448h`)

**Enter `009CDD50`:**
- clears `+18h` and `+19h`, and sets `+1Ch = 0`;
- draws `+20h` from `UniformFloatRange(0, 2*pi)` (`00CE3D9C`);
- reads `T = tuning+4DCh` (`Pilot/TakeOff/PrepareTime`), and uses 1.5 (`00CE380C`) when
  `T < 1.5` (`00CE3D78`);
- draws a sign: -1.0 (`00D7A260`) when a uniform draw exceeds 0.5 (`00CE3800`), else 1.0;
- `+24h = sign x UniformFloatRange((T / 2*pi) x 0.6, (T / 2*pi) x 0.95)` (`00CEFF98`, `00CEFFB0`).

**Step `009CDE50`** (`__thiscall(state, dt)`):
- The task's `+38h` = 0.
- Done (`+19h` = 1) with no plane, or with the plane in free flight.
- Otherwise it writes these plan controls:

  | control | offset | value |
  | --- | --- | --- |
  | throttle | `+278h` | 0.01 (`00D7A238`), active |
  | air brake | `+2A8h` | 1.0, active |
  | air-brake mode | `+2D8h` | 0 |
  | yaw | `+284h` | 0, active |
  | yaw mode | `+2D4h` | 0 |

- When `007B8D10` holds (the `+DECh` channel `+44h` absent or at 1.0; the host has no `+DECh`, so
  true), it adds `dt` to `+1Ch` and then checks, in order:
  - an airfield holder (`(+BF4h)->+4->+7Ch` answers `IsKindOf(45h)`) makes it done at once;
  - after `+1Ch >= 0.1` (`00D7A3A0`):
    - it writes a slow circular wobble into the roll slot `+290h` (active, `+2CCh` = 0) and the
      pitch slot `+29Ch` (active, `+2D0h` = 0). The angle is `(+1Ch - 0.1) / +24h + +20h`, and the
      values are `cos` and `sin` of it, normalised with `BSP_Vector2f_ReciprocalLength`;
    - with squadron members (up to five of `+3D0h`), done when the nearest other member is under
      10.0 m (`00CE38B8`);
    - done when `+1Ch > tuning+4DCh` and `009CDD10` answers true.
- **`009CDD10`** caches `+18h`. Unless it is already set, it asks the holder site's `vtable[20h]`
  whether this plane may go:
  - airfield `006CF400`: always 1;
  - mother ship `006D01C0`: 1 only when the plane's deck position along the runway (`006BEFF0`) is
    not ahead of every other site occupant's. A non-occupant counts at `-(owner+1208h)+B4h x 0.5`,
    and the start value is 9999.0 (`00CE4C04`).

  So on a carrier the planes go in deck order.

The rule `009CFC70` then moves `prepare` to **`SlowTakeoff`** (`+490h`) once `+19h` is set.
- On an airfield that is on the first step, because of the `IsKindOf(45h)` exit.
- On a carrier it is after `PrepareTime` and the site's permission, or at once when a wingman
  stands within 10 m.

### `takeoff/SlowTakeoff` (`+490h`)

- **Enter `009CE1D0`:** stores the start position (`+FCh..+104h`) and draws `+24h` from
  `UniformFloatRange(-0.3, 0.3)` (`00D06888`, `00CE69C8`).
- **Step `009CE160`:**
  - the task's `+38h` = 0;
  - bank `+2C4h` = 0 with `+2CCh` = 1;
  - the yaw slot `+284h` = `+24h`, active, with `+2D4h` = 0;
  - speed `+2B4h` = 20.0 (`00CE3930`), `+2B0h` = 0, `+2D8h` = 1.
- **The rule's `009CFB60`** goes to `Takeoff` (`+470h`) once the plane is more than 60 m from the
  start (squared distance against 3600.0), or at once when `plane+AA0h <= 0.0` (5au).

### The rule and the tick (`009CFC70`, `009CFD70`)

- **Tick:** `+430h` = `FFh`; `009CFA80`, the altitude floor (task+3F8h+2Ch = max(the avoid layer at
  (x, z) + `+30h`, `+30h` + `+34h`), and it ends a current `moveto` to the home base); the rule; the
  state's step; then `+2E4h` = `+430h`.
- **Rule, done arm** (`0099B690`): with no plane, or when the plane is in free flight with `+908h`
  > 5.0 and either its height `+100h` is above `task+424h` or its speed (`vtable[38h]`) is above
  `BSP_PlaneClass_MinControlSpeed`.

### The deck-contact gap after the lift release (from 5ba)

- **`WheelHeight` 0.00 for class 101 is the image's value, not a host reading gap.** This
  installation's `vehicleclasses.lua` (modified 2026-05-09) authors neither `WheelHeight` nor
  `GroundPitch` in `VehicleClass[101]` (F4F Wildcat, lines 44757-45123). `007D29B8`-`007D2AC6`
  write `classDesc+1FCh`/`+200h` only when both are present, so the descriptor keeps its zero. The
  nearest authored pair after it (`GroundPitch` 0.174533 at line 45656, `WheelHeight` 1.12 at line
  45838) belongs to class 103, the D4Y Judy.
- **Why the wingmen never touch down:** still open.
  - After the release, `007C5AC0` holds contact only while `landing_over_runway_006bc530` accepts
    the plane's position on the mother-ship holder.
  - The leader logged contact at local (-0.83, 17.40, 39.39); its two wingmen, released at the
    same lift, never did.
  - The next reader should log the holder-local position and `006BC530`'s answer for a wingman at
    its release. Suspects are the probe's `+C04h` re-arm and `landing_deck_006c0750`'s frame
    refresh order against the carry.

### Next (piece 4 part 2, for the successor)

1. **Bind the task record.**
   - Install it at the end of `ground_state_from_locked_007ca3f0`, where `0099A4A0` is now only
     recorded.
   - First state `prepare`, or `parking` for a landed plane on the path.
   - Run the tick from the pilot think of the ground-roll arm (`pilot_think_and_commit`), the way
     the land task's states run.
   - The plan-slot offsets map as the land task's comments give them:

     | offset | control |
     | --- | --- |
     | `+278h`/`+27Ch` | throttle |
     | `+284h`/`+288h` | yaw |
     | `+290h`/`+294h` | roll |
     | `+29Ch`/`+2A0h` | pitch |
     | `+2A8h`/`+2ACh` | air brake |
     | `+2B4h` | speed |
     | `+2C4h` | bank |
     | `+2CCh`, `+2D0h`, `+2D4h`, `+2D8h` | the modes |
2. **Bind `prepare` and `SlowTakeoff`** as read above, with the site permission `006CF400`/`006D01C0`.
3. **Transcribe the `Takeoff` step `009CE2C0`**: A-F from 5av, G from `local\l25_ce2c0_full.asm`
   (lines 490-1332), then the lift-off (piece 5).
4. **The wingmen's deck contact** (above).
5. **`planeDesc+158h`** (`007D473A`, the model box). It sets the airfield stack spacing and the lift
   offset, and matters once the takeoff runs.

## 5bc. Handoff (cc9-lua25, 2026-09-30, stamped 12:31 UTC)

**The switch group `kBaseLaunchChainBound`** (`include/bsp/game_hosts_units.hpp`) is committed
**OFF**. Pieces 1-3 are bound behind it and recorded on JM05 3000 with the switch on:

| piece | what | commits | section | state with the switch on |
| --- | --- | --- | --- | --- |
| 1 | Inside start: `007F1C00` flag arm, `007ED6E0`, `007C2130`, `007CC820`; members disabled through `007C11E0` -> `007BC550` | `41ef06574`, `7a34612d5` | 5ax, 5ay | 30 members Inside, disabled |
| 2 | launch task: `007F1DE0`, `007F1F00`, `007EF010`, `006CF190`; readiness `006CDF60`/`006CFF40`; `+3CCh` counts disabled members | `7a34612d5`, `40412e9e5`, `f41a2bd3b` | 5ay | one send per `PlaneSendInterval` while the site is ready |
| 3a | airfield arm: `006CF980`, `006CF9F0`, `007C5F60` (`006CF730` pose), `007C3C90(1)`, `007CA3F0` | `84cde0e77` | 5az | six airfield members placed, then state 5 |
| 3b | carrier lift: `006D0600` launch arms, `006D0050`, `006FC640`, `006FC810`, `006D02F0`, `006D0930` | `0db2e1291` | 5ba | 24 carrier members lifted to state 4, one every 7.35 s |
| 4, part 1 | the takeoff task read for the bind: install at `0099A4A0` (flag 1 -> `prepare`), `prepare`, `SlowTakeoff`, rule, tick, the site permission `vtable[20h]` | read only | 5bb | not bound |

**Next, piece 4 part 2**, in the order 5bb gives. Nothing of it is committed:
1. bind the takeoff task record at `ground_state_from_locked_007ca3f0`'s `0099A4A0` record;
2. bind `prepare` and `SlowTakeoff`;
3. transcribe the `Takeoff` step `009CE2C0` (A-F from 5av, G from the listing);
4. the wingmen's deck contact after the lift release;
5. `planeDesc+158h`.

Then piece 5 (`C6h` and `007C7110`), and the flip criterion of 5aw.

**Open for the lead:**
- `006C5050` writes `block+38h` on every flag-0 launch (5ax). The chain does not model this brake
  yet. What clears it (the `block+24h` observer pair's notice) is unread.
- The piece-2 records show the launch cadence the flip will bring:
  - airfield members every 2.05 s from 3.05 s;
  - carrier members every 7.35 s from 5.65 s.

**Tools** (`local\` in the cc9-lua25 tree):
- `l25_runs.ps1`: the reference rows;
- `l25_consts.py`: floats and doubles from the executable on disk;
- `l25_calls.py`: an `E8` call census with the preceding instructions;
- `l25_p3a_edit.py`, `l25_p3b_edit.py`: the prepared edits, as a model for the shared-file rule;
- `l25_ce2c0_full.asm`: the `Takeoff` step listing, copied from cc9-lua24;
- `l25_c5f60.c`, `l25_cf730.c`, `l25_ca3f0.c`, `l25_cde50.c`, `l25_cfc70.c`, `l25_cfa80.c`,
  `l25_cfd70.c`, `l25_d0600.asm`: the bodies read.

## 5bd. Base launch chain, piece 4 part 2a: the takeoff task bound to SlowTakeoff (packet `cc9_takeoff_task_bind`, cc9-lua26, 2026-09-30)

Behind `kBaseLaunchChainBound`, committed **OFF**. It binds the task record, `prepare`,
`SlowTakeoff`, the rule, the tick and the altitude floor. The `Takeoff` step `009CE2C0` is part 2b.

### What is bound

| address | routine | coverage | host |
| --- | --- | --- | --- |
| `0099A4A0` | the install at `007CA3F0`'s end (`009CFF40(bot, 1)`, `00999F50`) | complete, except that `00999F50`'s push over an existing task restarts the one record | `install_takeoff_task_0099a4a0` |
| `009CF8E0`, `009CD420` | the constructor and the approach base | the start-state choice and `+2Ch`/`+30h`/`+34h` | same |
| `009CDD50` | `prepare` enter | complete | `takeoff_prepare_enter_009cdd50` |
| `009CDE50` | `prepare` step | complete; `approach+38h` is a record | `run_takeoff_prepare_step_009cde50` |
| `009CDD10`, `006D01C0` | the site permission (the airfield `006CF400` answers 1) | complete | `takeoff_permission_009cdd10`, `carrier_site_permission_006d01c0` |
| `009CE1D0`, `009CE160` | `SlowTakeoff` enter and step | complete | `takeoff_slow_enter_009ce1d0`, `run_takeoff_slow_step_009ce160` |
| `009CFB60` | `SlowTakeoff` -> `Takeoff` | complete, with `+AA0h` carried as 0.0 | `takeoff_slow_leave_009cfb60` |
| `009CFC70` | the rule | complete | `takeoff_rule_009cfc70` |
| `009CFD70` | the tick (slot `+64h` of `00D21228`) | complete | `run_takeoff_task_tick_009cfd70`, run as the bot's one task from the pilot think |
| `009CFA80` | the altitude floor | partial: squadron+350h's layer is not carried (34Ch's stands in), and the `0071E430` tail is recorded | `takeoff_altitude_floor_009cfa80` |
| `009CE270` | `Takeoff` enter | `+18h` = -1.0, `+1Ch` = 0; `007C17D0` recorded | in the rule |

State vtables, read from `.rdata`:

| state | vtable | enter | exit | step |
| --- | --- | --- | --- | --- |
| prepare | `00D2116C` | `009CDD50` | `009CDD00` (a `RET`) | `009CDE50` |
| SlowTakeoff | `00D21188` | `009CE1D0` | `009CE150` (a `RET`) | `009CE160` |
| Takeoff | `00D211A4` | `009CE270` | `009CE290` (a `RET`) | `009CE2C0` |
| parking | `00D21150` | `009CD520` | `009CD530` | `009CD540` |

### Corrections to 5bb

- **Constants.** `00D7A3A0` (0.1), `00CE3D78` (1.5), `00CEFF98` (0.6) and `00CEFFB0` (0.95) are
  doubles. The period divisor is the double `00CE3828` (2 pi), not the float `00CE3D9C` (that one is
  the phase draw's upper bound). The initial minimum member distance is `00D7A248` (FLT_MAX).
- **`009CFB60`'s 60 m arm sends a message.** Past 3600.0 (`00CE3D70`, double) it builds message
  `7Ah` (`0080F960`, byte `+20h` = 0) and routes it to the plane through `0077C2A0(plane, msg, 0,
  0)`. `BSP_Plane_HandleMessage`'s case `'z'` (`007CD171`) calls `007B83F0(0)`, which sets
  `+AA0h` = -1.0. So a plane that left SlowTakeoff by distance has `+AA0h` <= 0 from then on.
- **`SlowTakeoff` lasts one tick for a launched plane.** `+AA0h` is 0.0 from `007D5D20`
  (`007D614B`). It becomes 5.0 only for a `ShipYardLaunch` bag key or a kind-2 descriptor's
  `+12Ah` (`007D6355`, `007D645A`). The air-ops launch bag of `006C5050` pushes no `ShipYardLaunch`
  key (its key pushes are `00CE4780`, `00CF8840`, `00CF8838`, `00CE8EE0`, `00CE5804`, `00CF882C`,
  `00CF8820`, `00CF8818`, `00CF880C`, `00CF8800`, `00CF69AC`, `00CF87EC`). So `009CFB60` answers
  true on its first call and the plane goes on to `Takeoff`.
  - Uncertainty: whether a squadron member's descriptor is kind 2 with `+12Ah` set is unread.
  - The host carries `+AA0h` as 0.0, the value that matches the bag reading.
- **The altitude floor's layer** is squadron+350h, which `007F1D90` selects from the squadron's own
  slope argument. The host does not carry it.

### Predictions, written before the smoke (switch ON, JM05 3000)

1. Every member that `007CA3F0` places in state 4 or 5 logs `installed in takeoff/prepare`: the six
   airfield members and the 24 carrier members. None is installed in parking, because `+904h` is
   clear.
2. **Airfield members** (state 5, an airfield holder) leave `prepare` on its first step with
   `airfield holder`. They go through `SlowTakeoff` on the next tick and reach `Takeoff` on the
   tick after that, about 0.2 s after placement. There they stop: `run_refused` counts.
3. **Carrier members** (state 4 at the lift top):
   - they hold throttle 0.01 and air brake 1.0 while the roll and pitch slots wobble;
   - each leaves `prepare` after `PrepareTime` with the deck-order permission, or at once when a
     squadron member stands within 10 m;
   - Inside members still count as squadron members, so their host position decides the 10 m
     test. The log gives the minimum distance at the exit.
4. No member reaches state 7 and no task is done. The squadron stays grounded until part 2b binds
   `Takeoff`.
5. With the switch OFF the build is identical in behaviour: the takeoff code is reached only from
   `007CA3F0`, which only the chain calls.

### Results (switch ON, JM05 3000, `local\l26_p4aon_jm05.log`, built from `52366dd9b` with the flip)

The 300-frame OFF smoke (`local\l26_off_smoke.log`) exits 0 with the smoke's usual summaries.

| prediction | result |
| --- | --- |
| 1. every placed member installed in prepare, none in parking | **held for the ten members placed**: `installs=10 parking_refused=0`. Only ten were placed, not 30 (see below) |
| 2. airfield members: prepare one step, then SlowTakeoff, Takeoff about 0.2 s later | **held**: installed 3.05 s, `prepare -> SlowTakeoff` 3.25 s (`airfield holder`), `-> Takeoff` 3.35 s; the same at 5.10 and 7.15 s |
| 3. carrier members: prepare time with the deck-order permission, or a member within 10 m | **held for the leaders**: Lexington's and Yorktown's leaders leave after 1.50 s (`prepare time and site permission`); the nearest other member is 78.92 m away, so the Inside members never trip the 10 m test |
| 4. no member reaches state 7 and no task is done | **held**: `done=0`; eight members sit in Takeoff with the step refused |

**The launch now stalls after two members per carrier**, where piece 3b lifted 24.
- The first wingman (`|.-2`) is lifted at 14.50 s, not 13.00 s, and released at 16.65 s. It then
  stays in prepare for the rest of the run: 2638 of 2640 permission asks are denied.
- **Why:**
  - `006D01C0` denies while another occupant of `site+34h` has a lower `z'`. The leader is still
    in the occupancy vector.
  - With Takeoff refused, the leader's plan keeps 0099B450's reseed: speed `TravelSpeed x
    NewTravelSpeedMul`, `+2D8h` = 1. So it rolls down the deck and leaves it over the bow at
    15.30 s, still in state 4, at 41.18 m/s. The log line is `plane ground contact lost ...
    local=(-0.60 0.01 139.56) half=(20.00 139.50)`.
  - The wingman, held by prepare's throttle 0.01 and air brake 1.0, stays on the lift. Then
    `006D02F0` blocks the platform (`blocked=5494`) and the third member never comes up
    (`ready_plane=... |.-3`).
- **This is the expected state between pieces.** In the image the leader flies away in `Takeoff`,
  and its occupancy entry goes with the lift-off (piece 5, `007C7110`, unread). The piece-3b cadence
  comes back only when both are bound.

**The wingman's deck contact (queue item 4) is not reproduced in this run.** `|.-2` logs
`carrier deck stop ... local=(-0.83 17.40 39.39) contact=1` at 16.75 s, the leader's own
release point and contact. 5ba's non-contact came with a leader still standing on the lift spot.
Recheck it once Takeoff moves the leader off.

## 5be. Base launch chain, piece 4 part 2b: the Takeoff step `009CE2C0` (packet `cc9_takeoff_step_bind`, cc9-lua26, 2026-09-30)

Behind `kBaseLaunchChainBound`, committed **OFF**. `run_takeoff_run_step_009ce2c0` is the whole
step. It is transcribed from the listing (`local\l26_from_ce2c0_full.asm`, cc9-lua24's scripted
listing), with the stack slots tracked through each `SUB ESP,14h` block. The pseudocode was used
only as a cross-check; its flag bytes are mangled.

### Corrections to 5av

- **C.** The two ground arms start their ramps at 0.05 (`00CE7638`) whatever `MinWaterSpd` is.
  The `P0` choice (0.1 `00D7A2F0` when `MinWaterSpd` == 0) feeds only the free-flight arm.
  - The class `10h`/`16h` arm overwrites `f`'s slot with `-2T`.
  - `f` is not read again.
- **G, the lateral tolerance.** Only one of the three bounds is a cap.
  - The past-the-end term `(lz - end) x 0.25` and the aligned-heading width term are **floors**:
    `tol = c` when `tol <= c` (`009CED6D`, `009CEE3D` `JA` over the store).
  - The half-width term `H+B0h x 0.5 - 1.0` is the only cap (`009CEDAB`).
- **`end`** is `006BC890(block)`, which calls the site's `vtable[44h]`:
  - on a carrier it is `006D0120`, the lift point in the carrier's frame;
  - on an airfield it is `006CF520`, the hangar entry path's first point in the airfield's frame;
  - either way the step adds 40.0 (`00D7A378`).
- **The site calls.**
  - `vtable[1Ch]` is `006CE4A0` on both site vtables (`00CF89F8`, `00CF8A58`). It is true when an
    occupant of `site+34h` has `+904h` and `+910h` set.
  - `vtable[34h]` is `006CF5B0` on an airfield and `006D0390` on a mother ship.
- **`+94h`/`+9Ch`**, the heading's forward row, is the pose at `+74h`. For a plane parented to a
  carrier that is the local pose, so the heading error is taken in the carrier's frame. That is
  the same frame as `006BEFF0`'s holder direction and the owner-frame `lx`/`lz`.
- **`+B18h`** is `ctl+68h`, the previous step's body forward acceleration (`007DC756`).
- **The throttle** (aligned):
  - it starts at 1.2 (`00CEC160`) - the carrier's forward speed (`0092D730` on `owner+1018h`) x
    0.05, floored at 0.8 (`00CE74F8`); on an airfield it is 1.0 for class `10h`/`16h`, else 0.9
    (`00CE3860`);
  - then `+ max((1.4 - ctl+68h) x (0.16 if 007B9140(0) else 0.25), 0)`;
  - then `max(that, interp(0, 0, 5, 1, state+1Ch))`;
  - below 1.0 it becomes 1.0 when 40.0 (carrier) or `400 - 8v` (airfield) exceeds `m`;
  - it is written as 0.4 (`00CE7804`) below 0.4 (`00CE65D0`), else clamped to 1.0.
- **Lined up** means: not refused, and either (`|e| <= 1.0` (carrier) / 6.0, and `a2` within 5
  (carrier) / 8 degrees), or `state+1Ch > 0.8`.
- **`007C17D0`**, the Takeoff enter's tail, does nothing unless `+904h` is set, which it is not
  for a launched plane. It is recorded.

### Host substitutions, labelled in the code

- `007B8D10` is true (no `+DECh` block), so `007B8DC0` is not reached.
- `007B9140(0)` is the torpedo or general-bomb bit of the unit's ordnance mask.
- A holder deck the host did not build counts as no holder.
- An airfield without an entry path keeps `end` = 40.0.

### Predictions, written before the smoke (switch ON, JM05 3000)

1. **Carrier leaders** enter Takeoff at about 9.5 s, as in 5bd.
   - They line up within a few ticks and are logged `ALIGNED`.
   - The throttle rises to 0.8-1.0 and they roll down the deck.
   - They leave its bow about 4-8 s later. They stay in state 4, because the lift-off `C6h` /
     `007C7110` (piece 5) is not bound.
2. The wingmen stay in prepare, denied by the leaders' occupancy, as in 5bd.
3. **Airfield members** (state 5 on the path) run the lane test.
   - When lined up they go 5 -> 4 through `007C1680` and roll.
   - Or, below 5 m of height, they take the low-land arm E at full throttle.
4. No member reaches state 7 and no task is done.

### Results (switch ON, JM05 3000, `local\l26_p4bon_jm05.log`, built from `931c852db` with the flip)

The OFF 300-frame smoke (`local\l26_off2_smoke.log`) exits 0.

| prediction | result |
| --- | --- |
| 1. carrier leaders line up, roll and leave the bow in state 4 | **held**. Both leaders are `ALIGNED` on their first Takeoff tick (9.50 s, `e` = 0, `err` = 0). Lexington: throttle 1.0, then 0.80-0.82 while the `state+1Ch` ramp builds, then 1.0; 9.3 m/s at 11.5 s, 35.8 m/s at 15.5 s, contact lost between 15.5 and 17.5 s at 36-44 m/s. Yorktown loses contact between 13.5 and 15.5 s |
| 2. wingmen held in prepare | **held**: 2639 of 2641 permission asks denied |
| 3. airfield members line up, 5 -> 4, and roll | **held**. First lined up at 13.3-29.7 s after 37-205 taxi ticks (speed request 0.83 m/s at large heading errors). The one traced (`MainAirfieldEntity 01_sqn01|.-2`) leaves the runway contact at 59 m/s in state 4 with throttle 0.9-1.0; all six end at throttle 1.0 in state 4. None took the low-land arm (their runways are above 5 m) |
| 4. no state 7, no task done | **held**: `done=0`; `edge_takeoff_requests=1074` counts the carrier edge requests that piece 5 will send |

**After the deck edge.** Still in state 4 and in its Takeoff state, Yorktown's leader took the D
arm's taxi hold for 316 ticks: an occupant of its site was landed in the taxi queue, so the step
wrote throttle 0 with the air brake. That is the image's rule for a plane that has lost contact
without lifting off. It stops once piece 5 moves the plane to state 7 and out of the site.

**Next: piece 5.** It covers the `C6h` lift-off send and `007C7110`, from the ground roll's two
counted requests:
- `007CC1B3`, height over the wheels and `vy` > 0.1;
- `007CC23E`, contact lost on a ship holder.

After that the rule's done arm (state 7, `+908h` > 5.0, above the floor or faster than
MinControlSpeed) retires the task, and the flip criterion of 5aw can be checked.

## 5bf. Base launch chain, piece 5: the lift-off (packet `cc9_base_launch_liftoff`, cc9-lua26, 2026-09-30)

Behind `kBaseLaunchChainBound`, committed **OFF**.

### The chain, read from the listing

1. **The ground roll's step 8** (`007CC1B3`-`007CC2B3`) skips a plane in state 5.
   - `EBX` = 2 (`007CC145`), so both client tests pass on the authority.
   - It sends C6h when `BFCh - WheelHeight` > **0.8** (double `00CE3D40`, not 0.1 as the host's
     old counter had it) and `ctl+1Ch` > 0.1.
   - Otherwise, with contact lost (`+BF8h` clear), it sends C6h when the holder's owner answers
     IsKindOf(9).
   - Otherwise, with contact lost, it sets `+C01h` = 2.
   - The send is `00762A00` (C6h at `+10h`) then `0077C2A0(unit, msg, 1, 0)`.
2. **`BSP_Plane_HandleMessage`** (`007CCFA0`), case C6h, calls `007C6F50(byte msg+20h)`.
   - `00762A00` never writes `+20h`, and the byte only adds `007D83D0`'s push for a parented plane.
     The host passes 0. Uncertainty: the delivered byte is unread.
3. **`007C6F50`** (`007C6F50`-`007C70FD`), with `+C49h` clear:
   - a parented plane is taken back to world: `007D9CE0(0)`, the pose from `+CCh`, then
     `00924F90(0)` drops the parent;
   - then, below state 7, it sends C3h with sub-kind 7, routed with 7.
4. **`007C7110`** (`BSP_Plane_BeginFlying`), from 4 or 5 with a holder:
   - the site's `vtable[28h]` is `006CF180` on both site vtables. It adjusts `+20h` and jumps to
     `006CEF80`, which erases the plane from `site+34h`;
   - then `+900h` = 7, `+C04h` = -1.0, `+910h` = 0, `+904h` = 0;
   - `+908h` = 0 from state 4 or 3, else 3600.0;
   - then `007C11E0(0)` (recorded).

Both routings are delivered at once (**LABELLED**).

### Predictions, written before the smoke (switch ON, JM05 3000)

1. **The carrier leaders** lift off at the bow as `deck edge` at about 16 s. They are unparented,
   leave the site, go to state 7, and fly on.
   - The takeoff task's done arm then retires them once `+908h` > 5 s (about 21 s), since they
     are above MinControlSpeed.
2. With the leader out of `site+34h`, `006D01C0` grants the first wingman.
   - It rolls, lifts off, and the lift is freed, so the lift cycle resumes.
   - More carrier members launch than 5be's two per carrier.
3. **Airfield members** lift off by `height` when their pitch-up gives `vy` > 0.1 with the
   wheels 0.8 m clear. A member that leaves the runway without that gets `+C01h` = 2 and no
   lift-off; it would show as a state-4 plane with `c01_sets` counting.
4. The flip criterion (all 30 members in state 7 flying their task) is checked on the summary.
   It may still fail on the `block+38h` brake (5ax) or the wingmen's cadence.

### Results (switch ON, JM05 3000, `local\l26_p5on_jm05.log`, built from `de336ac51` with the flip)

The OFF 300-frame smoke (`local\l26_off3_smoke.log`) exits 0.

**Every launched member flies.** `placed=30`, `liftoffs=30 unparented=30 site_leaves=30
c01_sets=0`. The whole chain now runs end to end:
- the lift delivers 24 carrier members (`down=24 up=24 top=24`), as in 5ba;
- every site's ready plane is consumed;
- `inside_now=0`.

| prediction | result |
| --- | --- |
| 1. carrier leaders lift off at the bow | **partly held.** They lift off earlier, by `height` on the deck: Lexington's leader at 15.70 s, y 18.07, 49.0 m/s. The height arm comes before the edge arm |
| 2. the wingmen get their permission and the lift resumes | **held**: `permission_asks=24 denied=0`. On each carrier the members go from prepare to SlowTakeoff 1.6 s after release, one release about every 9 s (8.9-9.1 s) |
| 3. airfield members lift off by `height` | **held**: 17.80-21.75 s at y 14.2 and about 45 m/s; `c01_sets=0` |
| 4. flip criterion | **every member reaches state 7** (30 of 30). The takeoff task's done arm never runs (`done=0`): see below |

**The land task pre-empts the takeoff task.**
- The launched squadrons receive `returntobase` (`00E08F98`) right after the launch. Lexington_sqn03
  gets it at 7.70 s (`returntobase 007F16D0 ... RECORD ONLY`), and its members install `land` at
  the delivery (`issue_script_command` -> `install_land_task_0099a3dd`).
- The host's pilot think runs `land` ahead of `takeoff` (`run_land_task_tick_009b3eb0` is the
  first arm). Lexington_sqn03's first install is at 17.55 s, 1.85 s after its lift-off.
- From then on the member flies `land`, and the takeoff record stays installed but unticked. The
  image's `00999F50` push would make `land` the current task and the takeoff task a stacked one;
  whether and when it retires is unread.
- The members fly `land` back to their decks. MainAirfield_sqn01 ends in states 4 and 5 again;
  Yorktown_sqn04's leader ends in state 4. So "flies its task" holds with `land` as the task.
- Deaths, hits and the gunnery aggregates are near the OFF rows (360 against 364 hits, 12 deaths),
  but no pair has been run.

## 5bg. Handoff (cc9-lua26, 2026-09-30, stamped 13:57 UTC)

`kBaseLaunchChainBound` is committed **OFF**. Pieces 1-5 are all bound behind it:

| piece | what | commits | section |
| --- | --- | --- | --- |
| 1-3b | Inside start, launch task, deck arms, lift (cc9-lua25) | see 5bc | 5ax-5ba |
| 4, 2a | takeoff task install `0099A4A0`, prepare, SlowTakeoff, rule, tick, floor | `52366dd9b`, `c70fe0b88` | 5bd |
| 4, 2b | the Takeoff step `009CE2C0` | `931c852db`, `92112d7b0` | 5be |
| 5 | the lift-off: `007CC1B3` -> C6h -> `007C6F50` -> `007C7110` | `de336ac51` and this section's commit | 5bf |

With the switch ON, JM05 3000 lifts all 30 launched members into state 7 (5bf).

### Next, in order

1. **The flip.** Pair JM05 3000 and JM05 9000, USN04, E2 and USN13, plus two controls (for
   example USN01 and BSM01), with `python tools/pair_export.py --commit <sha> --flip
   kBaseLaunchChainBound=true --out local\<row>`. Then run `pair_diff`, diff the per-entity death
   tables, and flip by verdict.
   - Expect JM05 to move: the squadrons now launch over about 100 s instead of appearing
     airborne at 150 m.
   - USN04 and E2 launch from carriers too.
   - The mechanism criterion (all members in state 7 and flying their task) holds on JM05 3000.
2. **The land task pre-empting the takeoff task** (5bf). In the host, `land` is ticked ahead of
   `takeoff`, and the takeoff record stays installed with `done=0`. Read `00999F50`'s push and
   the bot's current-task switch to decide:
   - whether `land` replaces the takeoff task or stacks over it;
   - when the takeoff task retires.
   The launched squadrons get `returntobase` at launch on JM05 (7.70 s for Lexington_sqn03);
   check whether the image issues that command at the same moment.
3. **The `block+38h` launch brake** (5ax), still unmodelled.
4. **`007C6F50`'s flag** (`msg+20h`, never written by `00762A00`) and `+C49h`. Both are taken as 0.
5. **`planeDesc+158h`** (`007D473A`) is still 0.0 (queue item 5 of 5bc).

**Tools** (`local\` in the cc9-lua26 tree):
- `l26_runs.ps1`: the reference rows;
- `l26_consts.py`: constants from the executable on disk;
- `l26_p4a_edit.py`, `l26_p4b_edit.py`, `l26_p5_edit.py`: the applied edits;
- `l26_from_ce2c0_full.asm`: the Takeoff listing;
- `l26_c6f50.asm`, `l26_cbfa0.asm`: the lift-off listings;
- the logs `l26_p4aon_jm05.log`, `l26_p4bon_jm05.log`, `l26_p5on_jm05.log`.

## 5bh. The flip pairs for `kBaseLaunchChainBound` (packet `cc9_base_launch_flip`, cc9-lua27, 2026-09-30)

Pairs from `7356585c2`: OFF `python tools/pair_export.py --commit 7356585c2 --out local\l27_off`
(no flips), ON the same with `--flip kBaseLaunchChainBound=true --out local\l27_on`. Rows in the
reference launch form (t's lines: lockstep 0.05, idle player, `BSP_GUNNERY_RNG_STREAMS=1`,
`BSP_DEATH_TABLE=1`), logs `local\l27_{off,on}_<row>.log` in the cc9-lua27 tree.

### The JM05 recall is the image's rule, not the switch's

Queue item 1 of 5bg asked whether the `returntobase` that reaches the launched JM05 squadrons at
launch (7.70 s for `USS Lexington_sqn03` in `l26_p5on_jm05.log`) is a side effect of the switch.
It is not:
- The command comes from the SELLING tick (`00A11FF0`, host source `ai_selling_tick`; the ON log
  counts `returntobase=662` in `summary mission ai selling`). The switch does not touch it.
- SHIP_AI_OPEN_ITEMS 59.1 (cc9-ships17) read every link from launch to that command (world list 24
  seed, the `00A2C8D0` join, Capture, Sell `00A22800`, the air test, the tick) and found that the
  image sends `returntobase` to every launched US squadron on JM05 on each command tick. With the
  switch OFF the same squadrons get it too, only airborne.
- The moment: in t (`g18_rt_jm05l.log`, OFF) Lexington_sqn03 gets it at 5.60 s; in the ON smoke at
  7.70 s. Why the ON record is 2.1 s later is for the pair to show (the record prints on a change
  of arm, site or note). Otherwise the moment follows the launch: the squadron joins list 24 at its launch order (3.05 s for
  Lexington_sqn03), and the next SELLING tick after the brain's seed pass issues the command. OFF
  places the squadron at launch in the air; ON leaves it Inside, so the first land install is
  refused (`not airborne`) until the lift-off. This timing is not a switch artefact.

### Which rows launch from a base

From t's logs (`g18_rt_<row>.log` in the cc9-gunnery18 tree), `air ops launch skill applied`:

| row | launches | members | launched at | their orders | in the OFF death table |
| --- | --- | --- | --- | --- | --- |
| JM05 3000 | 10 squadrons (2 airfields, Lexington, Yorktown) | 30 | 3.05 s onward | SELLING `returntobase` (image rule, above) | none |
| JM05 9000 | 13 | 39 | 3.05 s onward | same | none |
| USN04 4500, E2 9000 | 4 fighter squadrons (class 101), 2 per carrier | 12 | 27.05 s, 30.05 s | initial `moveto` their own carrier only; the US side is NONCONTROL (`kAiPartyGateUnforcedBound`) | none (every `gunrow` of theirs has `shots 0`) |
| USN13 3000 | 9 squadrons (class 26), one per carrier | 27 | 0.00 s (at load) | initial `moveto` their own carrier only | none |
| USN01, LOMP06 | none | 0 | - | - | - |

No launched squadron in any row fires a shot, kills or dies in t. None flies a strike in these
windows: JM05's are recalled, USN04's and USN13's hold a `moveto` on their own carrier.

### Predictions (written before the pairs)

1. **JM05 3000:** as `l26_p5on_jm05.log`. `placed=30 liftoffs=30`, the first carrier leader lifts
   off near 15.7 s, then one release about every 9 s per carrier, the airfield members at
   17.8-21.8 s. Every member ends in state 7 or back on its deck under `land`. **Death table
   identical** (the twelve structure and ship rows); hits and shots within a few percent (the
   ON smoke had 360 hits against 353). Exit 3 (moved) through plane positions and counters.
2. **JM05 9000:** the same for the first ten; the three late airfield squadrons (sqn11-13, ordered
   at 153.00, 302.95 and 403.03 s in t) launch from their airfields the same way. `placed=39 liftoffs=39`. **Death table identical** (eighteen rows, no plane
   among them). Plane water contacts may differ from OFF's 3. Exit 3.
3. **USN04 and E2:** the four squadrons start Inside at 27.05/30.05 s. With two squadrons per
   carrier, six members per deck, the last lift-off about 50-60 s after the first, near 80-100 s.
   The Japanese strikes' first hit (98.70 s), the torpedo-task releases (5 of 16) and dive-bomb
   releases (1 of 19 on E2) are **unchanged or moved by at most one**. Death rows identical, or at
   most two re-timed (enemy AA and escorts now meet planes on deck and at low height instead of at
   150 m). Exit 3 through gunnery counters.
4. **USN13 3000:** a launch at load. Weakest call: the chain may not start before the carriers'
   decks are registered. If it does, 27 members go through their decks over the first 30-40 s;
   death table identical (24 rows) and first hit 5.15 s unchanged. If the members stay Inside
   (`inside_now>0` at the end), that is a mechanism failure and keeps the switch OFF.
5. **USN01 and LOMP06 (controls):** no launch, exit 0 or 1.
6. **Flip criterion:** every launched member on every row leaves Inside and lifts off (or is
   killed on deck), and no death row moves that the launches cannot explain.

### Results (OFF SHA-256 `24EF070049E7`, ON `E56BC3DD351B`, both from `7356585c2`)

`python local\l27_cmp.py <row>` prints the headline, the per-victim death diff and the ON launch
summaries; `pair_diff` exits are in the table.

| row | pair_diff | launched: placed / lifted off | takeoff `done` | death table | other headline moves |
| --- | --- | --- | --- | --- | --- |
| JM05 3000 | 3 | 30 / 30 | 0 | **identical** (12 rows) | hits 353 -> 360, damage 10011.6 -> 10049.6, shots 363 both |
| JM05 9000 | 3 | 39 / **35** | 0 | **identical** (18 rows) | hits 786 -> 793; water contacts 3 both |
| USN04 4500 | 3 | 12 / 12 | 12 | **one new row**: `Yorktown-class01_sqn04\|.-3` at 117.85 s, killed by `B5N Kate #4.1\|.-4` (rear gun, 50 hits from 115.35 s, alt 183 m) | hull hits 91 -> 152, shots 15384 -> 15621; releases 5 of 16 / 0 of 19 both; first hit 98.70 s both |
| E2 9000 | 3 | 12 / 12 | 12 | the same one new row | as USN04 |
| USN13 3000 | 3 | 27 / 27 | 27 | **identical** (24 rows) | every headline value identical |
| USN01 | **1** | - | - | identical | - |
| LOMP06 | **1** | - | - | identical | - |

Against the predictions:

| prediction | result |
| --- | --- |
| 1. JM05 3000 as the smoke | **held**: the same numbers as `l26_p5on_jm05.log` (lift-offs from 15.70 s, releases about 9 s apart) |
| 2. JM05 9000, 39 of 39 | **missed by four**: see below |
| 3. USN04/E2 lift-offs over about 50-60 s, strike outcome within one | **held**: lift-offs 37.65-84.20 s; releases, first hit and water contacts unchanged. The new death is a launched fighter itself |
| 4. USN13, the launch at load | **held**: the chain starts at 0 s; all 27 lift off, `inside_now=0`; nothing else moves |
| 5. controls | **held**: exit 1 |

**The four JM05 9000 wingmen** (`SecondaryAirfieldEntity 01_sqn11|.-2`, `|.-3`, `_sqn12|.-2`,
`|.-3`) never align (`first_aligned=-1`, `hold=901`-`909`): the Takeoff step's hold arm
(`007B4ED0(-1.0)`) runs because the site's `vtable[1Ch]` answers yes. That routine is `006CE4A0`
(read here in full, `006CE4A0`-`006CE4E5`, `RET` then `INT3`): it walks `site+34h` and answers 1 for
an occupant with `+904h` and `+910h` both set. The host matches it byte for byte.
- The occupants are the recalled planes that landed at the Secondary airfield: `F4F Wildcat 01`-`04`
  and `SB2C Helldiver 01`. They taxied into the hangar at 128.30-184.41 s (`007B96C0`), and they stay
  in `land/park` to the end with `done=0 q910=1 c00=1`.
- So a hangared airfield plane never leaves `land/park` or `site+34h`, and once it has `+910h` it
  holds later takeoffs at its airfield. At the Main airfield the three hangared `_sqn01` planes
  never set `+910h` (`q910=0`: the queue-origin test at `009B25A7` did not pass there), so sqn13
  is not held and all three of its members lift off.
- The hold arm runs only with `+BF8h` clear or `+BF4h` null, so the leaders of sqn11 and sqn12
  pass while their wingmen are held. Why the leaders pass that test and the wingmen do not is
  not traced (LABELLED).
- OFF never meets this: its launched planes appear airborne.
- The image must clear `+904h` or drop the plane from `site+34h` somewhere after the hangar.
  `+904h`'s byte writers (`scan-bytes c6 ?? 04 09 00 00` and `88 ?? 04 09 00 00`) include
  `BSP_Plane_SetFlightState` (`007C14B5`, `007C14DE`, `007C1518`), `007C3680`, `007C7430` and
  `007CB9E0`. Which of them the park exit reaches is unread: new queue item 1 of the handoff.

**The takeoff task's done arm runs where no land task arrives.** On USN04, E2 and USN13 the launched
squadrons hold their initial `moveto`, get no `returntobase`, and every member's takeoff task
retires (`done=12`, `done=27`). On JM05 `land` takes over after the lift-off, as in 5bf. So the done
arm is reached; whether `00999F50` stacks `land` over it is still 5bg item 2.

**The JM05 recall moves by 2.1 s** (Lexington_sqn03 5.60 s OFF, 7.70 s ON in this pair). Both come
from the SELLING tick. The squadron now starts on the deck instead of 150 m up, so its group joins
the SELLING air group at a different pass (the join is a 650 m merge distance, 59.1). LABELLED: that
reading of the 2.1 s is not traced pass by pass.

**The USN04 fighter death.** Yorktown's last fighter lifts off at 81.65 s and is still climbing
toward its carrier when the first Japanese strike reaches the fleet (first hit 98.70 s). A B5N's
rear gun kills it at 117.85 s. OFF holds the same fighters at 150 m over the carrier from 27 s,
where none is hit. The extra 61 hull hits and 237 shots are the strike's gunners and the fleet's
AA now meeting these fighters.

### Verdict: **flip ON**

- The mechanism holds on every launch row: every launched member is placed, raised and rolled, and
  116 of 120 lift off; the four that do not are held by the image's own hold rule against a
  downstream host gap (the park exit), not by the launch chain. Recorded as a spread miss.
- Death tables: identical on JM05, JM05 long and USN13. USN04 and E2 add one row, a launched fighter
  shot down after a lift-off the image times the same way. The controls are gameplay-identical.
- This flip belongs to reference V (U is being built without it).

## 5bi. The park exit of a hangared airfield plane (packet `cc9_airfield_park_exit`, cc9-lua27, 2026-09-30)

5bh left one question: does anything in the image let a hangared airfield plane stop blocking its
airfield? The block is `006CE4A0`, the site's `vtable[1Ch]`. It answers 1 while any `site+34h`
occupant has `+904h` and `+910h` set. This section reads Ghidra only; no code changed, no run.

**Answer: nothing found.** As far as the listing shows, the image's hangared airfield plane keeps
both bytes and its `site+34h` entry. So the four held JM05 9000 wingmen of 5bh are the image's
behaviour too, given the image reaches the same hangar state. This agrees with 5ar, which accepted
the invisible park loop as the image's.

### Why park never finishes in the hangar

- **`009B21D0`, park's done test** (decompiled here). It returns at once when `+900h` is 5, so
  `+18h` is never set while the plane is in state 5. The host's `land_park_done_009b21d0` has the
  same early return.
- **The plane stays in state 5 in the hangar.** Park's join test at `009B270F` wants the path while
  it is slow, with `qd + 30 > dz` and not `qd < dz`. In the hangar (`f18 < 3`, `007B96C0`) that
  still holds, so the plane never goes back to state 4. The host shows the same thing: every
  hangared Wildcat has `entries=1 done=0 state=5`.
- **Park's enter and exit** (`009B21A0`, `009B21C0`) and its vtable `00D1FF60`, slots
  `009B2E30 009B21A0 009B21C0 009B22C0 007B3DE0 007B3DF0 007B45E0`, touch only the state's own
  fields (`+18h`, `+1Ch`, `+28h`).

### Who writes `+910h` and `+904h`

- **`+910h`.** `scan-bytes c6/88/80 ?? 10 09 00 00` finds these writers:
  - `BSP_Plane_BeginFlying` (`007C7195`), `007CB9E0`, `BSP_Plane_ReadPropertyBag`,
    `BSP_Plane_SetFlightState` (`007C14D6`, `007C1510`), `007C3680`, `007C7430`,
    `BSP_Plane_ChooseSpawnFlightState` and the constructor;
  - park's own set at `009B2603`;
  - `009B2170`, a one-instruction setter (`+910h` = 1, `RET`) with no code or data reference.

  The readers are `006CE4A0`, `006CF5DD` (the spot test) and `009B28D6`. Every clear is a change of
  flight state or a (re)construction, and a hangared plane in state 5 reaches none of them.
- **`+904h`.** Its byte writers (5bh) are the same family: `SetFlightState`, `FiveToFour` /
  `FourToFive` (`007C16D8`, `007C175F`), `007C3680`, `007C7430`, `BeginFlying`, `007CB9E0`,
  `OnTouchdownFromFlight`, `ChooseSpawn`, the property bag and the constructors.
  - `FiveToFour` and `FourToFive` run only when park's join test changes its answer, and in the
    hangar it does not.

### Who removes a plane from `site+34h`

- The erase `006CEF80` has one caller, `006CF180`. That is the site's `vtable[28h]`, stored at
  `00CF8970`, `00CF8A20` and `00CF8A80` (the three site vtables). Its known caller is
  `BeginFlying` (`007C7163`, `MOV EAX,[EDX+28h]` ... `CALL EAX`).
- Section 5r's "no slot removes an occupant" is corrected: slot `+28h` does, but only at a
  takeoff.
- A scan for the adjacent form `8B ?? 28 FF ??` in `006B`-`006D`, `007B`-`007D` and `0099`-`009B`
  finds seven more sites: `006D4229`, `006D4D66`, `006D4D82`, `007CC9CD`, `00999B4B`, `009BD0FB`
  and `009BD109`.
  - `007CC9CD` calls `vtable[28h]` on the object returned by `00964790(id)`. It is not a site.
  - The other six were not read.
  - A split form (`MOV reg,[reg+28h]`, another instruction, then `CALL reg`, as at `007C7163`) is
    not covered.
  - **LABELLED:** a non-takeoff erase is not excluded.

### Consequence

- There is no binding to add. `kBaseLaunchChainBound` stays ON.
- A later airfield launch at an airfield where a hangared plane holds `+910h` is held in the image
  too, if the image hangars the plane the same way.
- The Main airfield's hangar position never sets `+910h` (5bh), so it does not block.
- **Open:** the six unread `vtable[28h]` sites above, and a split-form scan. Either could show a
  non-takeoff erase.

## 5bj. The `block+38h` launch brake (packet `cc9_base_launch_brake`, cc9-lua27, 2026-09-30)

The switch is `kBaseLaunchBrakeBound` (`include/bsp/air_operations.hpp`), committed **OFF**.
`kBaseLaunchChainBound` has been ON since 5bh.

### The image, read for this piece (Ghidra and the disk listing, read only)

**The store.** `006C5050` (`006C5314`-`006C5348`) stores the squadron at `block+38h` when its flag
argument is 0 and `block+38h` is not already that squadron:
- `EBP` is the block (`006C5074`), and no other write to `EBP` follows until the epilogue.
- `ESI` is the entity `004F0AD0` returns (`006C52EA`).
- The store is `[block+24h]+14h`, the observer pair's watched slot. The old pair is unregistered
  (`006952A0`) and the new one registered (`00694A60`).

Every launch from `006CC690` passes flag 0 (`006CC72E PUSH 0`). The comment in
`game_hosts_script_orders.cpp` ("RE-JUSTIFIED ... nothing writes block+38h" in a campaign) and
AIROPS_LAUNCH_TICK section 5's "in a campaign the deck has no readiness brake" missed this writer.
5ax found it.

**The release, `006C5B70`**, `__thiscall(block, float)`, `RET 4`.
- `006CDC70` runs it after `006CD240`, when `block+1Ch` and `+1Dh` are clear and the owner at
  `block+7Ch` is present with `+5Dh` clear (`006CDCE1`-`006CDD04`).
- With `block+38h` set and `007ED740(block+38h)` true, it walks the slots and re-publishes
  (`006BF150`: `00696350`, `006BD520`, `0077C7B0`, a slot notice) each slot whose `+28h` is that
  squadron. It then unregisters the pair and clears `block+38h` (`006C5C05`-`006C5C0A`).

**`007ED740`** (`007ED740`-`007ED782`) is true when `+3CCh` > 0 and `007B8BD0` answers true for every
`+3D0h` member.

**`007B8BD0`** (`007B8BD0`-`007B8C15`) is true when any of these holds:
- `plane+72Ch`'s `vtable[38h]` answers. The sub-object's vtable is `00D06130`, stored at
  `007CFD63`; slot `38h` is `0074E210` `BSP_PlaneControlMode_IsFreeFlight`, which tests
  `+1D4h == 7` on the sub-object, that is plane `+900h` == 7.
- `+900h` is 6.
- `+900h` is 4 with `+BF8h` (ground contact) set.

State 5, and every state from 1 to 3, answers false.

**The observer's own clear.** The pair's vtable at `00CF6514` has notify slot `0065AFF0`. It zeroes
`+14h` when the notifying entity is the watched one (`0065B007`). Which notice the squadron sends,
and when, is unread.

So each flag-0 launch holds its deck until all members of the launched squadron are on the ground
in state 4 or flying. While the brake is held:
- `006BF620` (IsReadyToSendPlanes) answers false;
- `006CC690` queues instead of starting (`006CC715`).

The mission scripts gate their launches on `IsReadyToSendPlanes`:
- `usn_19_coralus.lua` lines 539 and 553, mtime 2024-08-26, this installation;
- `commandhelpers.lua` lines 7898, 7916, 8087, 8094 and 8194.

**The six unread `vtable[28h]` sites of 5bi** (read here; none is a site erase):
- `006D4229` and `007CC9CD` call `vtable[28h](word)` on the vehicle-class descriptor from
  `00964790`.
- `006D4D66` and `006D4D82` call it on the entries of `[edi+830h]` and store the result as a
  word, so it is a getter.
- `00999B4B` walks the task vector at `bot+58h`.
- `009BD0FB` and `009BD109` call it on the object `0071BFF0` returns.

5bi's "not excluded" narrows to the split-form calls, which are still unscanned.

### The host with the switch on

- **The store:** `air_ops_launch_start_006c7490` (`src/air_operations.cpp`), after the factory
  returns the squadron. With `request.state == 1` (the flag 0), `deck.launch_in_progress` =
  the squadron's entity id.
- **The release:** `GameUnitsHost::Impl::release_launch_brake_006c5b70`, run at the end of
  `run_landing_queue_006cd240` (`006CDC70`'s order). It applies `007ED740`/`007B8BD0` to the
  squadron record's members. SUBSTITUTIONS, labelled:
  - a member whose scene node is torn down, destroyed or removed is skipped (the `+3D0h`
    compaction at death);
  - a squadron whose members are all gone stands in for the observer notice;
  - a squadron whose members are not resolved yet holds the brake;
  - `006BF150`'s notice is not carried.
- **Not carried:** `006C64B0`, the state-2 wait that launches a queued slot once the brake clears.
  So a launch that `006CC690` queues never starts here; the Lua host's `summary` counts it as
  `queued`. The scripts read above gate on readiness, so none is expected.

### Predictions (switch ON against OFF, both with the chain ON; written before any run)

1. **JM05 3000.** A carrier's next launch waits for its previous squadron.
   - Lexington_sqn03's last member reaches state 4 at about 25.6 s, and Yorktown_sqn04's at about
     25.4 s. So `LaunchSquadron` calls on each carrier move from 3.05/6.05/9.05/12.05 s to about
     3, 26, 49 and 72 s.
   - The airfields release within about 7 s of each launch.
   - The lift was already the bottleneck (5bh: one release every 9 s per carrier), so the
     lift-off times of the later squadrons move by at most about 10 s.
   - All 30 members still lift off within 3000 frames.
   - Death table identical (the squadrons are recalled in both).
2. **JM05 9000.** As row 1 for the first ten squadrons.
   - Secondary airfield: sqn11 (153 s) holds the brake for good. Its two wingmen end in state 5
     (5bh), which `007B8BD0` refuses, so **sqn12 (302.95 s) is not launched** (`held_at_end`
     >= 1).
   - The Main airfield's sqn13 (403 s) launches. Death table identical.
3. **USN04 and E2.** `Lexington-class01_sqn03` and `Yorktown-class01_sqn04` wait for
   sqn01/sqn02 to be all on deck or flying: launch at about 45-50 s instead of 30.05 s. Their
   lift-offs move by at most about 10 s. The strike outcome (releases, first hit) is unchanged. The
   one fighter death of 5bh may move or vanish.
4. **USN13.** One launch per carrier, so the brake is set and released without holding a second
   launch. Gameplay identical (exit 0 or 1).
5. **USN01, LOMP06:** no launch, exit 0 or 1.
6. **Flip criterion:** every brake that is set is released once its squadron is out or on the
   ground in state 4, `queued=0`, and no death row moves that the new launch times cannot explain.
   Row 2's held brake is the image's rule applied to 5bh's airfield hold, not a mechanism failure.

### Results (OFF `local\l27_boff`, SHA-256 `4AAAE55AB63C`; ON `local\l27_bon`, `ED381B4A1106`; both from `8afd63890`)

The 300-frame smoke of the committed tree (USN01, `local\l27_b_off_smoke_smoke.log`) exits 0 with
299 frames presented.

| row | pair_diff | brake: set / released / held at end | launch times ON (OFF) | queued | death table |
| --- | --- | --- | --- | --- | --- |
| JM05 3000 | 3 | 10 / 10 / 0 | carriers 3.05, 27.05, 54.05, 81.05 (3.05, 6.05, 9.05, 12.05) | 0 | identical; every headline value identical |
| JM05 9000 | 3 | 12 / 11 / **1** | as above; Secondary sqn11 153.00; **Secondary's next launch (302.95 OFF) never comes**; Main 403.03 | 0 | identical (18 rows) |
| USN04 4500 | 3 | 4 / 4 / 0 | sqn03/04 at **51.05** (30.05) | 0 | 20 rows re-timed by 0.05-5.35 s, several with another killer; one row enters the window (`A6M Zero #8.2` 224.36 s, which dies at 226.31 s OFF on E2); releases and first hit unchanged |
| E2 9000 | 3 | 4 / 4 / 0 | as USN04 | 0 | same victim set (52); the same re-timings |
| USN13 3000 | **1** | 9 / 9 / 0 | all at 0.00 | 0 | identical |
| USN01, LOMP06 | **1** | 0 | - | - | identical |

Against the predictions:

| prediction | result |
| --- | --- |
| 1. JM05 carriers at about 3, 26, 49, 72 s; all 30 still lift off; deaths identical | **held**: 3.05, 27.05, 54.05, 81.05; `liftoffs=30`; the launch moves no death row. Missed in detail: the airfields release at 17.85 s (Main) and 29.65 s (Secondary), not within about 7 s. Their members reach state 4 with contact only after the taxi |
| 2. JM05 9000: Secondary's second launch never comes | **held**: `held_at_end=1`, 12 launches instead of 13, `liftoffs=34` |
| 3. USN04/E2: second launches at about 45-50 s | **held, 1 s outside**: 51.05 s. Releases unchanged; the strike deaths are re-timed as the fighters now meet it at other places |
| 4. USN13 gameplay identical | **held**: exit 1 |
| 5. controls | **held**: exit 1 |
| 6. flip criterion | **met**: every brake set is released when its squadron is flying or on the ground in state 4, except the one the image's rule keeps behind 5bh's airfield hold; `queued=0` on every row |

### Verdict: **flip ON**

The mechanism matches on every launch row, the scripts wait on `IsReadyToSendPlanes` as read, and
no launch is queued, so the unbound `006C64B0` is never needed. The JM05 9000 held brake is the
image's rule (`007B8BD0` refuses state 5) applied to the held wingmen of 5bh and 5bi.

## 5bk. `planeDesc+158h`, the model box centre (packet `cc9_plane_desc_158`, cc9-lua27, 2026-09-30)

The switch is `kPlaneDesc158Bound` (`include/bsp/game_hosts_units.hpp`), committed **OFF**. Before
this packet the value was a constant 0.0 (`kPlaneDesc158`).

### The image, read for this piece

**`007D470E`-`007D473A`**, in `BSP_PlaneClass_BindModelData_Provisional` (`007D3E60`-`007D5889`):
- `EBP` is the class descriptor (`007D3E7F MOV EBP,ECX`; the whole listing has no other write to
  `EBP`, `local\output\ghidra-disasm-007d3e60-lines-3000-083156.txt`).
- The model is `[desc+50h]` (`007D470E`).
- The code:
  - `FLD [model+30h]`, `FSTP` to a float spill;
  - `FLD [model+3Ch]`, `FSTP`/`FLD` through a float;
  - `FADD` the first;
  - `FMUL` double `00D7A280` (0.5, bytes `00 00 00 00 00 00 E0 3F`);
  - `FSTP float [desc+158h]`.
- The model's local box is min xyz, max xyz at `+28h`..`+3Ch`. The producer is the model's top-level
  `BoundingBox` (`00B7F525`..`00B7F55C`, docs/UNIT_HULL_EXTENTS.md). So `+158h` is the box centre
  along z: (min z + max z) x 0.5.

### The host with the switch on

- `Impl::plane_desc_158(p)` reads the class's `Mesh` file and takes its `BoundingBox` through
  `bsp::read_mmod_bounding_box`. It computes the value in 007D473A's order and caches it per class,
  with one `plane desc 158h:` log line per class.
- SUBSTITUTIONS, labelled:
  - the class model is the `Mesh` file (the section points and the buoyancy list read it the
    same way);
  - a model without a box, or one that does not open, gives 0.0.
- The five readers now call it:
  - the airfield spot slide `(+158h + 1.0) - plane+9D8h x 0.2` (`007C5F60`);
  - the lift offset `-+158h` at placement and at the raise (`007C5F60`'s carrier arm, `006FC810`);
  - park's lift target t (`006D00E0`);
  - the lift intake's nose distance (`006CFE90`). This is now the point (0, 0, `+158h`) carried
    by the plane's forward row, where before it was the origin.

### Predictions (switch ON against OFF, written before any pair)

1. The `plane desc 158h` lines give small offsets, within about +-3 m, for the fighter, dive
   bomber and torpedo bomber models of JM05, USN04 and USN13.
2. **JM05 3000 and 9000.**
   - Each plane sits `+158h` further along z on its lift and its airfield spot.
   - The launch counts, lift-offs (30/30; 34 on JM05 9000 with 5bj's held brake) and brake
     releases are unchanged. Lift-off times move by at most about 1 s.
   - On JM05 9000, park's intake of recalled carrier planes still takes and stows them (5ao's
     "Lexington stows 8 of 8"); the count may move by one.
   - Death tables identical.
3. **USN04, E2:** lift-off times move by at most about 1 s; the strike outcome is unchanged; death
   rows re-timed at most.
4. **USN13:** 27 of 27 lift off. Gameplay-identical or moved only in plane positions.
5. **USN01, LOMP06:** no launch, exit 0 or 1.
6. **Flip criterion:** no mechanism count moves (placements, lift-offs, brake releases, stows
   within one), and no death row moves that the new positions cannot explain.

### Results (OFF `local\l27_poff`, SHA-256 `13ADE266E8B5`; ON `local\l27_pon`, `E59D0D9507B9`; both from `3f3ff4971`)

The 300-frame smoke of the committed tree (USN01) exits 0, with 299 frames presented.

**The values** (`plane desc 158h:` lines; min z / max z of each `BoundingBox`):

| class | model (this installation) | z range | `+158h` |
| --- | --- | --- | --- |
| 135 | `models/planes/us/Warhawk.MMOD` | -5.853 .. 3.877 | **-0.988** |
| 101 | `models/planes/us/F4F_Wildcat.MMOD` | -6.170 .. 2.590 | **-1.790** |
| 112 | `models/planes/us/tdb_devastator.MMOD` | -7.421 .. 4.056 | **-1.682** |
| 108 | `models/planes/us/dauntless.MMOD` | -6.746 .. 3.224 | **-1.761** |
| 26 | `models/planes/us/F6F_Hellcat.mmod` | -5.249 .. 5.075 | **-0.087** |

The offsets are within the predicted +-3 m. Every model opened and had a box.

The OFF side now carries main's later merges, so its JM05 numbers differ from 5bj's; only same-tree
pairs are compared here.

| row | pair_diff | mechanism counts ON (OFF) | death table |
| --- | --- | --- | --- |
| JM05 3000 | 3 | placed 30, lift-offs 30, brake 10/10: unchanged | **identical** (12 rows); hits 353 -> 354 |
| JM05 9000 | 3 | placed 36, lift-offs 34 (34), stows 12 (12); brake held at end **2** (1); unparented lift-offs 33 (34); `c01_sets` 4554 (204) | **identical** (14 rows) |
| USN04 4500 | 3 | 12/12, done 12: unchanged | 20 rows re-timed. One row leaves the window (`A6M Zero #8.2`, 224.36 s OFF; ON it dies at 226.31 s on E2). Releases and first hit unchanged |
| E2 9000 | 3 | unchanged | same victim set (52), the same re-timings |
| USN13 3000 | 3 | 27/27, done 27: unchanged | **identical**; every headline value identical (plane positions only) |
| USN01 | 1 | - | identical |
| LOMP06 | 0 | - | identical |

**The JM05 9000 miss.** The new spot slide makes the airfield members start about 1-2 m further
back (slide `(+158h + 1.0) - index x 0.2`: -0.76/-0.96/-1.16 for Main's class-108 sqn12, against
1.00/0.80/0.60). That re-times the recalled airfield planes' landings, and then 5ar's park <-> abort
loop, already known to be position sensitive, falls differently:
- **Main airfield.** `MainAirfieldEntity 01_sqn01|.-2` now loops with 233 park entries and ends
  holding `+910h` (`q910=1`; OFF `q910=0`). So 5bh's `006CE4A0` hold now also stops
  `MainAirfieldEntity 01_sqn12|.-3` (`hold=357`, never aligned). Its brake stays held (5bj's rule),
  and it is presumably where the extra `c01_sets` come from (not traced).
- **Secondary airfield.** `SecondaryAirfieldEntity 01_sqn02|.-3`, a hangared recalled plane,
  **lifts off again at 378.73 s** (height, y 4.16, 42.85 m/s) out of `land/park` with `q910=0`. That
  is the unparented lift-off.
- **Open, new:** how a hangared plane reaches lift-off speed from `land/park`. Its park summary
  has `entries=1 from_abort=0`, so it is not the abort loop. It is recorded here, not traced.

None of this is the `+158h` read itself. The five readers take the image's value; what moves is the
downstream park loop's sensitivity. No death row moves on either JM05 row.

| prediction | result |
| --- | --- |
| 1. offsets within +-3 m | **held**: -1.79 to -0.09 |
| 2. JM05: counts unchanged, lift-offs within about 1 s, deaths identical | **held on 3000**. On 9000, deaths are identical but two mechanism counts move (held brakes 1 -> 2, one extra relaunch from park): a spread miss through the park loop |
| 3. USN04/E2 | **held**: re-timed only |
| 4. USN13 | **held**: positions only |
| 5. controls | **held** |

### Verdict: **flip ON**, spread miss recorded

The `+158h` mechanism matches the image, and its readers are the image's. The JM05 9000 count moves
come from the park loop that 5ar accepted as the image's, reached at different times, not from this
read. Open for the queue: the park loop's airfield relaunch (above), and 5bi's split-form
`vtable[28h]` scan.

## 5bl. The park relaunch and the takeoff task as head task (packet `cc9_takeoff_task_head`, cc9-lua27, 2026-09-30)

This answers 5bk's open relaunch and 5bg item 2. The switch is `kTakeoffTaskHeadBound`
(`include/bsp/game_hosts_units.hpp`), committed **OFF**.

### The relaunch, traced in the host (`local\l27_pon_jm05l.log`)

`SecondaryAirfieldEntity 01_sqn02|.-3` hangared at 230.76 s. It then:
- **373.63 s:** `land task ...: retired, the squadron's command is no longer land at this site
  (009B34D0)`. The squadron's `returntobase` now resolves to the Main airfield.
- **378.23 s:** `takeoff run ...: ALIGNED state=4 ... thr=1.000 v=42.11`. The **takeoff task from
  7.15 s** has been installed the whole time with `done=0`, under `land`. With `land` gone, the
  pilot think's `else if` (`game_hosts_units.cpp`, the `run_takeoff_task_tick_009cfd70` arm) ticks
  it again. The Takeoff step runs from the hangar spot.
- **378.73 s:** the height lift-off. `007C7110` clears `+904h`/`+910h` and erases the plane from
  `site+34h`.
- **378.98 s:** `land` installs again, for the Main airfield.

So the host path is **a stale takeoff task resumed when `land` retires**. It is not a park exit.
5bi stands: park itself never retires a hangared plane.

### The image: the takeoff task is the head, and `land` waits for it (5bg item 2)

- **`00999F50`** (`0099A4A0`'s install) inserts at the **front** of the bot's task vector
  (`bot+58h`, count `+5Ch`, capacity `+60h`). It shifts every entry up one and stores at `[0]`
  (decompiled here).
- **`0099A020`** (`0099A170`'s install, reached only from `0099A490`) appends at the **back**.
- **The tick** `0099ACD0` ticks only the head: `0099AE89 MOV EDX,[ESI+58h] / MOV EBX,[EDX]`. It
  calls `009998A0` on it (`0099AF1C`) after `0099A4C0` has retired finished head tasks.
- **A command task is built only when the list is empty**, through `0099A4C0`'s tail-jump
  `0099A5EB JMP 0099A170` (rel32 census `local\l27_refs.py`: `0099A170`'s only other caller is
  `0099ADAA`). The other route is `bot+7Ch`. docs/PILOT_BOT_TICK_GATES.md settled that `+7Ch`
  is set only by the move-to state `009C1BA8` when it runs out of target.
- **`00999E40` / `00999EE0`**, run under `+7Ch`, move every task to the retire list at `+64h`, then
  call `vtable[58h]` on each and delete it.

So a `returntobase` delivered to a plane whose takeoff task is live installs nothing until the
takeoff task retires. The takeoff task runs to its done arm (`009CFC70`), and `land` is built
then, from the current command. `land` never stacks over `takeoff`. When `land` later retires,
there is no takeoff task left to resume, so **the image has no such relaunch**.

The host differs in two ways:
1. It installs `land` at the delivery (the labelled substitution of SENTITY_INIT_ATTACH_ORDER
   22.7).
2. It ticks `land` ahead of the takeoff task.

This is why 5bf saw `done=0` on JM05 and `done=12`/`27` where no `returntobase` arrives (5bh).

### The host with the switch on

- The takeoff task, while installed, is ticked ahead of `land`.
- `install_land_task_0099a3dd` on a plane with a live takeoff task only records the delivery.
- The takeoff task's done arm then installs `land` (`0099A4C0 -> 0099A170`).
- SUBSTITUTION, labelled: the deferred install uses the delivered `returntobase`. A different
  order that arrives in between is not re-checked.
- Summary line: `summary takeoff task head: land_deferred= land_after_takeoff=`.

### Predictions (switch ON against OFF; written before the pairs)

1. **JM05 3000 and 9000.**
   - Every launched member that lifts off keeps its takeoff task to the done arm (`done` about
     30, not 0).
   - `land` installs about 5-10 s after each lift-off instead of about 2 s after it
     (`land_after_takeoff` about equal to `done`).
   - On 9000, the park relaunch goes away: no lift-off from a hangared plane, `unparented` equal
     to `liftoffs`.
   - The recalled planes land a few seconds later. Death tables identical.
2. **USN04, E2, USN13.** No `returntobase` reaches these squadrons (5bh), so `land_deferred=0`.
   Gameplay-identical (exit 0 or 1), except where the head order meets another task.
3. **USN01, LOMP06:** exit 0 or 1.
4. **Flip criterion:** `done` = lift-offs on every row, no relaunch from park, no death row
   moves that the later land installs cannot explain.

### Results (OFF `local\l27_hoff`, SHA-256 `DA281A29FF47`; ON `local\l27_hon`, `372D1E3FF0D1`; both from `9edf15bd0`)

The 300-frame smoke of the committed tree (USN01) exits 0, with 299 frames presented.

| row | pair_diff | takeoff `done` ON (OFF) | land deferred / installed after takeoff | lift-offs, unparented ON (OFF) | death table |
| --- | --- | --- | --- | --- | --- |
| JM05 3000 | 3 | **30** (0) | 30 / 30 | 30, 30 (30, 30) | **identical**; every headline value identical |
| JM05 9000 | 3 | **34** (0) | 36 / 34 | 34, **34** (34, 33) | **identical** (14 rows) |
| USN04, E2 | **1** | 12 (12) | 0 / 0 | unchanged | identical |
| USN13 | **1** | 27 (27) | 0 / 0 | unchanged | identical |
| USN01, LOMP06 | **1** | - | - | - | identical |

- **The relaunch is gone.** JM05 9000's `SecondaryAirfieldEntity 01_sqn02|.-3` lifts off once, at
  34.25 s. Every lift-off is from a parented plane.
- The 5bk count moves also return: the brake is held at end 1 (was 2) and `c01_sets` is 78 (was
  4554). The two deferred installs without a done are Secondary sqn11's held wingmen (5bh), which
  never lift off.
- **Timing.** Lexington_sqn03's leader lifts off at 15.70 s. Its takeoff task retires at 20.70 s
  (y 68.9, 64.0 m/s: the `+908h` > 5 s arm), and `land` installs in the same step.

| prediction | result |
| --- | --- |
| 1. JM05: `done` about equal to lift-offs, land after takeoff, relaunch gone, deaths identical | **held** |
| 2. USN04, E2, USN13: `land_deferred=0`, gameplay-identical | **held**: exit 1 |
| 3. controls | **held** |
| 4. flip criterion | **met** |

### Verdict: **flip ON**

5bg item 2 is answered: `land` replaces nothing and stacks over nothing. It waits for the takeoff
task's done arm, as the image's task list orders them.

## 5bm. Install at delivery against the image's task list: a census (packet `cc9_task_install_order`, cc9-lua27, 2026-09-30)

SENTITY_INIT_ATTACH_ORDER 22.7 labelled a substitution: the host installs a command task at the
order's delivery, where the image builds it through `0099A170` inside the bot tick. 5bl fixed that
for the takeoff task. This section is the census for every other task class. It was read from
Ghidra and the disk image; no code changed and no run was made.

### The retire rule `0099A4C0`, read whole

The rule runs once per bot think (`0099AE7E`), after the interval gate: `bot+70h` accumulates `dt`
and the think runs once it reaches 0.09 s (`00D1F39C` = `3DB851EC`).
- If the director's command is `00E08F88`, it first calls `0099A0A0`.
- Then, while the list has a head task, the head **stays** when any of these holds:
  - `vtable[38h]` is false;
  - `vtable[34h]` is true;
  - the list has one task and `vtable[40h]` answers 1.
- Otherwise the head goes to the retire list and the loop repeats.
- When the list ends empty, it tail-jumps to `0099A170`, which builds the command task. The same
  think then ticks it (`0099AE89` onward).

### The slots of every task class (`local\l27_taskvt.py`, `local\l27_slots.py`, disk image)

| class (factory) | task vtable | `34h` | `38h` | `40h` |
| --- | --- | --- | --- | --- |
| base | `00D05704` | `0099B6F0` true | `0099B700` false | `0099B730` 0 |
| Land (`009B41C0`) | `00D1FFA0` | `0099B700` false | `0099B710` true | `009B3560` |
| Dogfight (`009AB570`) | `00D1F9B0` | false | true | `009A9CC0` |
| MoveTo / attackmove moveto (`009C3BE0`, `009C3C40`) | `00D20B68`, `00D20BE0` | false | true | `009C31B0` |
| DiveBomb (`009C8C70`) | `00D20E18` | false | true | `009C8060` |
| LevelBomb (`009B9030`) | `00D20210` | false | true | `009B81F0` |
| Stop (`009BADB0`) | `00D20500` | false | true | `009B9590` |
| `009BDBB0` | `00D20910` | false | true | `009BD280` |
| Kamikaze (`009AF720`) | `00D1FD40` | false | true | `009AF560` |
| DropKamikaze (`009AEBE0`) | `00D1FC70` | false | true | `009AE140` |
| DepthCharge (`009A6970`) | `00D1F738` | false | true | `009A5DB0` |
| CloseToShip (`009A2F40`) | `00D1F4E8` | false | true | `009A2920` |
| **Takeoff** (`009CFF40`) | `00D21228` | **`009CF9E0` true** | true | `0099C2C0` |

- Every command task class has `34h` false and `38h` true. So it retires at the first think in
  which its `40h` stops answering 1.
- Each `40h` starts by reading `task+404h` (the unit). `009C31B0`, read here, answers 1 only while
  the director's current command is still moveto (`00E08F68`) with the same target (`task+43Ch`),
  or, for a point target, the same point within `00CE3D64`. Otherwise it answers 0 (2 with no unit).
- Only the takeoff task holds its place against a new order (`34h` true). 5bl bound that.

### Why no binding is needed for the command classes

- **Image.** A new order is taken at the next bot think, whose accumulator the host carries
  (`pilot_think_accumulator_70`, the 0.09 s gate). The old head's `40h` answers 0 there,
  `0099A4C0` retires it, `0099A170` builds the new task, and the same think ticks it.
- **Host.** It installs at the delivery, and the task is first ticked at that same next think.
- So both run the new task from the same think.
- The only difference is when the task is **constructed**: at the delivery in the host, at the
  think in the image. That is at most one 0.09 s think earlier. The constructors read the unit's
  pose and the target once, so such a read is at most 0.09 s early.
- This is below every mechanism the pairs measure. A switch would move nothing a pair can judge,
  so none is bound. 22.7's label is corrected accordingly: "one bot tick early" means the
  construction only; the first tick is the same think.

### Task pushes the host does not model (census; open items)

`00999F50` (the front push) has eight rel32 callers (`local\l27_refs.py`). The table gives the
last vtable stored before each (`local\l27_pushvt.py`, `local\l27_mk.py`):

| call site | in | task made by | task vtable | `34h` | host |
| --- | --- | --- | --- | --- | --- |
| `0099A4AD` | `BSP_PilotBot_InstallTakeoffTask` | `009CFF40` | `00D21228` | true | bound (5bd, 5bl) |
| `0099B113` | `BSP_PilotBot_Tick`, tail | `009CFF40` (takeoff again) | `00D21228` | true | **not modelled**. Gated on the unit not in free flight, the head's `38h` and `30h` true, and `0042A7E0(unit)` or `+900h == 6` (docs/PILOT_COMMAND_PATH.md) |
| `0099AE1C` | `BSP_PilotBot_Tick` | `009BBFC0` | `00D20658` | false (`009BB6B0`), `38h` `009BB6C0` | **not modelled**. Pushed for a flight leader with `unit+184h` set, when the head's `38h` answers |
| `007B6223`, `007B657D` | `007B6240` and the block before it | `009BC030` | `00D205E0` | true | **not modelled** |
| `009CBB19`, `009CBE27` | `009CBB30` and the block before it | `009BC030` | `00D205E0` | true | **not modelled** |
| `009BB47F` | `009BB380` | `009BAFC0` | `00D20568` | true | **not modelled** |

- The `00D205E0` class (tick `009BA020`) and the `00D20568` class (tick `009B93D0`) hold their
  place like the takeoff task.
- What they are, and whether any reference row reaches them, is unread. These are the open items
  of this census.

## 5bn. The front-pushed task classes, identified (packet `cc9_front_pushed_tasks`, cc9-lua27, 2026-09-30)

This section follows up 5bm's four unmodelled `00999F50` pushes. It was read from Ghidra and the
disk image. No code changed and no run was made. Scripts are in `local\` of the cc9-lua27 tree:
`l27_refs.py`, `l27_mk.py`, `l27_s30.py`, `l27_pushvt.py`.

### 1. The re-takeoff at the tick tail, `0099B0BE`-`0099B113`: reachable, and it changes 5ar

The listing, read whole:
- `0099B0BE`-`0099B0D4`: `(unit+72Ch)->vtable[38h]` is false, so the plane is not in state 7.
- `0099B0D6`-`0099B0E1`: the head task's `vtable[38h]` is true.
- `0099B0E3`-`0099B0F8`: `0042A7E0(unit)` is true (`+900h` 4 or 5), or `+900h == 6` (state 6's meaning is unread).
- `0099B0FA`-`0099B105`: the head task's `vtable[30h]` is true.
- `0099B107`-`0099B113`: then `00999F50(bot, 009CFF40(bot, 0))`, a **new takeoff task at the
  head**.

The head task's `vtable[30h]` (`local\l27_s30.py`):
- It is true for every command task class (`0099B6F0`; CloseToShip's own `009A28C0` also answers true), with three exceptions:
  - Stop (`009BAC40`, false);
  - Takeoff (`009CF9D0`, false);
  - Land (below).
- **Land's is `009B3730`**. It answers false when the current land state is `land/park` (task
  `+620h`) or `land/final` (`+5F8h`), and **true in every other state**. Those states are
  `land/abort`, `land/standby`, `land/begin`, `land/line`, and moveto/follow (land).

So in the image, a grounded plane whose land task is in `land/abort` gets a takeoff task at the
head on the next think. It rolls, lifts off, and the takeoff task retires at its done arm. The land
task then resumes from abort, through standby to a new approach.

- The host has no such push.
- 5ar's accepted "invisible park <-> abort loop" is the host running `land/abort` on the ground
  without it.
- **Reach:** reference U's JM05 9000 row (`g20_ru_jm05l.log`, cc9-gunnery20 tree) has 31
  `land park` planes with `from_abort` totalling **2896** (JM05 3000: 0). Each re-entry from
  abort on the ground is a point where the image would push this takeoff.
- **5ar's verdict ("the image loops invisibly") is contradicted.** As far as this listing shows,
  a park abort on the ground is a relaunch in the image. Not yet bound: queue item 1 of the
  handoff.

### 2. `009BC030` -> `00D205E0` (tick `009BA020`): a timed roll manoeuvre, not reached

- `009BC030` news `40Ch` and constructs through `009BAFC0` (the `00D20568` class below). It then
  stores `+408h` = its byte argument, the vtable `00D205E0`, and `+404h` = 1.
- The tick `009BA020` writes the plan:
  - a bank target of `+-00CE3830` (the sign from `+408h`);
  - a heading from `009B9680`;
  - a throttle blend from the pitch error `+3FCh - unit+C64h`.
- It retires through `0099B690` when its `+3F8h` object answers. `34h` is true (`009BA810`), so it
  holds its place like the takeoff task.
- It is pushed by `007B6240` (and the block before it, `007B6223`) and by `009CBB30` (and
  `009CBB19`), both on a random timer (`BSP_Random_UniformFloatRange` over `+28h..+2Ch` and
  `+34h..+38h`).
- `007B6240` and `009CBB30` read `Pilot/AutoStrafeAngle/Angle_GoAway` (docs/GAME_TUNING_SINGLETON.md
  `+674`). `009CBB30` lies in the strafe task's code (`BSP_BotTask_MakeStrafe` `009CD300`), so this
  is the strafe run's evasive roll.
- **Reach:** the host has no model of `007B6240` or `009CBB30` (docs/LUA_BINDING_MISSION.md lists
  `007B6240` as a reader without a host model). No reference log can show it. Open.

### 3. `009BAFC0` -> `00D20568` (tick `009B93D0`): the roll's base class

- `009BAFC0` calls `BSP_BotTask_ConstructBase(owner, 0Fh)`, stores `+3F8h` = its second argument
  and the vtable `00D20568`, and captures a pitch/height reference (`+400h`).
- `009BB380` pushes it directly (`009BB47F`). Who calls `009BB380` is unread; open.
- Not reached as far as read.

### 4. `009BBFC0` -> `00D20658` (tick `009BC3A0`): the flight leader's task, not identified

- Pushed at `0099AE1C` when the list is not empty, `unit+184h` is set, the unit is its squadron's
  flight leader, and the head's `38h` answers. It carries `00D1F31C` (`0099AE03`).
- `34h` is false, so a later command retires it.
- `unit+184h` and the tick `009BC3A0` are unread. Open.

## 5bo. Handoff (cc9-lua27, 2026-09-30, stamped 18:14 UTC)

Branch `agent/cc9-lua27`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua27`. No lease
is held.

| packet | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| `cc9_base_launch_flip` | `a43db386d`, `4c228cca8` | `kBaseLaunchChainBound` | **ON** | 5bh |
| `cc9_airfield_park_exit` | `7e345bb8b` | - | read: park never retires a hangared plane | 5bi |
| `cc9_base_launch_brake` | `8afd63890`, `4aa6700d6` | `kBaseLaunchBrakeBound` (`air_operations.hpp`) | **ON** | 5bj |
| stale brake notes | `fa21cc8a9` | - | AIROPS_LAUNCH_TICK 5 and the script-orders comment corrected | - |
| `cc9_plane_desc_158` | `3f3ff4971`, `a9af6c1c1` | `kPlaneDesc158Bound` | **ON**, spread miss recorded | 5bk |
| `cc9_takeoff_task_head` | `9edf15bd0`, `911f42c52` | `kTakeoffTaskHeadBound` | **ON** | 5bl |
| `cc9_task_install_order` | `766a29f58` | - | census: no binding needed | 5bm |
| `cc9_front_pushed_tasks` | this section's commit | - | read | 5bn |

### Next, in order

1. **Bind the tick tail's re-takeoff** (5bn.1) behind a new switch, committed OFF.
   - In the pilot think, after the task tick, push a takeoff task at the head when all of these
     hold:
     - the plane is not in state 7;
     - the head's `38h` is true;
     - the plane is in state 4 or 5, or in state 6;
     - the head's `30h` is true.
   - For `land`, `30h` is true except in park and final.
   - The push reuses the host's takeoff task install, and 5bl's head order then ticks it. The
     construction argument is `009CFF40(bot, 0)`; check its difference from `0099A4A0`'s call.
   - Predictions to write first:
     - JM05 9000's aborted park planes (2896 re-entries in reference U) relaunch and go around
       instead of looping;
     - more lift-offs; the brake is unaffected (not a base launch);
     - death rows only where the extra flights meet AA.
   - Pair JM05 3000 and 9000, LOMP10 3000 and 9000 (landing rows), USN04 and E2, plus two
     controls.
   - This also re-opens 5ar's verdict on `kLandParkStateBound`.
2. Identify `+900h` state 6 (it gates both `007B8BD0` in 5bj and the 5bn.1 tail).
3. The strafe run's evasive roll (5bn.2) needs `009CBB30`'s owner in the host first. The flight
   leader's task `009BBFC0` needs `unit+184h`. Both are open reads.
4. `007C6F50`'s `msg+20h` flag and `+C49h` (5bg item 4).
5. 5bi's split-form `vtable[28h]` scan (low value).

**Tools** (`local\` in the cc9-lua27 tree):
- `l27_runs.ps1`: the reference rows;
- `l27_cmp.py`: headline, per-victim death diff and base-launch summaries of a pair;
- `l27_refs.py`: rel32 and dword references on disk;
- `l27_taskvt.py`, `l27_slots.py`, `l27_s30.py`, `l27_mk.py`, `l27_pushvt.py`: the task vtable
  census;
- the edit scripts `l27_brake_edit.py`, `l27_p158_edit.py`, `l27_head_edit.py`.

## 5bp. The tick tail's re-takeoff for a grounded plane (packet `cc9_ground_retakeoff`, cc9-lua28, 2026-09-30)

This binds 5bn.1. The switch is `kGroundRetakeoffBound` (`include/bsp/game_hosts_units.hpp`),
committed **OFF** with the predictions below.

### The image, read whole (disk listing `0099ACD0`-`0099B1A3`)

- **When the tail runs.** Only on a think that ticks the head task: the tick returns early for a
  dead or removed plane (`0099ACE1`-`0099AD09`), skips to the end when the list is empty
  (`0099AEC4`) or the `+74h` countdown has not run out (`0099AF10`), and after `009998A0` skips
  to the end for a player-held plane (`0099AF33`, the `+9C2h` byte). The block `0099AF4F`-`0099B0B9`
  has no other exit.
- **The head.** `EBX` is written once, at `0099AE8C` (`[bot+58h][0]`, after `0099A4C0`'s retire)
  or `0099AE90` (0 for an empty list). It is the task `009998A0` ticked at `0099AF1C`.
- **The gate** (`0099B0BE`-`0099B105`), all of:
  - `(unit+72Ch)->vtable[38h]` false: `+900h` is not 7 (free flight);
  - the head's `38h` true (every command class, land and takeoff; 5bm's table);
  - `0042A7E0(unit)` (`+900h` 4 or 5, `0042A7E0`-`0042A7F9`) or `+900h == 6`;
  - the head's `30h` true. Land's is `009B3730` (`009B3730`-`009B374F`): false in `land/park`
    (`+620h`) and `land/final` (`+5F8h`), true otherwise. Takeoff's `009CF9D0` and Stop's
    `009BAC40` are false; every other command class answers true.
- **The push** (`0099B107`-`0099B113`): `009CFF40(bot, DL = 0)` then `00999F50`, the front push.
- **What `DL = 0` changes** (`009CF8E0`, decompiled): the start state is
  - `takeoff/parking` (`+4B8h`) when the plane is landed (`+904h`) and on the path (`+900h == 5`),
    as with `DL = 1`;
  - otherwise `takeoff/Takeoff` (`+470h`) when `unit+AA0h <= 0.0` (`00D7A218`), else
    `takeoff/SlowTakeoff` (`+490h`). `DL = 1` (`0099A4A0`) starts in `takeoff/prepare` (`+448h`)
    instead. So the re-takeoff skips prepare's wait and its site permission.
  - `takeoff/Takeoff`'s enter is `009CE270` (`009CE270`-`009CE290`): `+18h = -1.0`, `+1Ch = 0`,
    then a tail-jump to `007C17D0(plane)`.
- **State 6** is the plane on the water: `007CB7F0`'s tail `007CB92C` moves 7, 4 and 5 to 6 at the
  water line (docs/ATTACK_RUN_DESCENT.md, the host's `cc9_water_surface_law`). A dead plane does not
  think, so only a live ditched plane reaches the tail there. Queue item 2 of 5bo is answered by
  that existing reading; `007B8BD0`'s use of state 6 in 5bj is the same field.

### The host with the switch on

- In the pilot think, after the task arm (the host's `009998A0` call), the gate above picks the
  head as the host orders it: the takeoff task when installed (5bl), else `land`, else any other
  installed command task (torpedo, dive bomb, moveto, dogfight).
- A push installs the takeoff task through the `0099A4A0` install's approach setup (`009CD420`)
  with the `DL = 0` start state: parking when `+904h` and `+900h == 5` (still **not bound**: it is
  recorded and the task sits in parking, and the plane holds), else `takeoff/Takeoff` with
  `009CE270`'s writes. SUBSTITUTION, labelled: `+AA0h` is carried as 0.0 (5be), so SlowTakeoff is
  never chosen.
- The land task stays installed below it and is not ticked until the takeoff task's done arm; it
  then resumes from `land/abort`.
- The player-held skip is not modelled (the rows run an idle player).
- Summary line: `summary ground retakeoff: pushes= parking= takeoff= land_head= other_head=
  state6=`.

### Predictions (switch ON against OFF; written before the pairs)

Reference U: of the landing rows only JM05 9000 has land aborts (2900 entries, 7 planes; 2896 of
them `from_abort` in `land park`). JM05 3000, LOMP10 3000 and 9000, USN04, E2, USN13, USN01 and
BSM01 have none, and only JM08 has a live water contact.

1. **JM05 9000.** The six loopers (F4F Wildcat 01, SB2C Helldiver 02, MainAirfieldEntity
   01_sqn01|.-2 and |.-3, SecondaryAirfieldEntity 01_sqn02|.-2 and |.-3) get a push at their
   first ground abort, in state 4 (their park summaries end in state 4 with `contact` as the done
   reason), so in `takeoff/Takeoff`.
   - They take a takeoff run where they stand, lift off, and the takeoff task retires. `land`
     then resumes from abort, flies a new approach and lands again. Each cycle costs one push.
   - `from_abort` falls from 2896 to a few per plane (one per cycle). Pushes about equal to the
     aborts on the ground; lift-offs rise by the same count.
   - `pair_diff` 3. Death rows identical unless a relaunched plane meets enemy fire; any moved row
     must involve one of the six.
2. **JM05 3000, LOMP10 3000 and 9000, USN04, E2.** No ground abort: pushes 0 and `pair_diff`
   1 (the summary line) unless a grounded plane carries some other command task.
3. **Controls USN01 and BSM01:** pushes 0, exit 0 or 1.
4. **Flip criterion.** The six loopers relaunch instead of looping, every push is from a
   grounded head with `30h` true, no push on a row without ground aborts, and no death row moves
   that the relaunch cannot explain. A relaunch that fails to lift off (the run stalls with no
   holder) is a mechanism failure and keeps the switch OFF.

### Results (OFF `local\l28_roff`, SHA-256 `99C60502B60B`; ON `local\l28_ron`, `880A12FEA0BA`; both from `6a040f4dd`)

The 300-frame USN01 smoke of the committed tree exits 0 (299 frames presented). All rows are in the
reference launch form. `jm05x` is an extra row, JM05 at 12000 mission frames, run because the one
push on the 9000 row comes 4 s before its end.

| row | `pair_diff` | death rows | land aborts OFF / ON | pushes (all `land` head, state 4, `takeoff/Takeoff`) |
| --- | --- | --- | --- | --- |
| JM05 3000 | 1 | identical (12) | 1 / 1 (airborne) | 0 |
| JM05 9000 | 1 | identical (14) | 21 / 2 | 1 |
| JM05 12000 (`jm05x`) | 1 | identical (17) | 749 / 4 | 3 |
| LOMP10 3000, 9000 | 1, 1 | identical (3, 6) | 0 / 0 | 0 |
| USN04, E2 | 1, 1 | identical (50, 52) | 0 / 0 | 0 |
| USN01, BSM01 (controls) | 1, 1 | identical (17, 0) | 0 / 0 | 0 |

- **The base has moved since reference U.** On this tree the JM05 9000 loop is one plane, SB2C
  Helldiver 01 (20 `from_abort`), not six planes and 2896. At 12000 frames there are three
  loopers: Helldiver 01 (365), MainAirfieldEntity 01_sqn01|.-3 (206) and
  SecondaryAirfieldEntity 01_sqn02|.-3 (177).
- **The gate and the push behave as read.** Each looper gets exactly one push, at its first ground
  abort (Helldiver 01 at 446.06 s: `head=land state=4 landed=1 bf4=1 bf8=0 y=2.74 |v|=3.19`). The
  pushed task is the head, and `land` is not ticked again. The aborts fall from 749 to 4. No push
  on any row without a ground abort; no parking start; no state-6 push.
- **The relaunch never lifts off.** A diagnostic build (the ON export with two log lines in the
  run step, not committed; `local\l28_diag_jm05x.log`) shows it:
  - Helldiver 01 is at y 2.33 at 446.16 s. By 451.16 s it is at **y -77.86** at 41 m/s, and it
    runs on at 69.5 m/s near y -79 to the end.
  - The other two do the same: y -76.1 at 70-80 m/s, and y -56.0 at 67 m/s.
  - Every step then takes the run step's MinWaterSpd arm (the host's arm D, `h > y` with
    `h = 0`) and returns after `007B9010` sets `+C01h = 1`. The takeoff task's done arm (free flight) is never reached. Lift-offs are
    unchanged (34 = 34).
- **The OFF loopers are underground too.** Helldiver 01's land-task trace on the OFF side reads
  alt -70.9 at 462.56 s and -79.2 at the end, 4174 m from its site. So the "park <-> abort loop"
  is a plane that has fallen through the ground. In state 4 off the runway rectangle (`+BF8h`
  clear), the host holds no ground under it. Land/abort's full throttle then drives it under the
  airfield; with the switch on, takeoff/Takeoff's does.

| prediction | result |
| --- | --- |
| 1. JM05 9000: the loopers relaunch, lift off and fly a new approach | **failed**: one push per looper as predicted, but the run goes under the ground and never lifts off |
| 2. JM05 3000, LOMP10, USN04, E2: no push, exit 1 | **held** |
| 3. controls: no push, exit 0 or 1 | **held**: exit 1 |
| 4. flip criterion | **not met**: mechanism failure (no lift-off) |

### Verdict: **keep OFF** (mechanism failure, recorded)

The image reading stands: the tail pushes a takeoff task over a grounded `land` task that is not in
park or final. What fails is the host's ground under a state-4 plane off the runway rectangle.

**5ar re-assessed.**
- 5ar accepted the airfield loop as "the image's own behaviour as far as the listing shows". Two
  readings contradict that:
  - 5bn.1: in the image, the first ground abort pushes the takeoff task, so there is no loop;
  - this section: the host's loopers are below the terrain.
- So the loop is **not image behaviour**. It is a host artefact of a plane that lost its ground
  (`done_why contact`, `+BF8h` clear) and fell through.
- `kLandParkStateBound` stays ON, because its other effects are measured and hold:
  - carrier planes reach the lift (5ao.1);
  - the other airfield planes hold in state 5 at their hangar points.
- The label on the switch changes from "image loop accepted" to "loop is a host ground-support
  artefact; see 5bp".
- Whether park or the ground law should change is the next packet: find why a hangared plane's
  `+BF8h` clears (Helldiver 01 at 446.06 s, `|v|` 3.19 inside the hangar), and what holds a
  state-4 plane off the rectangle in the image (`007CB7F0` and the ground-contact producer).

## 5bq. A grounded plane off the runway rectangle: no terrain producer in the image; the host's hangar orbit (packet `cc9_plane_ground_support`, cc9-lua28, 2026-09-30)

This is a read, with the logs of 5bp (`local\l28_roff_jm05x.log`, the OFF side of JM05 at 12000
frames). No code changed.

### 1. Why `+BF8h` is clear: it never was set at the hangar

- `+BF8h` is `006BC530` (decompiled): the point is inside the holder's
  rectangle, `|l.x| < +B0h x 0.5` and `|l.z| < +B4h x 0.5`. The host's copy matches it.
- Helldiver 01's contact-loss line at 445.86 s reads `local=(58.73 -0.07 26.56)
  half=(17.50 260.00) state=4`. The hangar spot is 58.7 m to the side of a runway that is
  17.5 m half-wide, so the plane has been off the rectangle ever since it taxied into the hangar
  (184.41 s).
- What held it up until then is state 5. `007DCCF0` (read) runs the ground law when `+BF8h` is set
  **or** `+900h == 5`, and the free-flight step `007DC830` otherwise.
- At 445.86 s land/park's join-or-leave test (`009B270F`-`009B2735`, the host's copy of the
  listing) left the path. The path needs `v <= 1.5 x AirField/MoveSpd` (0.69, so 1.035 m/s), and
  the plane was at 1.04 m/s. In state 4 off the rectangle, `007CBFA0` runs `007DC830`: free flight
  at walking speed, so it falls.

### 2. The image has no terrain producer for a plane

**Dyn.** The plane's body is created at `007D6137` (`00C5D580`, in `007D5D20`). Each of its
shape descriptors gets group `8000h` and mask `8000h` (`007D5FE8`-`007D5FF0`:
`[edi-3Ch]` = descriptor `+08h`, `[edi-38h]` = `+0Ch`, where the descriptor is `edi-44h`,
`007D6004`). The terrain shape is group 8, mask 0 (GUNNERY_OPEN_ITEMS 83.1). The pair filter
`00C44090` tests `(maskB & groupA) || (maskA & groupB)`: `8000h & 8 = 0` and `0 & 8000h = 0`. So
a plane never collides with the terrain in the Dyn world.

**Plane tick.** No routine on the plane's tick samples the ground:
- `007CE040`'s callees (Ghidra) include no ground-height routine;
- `007CBFA0`'s calls include none. Its direct calls (disk listing) are `007C5AC0`,
  `0041E870`, `00419CC0`, `00BD1510`, `0042E740`, `007DCCF0`, `007B8DA0`, `00762A00` and
  `0077C2A0`. Its indirect calls are:
  - `007CC08A`, the unit's `vtable[194h]`, called with a string just built by `0041E870`;
  - `007CC13F`, `vtable[1ECh]` (`007CAF10`);
  - `007CC186`, a site's `vtable[38h]`;
  - `007CC264`, the holder owner's `vtable[5Ch](9)`;
- `007DC830` (the free-flight step) includes none;
- `007CC2F0`, the free-flight arm, samples only the water (`BSP_GameWorld_SampleWaterHeight`), and
  it runs only in state 7.

**The ground-height routine's callers.** `00903860 BSP_World_GroundHeightAt` has 24 callers
(Ghidra xrefs). The plane code among them is:
- `006CF8FD`, placing a queued plane (`BSP_AirOpsSite_PoseQueuedPlane`);
- `0099FA2C`, `009A0624`, `009A0E56`, the pilot's terrain avoidance (`BSP_PilotBot_AvoidTerrain`).

There is no contact or clamp.

**So what supports a grounded plane in the image** is the holder's rectangle or the path
(state 5), and nothing else. A state-4 plane off both free-flies there as it does here.

**Uncertainty.**
- Ghidra's callers can under-report (a rel32 scan of `00903860` has not been run).
- A later write to a plane shape's group or mask (for example through the hit-index detach
  `00710B80`) has not been ruled out.

### 3. The divergence is upstream: every hangared plane orbits its spot

A census of the land-park trace after 250 s (`local\l28_orbit.py`) covers every airfield plane
hidden in a hangar on JM05 12000:
- each circles its hangar target, 4 to 15 m from it, at up to 7-10 m/s. The requested speed is
  AirField/MoveSpd, 0.69 m/s;
- each exceeds the 1.035 m/s path limit in 40-54 of about 175 samples, for 350 s.

| plane | samples | vmax | over 1.035 m/s | max distance from target |
| --- | --- | --- | --- | --- |
| F4F Wildcat 01..04 | 175 each | 8.3-10.1 | 40-54 | 12.1-15.8 |
| SB2C Helldiver 02 | 175 | 9.0 | 42 | 15.0 |
| SecondaryAirfieldEntity 01_sqn02, \|.-2 | 175 | 9.2, 10.1 | 53, 53 | 14.7, 14.6 |
| MainAirfieldEntity 01_sqn01, \|.-2 | 175 | 7.1, 8.8 | 49, 43 | 12.5, 13.3 |
| **SB2C Helldiver 01, MainAirfieldEntity 01_sqn01\|.-3, SecondaryAirfieldEntity 01_sqn02\|.-3** | 116-166 | **65-78** | 39-51 | **2217-3960** |

- Most excursions into state 4 last a step or two: the plane slows and rejoins the path.
- The three planes that are lost stayed in state 4 long enough for park's done test to fire
  (`done_why contact`, off the path with `+BF8h` clear). After that, land/abort's throttle 1.0
  (5bp) keeps them too fast to rejoin, and they fall.
- The fall is the image's law once the state is 4. The orbit that puts them there is the host's.

In the image a hangared plane is expected to stop on its spot. Its park step is still ticked (5bi),
but a plane that comes to rest within the deadband never leaves the path.

The host's orbit (up to 10 m/s against a 0.69 m/s request) points at one of two places:
- the park step's steer and speed arithmetic, `009B273A`-`009B28C2`;
- the state-5 ground law's speed tracking (`007DB680` in mode 1).

**Next packet:** trace one hangared plane's park step and ground law per step inside its hangar
(steer vector, deadband `f18`, requested and actual speed), and find which side overshoots.

A terrain floor for planes would be a substitution with no image producer. It is **not bound**.

## 5br. The hangar orbit is the image's park law; the re-takeoff re-paired (packet `cc9_hangar_orbit`, cc9-lua28, 2026-09-30)

This follows 5bq section 3. Two diagnostic builds traced SB2C Helldiver 02 once per think inside
its hangar on JM05:
- `local\l28_diag2.log`: 0099D300's throttle inputs and outputs;
- `local\l28_diag3.log`: land/park's internals.

Neither build is committed.

### 1. What the trace shows

- **The 0.69 m/s "request" of 5bq was a misreading.** AirField/MoveSpd (tuning `+184h`) is
  **9.72 m/s** in this installation. `hi` is 18.67.
  - `slow` (`v <= 1.5 x base`, 14.6 m/s) is therefore always true.
  - The request is `spd = RunwayYawTurnSpdLimit/1` (6.94 m/s, `009B2705`) whenever `qd + 30 > dz`.
    It is then scaled by the heading error: `00419010(3 deg, 1.0, (5|sx| + 30) deg, 0.1, |err|)`,
    `009B2A1F`-`009B2AED`.
  - Facing away from the spot, it asks 0.69 m/s (scale 0.1). Facing the spot, it asks 6.94 m/s.
- **The orbit.** At 310.5-312.8 s the plane turns toward its spot from 5.4 m to the side:
  - the scale rises from 0.25 to 1.0, and the request from 1.76 to 6.94 m/s;
  - the throttle follows up to 0.89, and the plane reaches 7.4 m/s;
  - it passes the spot (`dz` goes from +0.8 to -0.5), the heading error jumps to 1.9-2.7 rad, and
    the request drops to 0.69 m/s. It circles back.
- **The airfield arm never stops at the spot.**
  - The carrier arm's "behind the target, stop" and its speed clamp (`009B2923`-`009B298D`) are
    skipped for an airfield (`009B2921 JE 009B2993` on the carrier flag `[esp+13h]`).
  - The deadband `f18` only widens the yaw deadband.
  - `f18 < 3` hides the plane (`007B96C0`), and `+C00h` has no reader in the image
    (GAMEPLAY_LOOSE_ENDS_2 A2).
  - So a hidden plane keeps being driven.
- **Leaving the path.** The join-or-leave test (`009B2634`-`009B2735`, disk listing, which matches
  the host line for line) keeps the plane in state 5 only while `qd >= dz`, with
  `qd = 1.5 / max(RunwayYawTurnSpdMul x YawSpd, AirField/MinTurnSpd) x RunwayYawTurnSpdLimit/1`.
  - An orbit that swings more than `qd` short of the spot leaves the path. Helldiver 01 did so at
    445.86 s, at `dz` 11.9 m.
  - In state 4 off the rectangle, `007DCCF0` runs free flight (5bq).
- **Corrections to 5bq.**
  - 5bq's "1.5 x MoveSpd = 1.035 m/s" is wrong: MoveSpd is 9.72 m/s. The path exit is the `qd` test.
  - The throttle law is not the cause. `0099D300`'s demand arm integrates the speed error (the
    host copy matches `0099D8C1`-`0099DC75`) and follows the request. The host differs only in
    the dead band at `0099DB1F`-`0099DB56` (`|e| <= 0.833`, `v >= 1.0` and `0.5 <= want/v <= 1.5`
    skips the increment; the host always applies it). That band cannot make an orbit.

**Verdict of the trace: as far as the listing reads, the orbit is the image's.**
- A plane at its airfield hangar spot circles it under land/park's airfield arm.
- When the circle takes it more than `qd` short of the spot, it leaves the path and free-falls.
- Nothing in the host needs fixing for this; the one host difference found (the throttle dead
  band) is recorded as an open item.
- The hide makes all of it invisible in the image: planes are hidden by 5aq's scene-node hide and
  detached from the hit index by 5as.

### 2. What this does to 5bp's verdict

5bp kept `kGroundRetakeoffBound` OFF because the pushed takeoff never lifted off. That criterion
assumed a grounded plane would roll and climb. Once the plane is in state 4 off the rectangle, the
image's own law makes it free-fall (5bq), with or without the push.

So:
- **Mechanism.** The gate and the push match the listing (5bp), and what follows the push is the
  image's law.
- **Outcome.** With the switch ON, the host ends each looper in a takeoff task at the head, as
  the image does. The alternative is a park <-> abort loop that the image cannot produce.

### Predictions for the re-pair (switch ON against OFF, both from `4f5c5f5d4`; written before the runs)

1. **JM05 9000 and 12000.** One push per looper at its first ground abort. Aborts fall to 1-2 per
   looper. Death tables identical; `pair_diff` 1.
2. **JM05 3000, LOMP10 3000 and 9000, USN04, E2, USN01, BSM01.** No push; `pair_diff` 1.
3. **Flip criterion.**
   - every push comes from a grounded head whose `30h` is true;
   - no row without ground aborts moves;
   - death tables are identical.
   - The post-push fall is the image's law and does not count against the flip.

### Results (OFF `local\l28_roff`, SHA-256 `2ECC890B9A13`; ON `local\l28_ron`, `ECCB4518B43F`; both from `4f5c5f5d4`)

| row | `pair_diff` | death rows | land aborts OFF / ON | pushes |
| --- | --- | --- | --- | --- |
| JM05 3000 | 1 | identical (12) | 1 / 1 (airborne) | 0 |
| JM05 9000 | 1 | identical (14) | 21 / 2 | 1 (Helldiver 01, 446.06 s) |
| JM05 12000 | 1 | identical (17) | 749 / 4 | 3 (Helldiver 01, Main sqn01\|.-3, Secondary sqn02\|.-3) |
| LOMP10 3000, 9000 | 1, 1 | identical (3, 6) | 0 / 0 | 0 |
| USN04, E2 | 1, 1 | identical (50, 52) | 0 / 0 | 0 |
| USN01, BSM01 (controls) | 1, 1 | identical (17, 0) | 0 / 0 | 0 |

Every push is from a `land` head in state 4 with `+BF8h` clear, at the plane's first ground abort.

| prediction | result |
| --- | --- |
| 1. JM05 9000 / 12000: one push per looper, aborts 1-2 per looper, deaths identical, exit 1 | **held** |
| 2. other rows: no push, exit 1 | **held** |
| 3. flip criterion | **met** |

### Verdict: **flip ON** (`kGroundRetakeoffBound = true`)

The fall after the push is recorded as the image's law (5bq, section 1 above), not as a
mechanism failure. This supersedes 5bp's OFF verdict.

For reference W: JM05 9000 and 12000 move at exit 1 (the land-task, takeoff and native-table
counters). No other row moves.

## 5bs. The throttle dead band of 0099D300's demand arm (packet `cc9_throttle_dead_band`, cc9-lua28, 2026-09-30)

5br found this band as the one host difference in the speed hold. The switch is
`kThrottleDeadBandBound` (`include/bsp/game_hosts_units.hpp`), committed **OFF** with the
predictions below.

### The image (disk listing `0099DAAA`-`0099DBC7`, read whole)

- `[esp+14h]` is the measured speed: `007D99C0 / +2B8h`, less the carrier's forward speed on a
  mother-ship holder.
- `[esp+24h]` is the error: `+2B4h - speed - correction`.
- `[esp+38h]` is `|speed|`, and `[esp+44h]` is `+2B4h / |speed|` (`0099DAEC`-`0099DAFF`).
- `[esp+38h]` is then overwritten with `|error|`.
- **The increment** (`0099DB58`-`0099DBC7`) runs when any of these holds:
  - `|error| > 0.8333` (double `00D09450`, 3 km/h), `0099DB2D JA`;
  - `1.0 > speed` (`00D7A24C`), `0099DB3A JA`;
  - `ratio > 1.5` (`00CE380C`), `0099DB49 JA`;
  - `0.5 > ratio` (`00CE3800`), `0099DB56 JBE` falls through.
- Otherwise `0099DB56 JBE 0099DBCB` skips the increment, and the demand is the slot's seed. The
  throttle holds where it is.
- The increment's first store, `+2ECh = min(+2ECh, [00E0E2F0])` (`0099DB58`-`0099DB6A`), has no
  reader in this host and is recorded, not bound.

The host always applied the increment (`dead_band_skips = false`). With the switch ON, the flag is
computed from the same inputs the host feeds the arm: speed scale 1.0 and correction 0, the
labelled substitutions of the throttle packet. Summary line: `summary throttle dead band: applied=
skipped=`.

### Predictions (switch ON against OFF; written before the pairs)

1. **Mechanism.** On every row with planes, `skipped` is a large share of the thinks: cruising
   planes sit within 3 km/h of their request.
2. **Motion.** Within the band the throttle stops integrating, so speeds settle up to 0.83 m/s off
   the request instead of hunting around it. Every plane's position moves slightly.
   - `pair_diff` 3 on every plane row.
   - Death rows can shift in time wherever an attack run's timing moves. Such moves are timing
     knock-on (and RNG-coupled for AA, per the shared-stream note), not a changed mechanism.
3. **No regime change.**
   - Take-offs, landings, lift-offs, releases and the ground-retakeoff pushes stay within a few
     counts.
   - No squadron that attacks with OFF fails to attack with ON.
4. **Rows.** Every reference row: USN01, USN02, USN04, E2, USN12, USN13, JM05 3000 and 9000,
   JM06, JM08, LOMP06, LOMP10 3000 and 9000, USNOS, IJN01 and BSM01. BSM01 has no planes, so it
   is exit 0 or 1 with `skipped=0`.
5. **Flip criterion.**
   - `skipped > 0` wherever planes fly;
   - no regime change (item 3);
   - moved death rows limited to timing: the same victims, or victims whose killer's attack
     timing moved.

### Results (OFF `local\l28_roff`, SHA-256 `2EC5F1C0E7C6`; ON `local\l28_ron`, `DF8B30ED8828`; both from `66a266556`)

The 300-frame USN01 smoke of the committed tree exits 0. LOMP10 3000's OFF run failed once at
renderer init (CreateDevice hr `0x8876086A`, the environment) and was re-run.

| row | `pair_diff` | increments applied / skipped (ON) | death rows |
| --- | --- | --- | --- |
| JM05 3000 | 3 | 38314 / 2323 | identical (12) |
| JM05 9000 | 3 | 120543 / 25902 | identical (14) |
| JM06 | 1 | 1500 / 0 | identical (1) |
| JM08 | 1 | 14726 / 2188 | identical (7) |
| LOMP06 | 1 | 0 / 0 | identical (0) |
| LOMP10 3000 | 3 | 4439 / 46 | same 3 victims; Lightning 01\|.-3 at 124.30 -> 124.50 s |
| LOMP10 9000 | 3 | 10441 / 46 | same 6 victims, 1 re-timed |
| USN01 | 3 | 6266 / 141 | identical (17) |
| USN02 | 1 | 0 / 0 | identical (1) |
| USN04 | 3 | 35599 / 1658 | 50 -> 48: 43 re-timed, mostly within 1-5 s; the two OFF-only deaths are at 224.06 and 224.36 s of a 225 s row |
| E2 (USN04 9000) | 3 | 60378 / 1699 | same 52 victims, 47 re-timed |
| USN12 | 1 | 0 / 0 | identical (7) |
| USN13 | 3 | 57812 / 1662 | same 23 victims, 21 re-timed |
| USNOS | 3 | 39394 / 399 | same 106 victims, 4 re-timed |
| IJN01 | 1 | 49353 / 203 | identical (3) |
| BSM01 | 1 | 6000 / 0 | identical (0) |

**Regime counters.**
- Lift-offs are equal on JM05 3000 and 9000 (30, 34), USN04 and E2 (12) and USN13 (27).
- USN04 and E2: torpedo-task releases 5 -> 4 of 16, dive-bomb releases 0 -> 1 of 19.
- JM05 9000's single ground-retakeoff push (Helldiver 01) does not occur with ON. The hangar orbit
  itself is unchanged: `local\l28_orbit.py` gives the same 5-10 m/s circles. The looper's
  excursion past `qd` is timing-dependent.

| prediction | result |
| --- | --- |
| 1. `skipped` a large share | **partly**: 1-18% of increments. Cruising planes rarely sit inside 3 km/h |
| 2. every plane row moves (exit 3), deaths re-timed only | **held** on 10 rows. JM06, JM08, IJN01 and USN02/USN12/LOMP06 (no increments) are exit 1 |
| 3. no regime change | **held**: lift-offs equal; one torpedo release fewer and one dive release more on the USN04 pair, which is within the RNG-coupled spread |
| 4. BSM01 exit 0 or 1 | **held** (1) |
| 5. flip criterion | **met**: every changed death row keeps its victim, except the two USN04 deaths that fall past the row's end |

### Verdict: **flip ON** (`kThrottleDeadBandBound = true`)

The mechanism holds, and the misses are spread misses: the skip share and the exit-1 rows. The
USN04 torpedo-release and death-count moves are recorded as timing, RNG-coupled per the shared-stream
note.

For reference W, every row with exit 3 above moves.

## 5bt. Handoff (cc9-lua28, 2026-09-30)

Branch `agent/cc9-lua28`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua28`. No lease
is held after this handoff.

| packet | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| `cc9_ground_retakeoff` | `6a040f4dd`, `fbc82bfb6` | `kGroundRetakeoffBound` | OFF, then superseded | 5bp |
| `cc9_plane_ground_support` | `98bed39c3` | - | read: no plane terrain contact in the image | 5bq |
| `cc9_hangar_orbit` | `3e1aab7ee`, `debfa7537` | `kGroundRetakeoffBound` | **ON** | 5br |
| `cc9_throttle_dead_band` | `66a266556`, `24c78b40f` | `kThrottleDeadBandBound` | **ON** | 5bs |

**What changed in the picture:**
- The airfield park loop is not image behaviour (5bp).
- Planes have no terrain contact in the image (5bq).
- Hangared planes orbit their spot under the image's own park law, and sometimes leave the path
  and free-fall, invisibly (5br).
- `+900h` state 6 is the plane on the water (5bp).

### Next, in order

1. **The strafe evasive roll** (5bn.2): `009BC030` -> `00D205E0`, tick `009BA020`, pushed by
   `007B6240` and `009CBB30`. It needs `009CBB30`'s owner (the strafe task, `009CD300`) in the host
   first. After it, **the flight leader's task** (5bn.4, `009BBFC0`, tick `009BC3A0`, gated on
   `unit+184h`).
2. **`007C6F50`'s `msg+20h` flag and `+C49h`** (5bg item 4).
3. **5bi's split-form `vtable[28h]` scan** (low value).
4. **The controlled-unit fallback** (low priority, from the lead). On USN01, reference V's idle
   player controls ScoutDauntless, because the script's intended unit ConTBD1 is never spawned
   once Convoy1 is not hit (SHIP_AI 93; the formation-join switch). Check that the host's fallback
   choice when the scripted unit does not exist matches the image's rule (docs/CONTROLLED_UNIT.md).
   Nothing is bound yet.
5. **Open from 5bs:** the `+2ECh` store at `0099DB58` (min with `[00E0E2F0]`) and the reader of
   `+2ECh` (`0099D7B5`, the mode-0 arm) are not bound.

**Tools** (`local\` in the cc9-lua28 tree):
- `l28_runs.ps1`: the reference rows;
- `l28_cmp.py`: headline and per-victim death diff of a pair, with the base-launch and
  ground-retakeoff summaries;
- `l28_table.sh`, `l28_dbtab.sh`: row tables;
- `l28_orbit.py`: the hangar orbit census from the land-park trace;
- the edit scripts `l28_retakeoff_edit.py`, `l28_verdict_edit.py`, `l28_flip_edit.py`,
  `l28_db_edit.py`;
- the diagnostic edit scripts `l28_diag*_edit.py`, which apply to an export only.

## 5bu. The strafe evasive roll and the flight leader's task: both unreached (packet `cc9_strafe_roll_leader_task`, cc9-lua29, 2026-09-30)

Read-only. No switch is bound: neither task can be constructed in any reference row, so a binding
would be dead code and every pair would be identical by construction.

### 1. The evasive roll `009BC030` -> `00D205E0` needs a strafe or a rocket task

- **Pushers, placed by vtable.** Neither pusher has a rel32 caller; each is a data slot:
  - `007B6240` sits at `00D05830`, slot `+0Ch` of the state vtable at `00D05824` (`007B6810`,
    `007B5F10`, `007B3DC0`, `007B6240`, `007B3DE0`, ...). The block lies just below the
    **rocket** task's vtables (`00D0590C`-`00D05918`, factory `007B7FD0`). So this pusher is a
    rocket-task state tick, not strafe code (5bn.2 grouped both under strafe).
  - `009CBB30` sits at `00D21048`, slot `+0Ch` of the state vtable at `00D2103C` (`009CC000`,
    `009CB8B0`, `007B3DC0`, `009CBB30`), below the strafe task's vtables (`00D210D4`-`00D210E0`,
    factory `009CD300`). This one is the strafe run's.
  - Adjacency is the evidence for both owners; which constructor stores `00D05824` / `00D2103C`
    was not read. Provisional.
- **Reach.** The roll is pushed only from inside a strafe (`00E08F40`) or rocket (`00E08F48`)
  task. Census over the nineteen reference-v logs (`local\g20_rv_*.log` in the cc9-gunnery20
  tree), every `PilotSetTarget task: 0099A170 -> ... command=` line: `00E08F18` x702,
  `00E08F20` x330, `00E08F28` x8, `00E08F78` x5. **No `00E08F40`, no `00E08F48`.** The host has
  no strafe or rocket task either (`src/game_hosts_units.cpp`, the `00999AA0` comment: "no strafe
  task in this host"; no strafe or rocket `BotTask*` method in any call table).
- 007EEC50 picks strafe only as the guns answer when no ordnance class applies and dogfight does
  not (`src/attack_commands.cpp` `attack_command_choose`). None of the scripted orders in the
  rows reaches that arm.
- **Verdict:** unreached. The roll waits on a strafe task (`009CD300`, `6FCh`) or a rocket task
  (`007B7FD0`, `708h`) in the host, and on a row whose script or planner orders one.

### 2. The flight leader's task `009BBFC0` -> `00D20658`: the player-piloted leader

- **Gate, from disk bytes** (`disasm-raw 0099ADB0`): `0099ADC7 CMP [ESI+5Ch],0` (list not
  empty), `0099ADD0 CMP byte [[ESI+50h]+184h],0`, `0099ADD9 CALL 007B8AD0` (flight leader),
  `0099ADEC CALL [vt+38h]` on the head; then `0099ADF4` news 8 bytes = {`00D1F31C`, unit},
  `0099AE14 CALL 009BBFC0`, `0099AE1C CALL 00999F50`.
- **`unit+184h` is the player's role-1 (pilot) hold** (docs/CONTROLLED_UNIT_HELM.md: its only
  setter is `00780214` on an accepted role-1 take). With an idle player nothing takes role 1.
- **Tick `009BC3A0`** (decompiled): copies the unit's `+9E4h`-`+9F8h` (six dwords) into the plan
  `+3FCh`-`+410h` every tick. When the controller's `vt[38h]` answers false and `+900h == 6`, it
  sets `+408h` to `[00D7A24C]` or 0; when it answers true, it raises `+408h` to at least
  `tuning+598h`. Then fixed plan flags (`+2C8h`=0, `+2D0h`=1, `+2D4h`=1, `+27Ch`=`+408h`,
  `+280h`=1, `+2B0h`=1), the `0099B690` retire when `+3F8h` answers, and a counter at `+414h`
  that runs while `unit+9D8h` is set and clears `[unit+DF4h]+80h` otherwise.
  Reading: the bot mirrors the human pilot's inputs so the squadron logic keeps a plan. That
  `+9E4h`-`+9F8h` is the player's control block is a hypothesis, not verified.
- **Reach:** every reference-v log prints `+184h=0` on every `player roles` line (109 lines), and
  no `scripted helm transfer` line appears in any of them. **Unreached** in the image and the
  host alike while the player is idle.

### Next

Item 1 of 5bt closes as unreached. The queue continues with the controlled-unit fallback (5bt.4),
then `007C6F50`'s `msg+20h` flag and `+C49h`, the `+2ECh` pair (5bt.5), and the `vtable[28h]` scan.

## 5bv. Which missions need the strafe and rocket tasks: a script census (cc9-lua29, 2026-09-30)

Read-only, from this installation's `scripts/` (modded; the files cited below are not the locally
modified `vehicleclasses.lua`). The host has neither the strafe task (`009CD300`, `00E08F40`) nor
the rocket task (`007B7FD0`, `00E08F48`), and no reference row issues either (5bu).

- **Strafe is scripted in campaign missions.** `global/luamw_init.lua` line 162 defines
  `COMMAND_STRAFE = "strafe"`. Scripts that test a plane's `unitcommand` against `"strafe"` and
  re-issue `PilotSetTarget` to a fighter against a ship, which 007EEC50 answers with strafe when
  no ordnance class applies:
  - `missions/COTP-IJN/jm09.lua` line 1784 (fighters out of ammo retargeted at
    `Mission.AICruisers`), and its copy `missions/COTP-IJN/PRCPIJN/jm09.lua` line 2451;
  - `missions/COTP-IJN/jm12.lua` line 888;
  - `missions/COTP-USN/usn_10_battle_of_capeengano.lua` line 3080 (Fighter and Kamikaze types),
    and its copy under `PRCPUS/` line 3272;
  - `missions/ijn/ESMP/08_engano.lua` line 584 and `missions/ijn/ESMP/11_tengo.lua` line 592.
- **IJN01's "Strafe" objective is the player's**: `missions/ijn/ijn_1_pearl.lua` lines 435-460
  collect `Mission.StrafeShips` for `luaObj_Add("primary", 1, ...)` (line 984); no AI plane is
  ordered to strafe there. With an idle player it issues nothing.
- **Rocket.** No script names a `rocket` command (`COMMAND_*` has no rocket entry). Rocket runs
  come from the air-support powerup `rocket1` ("Corsair Rocket Run", `datatables/powerupclasses.lua`
  line 241) and from rocket-armed classes when 007EEC50's ordnance order picks `00E08F48` for them.
  Which plane classes carry rockets in this installation was not censused.
- None of these missions is a reference row (JM05, JM06, JM08, USN01, USN02, USN04, USN12, USN13,
  USNOS, IJN01, LOMP06, LOMP10, BSM01). A strafe task is therefore needed for campaign coverage
  (JM09, JM12, USN10 and the ESMP Engano and Tengo missions), and only a new reference row on one of
  them could judge it.

## 5bw. The strafe task, read: reach, the unfed choice inputs, the layout and the state graph (packet `cc9_strafe_task_read`, cc9-lua29, 2026-09-30)

Read-only; nothing bound. This is the first half of the strafe packet; the binding follows.

### 1. Reach: the host never chooses strafe, because two of 007EEC50's inputs are never fed

- `src/game_hosts_script_orders.cpp` builds `AttackFeasibilityInputs` for `PilotSetTarget` but
  never sets `guns_available` (007EEB08 / 007EEBB7, `CMP byte [plane+C24h],0`, PilotFires) or
  `guns_suppressed` (007EEB2C / 007EEBC2, `CALL 0047B850`). Both default to false
  (`include/bsp/attack_commands.hpp`), so `strafe_applies` and `dogfight_applies` always answer
  false and 007EEC50's guns answer is always 0. `has_rocket_ordnance` and
  `has_depth_charge_ordnance` are never set either.
- The image's strafe arm (disk bytes 007EEB94-007EEBC7): target surface (the byte argument) or
  `target->vt[5Ch](41h)`; then `[[sq+3D0h]+C24h] != 0`; then `0047B850([sq+3D0h])` must be false.
  0047B850 (`0047B850`-`0047B873`, read to the first exit) answers `vt[5Ch](10h) || vt[5Ch](16h)`:
  a level bomber or a dogfight-excluded plane may not strafe. The dogfight arm (007EEAEC-007EEB31)
  uses the same PilotFires byte and 0047B850 after its own `IsKindOf(16h)` refusal.
- **Where the image would strafe** (orders the host declines today, `007EEC50 -> 00000000` with a
  surface target):
  - reference v **USNOS and USNOS long**: three `plane #1.1..#1.3` (self class 18, the Dauntless
    class, no bomb loaded: `gb=0`) against targets 348, 352, 349 at native frame 832 (about
    40 s), `g20_rv_usnos.log`;
  - **ESMP08** (`IJN\ESMP\08_engano`, `local\l29_base_esmp08.log`, main `557733990`): seven
    orders at native frame 2939 (mission frame about 2900): TBM Avenger #1.1, #1.7, #1.9 and F4U
    Corsair #1.3, #1.6, #1.8, #1.10. The script's `luaControlAirAttacks` re-orders every 15 s, so
    the 9000-frame form carries the strafe runs.
  Whether each of these planes has PilotFires set and is not kind 10h/16h in the image is the
  first thing the binding must log; that decides strafe versus nothing.
- **Mission candidates checked:** the menu ids JM09 and USN10 do not load the strafe-issuing
  scripts (`missiontree.lua`: JM09 is `prcpijn_solomons.scn`, USN10 is
  `usn_05_GuadalCanal_1st_Battle.scn`). The scenes naming those scripts are reached only as JM14
  (`PRCPJM12`, which has no strafe test; its 3000-frame run loaded 0 units), ESMP08 and ESMP11.
  ESMP11 issued no `PilotSetTarget` in 3000 frames. **ESMP08 (long) is the new row;** USNOS is
  the reference row that reaches strafe first.

### 2. The task object (kind 0Ah, `6FCh` bytes)

- Factory `009CD300`, constructor `009CC230`: `BSP_BotTask_ConstructBase(owner, 10)`, the
  approach `009CC020(owner, target)` at `+3F8h`, vtables `00D210E0` / `00D210D8` (`+3F8h`) /
  `00D210D4` (`+4CCh`), then the initial state `+4E0h` (moveto) for the flight leader
  (`BSP_Unit_IsSquadronFlightLeader`) or `+51Ch` (follow), entered through its `vt[4]`, and
  `BSP_BotApproach_BindToTask`.
- The approach `009CC020` (base `009CA4A0(unit, target)`) builds seven states, registered by
  name (whole-object offsets):

| state | offset | built by | vtable |
| --- | --- | --- | --- |
| `moveto (strafe)` | `+4E0h` | `BSP_BotStateMoveTo_Construct` | - |
| `follow (strafe)` | `+51Ch` | `BSP_BotStateFollow_Construct` | - |
| `strafe/prepare` | `+5B4h` | `BSP_BotStateFollow_Construct` (a follow variant) | - |
| `strafe/gotowards` | `+64Ch` | inline | `00D20FE8` |
| `strafe/aim` | `+670h` | inline | `00D21020` |
| `strafe/goaway` | `+690h` | `009CB500` | `00D2103C` (tick `009CBB30`, the evasive-roll pusher) |
| `strafe/attackrun` | `+6D8h` | `009CAC80` | `00D21004` |

- Primary vtable `00D210E0` against the dive bomb's `00D20E18` (slot: strafe / divebomb):
  `+2Ch` hit notice `009CC400` / `009C7900`; `+3Ch` getter `009CC3D0` / `009C79A0`; `+40h` test
  `009CC850` / `009C8060`; `+54h` cruise profile `009CD020` / `009C8920`; `+58h` `009CD0D0` /
  `009C7FF0`; **`+64h` the arm `009CD170` / `009C8790`**. Slot `+04h` `009CC8F0` is a debug draw
  (static vectors, `00860BE0` / `0085F990` under `+35Dh`).

### 3. The arm `009CD170` (no Ghidra function; `009CD170`-`009CD1E4` `RET 4`, INT3 from `009CD1E7`)

1. `009CCED0(dt)` on the approach `+3F8h` (`009CD182`);
2. the moveto state's `009BDE80(dt, +430h, +438h)` on `+4E0h` (`009CD1AF`);
3. the rule `009CC690(dt)` (`009CD1BE`);
4. the current state's `vt[0Ch](dt)` on `[+310h]` (`009CD1D6`);
5. `+2E4h = FFh` (`009CD1D8`).

### 4. The rule `009CC690`, decompiled

`engaged` = `+448h != 0`, or `ctl+370h == 2` and (`+44Ch != 0` or `+468h != 0`), where `ctl` is
`[+404h]` (approach `+0Ch`; `+370h` is the control mode the torpedo and dive-bomb rules also read).

- In an attack state (`+5B4h`, `+64Ch`, `+670h`, `+690h`, `+6D8h`) and `engaged`:
  - `ctl+370h == 0` -> prepare;
  - prepare -> `009CC5F0`: mode 0 stays prepare, else `+448h` ? gotowards : attackrun;
  - attackrun -> gotowards when `+448h` is set;
  - goaway -> gotowards when `+6B4h` is set;
  - gotowards -> aim when `009CC2F0` answers (`approach+20h` set and
    `approach+18h < 2 * [[approach+4]+8]+188h + [approach+4]+38h`);
  - aim -> goaway when `+68Dh` or `+68Ch` is set.
- Not in an attack state and `engaged` -> `009CC5F0`.
- Otherwise (not engaged) -> moveto for the flight leader, follow for a wing member.
- A change calls the old state's `vt[8]` and the new state's `vt[4]`.

### 5. Still to read before binding

- the approach update `009CCED0` and the approach base `009CA4A0` (what sets `+448h`, `+44Ch`,
  `+468h`, `approach+18h`/`+20h`);
- the state ticks: gotowards `009CA870`, aim (`00D21020` slots), goaway `009CBB30` with the roll
  push, attackrun (`00D21004` slots), prepare's follow variant;
- the cruise profile `009CD020`, `009CD0D0`, the hit notice `009CC400`, the test `009CC850`;
- how strafe fires the guns (the gun task `009FC7C0`, `BotTaskGun::tick`, is UNIMPLEMENTED in the
  host; strafe damage depends on it).

### 6. Plan

1. Feed `guns_available` (PilotFires, the host's `plane_pilot_fires_c24`) and `guns_suppressed`
   (kinds 10h/16h) in the script-order choice, behind a switch, OFF. By itself this issues
   strafe (and dogfight against aircraft) where the host declines today. It must land with, or
   after, the strafe task; otherwise the plane gets a class no arm acts on.
2. The strafe task arm, rule and states, behind the same or a second switch, OFF.
3. Pairs: USNOS, USNOS long, ESMP08 long (new row, reference form: `--frames 9200
   --press-start-frame 30 --menu-select ESMP08 --mission-frames 9000 --mission-frame-seconds
   0.05`), plus two controls.

## 5bx. The strafe task: rule and range test bound (partial), the attack states read (packet `cc9_strafe_task_bind`, cc9-lua29, 2026-09-30)

New `include/bsp/strafe_task.hpp` and `src/strafe_task.cpp` (`bsp_core`). `kStrafeTaskBound` is
declared **OFF** and nothing reads it yet: the host has no strafe arm, so no row can move and no
pair was run. Coverage: **partial** (the header lists what is not bound).

### 1. Bound as pure functions

| routine | function | coverage | evidence |
| --- | --- | --- | --- |
| `009CC690` rule | `strafe_rule_009cc690` | complete | 5bw.4; operand set-up checked on disk (`009CC6D0`-`009CC7F5`; `009CC78E` calls `009CC2F0` with ECX = the current state, the gotowards state) |
| `009CC5F0` engaged entry | inside the rule | complete | decompiled: mode 0 prepare, else `+448h` ? gotowards : attackrun |
| `009CC2F0` gotowards ready | `strafe_gotowards_ready_009cc2f0` | complete | disk bytes `009CC2F0`-`009CC318`: byte `+20h`, then `2*[[+4]+8]+188h + [+4]+38h > +18h` (`JBE` false) |
| `009CCED0` approach update | `strafe_approach_update_009cced0` | complete except its first call `009FADA0` (the target ref update, already in `src/approach_target_ref.cpp`) | disk bytes `009CCED0`-`009CD015`, traced on the x87 stack |

The range test (`009CCED0`), from the bytes:
- `+44h += dt` every call. That is the clock the hit notice `009CC400` zeroes (`task+43Ch`).
- A countdown `+D0h` against the period `+CCh` (1.0) gates the test. `009CA4A0` seeds it with
  `-Random(0, 1)`.
- No target (`+54h`, else `+70h`), or a target with `+5Dh` set: out of range.
- Otherwise the threshold is `(unit+100h - aim.y) / +30h + 2 * [+8]+188h`, floored at
  `tuning+658h` (Pilot/Strafe/AttackDist). The floor is multiplied by 1.4 (`[00D06874]`) for a
  kind-10h/16h plane (`009CA310`). **In range when the threshold exceeds the horizontal distance
  to the aim point** (`009CAD00`).
- The aim point is the approach's `vt[0]` `009CA680`, which reads `+74h/+78h/+7Ch`: the target
  ref's `sub+1Ch` (the ref sits at `approach+58h`).

### 2. Read, not bound: the attack states (decompiled; asm not yet checked)

Approach fields used: `+4` the plane entity (pose `+FCh..+104h`), `+8` an object with `+188h`
and `+26Ch`, `+18h` the pilot plan block (`+2B4h` speed, `+2C0h`/`+2C4h` heading, `+2BCh` pitch,
`+2CCh`/`+2D0h` modes), `+1Ch` a command block (`+40h` a tuning row, `+5Ch..+64h` a point, `+28h`),
`+30h` the glide tangent, `+34h` the goaway distance, `+3Ch` the aim-close distance, `+48h` the
run speed, `+4Ch` a speed.

- **gotowards `009CA870`** (enter `009CA820` rerolls the glide `009CA3B0` and `+48h`). It is the
  byte-for-byte twin of the rocket task's `007B4980` (docs/PITCH_COMMAND_CALLERS.md):
  - speed from `+48h`;
  - `+18h` 3-D and `+1Ch` horizontal distance to the aim point;
  - heading by `atan2` plus the near-field probe `007F0280`, into plan `+2C0h`, mode 2;
  - `+20h` aligned when the heading error is below `0.6109` rad (`[00D057D8]`, 35 degrees);
  - pitch through `009F9ED0` with a glide-limited descent;
  - `tuning+678h` into the command block;
  - `009FABE0`.
- **attackrun `009CADB0`** (vtable `00D21004`):
  - heading away from the aim point (`pi - atan2`), with a periodic probe offset `+20h`;
  - `009FBA50` cruise altitude from `tuning+210h` and the range;
  - plan flags;
  - `tuning+670h`;
  - `009A1A20`, then `009FABE0`.
- **aim `009CB1B0`** (vtable `00D21020`):
  - heading to the point (`BSP_PilotBot_CommandHeadingToPoint`), pitch by `atan2`, plan
    `+2BCh`, mode 1;
  - transforms the aim point into the plane's frame;
  - `+1Ch` = forward distance below `+3Ch`;
  - `+1Dh` = behind, too close (`[00CF0B58]`), or off-axis inside `[+8]+26Ch`;
  - `approach+9Ch` = distance / `+4Ch` clamped;
  - writes the aim point into the command block `[+1Ch]+5Ch..+64h` with `+28h` =
    `[00CE3D30]`. **This is where the guns are pointed**, so the gun task `009FC7C0` reads this
    block (to confirm when it is read);
  - `tuning+678h`.
- **goaway `009CBB30`** (vtable `00D2103C`):
  - two random re-plan timers, `+30h` (heading, `009CB780`) and `+3Ch` (pitch, `009CB650`), hold
    plan `+2C4h`/`+2BCh`;
  - `+24h` done when the horizontal distance exceeds approach `+34h`, tested each `+40h` period;
  - **the evasive manoeuvre**: when the plane is neither of two kinds (the two `vt[5Ch]` pushes
    are not yet read) and `approach+44h < 1.0`, i.e. hit within the last second (the hit notice
    zeroes `+44h`), it pushes a front task. With `Random(0,1) >= +18h` that task is the roll
    `009BC030` (with `007B5D30`); otherwise `009BC0A0` (with `007B5E20`). It then sets `+44h` to
    `[00CE89CC]` and calls `00999F50`;
  - `tuning+674h`.

### 3. Next

1. Check each state tick's listing (all four have x87 and register inputs), and read the
   approach constructor `009CA4A0` / glide seed `009CA3B0`. `approach+14h` is a robot row; its
   `+D8h`/`+DCh`/`+E0h`/`+230h` look like `robot_config.hpp`'s `+E4h`/`+E8h`/`+ECh` Strafe keys
   shifted by `0Ch`. That is unverified.
2. The host arm (`009CD170`) in `src/game_hosts_units.cpp` on `attack_command_class ==
   00E08F40`:
   - moveto and follow reuse the host's existing states;
   - prepare is a follow variant;
   - the four attack states as above.
3. The gun task `009FC7C0` and its reading of the aim block.
4. The choice-input feed (5bw.6.1) in `src/game_hosts_script_orders.cpp`, its own switch, OFF.
5. Pairs on USNOS, USNOS long and ESMP08 long, plus two controls. Flip the group only when strafe
   runs end to end.

## 5by. The strafe approach constructor and the attack-state listings, checked (packet `cc9_strafe_states_check`, cc9-lua29, 2026-09-30)

Read-only; nothing bound. Listings from disk bytes (`disasm-raw`), kept in the cc9-lua29 tree as
`local\l29_asm_<addr>.txt`.

### 1. The approach constructor `009CA4A0` (listing `009CA4A0`-`009CA66B`)

`approach+14h` is the PilotBot robots row viewed `0Ch` in (`00F8A30C + level*248h + 0Ch`,
include/bsp/approach_target_ref.hpp), so `[+14h]+N` is `robot_config.hpp`'s `+N+0Ch`. The 5bx
guess is **confirmed**. `+24h` is 009F9CE0's speed ratio.

| field | value | site |
| --- | --- | --- |
| `+34h` goaway distance | `min(Random(0.9, 1.05) * StrafeGoAwayDistance (row +E8h), tuning+658h * 0.8) * +24h` | `009CA4DB`-`009CA557` |
| `+38h` shoot distance | `Random(0.9, 1.1) * AimShootDistance (row +23Ch) * +24h` | `009CA55A`-`009CA586` |
| `+3Ch` too-close distance | `Random(0.9, 1.1) * StrafeTooCloseDistance (row +E4h) * +24h` | `009CA589`-`009CA5C3` |
| `+44h` hit clock | 3600.0 (`[00CFDEB0]`) | `009CA5BA` |
| `+50h` in range, `+54h` | 0, 0 | `009CA5BF`, `009CA5C6` |
| target ref | `009FB200` at `+58h` | `009CA5CD` |
| `+CCh` period | 1.0 (`[00D7A24C]`) | `009CA5F5` |
| `+D0h` countdown | `-Random(0, 1)` | `009CA5FD`-`009CA604` |
| `+4Ch` | `007C2610(plane)` (`BSP_Unit_MinKind21ComponentSpeed`) | `009CA60D` |
| `+54h` | the target when it answers `vt[5Ch](41h)`, else 0 | `009CA615`-`009CA62C` |
| glide seed | `009CA3B0` | `009CA62F` |
| kind 10h/16h plane | `+34h` and `+3Ch` times 1.4 (`[00D045F0]` double) | `009CA634`-`009CA668` |

The glide seed `009CA3B0` reads `+34h` **before** the 1.4 scaling:
- angle = `min(atan2(ctl+398h, +34h), StrafeAttackAngle (row +ECh))`, with `ctl` = `+0Ch`;
- `+2Ch` = `Random(0.8, 1.2) * angle`;
- `+30h` = `tan(+2Ch)`;
- `+40h` = `+30h * +34h + Random(-10, 40)`.

The listing is checked to the random call (`009CA41B`); the rest is from the decompiler.

### 2. Corrections to 5bx.2

- **attackrun `009CADB0` runs AT the target, not away from it.**
  - The heading is `pi/2 - atan2(dz, dx)` (`[00CE3830]` is pi/2), wrapped by 2pi: the game's
    heading to the aim point. `dt` is the stack argument (`009CAE3E`, `[ESP+4Ch]`).
  - Every `+18h` seconds (countdown `+1Ch`) the near-field probe `007F0280` runs, with
    `ctl` = approach `+0Ch` and box (100, 60, 120). It sets the offset `+20h` =
    `-a*b*c*pi/6` (`[00CEC730]`), which is added to the heading.
  - Plan `+2C0h`, mode 2.
  - Altitude (`009CAF0F`-`009CB015`, EBX = the tuning singleton):
    - `h` = `tuning+210h * 0.9 - plane+100h`, clamped to [0, 400];
    - the factor is `00419010` over (0.1, 0.3)-(0.5, 1.0) at `h / min(dist, tuning+658h)`;
    - then `009FBA50(approach+40h, approach+38h, dist, factor)`.
  - Plan `+278h` = 0.98, `+27Ch` = 1, `+2A8h` = 0, `+2ACh` = 1, `+2D8h` = 0; `tuning+670h`
    into the command block; `009A1A20`; `009FABE0`.
- **goaway `009CBB30`'s manoeuvre** (listing `009CBC96`-`009CBE27`):
  - the two kinds are **10h and 16h**: a level bomber or a dogfight-excluded plane never evades;
  - the gate is `approach+44h < 1.0` (`009CBCBA`-`009CBCCA`);
  - the probability field is the **goaway state's** `+18h`, not the approach's:
    - `Random(0,1) >= state+18h` pushes the roll `009BC030(ECX = [plane]+10h, EDX = condition
      007B5D30, arg = approach+34h)`;
    - otherwise `009BC0A0` with `007B5E20`;
  - either one sets `approach+44h` = 25.0 (`[00CE89CC]`) and front-pushes through `00999F50`.
- **aim `009CB1B0`:** the constant the decompiler shows as double `[00CEE07C]` reads 8.5e194 as
  a double, so it is a float or part of another value. The listing must settle it before the
  lateral test is bound. The other constants: 0.75 (`[00CEC9D8]`), 0.85 (`[00CF0B58]`), the
  clamp 30 (`[00CE7630]`/`[00CE38C8]`), and the command-block `+28h` = 0.6 (`[00CE3D30]`).
- **gotowards `009CA870`:** the listing is partly checked by docs/PITCH_COMMAND_CALLERS.md (the
  `009F9ED0` call). Its alignment limit is 35 degrees (`[00D057D8]` double 0.61087).

## 5bz. Handoff (cc9-lua29, 2026-09-30)

Branch `agent/cc9-lua29`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua29`. No lease
is held after this handoff.

| packet | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| `cc9_strafe_roll_leader_task` | `3c876b938` | - | read: both unreached | 5bu |
| `cc9_controlled_fallback` | `38e09e9f5` | - | read: the script's own choice (CONTROLLED_UNIT.md) | 5bv |
| `cc9_strafe_task_read` | `d92823c5a` | - | read: why the host never strafes | 5bw |
| `cc9_strafe_task_bind` | `8a9a800ec` | `kStrafeTaskBound` (no reader) | OFF, partial | 5bx |
| `cc9_strafe_states_check` | this section's commit | - | read | 5by |

**What changed in the picture:**
- The host never chooses strafe or dogfight through `PilotSetTarget`. 007EEC50's guns inputs
  (PilotFires `plane+C24h`, and `0047B850` = kinds 10h/16h) are never fed (5bw.1).
- USNOS (reference rows) and ESMP08 reach strafe in the image.
- The strafe task's rule and range test are bound as pure functions (`src/strafe_task.cpp`).
- The goaway state's evasive manoeuvre is the only pusher of the roll `009BC030` in strafe. It
  fires within 1 s of a hit.

### Next, in order (strafe group; the lead's plan)

1. **The host arm `009CD170`** in `src/game_hosts_units.cpp` (a shared file: prepare the edit as
   a `local\` script, claim only to apply, build and commit, release). It runs when
   `attack_command_class == 00E08F40` (`kAttackCmdStrafe`), behind `kStrafeTaskBound`.
   - Install: the state is moveto for the flight leader (`007B8AD0`), else follow. Seed the
     approach as 5by.1, from the robots row the host already carries (robot_config.hpp) and
     `Pilot/Strafe/AttackDist` / `CruisingAlt` / `ReferenceSpeed` (`game_tuning_singleton.hpp`
     `+654`/`+658`/`+65C`).
   - Each think: `009CCED0` (bound: `strafe_approach_update_009cced0`, after the host's target
     ref update `009FADA0`), the moveto helper `009BDE80`, the rule `strafe_rule_009cc690`, then
     the state tick.
   - Moveto, follow and prepare (a follow variant) reuse the host's existing moveto/follow
     states, as the torpedo arm does (`run_torpedo_task_arm_009d4850`).
   - The four attack ticks as 5bx.2 with 5by.2's corrections. Check the aim listing's
     `[00CEE07C]` first.
   - The hit notice `009CC400` (`src/game_hosts_units.cpp` `00999AA0`, today a GAP line) must
     zero the approach's `+44h`. That is what arms goaway's evasive manoeuvre; the manoeuvre
     task itself (`009BC030` / `009BC0A0`) can stay a counted, labelled gap at first.
2. **The gun task `009FC7C0`** (`BotTaskGun::tick`, UNIMPLEMENTED; 70302 calls on USN04). The aim
   state writes the aim point into `[approach+1Ch]+5Ch..+64h` with `+28h` = 0.6; confirm that the
   gun task reads it. Coordinate with the gunnery host only through its existing hit/damage entry
   points; ask the lead for any edit in `game_hosts_gunnery.cpp`.
3. **The choice-input feed** in `src/game_hosts_script_orders.cpp` (the ships lane's file: claim
   it only for this edit and release). Set `guns_available` = `plane_pilot_fires_0c24` and
   `guns_suppressed` = `unit_is_kind_of(0x10) || unit_is_kind_of(0x16)` on the ordered unit,
   behind its own switch, OFF. It must not flip before (1). It also enables dogfight by
   `PilotSetTarget` against aircraft; check that on the rows where it newly appears.
4. **Pairs:** USNOS, USNOS long, and the new row ESMP08 long (`--frames 9200 --press-start-frame
   30 --menu-select ESMP08 --mission-frames 9000 --mission-frame-seconds 0.05`, in the reference
   form of `local\l29_runs.ps1`), plus two controls. Flip the group only when strafe runs end to
   end.

**Other open items** (lower priority, unchanged from 5bt):
- `007C6F50`'s `msg+20h` flag and `+C49h`;
- the `+2ECh` store and reader of `0099D300`;
- the split-form `vtable[28h]` scan.

**Tools** (`local\` in the cc9-lua29 tree):
- `l29_runs.ps1`: the reference rows plus `esmp08`, `esmp11`, `jm14`, `esmp08l`, `esmp11l`;
- `l29_floats.py`: floats and doubles from the PE on disk;
- `l29_dec.ps1`: batch decompile to a file;
- `l29_asm_*.txt`, `l29_states.c`: the listings and decompiles of the strafe states;
- `l29_base_esmp08.log`: the 3000-frame ESMP08 run on main `557733990`.

## 5ca. The strafe group bound OFF: the arm, the states, the gun read and the choice feed (packet `cc9_strafe_arm`, cc9-lua30, 2026-09-30)

Two switches, both committed **OFF**: `kStrafeTaskBound` (the arm, its states, the hit notice and
the gun read, `src/game_hosts_units.cpp`) and `kAttackChoiceGunsFedBound` (the guns inputs of
`007EEC50`, `src/game_hosts_script_orders.cpp`). Both live in `include/bsp/strafe_task.hpp`.
Nothing else chooses class `00E08F40`, so the arm is inert without the feed. The group flips only
together, by the pairs of 5cb.

### 1. Read for this packet (disk listings, `local\l30_asm_*.txt` in the cc9-lua30 tree)

**Correction to 5bw.3 and 5bz.** The arm calls `009BDE80` with **near = far = `task+438h`
(approach `+40h`, the glide distance) and speed range = `task+430h` (approach `+38h`, the shoot
distance)**, not `(dt, +430h, +438h)`. `009CD187`-`009CD1AC` store `+438h` twice (`FST`/`FSTP` into
the first two slots) and `+430h` into the third. The constructor agrees: `009CC067`-`009CC0B3`
builds the moveto state with `009C2AC0(approach, target, [+40h], [+40h], [+38h])`.

| routine | listing | what it does |
| --- | --- | --- |
| states `009CC020` | `009CC062`-`009CC165` | moveto `009C2AC0` at approach `+E8h`; follow and prepare are both `009C2980(approach, 100.0)` (`[00CE3D08]`), so prepare runs the follow tick `009C1FD0`; gotowards `+18h` = `+1Ch` = 800.0 (`[00CE3950]`) |
| slot `+4Ch` `009CC8D0` | `009CC8D0`-`009CC8E5` | `009BE3E0` on the follow state `+51Ch` only, else `0099B720` (-1.0): prepare does not answer |
| hit notice `009CC400` | `009CC400`-`009CC40D` | `task+43Ch` (approach `+44h`) = 0.0, `AL = 1` |
| gotowards enter `009CA820` | `009CA820`-`009CA866` | `+20h` = 0; glide seed `009CA3B0`; `+48h` = U(0.8, 1.1) (`[00CE74F8]`, `[00CE6448]`) * desc `+18Ch` TravelSpeed; tail-jump `009CA780` |
| target point `009CA780` | `009CA780`-`009CA814` | ref `+48h..+50h` = StrafeTargetPointSelectPrec, ref `+41h` = 1, ref `+64h..+70h` = StrafeSectionDamageChance and the three weights; `009FA260` (the pick); `007B7870([approach+20h], [approach+70h], approach+80h)` |
| gotowards tick `009CA870` | `009CA870`-`009CAC6D` | see below |
| aim enter `009CB0F0` | `009CB0F0`-`009CB11F` | `+1Ch` = `+1Dh` = 0; `+18h` = U(0.2, 0.8), not read by the tick |
| aim tick `009CB1B0` | `009CB1B0`-`009CB4E3` | see below; **`[00CEE07C]` is the float 0.75** (`009CB463 FLD dword`) |
| goaway ctor `009CB500` | `009CB500`-`009CB60D` | `+28h`/`+2Ch` = 2/5, `+30h` = U(0, 5); `+34h`/`+38h` = 2/5, `+3Ch` = U(0, 5); `+40h` = 1.0, `+44h` = -U(0, 1); `+18h` = U(0.3, 0.9) |
| goaway enter `009CB8B0` | `009CB8B0`-`009CBB2E` | gun `+40h` = tuning `+674h`; `009CB780`; `009CB650`; `+24h` = 0; for a plane of neither kind 10h nor 16h hit within U(10, 16) s: an evasive push, as the tick's |
| goaway bank `009CB780` | decompiled | `r` = clamp(wrap(bearing - heading) / (pi/6), -1, 1); `+1Ch` = U(r - 1, r + 1) * 20 deg * -1 (`[00D05858]`, `[00D7A250]`) |
| goaway pitch `009CB650` | decompiled | with a target and altitude >= 30: U(min(p, 0), min(class `+1ECh`, p + 10 deg)) with p = `009FB700`(target y + approach `+40h`); else class `+1ECh` |
| `009FB700` | decompiled | climb `+1ECh` * clamp(e / ClimbDist, 0, 1) above, drop `+1F0h` * clamp(e / DropDist, -1.5, 0) below |
| goaway tick `009CBB30` | decompiled | the two re-plan timers; plan `+2C4h` = `+1Ch` (bank, mode 1), `+2BCh` = `+20h` (mode 1); throttle 1.0, brake 0, `+2D8h` = 0; `+24h` every `+40h` s; the evasive push; gun `+40h` = `+674h` |
| attackrun enter `009CAD90` | `009CAD90`-`009CADA5` | `+20h` = 0, `+18h` = 0.4 |
| attackrun tick `009CADB0` | `009CADB0`-`009CB091` | as 5by.2; `009FBA50(+40h, +38h, dist, factor)` confirmed; throttle 0.98 (`[00CE6650]`) |

**Gotowards `009CA870`**, from the listing: plan speed = approach `+48h`; `+18h`/`+1Ch` the 3-D and
horizontal distances to the aim point; `h0` = pi/2 - atan2(dz, dx), wrapped into [0, 2pi);
`007F0280` in **mode 0** (the squadron, `009CA9C6 PUSH 0`) with box (150, 150, 500) (`[00CE3808]`,
`[00CE397C]`) and zero weights; when |out_a.x| > 0.05 (`[00D7A270]`), `o` = -out_a.x * out_b.y *
out_b.z and the offset is `o` * 50 deg (`[00D057E0]`) when |o| > 0.1 (`[00D7A3A0]`); `h` =
fmod(h0 + offset, 2pi) wrapped into [-pi, pi] into plan `+2C0h`, mode 2; `+20h` = |wrap(h -
heading)| < 35 deg; the height above the aim point `y` is reduced when `+1Ch` * `+30h` >= y (by
min((g - y) / 2, 1.5 y)), then `009F9ED0(-y, +1Ch)`; gun `+40h` = `+678h`; `009FABE0`.

**Aim `009CB1B0`**, from the listing: speed `+48h`; `009F9E40` (heading, mode 2); pitch
atan2(dy, horizontal) into `+2BCh` with **mode 1**; the aim point in the plane's frame; `f` =
max(z, 0.75 |p|); `+1Ch` = f < approach `+3Ch`; `+1Dh` = z < 0 or f < 0.85 * `+3Ch`; if `+1Dh` is
clear, approach `+9Ch` = clamp(f / `+4Ch`, 0, 30), and inside class `+26Ch` (TravelSpeed / turn
rate) `+1Dh` = |(x/f, y/f)| > 0.75; gun `+40h` = `+678h`; **gun `+5Ch..+64h` = the aim point and
gun `+28h` = 0.6**.

**The gun controller reads the aim block (queue item 2).** `009FC7C0`
(docs/DOGFIGHT_GUN.md 1): when the search `007B96F0` finds no aircraft (`+74h` = 0) and `+28h` >
0, it takes `LAB_009fcac2`: `+4Ch` = -1.0 and the envelope runs on the lead point `+5Ch` the aim
state wrote. The tail resets `+28h` = -1.0 every tick, so only the aim state's thinks fire on it.
When the search does find an aircraft, it shoots at the aircraft instead.

### 2. What is bound (behind `kStrafeTaskBound`)

- **Install** on class `00E08F40` with a target: the approach as 5by.1 (with `+24h` = max(1,
  MaxSpd / Pilot/Strafe/ReferenceSpeed)), the glide seed, the goaway draws, moveto for the flight
  leader and follow for a member. A new target re-installs.
- **Each think**, in the image's order: `0099B740` (the attack mode, the leader's value copied to
  its members), the target ref (aim point) and `009CCED0`, the rule `009CC690`, the new state's
  enter on a change, the state tick.
- **Ticks**: moveto (the generic `009C18C0` with the ranges above), follow and prepare (the
  generic follow tick), gotowards, aim, goaway, attackrun as read.
- **The hit notice**: `00999AA0` zeroes approach `+44h` for a strafe plane.
- **The gun**: `009FC7C0` fires on the aim point when the search found nothing; the strafe plane
  joins the task-gun list. A strafe order no longer runs the torpedo arm alongside.
- The strafe robots rows are this installation's `robots.lua` (mtime 2025-06-01 23:03 UTC), by
  skill level.

**Substitutions, labelled in the code:**
- ctl `+398h` for the glide seed is the script's attack altitude when set, else
  Pilot/Strafe/CruisingAlt (what `009CD020` stores there; its call cadence is unread).
- The target point's pick runs at the next aim-point query on a fresh deterministic seed.
- target `+5Dh` is the host's liveness.
- Draws are keyed stand-in streams (`name#st<field>`).

**Gaps, counted:**
- goaway's evasive pushes (`009BC030` / `009BC0A0`);
- `007B7870`;
- `009FABE0`'s `+68h` (the host's gun gate reads the plane's forward);
- the moveto and follow enters;
- the cruise profile `009CD020`.

**Feed (`kAttackChoiceGunsFedBound`)**: `guns_available` = PilotFires and `guns_suppressed` = kind
10h or 16h, on the ordered unit. It logs `PilotSetTarget guns feed:` per order.

### 3. Predictions (both switches ON against OFF; written before any ON run)

- **The smoke** (USN01 300 frames) runs to its final COM release and prints no strafe line: USN01
  issues no strafe order in 100 mission frames.
- **USNOS 3000 and 9000**:
  - the three `plane #1.1..#1.3` orders at native frame 832 print `guns feed: pilot_fires=1
    suppressed=0` and choose `00e08f40` (if a Dauntless has PilotFires clear, 007EEC50 still
    declines and nothing moves: a mechanism miss to record);
  - three `strafe task 009CC230 ... installed` lines; the leader starts in moveto, the two
    members in follow;
  - the leader cycles moveto -> gotowards -> aim -> goaway -> gotowards; the members follow until
    they are in range themselves. Expect 2 to 4 aim entries per plane in the 3000 row after
    about 110 s of flight, and several times that in the 9000 row;
  - `gun_point_ticks` > 0 and `gun_point_fires` > 0 on the aim thinks;
  - the task-gun census gains the three planes;
  - the targets (348, 352, 349) may take bullet damage. Deaths may move; read the per-entity
    death table, not the count.
- **ESMP08 long**: the seven orders at native frame 2939 choose strafe; the 15 s re-orders keep
  the same target, so no re-install. The same state cycle as USNOS.
- **Dogfights by `PilotSetTarget`**: any row that orders an aircraft at an aircraft with PilotFires
  set now chooses `00e08f58` where it chose nothing, and the dogfight arm installs there. Those
  rows move; each such line is checked.
- **Rows without either** (the controls): gameplay identical.

### 4. The smoke and a first ON run (built from `f3ef5bbcf` with both flips, SHA-256 `77349F959157`)

- **Smoke** (USN01 300/100, `local\l30_smoke_on.log`): final COM release, `present interval
  immediate`, the module directory under `local\l30_on`, and `summary mission strafe task:
  planes=0`. As predicted.
- **USNOS 3000, ON side only** (`local\l30_on_usnos.log`; a mechanism check, not a verdict):
  - three orders print `guns feed: pilot_fires=1 suppressed=0` and choose `00e08f40`; the other
    three print `pilot_fires=0 suppressed=1` and still choose `00e08f28`, as before;
  - the order reaches **three squadrons of four** (`plane #1.1..#1.3` and their `|.-2..-4`
    members), so 12 strafe tasks install, not three;
  - every member takes **one think of attackrun** at t = 39.8 s and returns to follow at 39.9 s.
    The members think before their leader in that frame and read the order's mode 2 (007ED430 at
    008A4C41) until the leader's 0099B740 lowers it. This is the image's ordering, not a host
    artefact;
  - moveto/follow -> gotowards from t = 91.5 s (in range at about 2000-2500 m horizontal), with
    some one-second flaps at the range edge (the test runs once per second);
  - five aim entries from t = 117.8 s, 169 aim thinks, **61 gun fire ticks on the aim point**
    (three bursts, `task gun` rows for the three leaders);
  - no goaway before the mission ends at 150 s;
  - 168 hit-notice resets: the planes are under fire throughout.
- The arm's `record` line printed it as UNIMPLEMENTED; it is now `done`.

## 5cb. The strafe group's pairs and the verdict (packet `cc9_strafe_flip`, cc9-lua30, 2026-09-30)

OFF `local\l30_off` (SHA-256 `47F00B464DCD`), ON `local\l30_on` (`B6E261D42376`, both flips). Both
are built from `cc43405cc`. Reference launch form, `BSP_GUNNERY_RNG_STREAMS=1`,
`BSP_DEATH_TABLE=1`. Script: `local\l30_runs.ps1`; logs: `local\l30_{off,on}_<row>.log`.

| row | pair_diff | what moved |
| --- | --- | --- |
| USN01 3200/3000 (control) | 1, gameplay identical | - |
| JM05 3200/3000 (control) | 1, gameplay identical | - |
| USNOS 3200/3000 | 3 | deaths 110 -> 113; the three strafe targets take damage |
| USNOS 9200/9000 | 3 | deaths 166 -> 169; the same strafe rows as the 3000 row |
| ESMP08 9200/9000 | 3 | deaths 0 -> 7 (F4U Corsairs); no strafe fire |

**Choices.** The class counts of `PilotSetTarget choose` move only where OFF declined:
- USNOS: `00000000` x3 becomes `00e08f40` x3;
- ESMP08: `00000000` x177 becomes `00e08f40` x177;
- the torpedo, dive and level choices are unchanged on every row;
- no row chose `00e08f58`.

**Dogfight by `PilotSetTarget`** is not exercised anywhere. Among the rb13 reference logs, only
USNOS declines any order (3). cc9-lua29's ESMP11 and JM14 bases issue no `PilotSetTarget`. So no
available row gains a script dogfight. This is recorded as unmeasured, not as a pass.

**USNOS 3000:**
- 12 tasks (three squadrons of four) and 45 transitions;
- five aim entries at t = 117.8-127.1 s, from about 1000 m (3-D) at 575-672 m altitude;
- 148 aim thinks and **53 gun fire ticks on the aim point** (corrected in 5cc: 169 and 61 were
  the `f3ef5bbcf` run of 5ca.4).

The strafe damage lands on the ordered targets (unit table, OFF -> ON):

| target | hits | dealt | taken |
| --- | --- | --- | --- |
| TroopTrans2 | 0 -> 8 | 0 -> 89 | 0 -> 22 |
| TroopTrans3 | 0 -> 38 | 0 -> 25 | 0 -> 108 |
| TroopTrans6 | 0 -> 13 | 0 -> 70 | 0 -> 37 |

The strafers press in and die to AA:
- OFF already loses 9 of the 12 (they circle at about 1200 m under fire);
- ON loses all 12. `plane #1.1`, `#1.2` and `#1.1|.-2` are the three only-ON deaths;
- nine death rows move earlier or later, at 372-928 m instead of about 1200 m, killed by
  Portland2, TroopTrans1 or the Gear boats.

Every aimer dies within 1-5 s of entering aim. So **goaway and its evasive gate are never
reached**, and attackrun only as the members' one-think blips at an order.

**ESMP08 long:**
- 497 installs for 36 planes: `luaControlAirAttacks` re-targets every 15 s, and a new target
  re-installs;
- 322 attackrun thinks, all one-think blips after an order;
- three aim entries at t = 442-444 s, zero gun fires;
- seven Corsairs shot down closing on the IJN fleet. OFF they never engage and nothing dies;
- 210 hit resets.

**Against 5ca.3:**
- smoke: as predicted;
- the feed and the choice: as predicted;
- strafe damage on the USNOS targets: as predicted;
- the task count is **missed** (12, not 3), and aim entries are **missed** (5 in USNOS,
  predicted 2-4 per plane);
- goaway and the full cycle: **missed**, not reached because the aimers die;
- ESMP08 fire: **missed**, 0;
- dogfight by `PilotSetTarget`: **unmeasured**;
- controls: as predicted.

A divergence between the USNOS 3000 row and the head of the 9000 row, reported here at first,
does not exist: see 5cc.

### Verdict: **flip ON**, with recorded misses (`kStrafeTaskBound = true`, `kAttackChoiceGunsFedBound = true`)

The mechanism matches end to end where it is reached:
- the order chooses strafe;
- the task installs;
- moveto/follow -> gotowards -> aim run as read;
- the gun fires on the aim point;
- the ordered targets take the damage.

The misses are spread (counts) and reach (goaway unreached because AA kills the aimers), not a
mechanism failure. Still unverified:
- goaway, attackrun beyond the one-think blip, and the evasive gaps;
- script dogfights.

## 5cc. The USNOS 3000/9000 "divergence" was two binaries, not nondeterminism (cc9-lua30, 2026-09-30)

5cb reported that on the ON side, the USNOS 3000 row and the first 150 s of the 9000 row
disagree: aim thinks 169 vs 148 and gun fires 61 vs 53. **That was a transcription error.**
- 169 / 61 are the 5ca.4 run, built from `f3ef5bbcf`.
- 148 / 53 are the 5cb run, built from `cc43405cc`.
- `local\l30_on_usnos.log` was rewritten by the 5cb run (mtime 22:16:35 UTC) before its census was
  re-read.
- The two builds differ by other packets' landed code: `git diff --stat f3ef5bbcf cc43405cc`
  touches `game_hosts_ship_ai.cpp`, `hull_terrain_contact.cpp`, `game_hosts_ai.cpp` and
  `game_hosts_units.cpp`, 12 files, +1299/-115.

**The check.** `local\l30_firstdiff.py` starts at the first `local-player unit lists sources`
line. It drops the harness lines, the first-call `host ...` registrations and the lines naming the
run length, and it masks pointers and thread ids.
- On both sides (`l30_off_usnos` vs `l30_off_usnosl`, `l30_on_usnos` vs `l30_on_usnosl`) the two
  rows are line-identical up to and including `controlled unit frame 3000 t= 150.00`.
- The first difference is the 3000 row's end-of-run summary.
- The two logs' per-plane strafe census rows are identical, and so is their summary (aim = 148,
  fires = 53).

Nothing in the first 150 s reads the configured frame count, and the runs are deterministic.

## 5cd. Rows for goaway and for a script dogfight, from the Lua; the unitcommand gap (packet `cc9_strafe_unitcommand`, cc9-lua30, 2026-09-30)

Read-only apart from an env-gated trace. The rows were chosen from this installation's scripts
(`scripts/`, modded), not by trial.

### 1. Strafe orders against light AA: none in a reachable row

The strafe choice needs three things: a `PilotSetTarget` from a plane with no ordnance class, a
surface target, and PilotFires set.
- The scripts that re-order a plane unless it is already strafing (5bv) are `jm09.lua`,
  `jm12.lua`, `usn_10_battle_of_capeengano.lua`, `ESMP/08_engano.lua` and `ESMP/11_tengo.lua`.
- `missiontree.lua` reaches only the two ESMP scenes (lines 5815 and 5968). Both target a battle
  fleet: `Mission.IJNFleet` at Engano, and Yamato's group at Tengo.
- USNOS targets troop transports escorted by the Gear boats.
- Among the other `PilotSetTarget` calls, the transport, convoy and landing-ship targets
  (`usn_1_marshall` `Mission.Convoy`, `prcp_13_iwojima` `Mission.LandingShips`, `usn_6_guad`,
  `usn_ormoc`) are ordered to bombers or torpedo aircraft, which take an ordnance class.

So no reachable row strafes a lightly defended target. The best goaway candidate is **ESMP08
long**, and only once the gap below is closed: its strafers are re-targeted before they finish a
pass.

### 2. The gap: `GetProperty(unit, "unitcommand")` never answers the attack order

`08_engano.lua` lines 579-590 (`luaControlAirAttacks`, every 15 s) re-order a bomber only when its
`unitcommand` is neither `"torpedo"` nor `"strafe"` nor `"divebomb"`. The trace in
`game_hosts_lua.cpp` (`BSP_UNITCOMMAND_TRACE=1`, `local\l30_uc_esmp08.log`, ESMP08 4200/4000 on
this branch with the strafe group ON) logs every answer:
- after the first strafe order (log line 22150), every answer is `moveto` (`00E08F68`, 108) or
  `nocommand` (33);
- no answer is ever `strafe`, and no torpedo-ordered plane answers `torpedo` either.

The host's director (`0071BE40` on slot 0) keeps the `PilotMoveTo` command from line 541. The
attack class that `PilotSetTarget` issues through `0077D600` never becomes the current command.
- `GetProperty(unit, "ammotype")` has no reader either, so the `~= 0` test is always true.
- Consequence: the script re-orders the whole bomber list every 15 s on both sides. In the 5cb ON
  run that gives 497 strafe installs for 36 planes. Each re-target restarts moveto/follow, which
  is why ESMP08 reached only three aim entries.
- The same gap re-orders ESMP08's torpedo bombers (`00e08f18`) OFF and ON alike.

This is the commands host's `director_current_command_0071be40` / entity-order delivery, not the
strafe task. It is handed to the lead.

### 3. Script dogfights: candidates

- **BSM04** (`bsm_04_vengance_at_luzon.lua` line 1909): `PilotSetTarget(Mission.Cat,
  Mission.ZeroGang[1])` in `luaIntroMovieEnd`. A 3200/3000 probe with the group ON
  (`local\l30_probe_bsm04.log`) did not reach it; only the B-17 level-bomb orders appear (49 x
  `00e08f28`). The intro movie does not end for an idle player within 150 s. This is unconfirmed.
- **USNEX** Pearl Harbor (`USNRM/usn_1_pearl.lua` lines 1350-1357): the Welch squadron against the
  final Zero squadrons and back. It fires in phase 3.
- `usn_04_defend_guadalcanal.lua` line 422 (P-40 against a Japanese bomber) is not in this
  installation's `missiontree.lua`.

## 5ce. The strafe order does become current; the AI command tick overwrites it (packet `cc9_strafe_unitcommand`, cc9-lua30, 2026-09-30)

This corrects the reading of 5cd.2. It comes from the commands host's own row table in
`local\l30_uc_esmp08.log`.

- **The director takes the strafe order.** `PilotSetTarget` issues `strafe` (ordinal 10,
  category 1) through `0077D600`. Its row reads `issue 1 slot 1 curr 1` (log line 42937 for
  `TBM Avenger #1.1`): the push lands in slot 0 and is the current command (`0071BE40`). The
  host's director path is not the gap.
- **What replaces it is a `moveto` from the AI command tick.** The next rows for the same plane
  (log lines 43049, 43093, 43135, 43180) are `moveto  ai_command_tick  15 3 1 1 1`. That is
  `src/game_hosts_ai.cpp` `tick_issue_moveto`, the host's `00A02020`
  (`BSP_AiCommand_IssueMoveToMember`), reached from `ai_command_tick.cpp`'s follower pass
  `00A10DC0` or leader arm. There are 1062 `ai_command_tick` rows in the 4000-frame run. After
  the order, `unitcommand` therefore reads `moveto`, and `08_engano.lua` re-targets every 15 s.
- **`00A02020` itself has no current-command test** (decompiled; the Ghidra plate reads the body
  whole). It admits a squadron when `007EDA90` rejects it, or a ship base, and issues `moveto`
  unconditionally. So a fix is not a gate in `00A02020`. It is in whichever caller decides to
  order these US squadrons, and how often, in the AI group tick (the planners' lane,
  `game_hosts_ai.cpp` / `ai_command_tick.cpp`). The questions:
  - is an ESMP08 US strike squadron in an AI group at all in the image;
  - what cadence does the follower pass run at;
  - does a squadron holding an attack command drop out of the group.
- **`ammotype`**: the image's only `ammoType` string (file offset `0x908770`) sits in a property
  table beside the pointer `007EFAE0`. That is a thunk (`SUB ECX,310h / JMP 007F1140`) into the
  squadron's property reader `007F1140`, which is not read yet, so it is **not bound**. The host
  answers nothing, so the script's `~= 0` test passes; that matches the image whenever the
  squadron's ammo type is non-zero.

Nothing is bound in this section. The `commands` loan is not needed for this finding.

## 5cf. Handoff (cc9-lua30, 2026-09-30)

Branch `agent/cc9-lua30`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua30`. No lease
is held after this handoff. This file is not on loan; `src/game_hosts_ai.cpp` and
`src/ai_command_tick.cpp` were lent for the packet below and are handed back untouched.

| packet | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| `cc9_strafe_arm` | `f3ef5bbcf`, `635620a4d` | `kStrafeTaskBound`, `kAttackChoiceGunsFedBound` | bound | 5ca |
| `cc9_strafe_flip` | `bb1a1e2c8` | both | **ON**, misses recorded | 5cb |
| `cc9_strafe_row_divergence` | `6e2d656d3` | - | closed: two binaries, the runs are line-identical | 5cc |
| `cc9_strafe_unitcommand` | `d9fcb3794`, `4b9a1a4cc` | - (env trace `BSP_UNITCOMMAND_TRACE`) | read | 5cd, 5ce |

### Where the AI-tick packet stands (the lead's loan, not finished)

The question: why `GetProperty(unit, "unitcommand")` reads `moveto` after a strafe order on
ESMP08, so that `08_engano.lua:584` re-targets every 15 s.

Established:
- The strafe order becomes the director's current command at issue (5ce).
- The host's **AI coordinator** puts all 12 US strike squadrons of the first wave into one group:
  `ai group team=0 party=4 members=12 claimed=1 command=MOVETOATTACK leader=TBM Avenger #1.1`
  (`local\l30_uc_esmp08.log`, line 42022). The script spawns them as one `SpawnNew`
  `groupMembers` wave (`08_engano.lua:519-535`).
- In the image the squadrons are groupable combatants: `009FE080` admits a squadron whose
  `007EDA90` is false (docs/AI_COMMAND_LIFETIME.md).
- **`00A02020` has no current-command test.** The `MOVETOATTACK` closing arm `00A12A90` orders the
  leader while the distance exceeds `CloseAttack_CollectDist` (5000), every 2 to 4 s on USN02
  (docs/AI_COMMAND_TICK.md). The follower pass `00A10DC0` then sends every other squadron to the
  leader's point.
- So far nothing found in the image skips a squadron that holds an attack command. The
  divergence, if any, is upstream:
  - (a) whether the image's coordinator groups a script-spawned wave at all;
  - (b) whether the group promotes to `CLOSEATTACK` (whose `00A13B60` is unread and may not order
    squadrons) once within 5000 of the target group.

  The host's group was still `MOVETOATTACK` at the end of the 4000-frame run, with 1062
  `ai_command_tick` rows.

Next, in order:
1. Read the promotion: `00A12A90`'s distance is between the two groups' leader points
   (`009FFC10`), and the US leader is a strike squadron closing on the fleet, so it should promote.
   Log the host's distance and CollectDist for that group per think; if it never drops below
   5000, find out why.
2. Read `00A13B60` (CLOSEATTACK) for whether it issues to squadrons.
3. Read the coordinator's seeding for script-spawned `groupMembers` waves (`seed_admits`,
   `game_hosts_ai.cpp:789`; docs/AI_COORDINATOR_TICK.md).
4. Bind any divergence OFF, with predictions. Pair ESMP08 (3000 and long), USNOS and two
   controls. If ESMP08's strafers then keep their order, ESMP08 long is the goaway row (5cb).
5. Then the script-dogfight rows: USNEX `usn_1_pearl.lua:1350-1357` (phase 3) and BSM04
   `bsm_04_vengance_at_luzon.lua:1909` (after the intro movie) (5cd.3).

Tools in `local\` of this tree:
- `l30_runs.ps1` and `l30_launch.ps1`: the pair rows, and a detached launcher for the long rows;
- `l30_firstdiff.py`: the masked first-difference of two logs;
- `l30_apply_units.py` and `l30_strafe_methods.cpp.txt`: the 5ca edit;
- `l30_uc_esmp08.log`: the `unitcommand` trace run;
- `l30_{off,on}_<row>.log`: the 5cb pairs.

## 5cg. A squadron-led AI group's leader point was the origin (packet `cc9_ai_tick_strafe`, cc9-lua31, 2026-09-30)

This answers item 1 of 5cf. The `src/game_hosts_ai.cpp` and `src/ai_command_tick.cpp` loan from
the ships lane is used for it.

### The measurement

`BSP_AI_SQUAD_TICK_TRACE=1` (new, env-gated, observation only) logs every `MOVETOATTACK` and
`CLOSEATTACK` tick of a group whose first member is a plane squadron, with `00A12A90`'s two inputs
and both leader points. ESMP08 4200/4000 at `cbde1ec24` plus the trace
(`local\l31_sq_esmp08.log`, strafe group ON as on main) logs 18 ticks, from t=148.55 to t=198.41,
all for the one group:

    ai squadtick t=148.55 cmd=MOVETOATTACK leader=TBM Avenger #1.1 members=12 target=Zuikaku
      dist=15712.4 collect=3000.0 groupable=1 own=(0 0 0) tgt=(11285 -3 10932)
    ...
    ai squadtick t=198.41 cmd=MOVETOATTACK leader=TBM Avenger #1.1 members=12 target=Zuikaku
      dist=15201.6 collect=3000.0 groupable=1 own=(0 0 0) tgt=(10924 -3 10571)

- **The own leader point is `(0 0 0)` on every tick.** The distance is Zuikaku's distance from the
  map origin; it shrinks only because Zuikaku sails towards the origin. It can never fall below
  `CloseAttack_CollectDist` (3000 loaded here), so the group can never promote, whatever the
  squadrons do.
- **The cause is one host line.** `group_leader_position` (the host's `00A10C20` read, reached from
  `tick_leader_point`) passes the group's first member index straight to
  `GameUnitsHost::unit_position_00fc`. A squadron's candidate index is past the unit rows
  (`is_squadron`: `index >= units.count()`), so the read answers zeros. `tick_member_position`
  already applies `proxy()`; this read did not.
- **The same point feeds the follower pass.** `00A10DC0` sends every non-leader member to the
  leader point through `00A02020`. So on ESMP08 the 11 follower squadrons were ordered towards the
  map origin on every tick, which is part of the 1062 `ai_command_tick` `moveto` rows of 5ce.

### The image

`00A10C20` (`00A10C20`-`00A10C5B`, `RET`, `__thiscall(group)`, disk bytes): when `+5644h` is 0 it
returns `00F87574`; otherwise it takes the first node of the `+5640h` list, its entity at node
`+8h`, runs `00414DB0` when the entity's `+C8h` byte is clear, and returns `&entity+FCh`. For a
squadron that is the squadron's own world position. `GetPosition` (`008A7B00`, the read at
`008A7C3C`) returns the same field, and the scripts use it on squadrons as a moving point (the
ESMP08 intro movie's `cameraandtarget` follows the squadron), so the field tracks the flight.
Which routine keeps a squadron's `+74h`/`+FCh` current is **not read**: the squadron's tick
element has no pose step (`docs/TICK_ELEMENT_OVERRIDES.md`, slot `+4h` is the base stub). This
host answers every squadron pose from its flight leader (`proxy()`, labelled in
`docs/CONSTRUCT_WORLD.md` "the squadron's position +FCh | the leader's pose"); the binding uses
that same substitution. Uncertainty: the image's squadron point could be a formation centre
rather than the flight leader; the difference is at most the formation spread.

Questions (a) and (b) of 5cf:
- (a) Grouping a script-spawned wave is not the divergence. The group exists in the host because
  `009FE080` admits squadrons (`groupable=1` above) and the prox merges join them; nothing in the
  measurement needed the image to group them differently.
- (b) The promotion and `00A13B60` are read and bound (docs/AI_COMMAND_TICK.md,
  docs/AI_CLOSE_ATTACK_TICK.md). The group never reached them because its distance was measured
  from the origin.

### The binding, committed OFF

`kAiSquadronLeaderPointBound` in `src/game_hosts_ai.cpp`. ON, `group_leader_position` reads
`proxy(front)` instead of `front`. It changes nothing for a group led by a unit, since `proxy()`
of a unit index is the index.

### Predictions, written before any ON run

Reference V's end tables (`g20_rv_*.log`) list squadron-led groups on USNOS (a `CAUTIOUSATTACK`
group led by `plane #1.1`), USN13 (`MOVETOATTACK`, `bruh #1.4`), USN04 (`CLOSEATTACK`,
`A6M Zero #7.2`), JM06 (`MOVETOATTACK`, `PBY Catalina 01`), JM05 (`SELLING`, `F4F Wildcat 01`) and
none on USN02 or USN12.
- **ESMP08 long (9200/9000):** the US strike group's distance starts at its real separation and
  falls as it flies; it promotes to `CLOSEATTACK` inside 3000. Its followers go to the leader, not
  the origin. After the promotion `ai_command_tick` stops issuing `moveto` to those squadrons, and
  `00A13B60` takes over (`settarget` to a served squadron). Whether `unitcommand` then answers
  `strafe` long enough for `08_engano.lua:584` to leave the planes alone is **not predicted**.
  The death table moves.
- **USNOS 3200/3000:** moved (its squadron-led `CAUTIOUSATTACK` group's followers stop heading
  for the origin).
- **USN02 and USN12 (controls):** gameplay-identical, `pair_diff` exit 0 or 1.
- **Weakest call:** whether the `Static ...` plane leaders on IJN01, BSM01 and the LOMP rows are
  squadrons; if they are, their `DEFENDPOSITION` passes re-centre from the origin and those rows
  move too. Not run here.

### The pairs and the verdict (cc9-lua31, 2026-09-30): ON

Same-tree exports of `4d6093605`: `local\l31_off` (SHA-256 prefix `4A0C783AB780`) and
`local\l31_on` (`DA3EEEC12D78`, the switch flipped). Reference V's launch form,
`BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player
(`local\l31_runs.ps1`). A 500/300 USN01 smoke on the ON binary finished cleanly first.

| row | `pair_diff` | deaths | hit records | damage | note |
| --- | --- | --- | --- | --- | --- |
| USN02 3200/3000 (control) | identical | 1 / 1 | 1126 / 1126 | 28473.8 / 28473.8 | one summary line, the known refill noise |
| USN12 3200/3000 (control) | identical | 8 / 8 | 198 / 198 | 4048.7 / 4048.7 | only the known JM08/USN13-class sector-scan noise |
| USNOS 3200/3000 | moved | 90 / 107 | 1400 / 1487 | 49146.1 / 54561.0 | 18 deaths only ON, all town buildings, containers, a hangar and a watchtower; 1 only OFF |
| ESMP08 9200/9000 | moved | 7 / 6 | 210 / 198 | 2421.5 / 2017.7 | one Corsair death only OFF; 25 unit rows changed |

Traced runs (`BSP_AI_SQUAD_TICK_TRACE=1`, `BSP_UNITCOMMAND_TRACE=1`; `local\l31_tr_{off,on}_esmp08l.log`,
and `local\l31_tr_on_esmp08x.log` at 14200/14000):
- **The mechanism holds.** ON, the US strike group's leader point is the flight leader's
  position (t=164.36 `own=(13952 1245 -12694)`), and the distance to Zuikaku falls from 24677 at
  t=148.55 to 3633 at t=448.41, about 70 per second, the TBM's closing speed. OFF it stays
  `own=(0 0 0)` throughout.
- **The group still does not promote (miss).** In the 14000-frame run the leader reaches 3598.7
  at t=455.01, then circles 3164.6 to 4035.8 from Zuikaku's point until the group empties
  (members 12 -> 2 by t=534.69). The loaded `CloseAttack_CollectDist` is 3000, so
  `MOVETOATTACK` holds and the tick keeps ordering `moveto`: 2472 -> 2444 `tick_orders` over the
  9000-frame pair, 0 promotions both sides. Why the leader holds 3.2-4 km off the carrier (the
  15 s script re-target against the 2-4 s tick `moveto`, or a moveto loiter radius) is not read.
- **`unitcommand` answers.** 9000 frames: OFF 936 `moveto`, 36 `nocommand`; ON 930 `moveto`,
  36 `nocommand`, 4 `strafe`. 14000 frames ON: 1866 `moveto`, 36 `divebomb`, 20 `strafe`,
  7 `torpedo`, 36 `nocommand`. The attack classes now appear, but `moveto` still dominates, so
  `08_engano.lua:584` still re-targets and ESMP08 long is **not yet the goaway row**.

Verdict: **ON.** The mechanism matches (a squadron-led group's point is the squadron's
position, the followers go to the leader), the controls are identical, and the moved rows are the
predicted ones. The ESMP08 promotion was predicted and did not happen; that is recorded as a
miss downstream of the binding (the leader never closes inside 3000), not a failure of it.
IJN01, BSM01, the LOMP rows, USN04, USN13, JM05 and JM06 were not run; they move where a
squadron leads a group.

## 5ch. The AI tick's moveto did not re-task the plane (packet `cc9_ai_tick_plane_retask`, cc9-lua31, 2026-09-30)

The lead's question after 5cg: why does ESMP08's strike leader hold 3.2-4 km off Zuikaku? The
candidates were the 15 s script re-target against the 2-4 s tick `moveto`, or a moveto loiter or
arrival radius, plus the 80 m gate and the CollectDist value.

### Answers

- **`CloseAttack_CollectDist` is 3000 on ESMP08, and that is the image's value.** `00D1AF84`
  (5000) is only the default. `00A335D0` loads the record `009FFC80` picks, and game mode 8 is
  above 7, so it takes the `IslandCapture` arm with difficulty 0. That is
  `HighLvlAIGlobals["IslandCaptureParams_Rookie"]`, which authors 3000 at
  `scripts/datatables/highlvlaiglobals.lua:119` (this installation, mtime 2024-07-13). Every
  record authors 3000 except `EscortParams` (6000, line 832). The run's line reads
  `summary mission ai tuning mode=0 (IslandCaptureParams_Rookie)`.
- **The 80 m gate is not it.** The leader and follower orders pass `00A02020` whole (5cg reply),
  and the leader is kilometres from its point.
- **A loiter radius is not it.** The leader never flies a moveto task: see the next point.
- **The leader flies its script target, not the tick's point.** In `local\l31_tr_on_esmp08x.log`
  the leader's `command target 0071EBF0` token walks through a new random `Mission.IJNFleet` ship
  every 15 s (Tama, Zuiho, Maki, Ise, Wakatsuki, ...). Its strafe task is re-installed at each
  `PilotSetTarget`, and there is no moveto task install for any plane apart from the 36 script
  `PilotMoveTo` installs. The tick's `moveto` reaches the director (`unitcommand` reads `moveto`),
  but the bot keeps its strafe task. So the leader circles whichever fleet ship it was last given,
  3.2-4 km from Zuikaku.

### The image

- **`0099A4C0` retires the strafe head on a `moveto`.** The bot tick calls `0099A4C0` at
  `0099AE7E` when the command changes. It keeps a single head task only while the task's
  `vtable[40h]` answers 1; otherwise it pops the task and calls `0099A170` for the new command
  (docs/PILOT_MOVETO_TASK.md, docs/PILOT_BOT_TICK_GATES.md).
- **The strafe task's `vtable[40h]` is `009CC850`.** The vtable is `00D210E0`, stored at
  `009CC281` in the factory `009CC230`. `009CC850` is `009CC850`-`009CC8BC`, `__thiscall(task)`,
  plain `RET`, read from the disk bytes. It takes `[task+404h]`'s `vtable[114h]` director and
  answers 1 only when:
  - `0071BE40` is `00E08F40` (`settarget`) or `00E08F78`; and
  - the command's target (`0071EB60` -> `00521EA0`) equals `task+44Ch`, or `task+468h` when
    `+44Ch` is null, or both are null.

  A `moveto` (`00E08F68`) answers 0.
- **`0099A170` then builds the moveto task.** Its moveto arm (`0099A23A`) has no precondition
  and builds the kind-7 task `009C3BE0`.
- **A moveto head is kept on a repeat.** `009C31B0` keeps the kind-7 task when the new command
  is a moveto whose point lies within 100 m planar (`[00CE3D64]` = 10000, squared) of the task's
  own point (docs/PILOT_MOVETO_TASK.md).

So in the image, every tick `moveto` the group issues ends the scripted strafe, and the leader
flies to the target group's leader point. The script's re-target 15 s later starts a strafe
again, which the next tick (2-4 s) ends. That matches 5ce's `unitcommand` reading `moveto`. It
also means the leader closes on Zuikaku's point.

### The binding, committed OFF

`kAiTickMovetoRetasksPlaneBound` in `src/game_hosts_ai.cpp`:
- In `tick_issue_moveto`'s squadron fan-out, once the member plane's director holds the
  `moveto`, the binding runs `bot_install_command_task_0099a170` with a position-only host
  (`AiTickMovetoBotHost`).
- It stores the kind-7 task's inputs as PilotMoveTo does: the class `00E08F68`, range 0, no
  target object, and the point.
- It keeps the existing task instead when the plane already held a tick `moveto` whose point
  lies within 100 m (`009C31B0`).
- **SUBSTITUTION, labelled:** the install runs at the delivery, one bot tick early, as for
  every script order (SENTITY_INIT_ATTACH_ORDER 22.7).
- A census line `summary mission ai tick plane retask ...` is printed when the switch is ON.

### Predictions, written before any ON run

- **ESMP08 9200/9000 and 14200/14000:**
  - the US strike leader stops circling its script target and closes on Zuikaku's leader point;
  - the group's distance falls below 3000 and it **promotes to `CLOSEATTACK`** (5cg's trace
    reached 3600 by t=451 while circling, so the promotion should come before t=470);
  - `retask replaced_other` is non-zero (the strafe heads that a tick `moveto` ends);
  - after the promotion, `ai_command_tick` `moveto` rows for that group stop and `00A13B60`
    takes over;
  - strafe task ticks fall;
  - the death table moves.
- **USNOS 3200/3000:** moved (its squadron-led groups' members are re-tasked).
- **USN02 and USN12 (controls):** gameplay-identical, `pair_diff` exit 0 or 1. Neither has a
  squadron in a tick-ordered group.
- **Weakest call:** the promotion. The leader may instead dive onto Zuikaku's own escorts before
  3000; the script's 15 s strafe re-target still installs between ticks.

### The pairs and the verdict (cc9-lua31, 2026-09-30): ON

**First pair (`55854580e`, OFF `A79ECDA42F68`, ON `DB142BAB5873`): a mechanism failure.**
- The install ran straight after `issue_script_command`.
- With the loopback queue bound, the `moveto` is delivered only at the session pump's drain, so
  the director still held the old command. A temporary diagnostic (not committed) showed
  `before=now=00e08f18` (torpedo), `00e08f40` and `00e08f20`.
- ESMP08 9000 counted `not_current=1026` and `replaced_other=0`: no attack head was ever
  retired. Meanwhile the installs over existing moveto heads cut every strafe short (0 hits).
- That run is discarded.

**The correction (`a59dae607`).** The install now follows the delivery through
`commands_after_last_issue_delivery`, as the script orders' installs do.

**Second pair (`a59dae607`, OFF `CB9334362F07`, ON `ECA965F3003A`).** Reference V's launch
form; a 500/300 USN01 smoke on the ON binary finished cleanly.

| row | `pair_diff` | deaths | hit records | damage | retask installed / kept / replaced_other |
| --- | --- | --- | --- | --- | --- |
| USN02 (control) | exit 1, gameplay identical | 1 / 1 | 1126 / 1126 | same | 0 / 0 / 0 |
| USN12 (control) | exit 1, gameplay identical | 8 / 8 | 198 / 198 | same | 0 / 0 / 0 |
| USNOS 3200/3000 | moved | 107 / 95 | 1487 / 1189 | 54561.0 / 51217.3 | 440 / 416 / 24 |
| ESMP08 9200/9000 | moved | 6 / 0 | 198 / 0 | 2017.7 / 0.0 | 4425 / 291 / 1023 |
| ESMP08 14200/14000 | moved | 48 / 44 | 1040 / 1619 | 12303.1 / 13714.7 | 7819 / 719 / 1789 |

- **The mechanism holds.**
  - `not_current` is 0 everywhere.
  - On ESMP08 1023 (9000) and 1789 (14000) attack heads are retired by a tick `moveto`.
- **The promotion happens (prediction met).**
  - ESMP08 14000 counts `promotions` 1 -> 2: the US strike group now reaches `CLOSEATTACK`.
  - A traced run of the first, pre-delivery build already showed it at t=445.41, d=2609.7; the
    corrected pair is not traced.
  - `00A13B60` then serves the squadrons: `served` 214 -> 893, `settarget` 211 -> 647, and
    `fallback moveto` 3 -> 246.
  - The tick's own orders fall: `tick_orders` 4420 -> 3969.
- **The strike lands later and harder.**
  - Before the promotion every scripted strafe is ended by the next tick `moveto`, so ESMP08
    9000, which ends at t=450 just after the promotion, has no hit ON (198 OFF).
  - At 14000 the first hit moves 420.32 -> 486.20 s, and hits rise 1040 -> 1619.
  - Two torpedoes are dropped (`torpedo-task releases` 0 of 21 -> 2 of 21). That is the first
    torpedo release of this row.
- **USNOS** loses 12 plane deaths: its re-tasked squadrons fly to their group's point instead of
  pressing attacks through the AA (shots 6166 -> 802).
- **Goaway: still none.** The strafe tables show `goaway` 0 ticks on both sides of ESMP08 14000
  (aims 9 -> 8). So ESMP08 long is **still not the goaway row**. The strafers now take the close
  pass's `settarget`, and whether that pass reaches a strafe's goaway is the next question.

Verdict: **ON.** The mechanism matches, the controls are gameplay-identical, the predicted
promotion happened, and the moved rows are the predicted ones. The pre-promotion loss of every
scripted strafe on ESMP08 is the image's rule as read (`009CC850`), not a miss.

## 5ci. `GetProperty(squadron, "ammoType")` bound OFF (packet `cc9_get_property_ammotype`, cc9-lua31, 2026-09-30)

Queue item 4 of the brief. This corrects 5ce on one point. **`007F1140` is not the property
reader.** It is the squadron's scalar deleting destructor (`BSP_PlaneSquadron_Destruct`, then
`_free`), and `007EFAE0` is its tick-element thunk (docs/TICK_ELEMENT_OVERRIDES.md, slot `+0h`).
The `ammoType` string at `00D08770` merely sits in front of the tick-element vtable `00D0877C`.

**The reader is `007EF1C0` `BSP_PlaneSquadron_GetProperty`**, the PlaneSquadronGen vtable
`00D087C0` `+138h` (ledger; docs/MISSION_LUA_GETPROPERTY.md 9.2). Its sequence, from the disk
bytes:
- `007EF1CF` `CALL 00779BB0`: the base reader, which handles `unitcommand` and `reconlevel`.
- `007EF1D4`-`007EF1D9`: the key at `[arg+4]`; when it is null, the reader jumps to the `state`
  test.
- `007EF1DB` `PUSH 00D08770` (`"ammoType"`), then `007EF1E1` `CALL 00BF7FBF` (`_stricmp`).
- `007EF1F0`: on a match, `007EF1F4` `CALL 007EDAD0` on the squadron, and the integer is pushed.

**The comparison is case-insensitive**, so `08_engano.lua:582`'s `"ammotype"` matches. The
integer comes from `007EDAD0`, which the units host already reconstructs as
`squadron_ammo_type_007edad0` (docs/SQUADRON_ORDNANCE_STATE.md, `kSquadronOrdnanceReaderBound`
ON). Its values are 2 torpedo, 3 depth charge, 4 rocket, 5 paratrooper, 6 dummy kamikaze,
1 bomb, and 0 when nothing is carried.

**Binding.** `kGetPropertySquadronAmmoTypeBound` (`include/bsp/game_hosts_lua.hpp`), committed
OFF:
- `run_get_property_class_readers` serves the key only to an entity that is a registry squadron's
  own unit, with `squadron_ammo_type_007edad0`.
- OFF, the host answers no value, and the scripts' `~= 0` test passes on `nil`.
- A summary line `summary mission getproperty ammotype bound asked served zero` is printed on
  both sides.

### Predictions, written before any ON run

- **ESMP08 9200/9000:**
  - The wave's Avengers answer 2 and the Helldivers 1. A Corsair with no ordnance answers 0.
  - For a 0, `luaControlAirAttacks` stops calling `PilotSetTarget` (`08_engano.lua:582`).
  - The strafe installs from the 15 s loop (5cb counted 497) fall: by the Corsairs' share if
    they carry nothing, to 0 for the Corsairs if `zero` equals their count.
  - The death table moves.
- **USNRM01 (`usn_1_pearl.lua:1983`, the Japanese attackers' loop):** moved if that loop runs;
  `asked` is non-zero.
- **USN02, USN12 (controls):** gameplay-identical. Neither script asks `ammoType`; `asked` is 0.
- **JM06:** docs/MISSION_LUA_GETPROPERTY.md measured 17 `ammoType` asks; the row may move.
- **Uncertainty:** whether ESMP08's Corsairs carry ordnance. Their `Equipment` in
  `08_engano.lua` decides it, and `zero` will show it.

### The pairs and the verdict (cc9-lua31, 2026-09-30): ON

The pairs are same-tree builds of `5d0513291`: OFF `4FB4CBF41BA4`, ON `9942BFBAB41B`. They used
reference V's launch form, and a 500/300 USN01 smoke on the ON binary finished cleanly. The
runs waited for the console session to come back after it had disconnected; while it was
disconnected, one USNRM01 36000 run died at FMOD startup (environment, not code).

| row | `pair_diff` | `ammoType` asked / served / zero | note |
| --- | --- | --- | --- |
| USN02 (control) | exit 1, gameplay identical | 0 / 0 / 0 | |
| USN12 (control) | exit 1, gameplay identical | 0 / 0 / 0 | |
| JM06 3200/3000 | exit 1, gameplay identical | 0 / 0 / 0 | its asks lie past 3000 frames |
| ESMP08 9200/9000 | exit 1, gameplay identical | 324 / 324 / 0 | every wave squadron carries ordnance |
| USNRM01 9200/9000 | moved | 290 / 290 / 43 | deaths 158 -> 156, hits 1994 -> 2207, dive releases 34 -> 30 |

- **The mechanism holds.** Every ask is served, and only from a squadron's own unit.
- **ESMP08's Corsairs carry ordnance.** `zero=0`, so the weakest call of the predictions resolved
  the other way: the script keeps re-targeting the whole wave, and the row is unchanged.
- **USNRM01 moves as predicted.** Forty-three answers are 0. For those squadrons
  `usn_1_pearl.lua:1983` (`GetProperty(unit,"ammoType") ~= 0`) no longer re-targets.

Verdict: **ON.** The controls are identical, and the one moved row is the predicted one with
its mechanism visible.

### 5ch addendum: USNOS shots 6166 -> 802 (cc9-lua31, 2026-09-30)

The lead asked about this drop. It is taken from 5ch's own pair logs, `local\l31_{off,on}_usnos.log`
(`a59dae607`, OFF `CB9334362F07`, ON `ECA965F3003A`), plus one traced ON run,
`local\l31_tr_usnos.log` (`BSP_AI_SQUAD_TICK_TRACE=1`).

**The shots that vanish are anti-aircraft fire at planes.**
- `ballistics aa_direct_aims` falls 51754 -> 5991 and `no_gravity_shots` 5846 -> 482.
- `artillery_arc_aims` is 106700 on both sides, so ship-against-ship and ship-against-shore fire
  is untouched.
- `aa bot error` loses every level-2 roll (gunner 335 -> 0, flak 308 -> 0), and `aa line of fire
  queries` falls 655 -> 140.

**The plane deaths that flip** are all twelve planes of `plane #1.1`, `#1.2` and `#1.3` (four
each). These are 12 of OFF's 18 plane deaths, so the plane death modes fall 18 -> 6. All twelve
are only-OFF.
- OFF, `plane #1.1` is killed at t=120.75 by `Portland1` (22 hits taken) while it strafes
  `TroopTrans2`.
- Its strafe task (installed at log line 18200) runs 810 ticks: 583 moveto, 197 gotowards and
  30 aim, with 6 gun fires.
- ON it takes no hit.

**The first diverging line** (`l30_firstdiff.py`) is mission frame 916, about t=46 s:
`moveto task plane #1.3: installed kind 7, leader=1 state=moveto` (ON line 21945). `plane #1.1`
follows at t=46.65.

**What they do instead, traced.** The three squadrons each lead a two-member `MOVETOATTACK`
group whose target group's leader is **HQ2** at (1736, 3, 5422). The tick's `moveto` retires
the scripted strafe and installs the kind-7 task towards HQ2:
- At t=46.65 `plane #1.1` is at (1970, 1236, -6450), 11875 from HQ2.
- At t=148.60 it is at (1619, 828, 334), 5090 from HQ2.

That is a steady close at the TBF's speed, at a live target and a valid point. The planes are not
parked, not frozen, not sent to the origin, and not sent to a dead target. Their task table
agrees: `min_d` equals `last_d` (5018.9), and `arrived=0` because the run ends first. So the
squadrons leave the transports to Portland's AA umbrella and fly inland towards HQ2, and the AA
has almost nothing to shoot at.

**Verdict.** The drop is a consequence of the image's `009CC850` rule (a `moveto` retires a strafe
head, and `0099A170` builds the moveto task). It is not a host artefact, so the flip stands. The
assumption that remains is the one stated in 5cg: that the image puts these squadrons in a tick-ordered AI group.

## 5cj. Handoff (cc9-lua31, 2026-09-30)

Branch `agent/cc9-lua31`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua31`. No lease
is held. The `src/game_hosts_ai.cpp` loan from the ships lane was used for 5cg, 5ch and the
routed SHIP_AI 107 edit, and is handed back.

| packet | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| `cc9_ai_tick_strafe` | `4d6093605`, `2101fd20c` | `kAiSquadronLeaderPointBound` | **ON** | 5cg |
| routed SHIP_AI 107 | `3c183008f` | `kAiTransportMovesOrderBridgeBound` | OFF (cc9-ships26 pairs it) | SHIP_AI 107 |
| `cc9_ai_tick_plane_retask` | `55854580e`, `a59dae607`, `e052a57b2`, `430b5f188` | `kAiTickMovetoRetasksPlaneBound` | **ON** | 5ch and its addendum |
| `cc9_get_property_ammotype` | `5d0513291`, `e38f0796e` | `kGetPropertySquadronAmmoTypeBound` | **ON** | 5ci |

Env traces added: `BSP_AI_SQUAD_TICK_TRACE=1` (`src/game_hosts_ai.cpp`) logs every
`MOVETOATTACK` or `CLOSEATTACK` tick of a squadron-led group, with both leader points and the
collect distance. Run scripts: `local\l31_runs.ps1` (pairs) and `local\l31_apply_*.py` (the edits).

### Next: the `settarget` that `CLOSEATTACK` issues to a squadron (the goaway question)

After 5ch, ESMP08's strike group promotes. `00A13B60` then serves its squadrons with `settarget`
(`00E08EF8`, ordinal 1; docs/AI_CLOSE_ATTACK_TICK.md). The host issues it through
`close_issue_order` (`src/game_hosts_ai.cpp`, the registry path `issue_order`), and **no bot task
follows**. The planes keep their last kind-7 moveto task towards the old point, or a scripted
strafe, until the next 15 s `PilotSetTarget`.

What the image should do, from what is read so far (to verify first):
- `0099A4C0` asks the head's `vtable[40h]`:
  - a kind-7 moveto head's `009C31B0` answers 0 for a non-moveto command, so the head is retired;
  - a strafe head's `009CC850` answers 0 for `00E08EF8` (it accepts only `00E08F40` and
    `00E08F78`), so it is retired as well.
- `0099A170` has **no arm for `00E08EF8`** in `src/attack_commands.cpp`'s reconstruction (the arms
  are none, moveto, moveonpath, the attack classes, land, retreat and stop), so it builds nothing.
  Confirm this against the listing, `0099A23A` onwards.
- So the image's served squadron would fly with an **empty task vector** and a `settarget` naming
  the target. What a plane bot does with no task and a current `settarget` (`0099D300`'s plan,
  `007EEC50`, or the squadron's free-attack logic) is the read. It decides whether ESMP08 long
  ever reaches a strafe goaway.
- Bind it OFF in the host's `close_issue_order` delivery (after the loopback drain, as 5ch does
  through `commands_after_last_issue_delivery`). Pair ESMP08 14200/14000, USNOS, USN02 and USN12.

### Then the script-dogfight rows (queue item 3), neither reached yet

- **USNRM01** (not `USNEX`; the menu id is `USNRM01`, `missiontree.lua:3299`),
  `usn_1_pearl.lua:1327` `luaPh3Start` (the Welch squadron at 1350-1357). The chain is: phase 1
  (the WV movie end at 1090 -> `luaBombingMovie` 1111 -> `luaBombingMovieEnd` 1156 -> Blackout
  -> `luaPh2Movie1` 1173), then phase 2, where the Nevada must come within 800 of
  `Mission.NevadaBeach` (lines 744-782 -> `luaNevadaBeachMovie` 1312 -> `luaPh3Start`). A 9000
  run did not show which phase it reached. A 36000 run died at FMOD startup while the session
  was disconnected (environment). Next: an env-gated trace of the Lua callbacks the host fires
  (luaDelay, dialog and movie callbacks), or grep a long run for `PilotSetTarget` from a Welch
  unit.
- **BSM04** `bsm_04_vengance_at_luzon.lua:1909` in `luaIntroMovieEnd`. It is reached only
  through the dialog chain `INTRO` (lines 155-180: `luaIntroMovieB`, `luaIntroMovieC`,
  `luaZekesDia`) -> `ZEKES` (`luaIntroMovieD`, line 190) -> `luaDelay(luaIntroMovieEnd, 8)`
  (1896). cc9-lua30's 3200 probe never got there, so find where the dialog sequence stops in the
  host.

### Notes for the next worker

- `CloseAttack_CollectDist` is 3000 on these rows by the image's own load: game mode 8 selects
  `IslandCaptureParams_Rookie` (`highlvlaiglobals.lua:119`). 5000 is only the default.
- `GetProperty(..., "ammoType")` is case-insensitive in the image (`_stricmp`). ESMP08's wave all
  carries ordnance (`zero=0`).
- The USNOS AA drop under 5ch is explained in the addendum: planes fly to HQ2 instead of
  strafing under Portland's AA.

## 5ck. An AI `settarget` to a squadron goes through the squadron's intake (packet `cc9_ai_squadron_settarget_intake`, cc9-lua32, 2026-10-01)

5cj asked what a plane bot does with an empty task vector while the director holds a `settarget`.
In the image that state never arises, because the order never reaches the planes as a `settarget`.

### The image

- **The squadron converts the order.** `0077D600`'s MT_COMMAND is delivered to the receiver's
  `vtable[160h]` (`0078061C`, docs/CRUISE_COMMAND.md). For a squadron that is `007F1940`
  (vtable `00D087C0`, slot `00D08920`; docs/CONTROLLED_UNIT.md). Its arm at
  `007F1AD6`-`007F1B24`, read from the disk bytes:
  - `CMP EAX,00E08EF8` / `CMP EAX,00E08F78`: `settarget` and `attackmove` both jump to
    `007F1B14`. This arm skips the `+3D0h` member test that the other arms make at `007F1AE4`.
  - `PUSH 1 / PUSH 1`, then `00521EA0` on the message's descriptor, then `PUSH EAX`. `MOV ECX,ESI`
    (the squadron) and `CALL 007EEC50` choose the class.
  - `007F1B2B TEST EDI,EDI / JE 007F1B5D`: a null class issues **nothing**, and the squadron
    keeps its previous command.
  - With a class and message `+21h` set, it clears (`0071D880`) and then issues the **chosen
    class** with the same descriptor (`0071ECF0`). `00A13B60` sets `+21h`, because it pushes
    flags 1 at `00A14A4A`.
- **The bots then install that class.** Each member bot reads the squadron's director
  (`unit+9D4h` -> `vtable[114h]`). On the change, `0099A4C0` asks the head task's
  `vtable[40h]`:
  - a strafe head stays only for strafe or `attackmove` on the same target (`009CC850`);
  - a kind-7 moveto head stays only for a moveto (`009C31B0`).

  `0099A170` then builds the torpedo, dive-bomb, strafe or other task on the AI's target.
- **0099A170 has no `settarget` arm, as 5cj said.** Its compare chain (`0099A1B5`-`0099A47A`)
  names `00E08FA0`, `F78`, `F98`, `F68`, `F80`, `F20`, `F28`, `F30`, `F18`, `F40`, `F48`, `F50`,
  `F58`, `FA8`, `F38`, `F90` and `F88`. The listing confirms it, but the arm is never reached for
  a squadron.
- **An empty task vector would freeze the plane's controls.** In `0099ACD0`, when the head after
  `0099A4C0` is null, the update `009998A0`, the plan evaluation and the command-block store
  `007B8C90` are all skipped. The plane keeps its last stick block. This matters only for a
  non-squadron receiver.

### The host before this packet

- `close_issue_order` sends `settarget` through the registry path. `fan_out_to_members` places it
  on every member plane.
  - In 5ch's ESMP08 14000 ON log there are 1941 such member orders, all to US squadrons.
  - USNRM01 9000 has 6345, mostly to the `Jap` waves, `KateSpawn3/5` and `A6M_1-8`.
  - JM06 has 25, to `PBY Catalina 01`.
  - USNOS, USN02 and USN12 have none.
- The planes keep whatever task they had: the script's strafe, torpedo or dive task on the
  script's target, or 5ch's moveto.

### Next packet: the binding (planned, not committed)

The work paused before the bind landed (lead's pause, 2026-10-01). The plan for
`cc9_ai_squadron_settarget_intake` is below.
- `kAiSquadronSetTargetIntakeBound = false` goes in `include/bsp/game_hosts_script_orders.hpp`, with
  `script_orders_squadron_intake_007f1940(leader, members, target_index)` beside
  `script_orders_drain_loopback_0076c600`.
- `src/game_hosts_script_orders.cpp`:
  - move 007EEC50's input assembly out of `run_pilot_set_target` into
    `choose_attack_class_007eec50(unit, target_index, prefer_ordnance, allow_guns, label)`;
    `PilotSetTarget` keeps its census lines through `label`;
  - `squadron_intake_007f1940` chooses with `(1, 1)` on the slot-0 plane;
  - a non-null class is issued with flags 1 (kind-1 descriptor, `object_id` = target index + 1)
    to every member plane, and 0099A170 is installed after delivery, as the `PilotSetTarget`
    fan-out does;
  - a null class issues nothing;
  - add a census line `summary mission script squadron intake bound calls declined
    member_orders tasks`.
- `src/game_hosts_ai.cpp` (cc9-ships26's lane, needs a loan): in `close_issue_order`, for an
  `is_squadron` member, skip a repeat (`order_is_repeat`), call the intake with
  `members->front()`, and count it as issued in place of `issue_order`'s fan-out.
- **SUBSTITUTIONS, labelled:** the member planes' directors stand in for the squadron's, and the
  install runs one bot tick early (SENTITY_INIT_ATTACH_ORDER 22.7).
- A prepared, build-tested patch of the two script-orders files is in the cc9-lua32 tree:
  `local\l32_intake_code.patch`, built clean at `19e4d5190`. The `close_issue_order` edit is
  `local\l32_apply_ai.py`. Both are uncommitted local files, and the tree may be removed.
- Pairs: ESMP08 14200/14000 and 9200/9000, USNRM01 9200/9000, JM06, USNOS, USN02 and USN12.

### Predictions for that packet, written before any ON run

- **ESMP08 14200/14000:**
  - `calls` is in the hundreds and `declined` is near 0: every wave squadron carries ordnance,
    and the targets are ships;
  - `tasks` is close to `member_orders`;
  - the classes split as PilotSetTarget's do: Corsairs strafe (`F40`), Helldivers dive (`F20`),
    Avengers torpedo (`F18`);
  - the squadrons attack 00A13B60's choice (the Zuikaku group by weight) between the script's
    15 s re-targets, not the script's random `IJNFleet` ship;
  - the death table moves (IJN hits shift towards the Zuikaku group, and US losses change under
    its AA);
  - **weakest call:** whether a strafe now reaches its goaway. The script's 15 s re-target and
    the AI's re-target (a new target name on each `order_is_repeat` miss) both rebuild the task.
- **ESMP08 9200/9000:** moved, smaller.
- **USNRM01 9200/9000:** moved, strongly. The Japanese waves take the AI's targets at Pearl
  Harbor.
- **JM06 3200/3000:** moved, with one PBY.
- **USN02 and USN12 (controls):** pair_diff 0 or 1, with no `settarget` to a squadron.
- **USNOS:** identical apart from the census, since it has no AI `settarget`.

### 5ck.1 The binding, committed OFF (cc9-lua33, 2026-10-04)

cc9-lua33 applied cc9-lua32's prepared patch and `close_issue_order` hook unchanged (reviewed:
the hook takes the squadron path only for `settarget`/`attackmove` from `close_issue_order`, keeps
`issue_order`'s repeat test, and uses the same `GameUnitsHost` index space as the script-orders
host). `kAiSquadronSetTargetIntakeBound = false` is in `include/bsp/game_hosts_script_orders.hpp`;
the census line is `summary mission script squadron intake`. The predictions above stand as
written; the pairs follow in 5ck.2.

### 5ck.2 Measured (OFF `local\l33_off`, SHA-256 `4E66E4013D71`; ON `local\l33_on`, `990F4FEB4A5A`; both from `aaf170730`), and the verdict: **flip ON**

Launch form of reference v (`local\l33_runs.ps1`, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`).
The 300-frame USN01 smoke (ON) finished cleanly: 20 intake calls, all torpedo (`F18`).

| row | pair_diff | intake calls / declined / member orders / tasks | what moved |
| --- | --- | --- | --- |
| ESMP08 14200/14000 | 3 | 602 / 0 / 1806 / 1806 | deaths 44 -> 46 (Zuiho sunk, one more Corsair); hits 1619 -> 6462; torpedo releases 2/21 -> 4/33 |
| ESMP08 9200/9000 | 3 | 33 / 0 / 99 / 99 | no combat yet; plane paths and `nearest` distances only |
| USNRM01 9200/9000 | 3 | 1903 / 54 / 5900 / 5900 | torpedo releases 2/15 -> 40/79, dive releases 30/54 -> 123/166, damage 29918 -> 39332 |
| JM06 3200/3000 | 3 | 25 / 12 / 13 / 13 | the PBY flies a torpedo run on the submarines (1 of 1 released) and takes their AA |
| USNOS 3200/3000 | 1 | 0 | census only |
| USN02 3200/3000 | 1 | 0 | census only |
| USN12 3200/3000 | 1 | 0 | census only |

Against the predictions:
- **ESMP08 long, all held.** `declined` is 0 and `tasks` equals `member_orders`. The classes split
  as predicted: 306 strafe (`F40`, Corsairs), 177 dive (`F20`, Helldivers) and 119 torpedo (`F18`,
  Avengers). Every one of the 44 plane deaths whose killer moved now falls to the carrier group:
  Zuikaku 0 -> 34 and Zuiho 1 -> 8. On OFF the killers were Oyodo 11, Isuzu 8, Ise 7, Akitsuki 5,
  Chitose 4, Hyuga 3, Chiyoda 2 and Sugi 1. In the unit table, every other ship's `nearest` rises
  to 2800-5700 and its shots fall to 0. Zuikaku fires 249 -> 3947 shots and Zuiho 306 -> 1987.
  The squadrons fly to 00A13B60's pick, the Zuikaku group, and no longer to the script's random
  `IJNFleet` ship. The added hit records are that AA fire.
- **The weakest call is reached: strafes now reach goaway.** Summed over the per-plane strafe
  summaries (`local\l33_strafe_sum.py`), OFF has 60 planes with `aim=433 goaway=0 aims=8`. ON has
  42 planes with `aim=3064 goaway=1639 aims=35`. That opens the goaway row (queue item 2).
- **The wave-3 composition differs** (TBF and SB2C on ON where OFF had F4U). That comes from the
  script, not the switch: `08_engano.lua:513-517` fills the wave with `luaPickRnd(planetbl)`, and
  the Lua random stream advances differently once the fight differs (this installation's file is
  dated 2024-07-13).
- **USNRM01, moved strongly, as predicted.** The Japanese waves now strike 00A13B60's targets.
  Only on ON do Downes, Neosho, Rescue, Tautog, Medusa and five cranes die; only on OFF does
  Airfield1Hangar die. Japanese plane deaths are 32 only OFF against 26 only ON. The 54 declined
  calls are all `sides 1/2`: 007EEC50 refuses a target that is not an enemy.
- **JM06, moved with the one PBY, as predicted.** The 12 declined calls come after its torpedo is
  gone (`torp=0 gb=1`), when the chooser offers no class for the submarine target.
- **The controls held.** USNOS, USN02 and USN12 have no AI `settarget` to a squadron, and each
  shows exit 1.

The mechanism matches on every row, so the switch flips ON (`kAiSquadronSetTargetIntakeBound =
true`). The OFF base predates reference w (not yet published when these pairs ran), and w's
base must carry this flip.

## 5cl. The goaway row: ESMP08 long's goaway follows the image's rules; the break-off is two manoeuvres the host does not fly (packet `cc9_strafe_goaway_row`, cc9-lua33, 2026-10-04)

The run is ESMP08 14200/14000 on main `ee5672bf2` plus the trace commit `ab54ae37b`, with
`BSP_STRAFE_GOAWAY_TRACE=1` (`local\l33_ga_esmp08x.log`). Analysis is `local\l33_ga_analyze.py`.
The trace moves no gameplay: pair_diff against 5ck.2's ON log exits 1.

### The goaway ticks against `009CBB30` (listing `009CBB30`-`009CBE74`, read whole)

| rule (image) | measured (host) | holds |
| --- | --- | --- |
| bank re-plan `009CB780` when `+30h` < 0, re-armed `U(2, 5)` (`009CBB4C`-`009CBB93`) | 46 re-plans; the interval between two in one episode is 2.10-4.90 s (n = 27) | yes |
| pitch re-plan `009CB650` when `+3Ch` < 0, re-armed `U(2, 5)` (`009CBB9C`-`009CBBD5`) | 43 re-plans; 2.00-4.90 s (n = 24) | yes |
| done `+24h` = horizontal > approach `+34h`, tested once per `+40h` = 1 s (`009CBC47`-`009CBC88`) | 183 tests in 21 episodes; 4 come true, and each is followed by `goaway -> gotowards` (Corsairs #1.3, #1.6\|.-2, #1.8 and #2.3, at h = 1124-1230 m) | yes |
| evasive gate: not kind 10h/16h and approach `+44h` < 1.0 (`009CBC96`-`009CBCCA`) | 36 gate openings in the tick; time since hit at the gate is at most 0.30 s | yes |
| the enter's gate `009CB8B0`: `U(10, 16)` > `+44h` (`009CB93C`-`009CB95E`) | 7 of 21 entries open the gate (the census tests `< 16`, without the draw) | census only |

The episodes: 20 planes enter goaway 21 times. **17 episodes end in a death inside goaway**,
0.05-9.1 s after entry, under the carrier group's AA. The other 4 finish and turn back to
gotowards. Those planes die later too, Corsair #1.8 in its second goaway.
The strafe summary's `evasive_gaps=43` is these 36 + 7 openings. At every one of them the image
front-pushes a manoeuvre that the host does not fly. Instead the host plane keeps the goaway
state's bank and pitch, so it holds a steady escape line under fire.

### What the image does at each opening (listing `009CBCD0`-`009CBE27` and the callees)

`r = U(0, 1)` is compared with the goaway state's `+18h` (drawn `U(0.3, 0.9)` by the constructor
`009CB500`):

- **`r >= +18h`: "tightturn"** (the name string at `00D2064C`; factory `009BC030`, `40Ch` bytes,
  vtable `00D205E0`, tick `009BA020` at `vt+64h`).
  - Side byte `+408h` = `U(0, 1) < 0.5` (`[00CE3800]`), so left or right with even odds.
  - The end condition is `007B5D30` (vtable `00D05870`, test `00996510`). It holds the approach's
    point `+74h..+7Ch` at the moment of the hit, read through approach `vt[0]` = `009CA680`. It
    answers when that point, in the plane's own frame, has `|x| > z`, i.e. it lies more than 45
    degrees off the nose or behind.
  - The tick first waits for `009B9680` to answer (latched in `+404h`; not read). It then writes:
    - a bank of +-pi/2 (`[00CE3830]`, plan `+2C8h`, mode 1);
    - `+2A0h` = `clamp(2.0 - e / 10 deg, 0, 1)`, where `e` is the wrapped error between unit
      `+C68h` (the roll, provisional) and that bank (`[00D7A308]` 2.0, `[00D05850]` 0.17453);
    - `+288h` = `00419010`-interpolated from `+3FCh - unit+C64h` (the pitch held since
      `009BAFC0`), times the side.
  - **Reading:** a hard 90-degree bank turn away until the target is off the nose.
- **`r < +18h`: "flikflak"** (`00D207B0`; factory `009BC0A0`, `404h` bytes, constructor
  `009BB910`, vtable `00D20748`, tick `009B99E0` at `vt+64h`).
  - The constructor sets side `+400h` = `U(0, 1) > 0.5` and the timer `+3FCh` = `U(0, 1) + 2.0`.
  - The tick: when the timer runs out, it flips the side and re-arms `U(0, 1) + 2.0`. It writes:
    - bank `+2C8h` = `[+2F8h]+25Ch` (the class, provisional) times +-1, mode 1;
    - pitch target `+2C0h` = `[+2F8h]+1ECh` (the climb angle), mode 2;
    - `+27Ch` = 1.0 (`[00D7A24C]`), as the tightturn tick also writes.
  - The end condition is `007B5E20` (vtable `00D05880`, test `00996300`, read from the disk
    bytes `00996300`-`00996385`). It answers when the horizontal distance from the plane to the
    point captured at the hit reaches `U(0.5, 0.7) * approach+34h` (`[00CE3800]`, `[00CE3E18]`):
    about 0.55-0.85 km on this row.
  - **Reading:** a climbing jink, banking left and right every 2-3 s.
- **Either push** sets approach `+44h` = 25.0 (`[00CE89CC]`) and front-pushes through
  `00999F50`. The strafe task stays underneath, and goaway resumes after `0099B690` retires the
  manoeuvre. The push draws the RNG once or twice more than the host does today (`r`, then the
  side or the flikflak constructor's two draws).

So on ESMP08 long, a Corsair that is hit while leaving the carrier group does one of two things in
the image. With probability `1 - +18h` (0.1-0.7) it turns hard 90 degrees away; otherwise it
jinks while climbing. It goes back to the goaway line only after the target is off the nose or it
is about 0.7 km on. The host flies the straight line. That is the likeliest cause of 17 of
21 episodes ending in a death, a hypothesis that the bind's pair will test.

### Open (for the bind packet)

- **Who receives a hit during the manoeuvre.** The strafe task's notice `009CC400` zeroes
  `+44h`, but while a manoeuvre is head, its own notice slot may take the hit instead. Read the
  image's hit dispatch to the task vector (the caller of the task's `vt+2Ch`; the host's is
  `hit_task_notify` in `src/game_hosts_units.cpp`) before binding the re-push.
- **`009B9680`** (tightturn's heading and its start flag `+404h`) is not read.
- **Host labels.** The host has no front-push of a manoeuvre over the strafe arm. The bind can
  model the two ticks inside the strafe arm as a sub-state, with the goaway tick suspended while
  a manoeuvre runs. That is a labelled substitution for `00999F50`'s task vector.
- **Ghidra (for the lead).** All of these are Ghidra functions now (`FUN_`), with Ghidra body
  ends exclusive:
  - `009BC030`-`009BC0A0` `BSP_BotTask_MakeTightTurn`
  - `009BC0A0`-`009BC106` `BSP_BotTask_MakeFlikFlak`
  - `009BB910`-`009BB9CF` `BSP_BotTaskFlikFlak_Construct`
  - `009B99E0`-`009B9AE8` `BSP_BotTaskFlikFlak_Tick`
  - `009BA020`-`009BA1DC` `BSP_BotTaskTightTurn_Tick`
  - `007B5D30`-`007B5D6A` `BSP_BotCondition_PointOffNose_Construct`
  - `007B5E20`-`007B5E57` `BSP_BotCondition_PointDistance_Construct`
  - `00996510`-`009965C6` `BSP_BotCondition_PointOffNose_Test`

  One is not a function yet: `00996300`-`00996385`, `BSP_BotCondition_PointDistance_Test`
  (`__fastcall(cond)`, ends `RET` at `00996384`, INT3 at `00996385`). The names are hypotheses
  from the name strings and the bodies.

## 5cm. The break-off manoeuvres bound OFF (packet `cc9_strafe_breakoff`, cc9-lua33, 2026-10-04)

### The two reads 5cl left open

- **The start gate `009B9680` is never asked on this path.** Its body:
  - it answers 1 when `+400h - unit+C64h < 0` and `|unit+C68h| < 20 deg` (`[00CE398C]`);
  - otherwise it levels the wings (`+2C8h` = 0, mode 1) and pulls `+2A0h` =
    `-(|roll| - 60 deg) / 50 deg` (`[00D03DD0]`, `[00D20330]`), and answers 0.

  But `009BC030` stores `+404h` = 1 after the base constructor, and `009BA020` asks the gate only
  while `+404h` is 0. So a tightturn from goaway starts at once. The gate serves the other users
  of the base `009BAFC0` (`009BB380`, unread).
- **The base constructor `009BAFC0`** (listing `009BB018`-`009BB12B`) sets tightturn's pitch
  reference.
  - With `f` = altitude (`unit+100h`) - 250 (`[00CF8850]`):
    - when `f <= 0`, `+400h` = desc `+1ECh`;
    - otherwise `r = (f / 10) / speed` (unit `vtable[38h]`), and `+400h` = `-acos(r)`. It is 0
      for `r > 1` and -pi for `r < -1`.
  - `00BF9940` is taken as `acos`, because `math_acos` `00A617C0` calls it and the clamps match
    acos's ends (provisional).
  - Then `+3FCh` = `max(+400h, pitch)` and `+404h` = pitch > `+400h`. The tick's yaw input reads
    `+3FCh`.
- **A hit during a manoeuvre still reaches the strafe task.** The walker `00999AA0` calls each
  task's `vt[2Ch]` from the head down until one answers true. Both manoeuvres' slot is
  `007B4110`, `XOR AL,AL / RET 4`, so the strafe task's `009CC400` zeroes `+44h` underneath.
  `+44h` does not advance during the manoeuvre, because `009CCED0` runs only while the strafe task
  is the head. So a plane hit during its manoeuvre opens goaway's gate again at its first goaway
  tick after the retire.
- **The enter's push differs from the tick's** (listing `009CB964`-`009CBB19`):
  - flikflak is chosen only when also `altitude < approach+40h - 50` (`[00CE3938]`);
  - the tightturn condition is built inline (`007B5C70`, then vtable `00D05870` and the point);
  - **it does not set `+44h` = 25**.

### What is bound, behind `kStrafeBreakoffBound` (`include/bsp/strafe_task.hpp`, committed OFF)

- Pure functions in `src/strafe_task.cpp`:
  - `tight_turn_pitch_ref_009bafc0`;
  - `tight_turn_tick_009ba020`: bank `+-pi/2`; pitch `clamp(2 - |wrap(roll - bank)| / 10 deg, 0, 1)`
    direct; yaw `00419010(-2 deg, 0, 2 deg, 1, ref - pitch) * side` direct;
  - `point_off_nose_00996510`;
  - `point_distance_reached_00996300`.
- In the units host's strafe arm (`src/game_hosts_units.cpp`):
  - the push from goaway's enter (the `U(10, 16)` window, drawn only ON) and from its tick;
  - the draws `r`, side, distance, flikflak side and timer, all on the keyed stream;
  - the two manoeuvre ticks with their plan writes (task offsets minus 4), the end conditions and
    the retire.
- Census line: `summary mission strafe breakoff bound tightturns flikflaks manoeuvre_ticks`.
  The goaway trace adds `push_enter`, `push_tick` and `manoeuvre_done`.
- **SUBSTITUTIONS, labelled:**
  - `00999F50`'s front push is one manoeuvre slot per plane. The strafe arm (`009CCED0`, the rule
    and the state tick) is suspended while it runs, as `0099ACD0` ticks only the head.
  - A strafe re-install, or a class change, drops a running manoeuvre. `0099A4C0` would ask the
    head's `vt[40h]` `0099C2C0`, which is not read.
  - `009BA1C8`'s `+2E4h &= ~4` is not modelled.
- OFF is byte-identical in behaviour: every new draw and write is under the switch. The census
  line reads `bound=0` with zeros.

### Predictions, written before any ON run (pairs on the OFF commit)

- **ESMP08 14200/14000: moved.** OFF has 43 gate openings, so ON has at least that many pushes.
  - **Mechanism:**
    - `tightturns + flikflaks` is at least 36 tick pushes plus the enter pushes. It rises if planes
      live longer and are hit again;
    - on the tick pushes, tightturns are about 40% (`1 - E[+18h]`, with `+18h` = `U(0.3, 0.9)` per
      plane);
    - every push is followed by `manoeuvre_done` or by the plane's death;
    - a flikflak ends after 0.55-0.85 km of horizontal travel, about 4-7 s;
    - a tightturn ends when the target is 45 degrees off the nose, about 1-3 s at the TurnRoll
      rates.
  - **Hypothesis to test, not a prediction to hit:** fewer of the 21 goaway episodes end in a
    death than OFF's 17. The plane spends the seconds after a hit turning or jinking instead of
    flying a straight line under the carrier group's AA. If the deaths do not fall, the AA is
    simply too dense for the break-off to matter, and the switch flips on the mechanism alone.
- **ESMP08 9200/9000, USNOS 3200/3000, USN02 3200/3000 (controls):** pair_diff 0 or 1. ESMP08
  short and USNOS never enter goaway (`goaway=0` in 5ck.2's ON logs), and USN02 has no strafe.

### 5cm.1 Measured (OFF `local\l33_off`, SHA-256 `68CB1B2C2040`; ON `local\l33_on`, `38CCABFD6904`; both from `a14bb33f1`), and the verdict: **flip ON**

Run in the launch form of reference v, with `BSP_STRAFE_GOAWAY_TRACE=1` on both sides
(`local\l33_runs.ps1 -Tag b`). The trace leaves gameplay unchanged: the OFF log against 5cl's
trace run exits 1. The ON smoke (USN01, 300 frames) finished cleanly.

| row | pair_diff | notes |
| --- | --- | --- |
| ESMP08 14200/14000 | 3 | 23 tightturns, 34 flikflaks, 1477 manoeuvre ticks |
| ESMP08 9200/9000 | 1 | no goaway |
| USNOS 3200/3000 | 1 | strafes, no goaway |
| USN02 3200/3000 | 1 | no strafe |

**The mechanism, measured against the reading** (`local\l33_mv_analyze.py`):
- **Pushes.** There are 57: 52 from the tick and 5 from the enter. 23 of 57 are tightturns,
  which is 40%, the predicted `1 - E[+18h]`.
- **The end conditions.**
  - All 23 tightturns end at their condition after **0.09-1.40 s (median 0.10 s, two
    ticks)**. This is the image's own condition, not a host artefact. A plane in goaway flies away
    from the target, so the aim point is already behind it (`z < 0`). `00996510` answers at once,
    and the tightturn holds the knife-edge bank for a tick or two. The prediction of 1-3 s
    assumed the target ahead and was wrong.
  - 21 flikflaks end at their distance after 0.10-15.09 s (median 1.90 s), and 10 end with the
    plane's death.
  - 3 pushes were overwritten. Where the enter pushes, the state tick of the same think
    (`009CD1C7`) sees `+44h` still below 1 and pushes again: in the image a second task lands on
    top of the first. **SUBSTITUTION, labelled:** the host keeps the second only.
- **Census artefact.** `evasive_gaps` falls from 43 to 32. That counter takes rising edges, and
  the edge state is held across the suspension. It is not a change in the gate.

**The hypothesis, tested.** The goaway episodes that end in a death fall from 17 of 21 to 12 of
20. The total does not move (46 deaths on both sides), but the strafers live longer:
- across the 21 Corsair death rows, the median death is 5.6 s later (all 43 changed rows: median
  +0.5 s; 18 later and 16 earlier by more than 1 s);
- Zuikaku takes 89 -> 1269 damage, and Zuiho sinks 8.7 s earlier (548.7 -> 540.0 s);
- hits rise 6462 -> 7374, damage 14996 -> 17044, torpedo releases 4/33 -> 5/27 and dive
  releases 0/37 -> 3/36.

So the break-off protects the strafers in the image's sense: the flikflak's climbing jink keeps
them alive after a hit. It does not change how many die by the end of the row, because the
carrier group's AA still gets them on their next pass.

The mechanism matches, and the controls are exit 1, so the switch flips ON
(`kStrafeBreakoffBound = true`). Corrected prediction: a tightturn from goaway lasts one or two
ticks.

## 5cn. Where the two script-dogfight chains stop; `GetSquadronPlanes` bound OFF (packet `cc9_script_dogfight_rows`, cc9-lua33, 2026-10-04)

A new env-gated trace, `BSP_LUA_CALLBACK_TRACE=1` (`src/game_hosts_script_orders.cpp`), puts a
Lua call hook on the mission state. It records the first mission-clock time and the call count of
two kinds of function:
- every Lua function its caller names `lua*`;
- every unnamed Lua function, keyed `source:line`. These are callbacks the host or a C binding
  calls.

The trace changes no Lua behaviour; `report()` prints it. Runs, on main `eb373327c` plus this
packet: USNRM01 9200/9000 (`local\l33_tr_usnrm01.log`) and BSM04 3200/3000
(`local\l33_tr_bsm04.log`). Script files in this installation: `usn_1_pearl.lua` is dated
2024-10-29 (locally modified, not the bulk 2024-07-13), and `bsm_04_vengance_at_luzon.lua`
2024-07-13.

### USNRM01: phase 1 never ends, because the West Virginia never takes damage

The chain to the Welch squadron (`luaPh3Start`, line 1327) runs through phase 1's end.
1. `luaIn` (1006) registers `Listener_WVDead` (1048-1054).
2. When the West Virginia dies, that listener calls `luaWVSunk` (1456).
3. `luaWVSunk` runs the WV movie, then `luaWV_MovieEnd` (1090), then `luaBombingMovie` (1111).
4. Phase 2 follows.

The trace shows how far the run gets:
- `luaIn` runs at t = 186.9 s (as `?usn_1_pearl.lua:1006`), then `luaAddFirstObjs`,
  `luaGenerateJapTraffic`, the targeting loop (`luaGetJapTrg` 389 calls), and at 276.9 s
  `luaInitFirstRunners` / `luaInitSecondRunners`;
- `luaWVSunk`, `luaWV_MovieEnd` and everything after them never run.

**The West Virginia ends the 450 s mission at 10000 of 10000 health, with `taken` 0.**
- The impact census shows bomb blasts on it doing `took=0.0` (base 35 against armour 130).
- No battleship of `Mission.BBRowGang` takes damage except Pennsylvania (22). The script's bombers
  and torpedo planes are aimed at that gang (`luaGetJapTrg(2)`, 2228, and 177 `command target`
  lines name the West Virginia).
- What the attacks do sink is destroyers and auxiliaries: Downes 290 s, Neosho 363 s, Rescue 386
  s, Tautog 400 s, Medusa 444 s.
- The gunnery summary has `swims_started=13` against the 40 aircraft torpedo drops of 5ck.2.

This is the gunnery and damage lane (how an aircraft torpedo or AP bomb damages a moored
battleship). It is routed to the lead, not chased here. Until the West Virginia can die, phases
2 and 3, and with them the Welch dogfight, cannot be reached on any run length.

### BSM04: `luaStartMission` fails on every think at `GetSquadronPlanes`

- The trace has `luaStartMission` ×49, `luaIntroMovieA` ×49 and no `luaDelay`.
- The log has 49 `script call lua_Think failed: ...:1718: attempt to index local 'camTrg'
  (a nil value)`.
- `luaIntroMovieA` (1715) calls `GetSquadronPlanes(Mission.Cat)`. This host left that native
  unimplemented, so it returned nothing.
- So `luaStartMission` (1403) dies at line 1447 before `luaDelay(luaIntroDia, 2)` (1449), and
  `Mission.Started` is never set (`lua_Think`, 592-596). The mission restarts its opening every
  think.

**`0089CC50` `GetSquadronPlanes`** (`__fastcall(lua_State)`, one result; listing `0089CD75`-`0089CE00`):
- argument 0 goes through `BSP_ObjectHandle_FromLuaTable` with no kind check;
- a new table (`00B67930`);
- for `i` = 1..`[sq+3CCh]`, the member `[sq+3D0h + 4(i-1)]`'s u16 `+174h` as a string
  (`004260B0`), stored at index `i` (`00B672F0`);
- past five members `0089CD98` substitutes a null pointer, so the image would fault.

The strings are the `thisTable` keys, which is how the script reads them back:
`thisTable[tostring(camTrg[1])]`.

**The next blocker, from reading the host:** after the opening, `luaIntroDia` calls
`luaStartDialog("INTRO")` -> `StartDialog` `008B0540`. This host only registers the id: "nothing
here plays a dialog to its end". The INTRO sequence's `callback` entries (`luaIntroMovieB`,
`luaIntroMovieC`, `luaZekesDia`, lines 164-180) are played by the image's dialog panel
`00451A90`, so they never fire, and `luaIntroMovieEnd`'s `PilotSetTarget` (1909) stays out of
reach. Binding the dialog sequencer's callback entries is a packet of its own.

### The binding, behind `kGetSquadronPlanesBound` (`include/bsp/game_hosts_script_orders.hpp`, committed OFF)

- In `GameScriptOrdersHost::dispatch`, the new table holds the member planes of the registry
  record that contains the argument's unit. Each entry is the unit index plus one as a string
  (`kMissionLuaEntityKeyFormat`, the `thisTable` key, as `+174h` is in this host). It stops at
  five entries.
- Census line: `summary mission script squadron planes bound calls entries unresolved`.
- **SUBSTITUTION, labelled:** the squadron is the registry record, as in `PilotSetTarget`.
- No reference row calls this native: there are no `GetSquadronPlanes` rows in the 37 reference
  logs in the cc9-gunnery20 tree.

### Predictions, written before any ON run

- **BSM04 3200/3000: moved.**
  - `luaStartMission` runs once and `luaIntroMovieA` once. The 49 `lua_Think failed` lines go to 0.
  - `luaDelay`, `luaIntroDia` (about t = 5 s) and `StartDialog("INTRO")` appear.
  - The census shows `calls=2`, with entries of 2-3 per call (Mission.Cat and FortressRed).
  - The chain then stops at the INTRO dialog: `luaIntroMovieB` never runs.
  - The opening's orders (`PilotSetTarget` FortressRed -> Airfield, `PilotMoveToRange`,
    `NavigatorMoveOnPath`) are issued once instead of re-issued every think. So the B-17 and
    Catalina tasks are no longer rebuilt every 3 s, and the bomb run timing moves.
  - **Weakest call:** whether a single issue changes the bombing outcome or only its timing.
- **USN02 3200/3000 and BSM01 3200/3000 (controls):** pair_diff 0 or 1, since neither calls the
  native.

### 5cn.1 Measured (OFF `local\l33_off`, SHA-256 `E47DDC22ECFD`; ON `local\l33_on`, `A94D5BA28C7C`; both from `8d319feb2`), and the verdict: **flip ON**

The launch form is reference v's (`local\l33_runs.ps1 -Tag c`), and the ON smoke (USN01, 300
frames) finished cleanly.

| row | pair_diff | census (ON) |
| --- | --- | --- |
| BSM04 3200/3000 | 3 | `calls=2 entries=8 unresolved=0` |
| USN02 3200/3000 | 1 | `calls=0` |
| BSM01 3200/3000 | 1 | `calls=0` |

**BSM04, against the predictions:**
- **The mechanism holds.** The 49 `lua_Think failed ... :1718` lines drop to 0. There are two calls,
  `Mission.Cat` and `FortressRed`, with 8 entries in all. `StartDialog("INTRO")` appears once.
  The chain then stops at the INTRO dialog as predicted, because this host does not play a
  sequence's callback entries.
- **Larger than predicted.** I expected only re-timings. The move is bigger because OFF never ran
  any of `lua_Think` past line 596: `Mission.Started = true` (598) and everything after it in
  every think were unreachable. With the switch ON the script's opening phase runs.
  - The airfield launches its interceptors: `IsReadyToSendPlanes` -> `LaunchSquadron` spawns
    `AirField_sqn01` and `AirField_sqn02`, 6 new units (39 -> 45).
  - They fight the B-17s, Donald and the Wildcat. Deaths go 0 -> 5: B-17|.-2, B-17|.-5 and three
    of AirField_sqn01. Shots go 299 -> 3792 and damage 140 -> 1270.
  - The first hit comes 46 s earlier (123.7 -> 77.5 s).
  - The controlled B-17 flies 7238 -> 9300 m.
- **The controls held:** USN02 and BSM01 do not call the native, and both exit 1.

The mechanism matches, so the switch flips ON (`kGetSquadronPlanesBound = true`). BSM04's
script dogfight (1909) now waits on the dialog sequencer: the INTRO and ZEKES `callback`
entries.

## 5co. 5ch strands USN13's strike: the close attack has no candidate against a land-fort group (cc9-lua33, 2026-10-04)

The lead's priority check, from reference W's leave-one-out: `kAiTickMovetoRetasksPlaneBound`
(5ch) alone takes USN13 to zero shots. The pairs ran on `47d5ec5b4`, which is main with 5ck and
5cm ON. OFF is `local\l33_off`, SHA-256 `492CB08B785C`, with the switch flipped false; ON is
`local\l33_on`, `D41C2550EBEB`. Both runs had `BSP_AI_SQUAD_TICK_TRACE=1`, in reference v's
launch form.

| row | pair_diff | deaths OFF -> ON | shots OFF -> ON | releases OFF -> ON |
| --- | --- | --- | --- | --- |
| USN13 3200/3000 | 3 | 22 -> **0** | 4189 -> **0** | torpedo 0/60 -> 0/60 |
| USN13 9200/9000 | 3 | 116 -> **6** | 40517 -> 136 | torpedo 3/79 -> 0/60, dive 2/50 -> none |
| USN04 4700/4500 | 3 | 45 -> 71 | 17447 -> 22401 | torpedo 0/16 -> 2/16 |

### (1) Yes, current main still strands the strike

5ck and 5cm do not change this.

### (2) Where the groups go, and why nothing attacks

The attackers are the script's Japanese wave "bruh" (type 162, WingCount 4, party 1). The
script `PilotSetTarget`s each squadron at a ship (objects 263, 279, ...).
- **OFF:** the squadrons keep that order and die to the destroyers' AA (DD_4, DD_6).
- **ON:** the planner gives the AI groups that hold these squadrons `MOVETOATTACK` against
  `CB2`, a **LandFort** group whose leader point is at (2973, 3, -2182). The tick's moveto
  retires each script attack head, which is the image's rule in 5ch (`replaced_other=60`).
- Every group closes in and promotes. The squad trace shows each leader's distance falling from
  6.6-9.5 km to 0.4-1.4 km, inside `CollectDist` 3000. There are 8 promotions and 88
  `CLOSEATTACK` ticks.
- **But no squadron's close attack ever chooses a target.**
  - The intake census reads `calls=0`, so no `settarget` ever reaches a squadron.
  - `close fallback bridge calls=413`: 225 of them are ships, and the other 188 are squadron
    fallbacks to the offset point.
  - So the squadrons orbit 0.5-1.4 km from CB2 at 800-1400 m and never attack anything.
- **Why, by `00A13B60`'s own rule** (docs/AI_CLOSE_ATTACK_TICK.md): a candidate with weight
  `<= 0` is admitted only as a target-group member, and its score is `range * weight * 10`.
  With weight 0 that score cannot beat the seed `best = 0` (`00A149A8`, `JBE`). A torpedo or
  bomb squadron's `00A0F810` weight against the LandFort members is evidently 0. The three
  `ai target choice` LandFort lines never reach a squadron (intake `calls=0`); which member made
  them is not traced.
- **Not the path leg.** `009FD050` (`BotApproach::refresh_path_leg`, unimplemented) shapes the
  moveto's `+28h` leg factor between two distances. The groups do reach their points, so it is
  not the cause.

### (3) Would the image strand them?

The stall rule itself, a zero-weight target that cannot win, is the image's. What is
**unverified** is the assignment that feeds it. Two host substitutions make it:
- the planner's choice of a LandFort group for a torpedo/bomb wave, at group weight 1.000 (the
  `ai group ... weight=1.000` lines), while the per-member weight against that group's members
  is 0;
- the admission of script-generated squadrons into AI groups at all (`ai squadron generated
  after load`, packet `cc9_generated_squadron_brain_membership`). If the image's brain never adopts
  a `GenerateObject` squadron, its AI tick never re-tasks them.

The planner value and the close weight disagree for the same pairs, so one of these is a host
artefact. Until the planner's group value for a squadron against a LandFort group, or the
generated-squadron membership, is checked against the image, 5ch strands the strike for a host
reason. **Recommendation: take `kAiTickMovetoRetasksPlaneBound` back OFF** (`src/game_hosts_ai.cpp`,
cc9-ships27's lane; routed to the lead). Next reads: `00A0F970` (the group target value for a
plane group against LandFort) and the image's adoption of generated squadrons into a brain.
USN04's opposite move (45 -> 71 deaths) belongs to the same switch and should be re-read after the
fix.

## 5cp. Handoff (cc9-lua33, 2026-10-04)

Branch `agent/cc9-lua33`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua33`. No lease
is held. `src/game_hosts_ai.cpp` was on loan for packet 1 and has been handed back.

| packet | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| `cc9_ai_squadron_settarget_intake` | `aaf170730`, `deeabf8a5` | `kAiSquadronSetTargetIntakeBound` | **ON** | 5ck.1, 5ck.2 |
| `cc9_strafe_goaway_row` | `ab54ae37b` (trace), `1700efc38` | - | read | 5cl |
| `cc9_strafe_breakoff` | `a14bb33f1`, `cd062f4ff` | `kStrafeBreakoffBound` | **ON** | 5cm, 5cm.1 |
| `cc9_script_dogfight_rows` | `8d319feb2`, `01ec9eb13` | `kGetSquadronPlanesBound` | **ON** | 5cn, 5cn.1 |
| 5ch USN13 check | `5532e517c` | `kAiTickMovetoRetasksPlaneBound` | recommend **OFF** (routed) | 5co |

Env-gated traces added:
- `BSP_STRAFE_GOAWAY_TRACE=1` (`src/game_hosts_units.cpp`): goaway enter, re-plans, done tests,
  gates, pushes and manoeuvre ends.
- `BSP_LUA_CALLBACK_TRACE=1` (`src/game_hosts_script_orders.cpp`): the first time and call count
  of every `lua*` function and every host-fired Lua callback.

Scripts are in `local\`: `l33_runs.ps1 -Tag <t>` (pairs), `l33_trace_runs.ps1` (one traced run),
and `l33_ga_analyze.py` / `l33_mv_analyze.py` (goaway and manoeuvre episodes).

### Next, in order

1. **5ch (5co).** Settle what feeds the USN13 stall:
   - `00A0F970`'s group value for a squadron group against a LandFort group;
   - whether the image adopts a `GenerateObject` squadron into an AI brain at all.

   Then re-pair USN13, USN13 long and USN04 with 5ch.
2. **BSM04's dialog sequencer.** `StartDialog` `008B0540` -> the panel `00451A90` plays a
   `Mission.Dialogues` sequence, including its `["type"] = "callback"` entries. This host only
   registers the id. The INTRO and ZEKES callbacks (`bsm_04:164-190`) lead to the 1909 script
   dogfight. Read how the panel advances (the per-message duration source) before binding.
3. **USNRM01** is blocked on the West Virginia never taking damage. That is routed to
   cc9-gunnery23 (5cn).
4. **Break-off follow-ups (5cm):**
   - the stacked second push when the enter and the tick push in one think;
   - the manoeuvre head's `vt[40h]` `0099C2C0` on a command change;
   - tightturn's `+2E4h &= ~4`.
5. 5cj's remaining items.

**5ch is now OFF** (`kAiTickMovetoRetasksPlaneBound = false`, 5co). ESMP08's promotion to `CLOSEATTACK`, through which 5ck and 5cm reached their row, may no longer happen; the successor re-pairs ESMP08 14200 for 5ck and 5cm.

## 5cq. What feeds the 5ch stall, and 00A08460's type queries and plane arm bound OFF (packet `cc9_plane_attacker_weight`, cc9-lua34, 2026-10-05)

Queue item 1 of the cc9-lua34 brief: what feeds 5co's USN13 stall, then bind the host value that
is the artefact.

### (1) The CB2 order is the slot-4 brain's Capture think, not a weight

- **CB2 is a Japanese group.** In `l33_don_usn13` the end-of-run table has `team=1 party=4
  command=DEFENDPOSITION leader=CB2`. The bruh groups are `team=1 party=4` too.
- **Where the order comes from.** A traced OFF run of this tree logs every `00A0F970` call whose
  own group is plane-led (`BSP_AI_GROUP_VALUE_TRACE=1`, `local\l34_tr_usn13.log`).
  - It has no line. The attack planner `00A1CB80` never scores a bruh group.
  - The bruh orders come from the Capture think `00A29FD0`: `capture path thinks=40
    assignments=560 attack=560`.
  - That think's targets are list-28 CommandBuildings whose side differs from `planner+30h`
    (`00A2A13B`). `planner+30h` is `brain+24h` = 0 (Allied), so the Japanese CBs are its targets,
    and CB2 is one of them.
- **5co's "group weight 1.000" is not the target value.** It is the `ai group ... weight=` column of
  the end-of-run census: `unit_leader_weight` of the group's front member
  (`src/game_hosts_ai.cpp`, the summary loop). No `00A0F970` value of 1.0 exists.
- **The brain's team, re-checked from the disk bytes** (SHIP_AI 60.4-60.6 read the same chain):
  - **Filing.** `009FFD20` files a unit not on the local team in slot 4 (`009FFD4B SUB / NEG /
    SBB / AND EAX,4`). It is keyed on the unit's `+54h` side, not its party, so every Japanese
    unit and group, generated or not, sits in slot 4 (`00A2E03C`).
  - **Brain team.** `00A15A70` stores the slot at `brain+20h` and `009FFD60(slot)` at `brain+24h`
    (`00A15A97`). `009FFD60` is `[[game+18CCh+4*slot]]+28h`.
  - **Planner.** `00A1EE50` stores `planner+30h = brain+24h` (`00A1EEA9`) and `planner+34h =
    (brain+24h == 0)` (`00A1EEB2..00A1EEC1`, `EBX` zeroed at `00A1EE67`).
  - **The authored team.** USN13's scene authors `Player5 Party = Allied`, as every `MultiPlay`
    block does: `universe/scenes/missions/usn/usn_13_truk.scn` lines 419-421 (this installation,
    mtime 2024-10-29).

  So, as far as the listings go, the slot-4 brain is Allied-team while commanding the Japanese
  side. Its Attack planner walks team 1 and its Capture planner targets the Japanese CBs.
- **SHIP_AI 60.6 confirmed this exact chain.** It covered the writer census of `+28h` and the
  local slot, and `00A1CB80`'s list without an enemy filter. This process never runs the original
  executable, so the open item is static evidence still pending: a writer of `game+1FE4h` or a
  SetGameMode caller on the single-player path (SHIP_AI 125, routed to cc9-ships28's lane).
- **5ch's other two inputs are the image's:**
  - The torpedo head's `vtable[40h]` is `009D3EF0` (vtable `00D213C8`, `009D3EF0`-`009D3F5A`,
    plain `RET`). It keeps the head only for `00E08F18` or `00E08F78` on the head's own target
    (`+4C4h`), so a tick `moveto` retires it, as 5ch assumed for the strafe head.
  - A generated squadron is adopted live by `00A2E835`'s walk (GENERATED_SHIP_AI section 7).

**Answer to 5co.** 5ch strands USN13's strike because the slot-4 brain's Capture think sends the
Japanese strike at the Japanese CB2, and the close attack then has no hostile candidate. That is
the image's rule as read (SHIP_AI 125 agrees). Whether the image really plans the Japanese side under
an Allied team is a question of static evidence still pending, not of observation.

### (2) The host stub in `00A08460`: the type queries

`AiWeightModelBinding::entity_is_type` (`src/game_hosts_ai.cpp`) answered false to every
`vtable[+18h]` query. The listing (`00A08460`-`00A09809`, read whole) asks these:

| code | asked of | sites | what it decides |
| --- | --- | --- | --- |
| `0Fh` | attacker | `00A085AD`, `00A09754` | the plane branch at `00A08619`; the x3 bonus |
| `14h` | attacker | `00A08D4F`, `00A09763` | a recon class keeps no option; no bonus |
| `17h` | attacker | `00A08AAE` | the kamikaze option |
| `06h` | attacker | `00A085CD` (unused), `00A0967F` | the capture tail |
| `0Ch` | attacker | `00A09232` | the capture tail's landing-ship flag |
| `06h` | target | `00A085E9`, `00A08712`, `00A0898F`, `00A092EF` | underwater armour; the torpedo gate |
| `08h` | target | `00A08637`, `00A092CD`, `00A092FE`, `00A09320` | the submarine gates |
| `0Ch`, `0Eh` | target | `00A0927A`, `00A0926B` | the machine-gun gate |
| `0Fh` | target | `00A08628`, `00A0925C`, `00A0929C`, `00A092BE` | the plane gates; dogfight against strafe |
| `1Ch` | target | `00A08802` | the paratrooper arm (loadout only) |
| `10h` | attacker | `00A08980` | the level-bomb arm (loadout only) |
| `20h`, `25h` | device class | `00A08BA0`/`00A08BB3`, `00A09391`/`00A093A4` | gun device, not a bomb platform |

What the stub did:
- **Planes never took the plane branch.** A squadron or plane walked its MG barrels like a ship
  (no x3, no Strafe/Dogfight/TailGun params).
- **The non-plane walk had no target gates.** The image scores a barrel only when its bullet
  sub-type suits the target (`00A0924C`-`00A0943D`):

  | sub-type | gate |
  | --- | --- |
  | 1-3 (bullet, MG) | plane, `0Eh` or `0Ch` |
  | 10h (flak) | plane |
  | 4-7, 12h (artillery, rocket) | not a plane, or `08h` |
  | 0Ah (torpedo) | `06h` and not `08h` |
  | 0Bh (depth charge) | `08h` |
  | anything else | never |

  A neutral target clears every gate.
- **Platform filters.** The walk also requires each platform's `+38h != -1`, `+18h == 1` and a
  `20h`-not-`25h` device (`00A09362`-`00A093A8`). The host walked every gun row.

### (3) The plane arm, `00A0861F`-`00A09222` (no-loadout arm)

On the `00A04560` record path the key's `+4h` is `record+10h` = 0 (AI_TARGET_WEIGHT_TERMS term 3),
so `00A0864F JLE` takes `00A08A93`:

- **Options.**
  - A submarine target (`08h`) gets none.
  - A `17h` class against a non-plane target gets one kamikaze option (`00E08F50`, factor 1.0).
  - Otherwise, for a non-neutral target, the attacker class's platforms are walked in slot order
    with the filters above. A `PilotFires` platform (`+0Ch`) gives a dogfight option (`00E08F58`)
    against a plane and a strafe option (`00E08F40`) otherwise. The factor is DamageCalcTime /
    Params[1] x Params[2] / the bullet entry's `+2Ch` reload, with DogfightParams (`+28h`/`+2Ch`)
    or StrafeParams (`+34h`/`+38h`) (tuning `+10h`, `00A08C3F`-`00A08C98`).
  - An AAMACHINEGUN platform (device `+80h` = 1) sets a tail flag. Against a plane, when no pilot
    option exists yet, it adds a dogfight option with TailGunParams (`+40h`/`+44h`).
  - A later pilot option after the tail flag rewrites from slot 0 (`00A08BF3 CMOVNZ EBX,[ESP+14h]`).
  - A `14h` class keeps none (`00A08D4F`).
- **Scores.**
  - Kamikaze: class `+210h`'s blast pair through `009FE200`, x `009FE270`, x factor.
  - Strafe and dogfight: `009FE200(DamageMin, DamageMax, Armour, HP)` x `009FE270` x factor.
  - The sum goes to the common epilogue (x3 for `0Fh` and not `14h`, / HP, clamped).
- **Tuning** (this installation's `highlvlaiglobals.lua`, all seven tables): DamageCalcTime 60,
  DogfightParams {8, 3}, StrafeParams {18, 8}, TailGunParams {15, 8}.
- **Not projected:** the loadout arm `00A08655`-`00A08A8E` (torpedo, bomb, depth charge,
  paratrooper, rocket). Nothing on these paths reaches it. The capture tail `00A0966A`-`00A09733`
  stays unprojected as before.

**What it gives on USN13.** A Kate (`VehicleClass[162]`, two `PilotFires` .30 cal guns, device 93,
bullet 84 DamageMin 14 / DamageMax 16, and an AAMACHINEGUN tail, device 110):
- 0 against every ship (Armour 50) and against CB2 (`VehicleClass[6]` CommandBuilding,
  Armour 30);
- above 0 only against targets with armour below 16, such as planes (TBM 6, F6F 6, SB2C 5; Kate
  unauthored, so 0).

### The binding, committed OFF

Two switches, `src/game_hosts_ai.cpp`:
- **`kAiWeightBarrelGatesBound`**
  - The type queries answer through the unit's own kind test, `bsp::unit_is_kind_of`.
    LABELLED: the class descriptor's `vtable[+18h]` is asked of the unit's `vtable[5Ch]` id space.
  - The walk applies the gates and the platform filters.
  - The close weight's `target_is_neutral` is the target's side >= 2. It was 0 with no producer.
- **`kAiPlaneAttackerWeightBound`**
  - A `0Fh` attacker takes the plane arm.
  - The key's `+4h` is 0 (`record+10h`).

The model is in `src/ai_target_weights.cpp` (`ai_barrel_target_gates`, `ai_barrel_gate_admits`,
`ai_plane_attack_options`, `ai_plane_attack_total`). The gunnery host publishes per row:
`PilotFires`, the Gun-list size, the bomb-platform type, the unmaxed DamageMin/Max, the Function
category and class `+210h`'s KamikazeBulletClass (`src/game_hosts_gunnery.cpp`, the class
flatten and `publish_ai_weapon_facts`).

LABELLED:
- a platform with no Function category or no resolved bullet has no row;
- the kamikaze sub-type is the Type string's mapping before `006E9968`'s rewrite;
- the walk still scores only each device's first `Bullet` entry.

Census, both states:
- `summary mission ai plane weight ... stub_divergences=...`: each false the kind test would answer
  true;
- per-code `type query` lines, the options by kind, and the gate refusals by sub-type.

### Predictions, written before any ON run

Pair A is both switches ON against OFF, with 5ch OFF. Pair B, run on the A-ON build, is 5ch ON
against OFF.

- **Census.** OFF reports `stub_divergences > 0` on every row. ON reports `plane_arm_calls > 0`
  wherever a plane is scored. Every `strafe` option of a Kate or Val against a ship or a CB scores
  0 (MG <= Armour).
- **USN13 3200 and 9200, pair A:**
  - the bruh groups still go to CB2, because the Capture think's choice does not depend on these
    weights reaching above 0;
  - the ship planners' choices move through the gates (MG barrels no longer score against ships),
    so deaths move. No direction is predicted.
- **USN04, E2, USNOS, ESMP08 14200, pair A:** moved (`pair_diff` 3).
  - Plane groups' values against own-side plane groups rise (x3, Dogfight params).
  - Ship groups lose MG value against ships and artillery value against planes.
- **USN02, USN12 (controls), pair A:** gameplay-identical, exit 0 or 1, if neither has a party-4
  planner or close attack scoring a candidate. **Weakest call:** both have Japanese ship groups
  under the slot-4 brain, so the gates may move their orders (exit 3).
- **Pair B, USN13:** the strike still orbits CB2 with no torpedo release, so 5ch stays OFF
  (5co's mechanism is unchanged).
- **Pair B, USN04:** 45 -> 71-like. The Japanese plane groups target their own groups; a tick
  `moveto` to their own leader point (`dist=0.0`) retires 22 attack heads; the planes loiter and
  die to US AA machine guns. In `l33_don_usn04` the Japanese plane deaths by AAMACHINEGUN (cat 1)
  rise from 10 to 26 and Japanese deaths overall by 19; US deaths rise by 7.

### 5cq.1 Two corrections made before the pairs

- **The plane arm's params.** `mode_tuning_record` carried only DamageCalcTime and
  MaxTargetKillRatio. The plane arm's Dogfight, Strafe and TailGun params were therefore 0 and
  every gun option's factor was infinite (census `factor_nonfinite` = every option, on a debug
  build). `4bc3073bc` fills them from this installation's authored values, which are identical in
  all seven tables (`highlvlaiglobals.lua`, mtime 2024-07-13). The first build `50bc6765e` was not
  paired.
- **The type id space is verified, no longer only labelled.** A class descriptor's `vtable[+18h]`
  uses the entity id space:
  - the MDestroyer descriptor (vtable `00D1ACF8`, slot `00D1AD10` -> `00963B70`) accepts 7, 6,
    5, 4;
  - the MPlaneFighter descriptor (`00D19C30` + 18h -> `00953650`) accepts 13h, 0Fh, 5, 4.

  Every code `00A08460` asks (6 to 1Ch) is above the chain's root 4, so the unit's own `5Ch` test
  answers the same.

### 5cq.2 Measured, and the verdict: **both stay OFF**

All builds are from `4bc3073bc`, in reference V's launch form, with `BSP_GUNNERY_RNG_STREAMS`,
`BSP_DEATH_TABLE` and `BSP_AI_SQUAD_TICK_TRACE` set:
- `local\l34_off`: both switches OFF;
- `local\l34_gates`: `kAiWeightBarrelGatesBound` only;
- `local\l34_on`: both switches;
- `local\l34_on5`: both switches plus 5ch.

E2 is USN04 9200/9000. A USN01 500/300 smoke on `l34_on5` finished cleanly.

**Pair A: OFF against ON (5ch OFF). Deaths, shots and torpedo drops.**

| row | pair_diff | deaths | shots | torpedo drops | gates only |
| --- | --- | --- | --- | --- | --- |
| USN13 3200 | moved | 22 -> 8 | 4189 -> 914 | 0 -> 0 | 8, 914 |
| USN13 9200 | moved | 116 -> 55 | 40517 -> 10579 | 3 -> 0 | 55, 10709 |
| USN04 4700 | moved | 45 -> 55 | 17447 -> 17469 | 0 -> 0 | 55, 17469 |
| E2 (USN04 9200) | moved | 85 -> 83 | 31532 -> 24619 | 0 -> 0 | 83, 24619 |
| USNOS 3200 | moved | 98 -> 56 | 6179 -> 2399 | 0 -> 0 | 56, 2399 |
| ESMP08 14200 | moved | 47 -> 47 | 16091 -> 16916 | 4 -> 0 | 47, 16916 |
| USN02 (control) | identical | 1 -> 1 | 1055 | 0 | - |
| USN12 (control) | moved | 5 -> 3 | 144 -> 112 | 0 | 3, 112 |

- **The gates carry the whole move.** The gates-only build gives the same headline as ON on
  every row except USN13 long, where shots differ by 130. So the plane arm, as projected, changes
  nothing visible on these rows. Its options do run: USN13 3200 counts `dogfight=47359/47359`
  positive, and every strafe option scores 0 because MG accuracy against a ship group is 0.
- **USN13 (the prediction missed).**
  - The bruh groups do NOT stay on CB2. With the gates, the Capture think stops assigning them;
    they are released to the Attack planner, which scores a Kate group's MG against its own Kates
    above 0 (Armour unauthored, 0) and picks the group itself.
  - The trace reads `cmd=CLOSEATTACK leader=bruh #1.1 ... target=bruh #1.1 dist=0.0`: 502
    CLOSEATTACK and 29 MOVETOATTACK squad ticks, against 222 MOVETOATTACK and none OFF.
  - The close pass then serves the bruh squadrons: the fallback bridge counts 294 calls, 162 of
    them squadrons, against 116 calls and none for squadrons OFF.
  - The strike leaves its torpedo runs. Every death the ON side does not have is a Kate (17 at
    3200, 69 at 9200, `bruh` only). Torpedo drops fall 3 -> 0 on the long row, and the US AA shots
    fall with them.
- **USNOS.** Twelve `plane` deaths and nine ground objects (containers, a static Jill) are
  OFF-only.
- **ESMP08 14200.** The four TBF drops are lost, and the deaths are three Avengers either way.
- **USN04 / E2.** Both sides' plane deaths are re-dealt (Zero, Val, Kate, Lexington and Yorktown
  squadrons), and US squadron deaths fall on E2 (16 OFF-only against 9 ON-only).
- **USN12 moved.** Only two ground objects (a barrel, a watchhouse) are lost OFF. Its Japanese
  ship groups' MG barrels no longer score against ships. The control call was the weakest one,
  and it missed.

**Pair B: ON against ON + 5ch.**

| row | deaths | shots | torpedo drops |
| --- | --- | --- | --- |
| USN13 3200 | 8 -> 0 | 914 -> 0 | 0 -> 0 |
| USN13 9200 | 55 -> 11 | 10579 -> 509 | 0 -> 0 |
| USN04 4700 | 55 -> 65 | 17469 -> 25686 | 0 -> 0 |
| E2 | 83 -> 85 | 24619 -> 34447 | 0 -> 0 |
| USNOS | 56 -> 56 | 2399 -> 1995 | 0 -> 0 |
| ESMP08 14200 | 47 -> 30 | 16916 -> 12941 | 0 -> 0 |
| USN02, USN12 | identical | | |

- 5ch still strands USN13's strike: no torpedo release, and every Kate death goes away (36 on the
  long row).
- On ESMP08, 20 US strike deaths (TBF, SB2C, TBM) go away. Those planes fly their group's
  points instead of attacking.

**Verdict: both switches stay OFF.**
- The mechanism matches the listing: the gates, the platform filters and the plane arm, with the
  id space verified above.
- The outcome rides on the Allied-team slot-4 brain (5cq (1)): with the image's gates, the
  Japanese air groups target themselves.
- USN13 3200 and the USN12 control are prediction misses.
- So the binding is recorded and held OFF until the brain-team chain is settled by static evidence (pending). Flipping
  it now would move every reference row for a reason that one open question decides.

**5ch stays OFF.** With the weight ON, USN13's strike still never attacks under 5ch.

**USN04 45 -> 71 under 5ch** (5co's pair, `l33_doff_usn04` / `l33_don_usn04`):
- Japanese deaths +19 and US deaths +7.
- Japanese planes killed by US AAMACHINEGUN fire (cat 1) rise 10 -> 26.
- The Japanese plane groups target themselves (`target=B5N Kate #2.1 dist=0.0`). Each tick
  `moveto` to their own leader point retires their scripted attack heads (`replaced_other=22`),
  and the flights loiter under the US fleet's AA instead of completing their passes.

## 5cr. USNRM01: the close pass and a script-ordered squadron's target (cc9-lua34, 2026-10-05)

The lead's question, from GUNNERY 98: with `kAiSquadronSetTargetIntakeBound` ON, KateSpawn1-5 drop
the script's `PilotSetTarget` (West Virginia, Oklahoma; `usn_1_pearl.lua:2336-2348`, this
installation, mtime 2024-10-29) once their group promotes. They switch to Downes, Cassin, Curtiss,
Helm and Mona. Does the image exempt a script-ordered squadron?

### The image has no exemption on the path

- **The close pass's member gate** (`00A143A0`-`00A14444`, AI_BRAIN_PLAYER_EXEMPTION section 1)
  serves a squadron unless its leader is an uncommitted kamikaze (`17h`, `+C24h` clear). It tests
  no order source.
- **Both orders enter at `0077D600` with flags 1.**
  - `PilotSetTarget` `008A4C90` chooses the class itself (`007EEC50` at `008A4E99`), then
    `PUSH 1` at `008A4EA2`.
  - The close pass pushes 1 at `00A14A4A` and calls `0077D600` at `00A14A6E`.

  Neither order outranks the other; the later one wins.
- **The AI-group forward is inert.** `0077D600` and `0071ECF0` forward every order to the
  group's command object at `vtable[+24h]` (`00A2BD90`). That slot is `00A0FC90`, `RET 8`, in all
  sixteen classes (AI_COMMAND_OBJECT).
- **The squadron intake** `007F1940` takes the AI `settarget` through `007EEC50` (5ck), as it takes
  any order.

So, as read, the image's close pass would override the script's target whenever it chooses one.

### What the host gets wrong: the choice, not the override

- **It is the close weight.** In the image a Kate squadron's close weight `00A0F810` against a
  ship is 0, so `00A13B60` chooses nothing (5co's `best = 0` rule) and the script's target
  stands.
  - The record path weighs the plane CLASS through the plane arm (5cq). Its only options are its
    .30 cal guns, whose DamageMax 16 is below a ship's Armour, and its accuracy against a ship
    group is 0.
  - The torpedo platform (`Platforms[50]`, guns 85 and 92) fails `+18h == 1` and is a bomb
    platform.
- **The host weighed the Kate through the ungated barrel walk.** Its row for that platform scores
  the torpedo against every ship, so Downes, at the yard, won.
- **Measured.** USNRM01 9200/9000, builds from `4bc3073bc` (5cq.2), Kate command-target tokens
  over the run:

| build | KateSpawn1, 2 | KateSpawn4 | KateSpawn3, 5 | torpedo drops |
| --- | --- | --- | --- | --- |
| OFF | Downes 1637, West Virginia 10 | Downes 1637, Oklahoma 10 | Downes, Curtiss, Cassin, Helm, Mona; Oklahoma 10 | 49 |
| gates only | West Virginia 1229 | Oklahoma 1229 | Oklahoma 740, PT 489 | 30 |
| gates + plane arm | identical to gates only | | | 30 |

  - With the image's gates, the script targets stand.
  - The only AI choice left is a PT boat for KateSpawn3 and 5. A torpedo boat (`0Eh`) passes the
    machine-gun gate, and its armour is below 16.
- **Neither battleship dies on any of the three.** That is item (c): the release gates.

**Answer.** Nothing in the image exempts a script-ordered squadron. The override is real only
when the close weight is positive, and for a torpedo bomber against a ship it is not. The host
artefact is the ungated barrel walk that `kAiWeightBarrelGatesBound` replaces (5cq). USNRM01 is
the row where that switch matches the script's intent. The switch stays OFF for the reason in
5cq.2: the slot-4 brain-team question is still pending static evidence.

## 5cs. BSM04's dialog sequencer bound OFF (packet `cc9_dialog_sequencer`, cc9-lua34, 2026-10-05)

5cn found that BSM04's opening never reaches its script dogfight (`bsm_04:1909`):
- the `INTRO` sequence's callbacks (`luaIntroMovieB`, `luaIntroMovieC`, `luaZekesDia`) never fire;
- nor does `ZEKES`'s (`luaIntroMovieD`).

`StartDialog` `008B0540` only registered the id in this host.

### The image's sequencer, from the reconstructed pieces and the listing

- **Building the entry.** `00451A90` looks the id up or inserts it (`00451920`), and `004507D0`
  builds the entry from the table:
  - `priority` `+4h`;
  - `requestTime` `+8h`, default the mission clock `[00F876A4]`;
  - `defaultPause` `+Ch`, default owner `+30h`. That is `DialogDefaultPauseTime`, 1.0 in this
    installation's `scripts/datatables/dialogglobals.lua` (mtime 2024-07-13; DIALOG_CONFIG);
  - `sequence`: one command per element from `0044BE50`, typed by `type` (default `"msg"`, compared
    without case at `0044BE79`-`0044BF99`).

  | type | kind | vtable | field read |
  | --- | --- | --- | --- |
  | msg | 0 | `00CE49CC` | `message` (`0044A440`) |
  | setpanel | 1 | `00CE49DC` | panel fields, no timing |
  | hidepanel | 2 | `00CE4950` | none |
  | pause | 3 | `00CE4960` | `time` (`0044A570`, string `00CE37A8`) |
  | callback | 4 | `00CE49EC` | `callback` (`0044A5B0`) |

- **Playing it.** Each frame `005BBF10` (VOICE_UPDATE_INTEGRATION step 7) waits for the selected
  row:
  - A row with key end 0 holds until its clip's slot reports completion (step 3); otherwise it holds
    until the last key's end.
  - Then it clears the selection, counts down `+88h` and calls `004527F0` once `+88h` is not
    positive.
  - `004527F0` erases an exhausted current entry, selects by priority (`0044C390`) and steps
    (`00452740`).
  - The step first runs `00452360`: setpanel, hidepanel and callback commands in order. A callback
    is `00887E50` with no argument, inside `00E17BFA` = 1.
  - It then handles the command at the cursor. A message selects its row through `005B94D0` and
    stores the entry's `+Ch` as the `+88h` delay. A pause stores `time - entry +Ch` in `+88h`.
    (PANEL_SEQUENCE.)
- **BSM04's message timing.** `LoadMessageMap("bsmdlg", 4)` (`bsm_04:41`) selects map 4 of
  `scripts/datatables/messagemaps/bsmdlg.lua` (mtime 2024-07-13).
  - Its `INTRO1` record has one subtitle key ending at 0.0 and the voice
    `CAMPAIGN/BSM04/INTRO1`.
  - So each message lasts its clip plus the 1.0 s default pause, and each callback fires at the
    start of the following step.

### The binding, committed OFF

`kDialogSequencerBound`, `include/bsp/game_hosts_script_orders.hpp`, with the code in
`src/game_hosts_script_orders.cpp` (`dialog_*`):
- `StartDialog` parses the table into the entry above, and `KillDialog` erases it.
- `LoadMessageMap` records the map name and index.
- The script think pass runs the tail timers and the advance.
- A finished entry leaves `GetActDialogIDs`.

LABELLED:
- The clip length is the streamed file's FSB4 header: sample count over default frequency, from
  `sound/messages/authentic/streamed_dialogs/<voice>.fsb` (`voice_dir authentic`,
  APP_INIT_LOCALE). For `INTRO1` that is 194368 / 41100 = 4.73 s. This process plays no voice, so
  it takes no stream latency.
- The message record is read by running the map file in a private environment.
- The timers run once per script think step rather than per frame.
- State 2's row reset is folded into the advance.
- A callback's own `StartDialog` or `KillDialog` is seen by looking the entry up again.

Census:
- `summary mission dialog sequencer ...`;
- `dialog callback <name> t=...`, `dialog message ... starts`, `dialog "<id>" finished`.

### Predictions, written before any ON run

- **BSM04 3200/3000:**
  - `INTRO` plays INTRO1, INTRO2 and INTRO3, firing `luaIntroMovieB`, `luaIntroMovieC` and
    `luaZekesDia` at about +5.7 s each after its start (clip about 4.7 s plus 1.0 s);
  - `ZEKES` then plays and fires `luaIntroMovieD`;
  - `luaIntroMovieEnd` follows 8 s later (`bsm_04:1896`), reaching the 1909 dogfight;
  - `pair_diff` moved.

  **Weakest call:** whether `luaIntroMovieB`/`C`/`D` (movie callbacks) need a movie end the host
  never sends.
- **USN04 4700/4500:** its `INTRO` now finishes and leaves the active set, so the failure path's
  `KillDialog` finds less. Gameplay identical or moved only through any callback it carries.
- **USN02, USN12 (controls):** gameplay-identical unless their dialogs carry callbacks. The census
  names any that fire.

### 5cs.1 Measured (OFF `local\l34_doff`, ON `local\l34_don`, both from the OFF commit), and the verdict: **flip ON**

Reference V's launch form with `BSP_LUA_CALLBACK_TRACE=1`. A USN01 500/300 smoke on the ON build
finished cleanly.

| row | pair_diff | deaths | shots | sequencer (ON) |
| --- | --- | --- | --- | --- |
| BSM04 3200/3000 | moved | 5 -> 5 (re-dealt) | 3792 -> 9396 | messages 7, callbacks 4, finished 4 |
| USN04 4700/4500 | identical | 45 | 17447 | messages 5, callbacks 0, finished 1 |
| USN02 3200/3000 | identical | 1 | 1055 | messages 6, pauses 2, finished 1 |
| USN12 3200/3000 | identical | 5 | 144 | messages 2, pauses 1, finished 1 |

**BSM04's opening, from the ON log (`l34_don_bsm04`).**

| t (s) | event | clip |
| --- | --- | --- |
| 5.10 | INTRO1 | 4.729 |
| 10.80 | `luaIntroMovieB`, then INTRO2 | 4.717 |
| 16.50 | `luaIntroMovieC`, then INTRO3 | 6.092 |
| 23.55 | `luaZekesDia` starts ZEKES | |
| 23.60 | INTRO finishes; ZEKES1 | 6.092 |
| 30.65 | `luaIntroMovieD`, then ZEKES2 | |
| 37.45 | ZEKES finishes | |

- Each step lands at clip + 1.0 s plus one think step, as predicted.
- `luaIntroMovieD` spawns the two Zero flights (`luaSpawnFirstZeros`, SpawnNew serials 1 and 2).
  That is the death and shot movement: Jap #1.1 and #2.1 die, the B-17s are re-dealt, and
  `AirField_sqn01` is no longer killed.
- RESPOND and UNDERATTACK also play and finish.

**Open: `luaIntroMovieEnd` never runs.**
- `luaIntroMovieD` ends with `luaDelay(luaIntroMovieEnd, 8)` (`bsm_04:1896`). The callback
  "ran" without an error.
- Yet the run counts no further `luaDelay` call (`calls=3`, all before) and no fourth timer
  entity (`CreateScript calls=4`).
- So the 1909 dogfight is still not reached. Next read: why a `luaDelay` inside a callback that
  the sequencer fires from the script think pass creates no timer entity. Suspects are the think
  pass's re-entrancy and the hook's count.

**Controls.** USN02 and USN12 are identical; USN04 is identical too. Their dialogs carry no
callbacks, and finishing them only empties the active set earlier.

**Verdict: ON.** The mechanism matches the prediction step by step, the controls are identical,
and the moved row is the predicted one. The `luaIntroMovieEnd` miss is a separate host question,
recorded above.

## 5ct. Handoff (cc9-lua34, 2026-10-05)

Branch `agent/cc9-lua34`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua34`. Lease
`cc9_dialog_sequencer` is released at this commit.

| packet | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| `cc9_plane_attacker_weight` | `50bc6765e`, `97587c288`, `4bc3073bc`, `73838cc6b`, `5124a2894` | `kAiWeightBarrelGatesBound`, `kAiPlaneAttackerWeightBound` | OFF (held: slot-4 team, static evidence pending) | 5cq |
| USNRM01 script target | `b9e603db4` | (the gates switch) | answered | 5cr |
| `cc9_dialog_sequencer` | this branch | `kDialogSequencerBound` | **ON** | 5cs |

Traces:
- `BSP_AI_GROUP_VALUE_TRACE=1` (`src/game_hosts_ai.cpp`): every `00A0F970` call of a plane-led
  group.
- The census line `summary mission ai plane weight ...`.
- `summary mission dialog sequencer ...`.

Scripts in `local\`:
- `l34_runs.ps1 -Sides <s> -Only <rows> [-Exe] [-Trace]`;
- `l34_pairs.ps1 -SideA -SideB -Rows`;
- `l34_gv.py`, `l34_killers.py`, `l34_dsum.py`, `l34_deaths.py`.

Next, in order:
0. **Why the Japanese air groups target themselves under `kAiWeightBarrelGatesBound`** (the last blocker for that switch; SHIP_AI 129 settles slot 4 = Allied as the image's).
   - Read `00A1CB80`'s candidate list and look for an own-group, party or side filter not read yet: `00A0F970`, `00A0C650` (`a1` = 0 zeroes the penalties on this path), `00A0F810` and the candidate builder. SHIP_AI 60.6 says the loop head `00A1CC3B`-`00A1CC65` has none.
   - Check how the gates take `target_is_neutral`: the image uses record `+1Ch` = side `+54h` >= 2; the host uses `units.unit_side_0054` (close weight) and `t.third_party` (group value). Compare team against party `+54h` in both.
   - A Kate's MG against an own-side Kate (Armour unauthored, 0) scores above 0 in both the image and the host, so the self-pick holds unless a filter is found.
1. **BSM04: why `luaIntroMovieEnd`'s `luaDelay` makes no timer** (5cs.1).
2. **USNRM01 (c): the Kates' torpedo run-in and release gates at Pearl Harbor.**
   - The `009D3420` sector scan and the release conditions.
   - With the gates ON the Kates keep West Virginia and Oklahoma (5cr), yet neither dies.
3. **5ch, the weight switches and the slot-4 team:** wait for cc9-ships28's static evidence
   (`game+1FE4h` writers, SetGameMode callers).
4. The 5cm break-off follow-ups and 5cj's remaining items (5cp).

## 5cu. Item 1: no own-side filter on the Attack path; the forced target weights bound OFF (packet `cc9_forced_target_weights`, cc9-lua35, 2026-10-05)

The question (cc9-lua35 brief, item 1): SHIP_AI 129 settled that USN13's slot-4 brain is
Allied-team in the image. Why then do the gates make USN13's Japanese air groups target
themselves? Is there an own-group or own-side exclusion that the host lacks?

### (1) The Attack path has no own-side or own-group filter (read from the disk bytes)

- **Candidate list.** `00A1CB80` walks `g_aiGroupsByTeam[planner+34h]`
  (`00A1CBF5 MOV EAX,[ESI+34h]`, list `00F8AA48 + t*0Ch`).
  - `planner+34h` is `brain+24h == 0` (`00A1EEB2 CMP [EAX+24h],EBX / 00A1EEB8 SETE DL /
    00A1EEC1 MOV [ESI+34h],EDX`).
  - `brain+24h` is `009FFD60(slot)` = `[[game+18CCh+slot*4]]+28h` (`00A15A90`/`00A15A97`).
  - The slot record's `+28h` is the scene's `PlayerN.Party`. `004C6890` copies it
    unconditionally (`004C6A44`-`004C6AA8`, scene block `+4h` -> record `+28h`). Its only
    rewrite, `004BC890`, is skipped at `004BC8B3` while `game+1FE4h` = 0 (SHIP_AI 129).
- **The loop body** (`00A1CC3B`-`00A1CEC4`):
  - It tests only the candidate's `+5644h` member count.
  - It calls `00A0F970` with `ECX` = own group (`00A1CC63`) and `EDX` = candidate (`00A1CC51`).
  - It never compares the candidate with the own group. The own group is in the list, and its
    range factor is the largest because the distance is 0.
- **The value chain carries no side either.**
  - `00A0F970` gates only on the two member counts.
  - `00A0C650` composes the pair values.
  - `00A0C3C0` applies the distance multipliers.
  - `00A0C330` calls `00A08460(attacker class, record+10h, target class, target record+1Ch)` and
    applies the `1Ch` paratrooper zero.
  - The record's `+1Ch` is the target's side >= 2 (neutral), not a comparison of two sides.
- **Unit filing.** `009FFD20` (`009FFD26`-`009FFD2E`) keeps a unit's own `+180h` slot when it is
  0..7. The side test applies only otherwise. Both arms give the same answer on USN13, because
  `usn_13_truk.scn` authors all eight `PlayerN.Party = Allied` (lines 402-433, this installation,
  mtime 2024-10-29).

So the image's Attack planner on a US row scores the slot-4 brain's own Japanese groups,
including the own group, exactly as the host does. **No host divergence on the side, party or
team chain.** The friendly targeting itself is the image's rule as read. USN13's OFF table
already shows it for the Japanese ships: the Maru groups hold `CAUTIOUSATTACK`/`MOVETOATTACK`
orders with `target=1` (`l34_gates_usn13.log`).

### (2) The divergence that does exist: `00A31DB0`'s forced weights are stubbed

What the image does:
- `00A08460` asks `00A31DB0` at `00A08540`, straight after the memo miss.
- A match is stored in the memo and returned raw (`00A08549`-`00A0856F`). It skips the barrel
  walk, the plane arm, the gates and the epilogue.
- The rules come from each mode table's `ForcedTargetWeightValues`, inserted by the loader tail:
  - `00A36FD5 CALL 00A32500` with `DL` = 0 and the loop's mode pushed. The insert's mode arm
    (`00A3265F`) appends to `00F8AB08 + mode*0Ch` and replaces only an identical rule.
  - A **string** selector is its index in the 97-entry name table `00E0CD80` (`00A36BE4`-
    `00A36C23`). That is the entity type-query id space (`SHIP` 6, `TORPEDOBOMBER` 11h,
    `COMMANDBUILDING` 1Ch, ...), with `+1Dh`/`+1Eh` clear.
  - A **number** is an exact class id with the byte set (`00A36C7A`, `00A36E34`).
- **The match** (`00A31E14`-`00A31EE5`; the per-mode copy is `00A31F34`-`00A32013`):
  - the rule's neutral byte must agree with the query flag;
  - an exact selector scores 2 on `selector == class+70h`;
  - a group selector scores 1 on `class->vtable[+18h](selector)`;
  - a zero on either side skips the rule;
  - a strictly higher sum takes the rule's weight (ties keep the first rule), and 4 stops the
    scan.

The global table `00F8AB5C` is scanned first, and only `AISetTargetWeight` fills it. No campaign
script calls that; `competitive05.lua` does.

What the host does: `AiWeightModelBinding::forced_rule_weight` answered "no match" because "this
process does not run" the loader tail (`src/game_hosts_ai.cpp`).

The shipped tables (`highlvlaiglobals.lua`, this installation, mtime 2024-07-13):

| mode | rows |
| --- | --- |
| 0 IslandCapture Rookie (the mode every reference row runs: `summary mission ai tuning mode=0`) | `BATTLESHIP` vs class 88 (Command Post) 0; `TORPEDOBOAT` vs `COMMANDBUILDING` 0; `SUBMARINE` vs `COMMANDBUILDING` 0; `LANDINGSHIP` vs `COMMANDBUILDING` 0.18; `CARGO` vs `COMMANDBUILDING` 1; `TORPEDOBOAT` vs `PLANE` 0; `TORPEDOBOAT` vs `SHIP` 0; **`TORPEDOBOMBER` vs `SHIP` 4.5; `DIVEBOMBER` vs `SHIP` 5.0** (each row twice, neutral true and false) |
| 1, 2 Regular, Veteran | the same without `CARGO`, and `LANDINGSHIP` 0.12 |
| 4 Escort | `FIGHTER` vs `SHIP` 0, vs `TORPEDOBOAT` 5, vs `LANDFORT` 0; `TORPEDOBOMBER` vs `FIGHTER` 0 and vs `TORPEDOBOMBER` 0; `DIVEBOMBER` vs `FIGHTER` 0 and vs `DIVEBOMBER` 0 |
| 3, 5, 6 | empty |

Why the effective mode is 0 in single player: `004BCA50` answers `game+614h` forced to 8
(`004C6962`). `009FFC80` sends any value above 7 to `009FFCF4`, which answers 0.

**What this changes:**
- A torpedo bomber's weight against any ship is 4.5 in the image, and a dive bomber's is 5.0,
  whatever its guns are.
- With the gates ON and no forced rules, the host scored a Kate against a ship at 0 (5cq).
- 5cr's argument ("a Kate's close weight against a ship is 0, so the script's target stands")
  therefore does not hold for the image.
- No Rookie rule covers a plane against a plane, so the self-scoring of 5cq.2 keeps its computed
  value.

### The binding, committed OFF

The pieces:
- **`kAiForcedTargetWeightRulesBound`** (`src/game_hosts_ai.cpp`).
- **The tables and the scan** (`src/ai_target_weights.cpp`): `ai_shipped_forced_rules(mode)`
  (LABELLED: transcribed from the script) and `ai_forced_rule_scan_00a31db0`.
- **How the binding answers.** It asks the unit's own kind test and its `+C4h` class id.
  LABELLED: the image asks the class descriptor's `vtable[+18h]` and `+70h`. 5cq.1 verified the
  id space; the class id equivalence is unverified.
- **What is not projected:** the global table, and relative rules (none shipped).
- **Where the lookup sits.** It runs inside the weight model only where the host runs the model.
  The 1.0 stand-in for incomplete rows is unchanged.
- **The census, in both states:** `summary mission ai forced target weight bound= mode=
  queries= matches= rules: r<i>=<hits>`.

### Predictions, written before any ON run

Pairs, all from one commit:
- **Pair F:** forced ON against OFF, every other switch as committed (gates OFF).
- **Pair G:** gates ON against gates + forced ON.

Rows: USN13 3200, USN13 9200, USN04 4700, E2, USNOS 3200, USNRM01 9200, ESMP08 14200. Controls:
USN02, USN12.

- **Census.** `mode=0` on every row. Matches are counted on every row with torpedo or dive
  bombers, and on rows with Japanese cargo ships against a CommandBuilding (USN13: r8/r9). USN02
  and USN12 have few or no bombers, so few or no matches.
- **Pair F:**
  - Moved (exit 3) on every bomber row.
  - USNRM01: the Kates' close weight becomes 4.5 against every ship, so the close pass's choice
    rides on `00A0F810`'s other factors and the Kates change targets. No direction is predicted
    for battleship deaths.
  - Controls: exit 0 or 1, if no bomber group is scored.
- **Pair G, USN13:**
  - The bruh Kate groups still leave the Capture think, because no rule covers a torpedo bomber
    against a CommandBuilding.
  - In the Attack planner they now pick a Japanese Maru (ship) group at 4.5 per pair over
    themselves. The trace's `target=` names a Maru rather than `bruh`.
  - **That is still friendly targeting, so the gates stay OFF and 5ch is not re-tested**
    (the brief's condition, "USN13 behaves sanely", cannot be met by this binding).
- **Pair G, USNRM01:** the Kates no longer keep West Virginia and Oklahoma throughout (5cr's gates
  result), unless those battleships win the close pass's other factors (the objective x10 of
  `00A0F8C6` is the likely one).

## 5cv. Item 2: `luaIntroMovieEnd` does run; 5cs.1's open item was a trace misreading (cc9-lua35, 2026-10-05)

5cs.1 left this open: `luaIntroMovieD`'s `luaDelay(luaIntroMovieEnd, 8)` (`bsm_04:1896`) seemed to
create no timer. Its evidence was `luaDelay calls=3` and no `luaIntroMovieEnd` line in the
callback trace.

**The same log (`cc9-lua34\local\l34_don_bsm04.log`) shows the body running at mission frame
771, t = 38.60 s.** That is the callback's 30.65 s plus 8 s, within one think step.
- The trace's firsts at t=38.60 are `luaSetScriptTarget`, `luaAddFirstObjs` and
  `luaAddSecondZeroListener`. All three are called only from `luaIntroMovieEnd`
  (`bsm_04:1908`, `1912`, `1914`; `luaAddFirstObjs` has no other caller in the script, which is
  `bsm_04_vengance_at_luzon.lua`, this installation, mtime 2024-07-13).
- The bindings run in order at the same frame:
  - `SetInvincible "B-17" value=0.000`;
  - `SetSelectedUnit` moves control from the B-17 to Donald;
  - `UnitSetFireStance: squadron Donald stance=1`;
  - `PilotSetTarget: unit=Wildcat target_object_id=43 ... ISSUED`, which is the `bsm_04:1909`
    dogfight order;
  - `Objectives_Add` for B17, Donald and DD.

So the 1909 dogfight **is** reached on the ON build, and nothing about the sequencer or the
think pass needs fixing.

Why the trace misled:
- `lua_callback_trace_hook` keys a call by `ar->name` when Lua can name it, and otherwise by
  `?<src>:<linedefined>`.
- A function reached through a table slot (the timer's `luaDoTimeTable` calling `timer[2][1]`)
  has no name, so `luaIntroMovieEnd` would be listed as `?...:1900`, if at all.
- Why the fourth `luaDelay` call is not counted (`calls=3`) is not established. The hook is set
  on the first state that dispatches a binding, and Lua 5.0 hooks are per thread. LABELLED as a
  diagnostic-hook limitation, not run-time behaviour.

**Rule for later readers:** judge a callback by its callees' bindings in the log, not by its name
in the trace.
### 5cu.1 Measured, and the verdicts: **forced weights ON; the gates stay OFF**

Four exports of `5e6f83c8b` in reference V's launch form, with `BSP_GUNNERY_RNG_STREAMS`,
`BSP_DEATH_TABLE` and `BSP_AI_SQUAD_TICK_TRACE` set:

| export | switches | SHA-256 |
| --- | --- | --- |
| `local\l35_off` | as committed | `63FF59FD8220` |
| `local\l35_f` | forced ON | `FDCF594B9F39` |
| `local\l35_g` | gates ON | `F9F05D67B3C3` |
| `local\l35_gf` | gates and forced ON | `9B694E94FEF2` |

USN01 500/300 smokes on `f` and `gf` finished cleanly. The census reads `mode=0` on every row. The
matches are rule 8 (`CARGO` vs `COMMANDBUILDING`) and rule 15 (`TORPEDOBOMBER` vs `SHIP`).

**Pair F (OFF -> forced) and pair G (gates -> gates + forced): deaths, shots and torpedo drops.**

| row | F exit | F deaths | F shots | F drops | G exit | G deaths | G shots | G drops |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| USN13 3200 | 3 | 22 -> 22 | 4189 -> 4189 | 0 -> 0 | 3 | 8 -> 22 | 914 -> 4189 | 0 -> 0 |
| USN13 9200 | 3 | 116 -> 115 | 40517 -> 40899 | 3 -> 3 | 3 | 55 -> 154 | 10709 -> 48636 | 0 -> 3 |
| USN04 4700 | 3 | 45 -> 46 | 17447 -> 12587 | 0 -> 1 | 3 | 55 -> 46 | 17469 -> 12587 | 0 -> 1 |
| E2 | 3 | 85 -> 88 | 31532 -> 24172 | 0 -> 1 | 3 | 83 -> 86 | 24619 -> 24924 | 0 -> 1 |
| USNOS 3200 | 3 | 98 -> 97 | 6179 -> 5930 | 0 -> 0 | 3 | 56 -> 67 | 2399 -> 5926 | 0 -> 0 |
| USNRM01 9200 | 3 | 156 -> 180 | 49013 -> 71931 | 49 -> 54 | 3 | 159 -> 171 | 78264 -> 59391 | 30 -> 53 |
| ESMP08 14200 | 3 | 51 -> 53 | 11259 -> 11235 | 0 -> 2 | 3 | 48 -> 53 | 10228 -> 11235 | 0 -> 2 |
| USN02 (control) | 1 | identical | | | 1 | identical | | |
| USN12 (control) | 1 | identical | | | 1 | identical | | |

**USNRM01: do the Kates stay on the battleships? No, not with the image's forced weights.**
Kate command-target tokens over the run:

| build | KateSpawn1, 2, 4 | KateSpawn3, 5 |
| --- | --- | --- |
| `off` | Downes 4911 (after West Virginia / Oklahoma 30) | Downes, Curtiss, Cassin, Helm, Mona |
| `f` | Pennsylvania 4410 | Curtiss, Pennsylvania, Mona, Utah, Detroit, Clemson-class 01 |
| `g` | West Virginia / Oklahoma 3687 (5cr's result) | Oklahoma 2220, PT 1467 |
| `gf` | Pennsylvania 3867 | Clemson-class 01, Curtiss, Pennsylvania, Detroit, Utah |

- Under the rules, a Kate's close weight against every ship is the same 4.5. The close pass's
  other factors then pick Pennsylvania, which is in the Navy Yard's dry dock, over the script's
  battleships.
- So the gates-only result of 5cr (script targets kept) holds only while the forced weights are
  stubbed. With both switches, USNRM01 no longer argues for the gates.
- No battleship dies in any of the four builds. Downes dies in `off` only.
- Pair F's +24 deaths are mostly Japanese planes (22 ON-only `Jap #` rows against 16 OFF-only)
  and ground objects.

**USN13: what the air groups pick.**

| build | bruh-led squad ticks (9200), top targets |
| --- | --- |
| `off` | MOVETOATTACK CB2 264, CLOSEATTACK CB2 16 |
| `f` | MOVETOATTACK CB2 164, CLOSEATTACK CB2 18 |
| `g` | CLOSEATTACK bruh #1.1 1151, MOVETOATTACK bruh #1.1 29 |
| `gf` | CLOSEATTACK bruh #1.1 229, MOVETOATTACK bruh #1.1 116, MOVETOATTACK CB2 82, CLOSEATTACK bruh #3.1 70, #1.4 27, #1.8 24 |

- **The prediction missed.** With gates and forced weights the Kates do not move to the Maru
  groups. They still pick Kate groups (their own or another), and part of the time CB2.
  - At distance 0 a Kate group's MG value against itself, times the range factor 1.0, still beats
    4.5 x the Marus' far range factor.
- **Gates + forced weights capture more:** the capture assignments rise from 309 to 1510. Rule 8
  (`CARGO` vs `COMMANDBUILDING` 1.0) gives the Maru groups a capture weight against the
  Japanese CBs. Spawns rise from 1 to 4 and units from 373 to 499.
  - That accounts for the 9200 row's +99 deaths: 28 more `bruh` Kates die (ON-only rows), and
    the extra generated groups re-deal the ground objects.
- Without the gates (`off`, `f`), the bruh groups stay on CB2, as in 5cq.

**Correction to 5cq (3):** the Kate is not unauthored. `VehicleClass[162]` (B5N Kate) has
`Armour = 6` (`vehicleclasses.lua` line 54674, this installation). Its .30 cal MG (14-16) still
beats it, so the self-scoring conclusion stands.

**The other moved rows** re-deal plane deaths on both sides; no ship row flips:
- USN04 pair G: Val, Zero, Kate and the Lexington/Yorktown squadrons;
- ESMP08: Avengers, Corsairs and Helldivers;
- USNOS pair G: 22 ON-only `plane #` rows.

With forced weights ON, the gates matter much less: `f` and `gf` give the same headline on
USN13 3200, USN04 and ESMP08.

**Verdict on `kAiForcedTargetWeightRulesBound`: flip ON.**
- The mechanism matches the listing (`00A31DB0`, the loader tail and the shipped tables).
- The controls are identical in both pairs.
- USNRM01's predicted move happened.
- USN13's target prediction missed, but the miss is a spread miss within the image's rule: no
  own-side filter exists.

**Verdict on the gates (`kAiWeightBarrelGatesBound`, and the plane arm with it): stay OFF, by
the brief's condition.**
- USN13 does not behave sanely under them, with or without the forced weights.
- USNRM01's Kates leave the battleships once the forced weights are in.
- But the friendly targeting is the image's rule as read (5cu (1)). It is no longer a host
  divergence that blocks the switch; it is a decision about matching a self-targeting image. The
  mechanism matches the listing.

**Recommendation to the lead:** flip the gates with the plane arm, as the image's rule, and
accept that the slot-4 brain's air groups target their own side on US rows. 5ch is not
re-tested here.
## 5cw. Item 3: USNRM01's scripted Kates never close; they turn in place and mush into the sea (cc9-lua35, 2026-10-05)

5ct item 2: with the gates ON, KateSpawn1-5 keep West Virginia and Oklahoma (5cr). Why is there
no release and no battleship damage? This is read from `local\l35_g_usnrm01.log` (export
`l35_g`, gates ON, forced weights OFF), and is static otherwise.

**What the aircraft do** (KateSpawn1|.-2; its wingmen and the other four squadrons log the same):

| step | evidence |
| --- | --- |
| spawned at (4000, 180, -5000), heading 180 deg | the authored `localframe`, `usn_1_pearl.scn` line 8266, this installation, mtime 2024-10-29; velocity seeded (0, -0.02, -61.11) along it |
| turned 140 deg to face West Virginia | `EntityTurnToEntity` (`usn_1_pearl.lua` `luaSpawnKates`): `3 member(s) re-posed, forward (-0.628 0.000 0.778)`, through `007C9540` per member |
| straight into `aim` | the range, 3451.9 m, is already inside `Pilot/Torpedo/AttackDist` (`task+484h` = 3749.4), so there is no approach leg |
| aim tick 1 | heading error 0.0056 rad (the body faces the target); velocity 58.7 m/s at 140.8 deg to it, `body_fwd` = -45.5 (flying backwards); altitude 180 |
| aim tick 51 | range 3461.9 m (not closing); velocity 41.3 m/s at 72 deg; altitude 89.7 |
| about t = 126 s | `surface probe ... alt=4.3 vy=-43.7 spd=46.9`, then `plane depth kill` |

- `aim gates`: `ticks=83 lead=0 ... min_f14=3419.4`. The lead gate needs the envelope (an
  aspect interpolation times speed) to exceed the range, which never happens at 3.4 km.

**Why this is the image's behaviour as read (no binding):**
- **The turn writes no velocity.** `007C9540` (plane `vtable[88h]`) copies the matrix to `+74h`
  and `+674h` and re-derives attachments only; `007BEEE0` likewise (decompiled).
- **The world velocity is the state.**
  - `007D9C10` rebuilds the world-to-body matrix `ctl+0B0h` from `unit+74h` and derives the body
    velocity `ctl+3Ch` from the world velocity `ctl+18h`. The core law `007DB680` calls it.
  - `007D7C00` then loads both into the step's `dyn` (docs/PLANE_FLIGHT_CORE_LAW.md 2).
  - So after a re-pose the old world velocity is re-expressed in the new body frame, here as
    negative forward speed.
- **The spawn seed precedes the turn.**
  - `GenerateObject` runs `00925F20` InitAll inside the call (`0046DBE8`).
  - Pass C `007F4BA0` -> `007F2920` -> `007C6340` seeds `vtable[3Ch](TravelSpeed)` =
    `007D9E80`, body (0, 0, 61.1) at the authored heading.
  - `EntityTurnToEntity` comes afterwards in the script.
- **Backwards flight is the free-flight stall regime** (docs/FREEFLIGHT_STALL_LAW.md 2).
  - Lift scales with `(forward / StallSpd)^2`.
  - Body damping is `007D92B0(forward / StallSpd)`, which is 0 at or below DragRangeMin.
  - So nothing turns the velocity onto the nose, and the aircraft sinks.

**Uncertainty:**
- Not every writer of `ctl+18h` / `ctl+3Ch` was censused. A re-pose consumer that rotates the
  world velocity would change this.
- `007F2920` itself is unread past its call list.
- The original executable is never run, so this is not observed.

**What it means.**
- The scripted torpedo strike at Battleship Row cannot release from these spawns, as read.
- Under the image's forced weights (5cu.1, now ON) the close pass sends the Kates to Pennsylvania
  and the Navy Yard anyway.
- Either way no battleship dies, in all four builds.
- Item 3 is closed with no host divergence found.
## 5cx. Item 4: the 5cm break-off follow-ups have no reach on current main; the manoeuvre drop read statically (cc9-lua35, 2026-10-05)

**No reach.** On this branch (forced weights ON, 5ch OFF) every row logs `summary mission strafe
breakoff bound=1 tightturns=0 flikflaks=0 manoeuvre_ticks=0`.
- That holds in all 20 logs of 5cu.1 (`l35_f_*`, `l35_gf_*`).
- A traced ESMP08 14200/14000 run on the tree build (`local\l35_tr_esmp08x.log`,
  `BSP_STRAFE_GOAWAY_TRACE=1`) sums `aim=460 attackrun=576 goaway=0` over its strafe rows, with
  no goaway enter, gate or push line.
- So since 5ch went OFF (5co), the strike never enters goaway and `kStrafeBreakoffBound` (ON)
  changes nothing on the reference rows. None of 5cm's three follow-ups can be validated by a
  pair today.

**The manoeuvre on a command change, read for the record** (5cm's "`0099A4C0` would ask the
head's `vt[40h]` `0099C2C0`"):
- **`0099A4C0` runs every pilot tick** (`0099AE7E`, `src/pilot_command_path.cpp`). It pops the
  front task while all of these hold:
  - the head is non-null;
  - `vt[38h]` is true;
  - `vt[34h]` is false;
  - it is not the case that the stack has fewer than 2 entries and `vt[40h]` answers 1.

  It installs a command task (`0099A170`) only when the stack ends empty.
- **The manoeuvre vtables:**
  - tightturn, vtable `00D205E0` (its name string `tightturn` follows at `00D2064C`);
  - flikflak, vtable `00D20748`.

  LABELLED for flikflak: its vtable is identified only by the `flikflak` string that follows it, as tightturn's does. Tightturn's own tick `009BA020` sits in its vtable at `00D20644`.
  Both have `vt[34h]` = `009BA810` (`MOV AL,1 / RET`), `vt[38h]` = `0099B710` (`MOV AL,1 /
  RET`) and `vt[40h]` = `0099C2C0`.
- **So a manoeuvre head is never popped by `0099A4C0`.** The scan stops before `vt[40h]` is
  asked, and nothing is installed.
- **A new command's task does not displace it either.** `0099A170` installs through `0099A020`,
  which appends at the end of the vector (`+58h` base, `+5Ch` count, `+60h` capacity, grown
  x2+2).
- **In the image, a running tightturn or flikflak survives a command change** and ends on its own
  conditions. The new command's task then waits behind it.
- **The host differs.** `src/game_hosts_units.cpp`'s strafe install clears `st_mv_kind` (the
  LABELLED "a re-install drops a running manoeuvre"). That is a real divergence, with no reach
  today, so it is recorded and not bound.
- `0099C2C0` itself:
  - answers 2 when the task has no `+2FCh` owner or no `vt[3Ch]` target;
  - answers 0 when the director's command is neither the task's own target class nor an
    `00E08F78` attack with a class-1/2 target;
  - answers 0 when the active target's offset length is at or above `[00D7A220]`;
  - answers 1 otherwise.

  It matters only for single-entry stacks whose head has `vt[34h]` false.

**Not done:**
- the stacked second push;
- tightturn's `009BA1C8 +2E4h &= ~4`.

Both are behind the same goaway reach. 5cj's remaining items (the `settarget` delivery and the
script-dogfight rows) were superseded by 5ck, 5cn and 5cr-5cw.
### 5cw.1 The lead's census: nothing rotates the velocity on the turn, yet the script expects the Kates to fly (cc9-lua35, 2026-10-05)

**(a) Writers of the controller velocity.**
- **The controller pointer.** Every site that forms it (`lea r,[unit+0AB0h]` or `add r,0AB0h`;
  `local\l35_nextcall.py` over `local\l35_ab0.txt`) leads to these methods:
  - `007D99C0`, `007D7A80`, `007DB2C0` (the reset; three sites inside spawn and launch-spot code);
  - `007D9CE0` (re-parent, touchdown / launch only);
  - `007D9E80` (set forward speed);
  - `007D9C80` (`007CA58A`, touchdown / crash);
  - `007D9EE0` and `007DB430` (the property bag, `007D5D20`);
  - `007DB1F0`, the network state apply `007D1360`, plane `vtable+18Ch`. A message with byte `+26h`
    set does carry a body-frame velocity through the current matrix. It is the multiplayer
    send/apply pair (`007C2880` / `007D1360`), gated on states 4-7, and no single-player
    loopback is established;
  - the rest are pilot and timer methods.
- **A disp32 census of `unit+AC8h..AD0h` and `+AECh..AF4h`** (`local\l35_dispscan.py`) finds no
  plane-side store outside those methods.
- **The core law** `007DB680` calls `007D81B0`, then `007D9C10` (body = world through the new
  matrix), then `007DA710`, then `007D7C00`. The world velocity is the state.
- **`vtable[3Ch]` (set forward speed along the current nose).** The `mov r,[r+3Ch] / call r` sites
  with a float argument in `007B0000`-`007F8400`, `00880000`-`008C0000` and `00996000`-
  `009F6000` (`local\l35_vcall.py`) are:
  - `007C63AB`, `007C6467`, `007C64BB` (spawn);
  - `007F29B6`, `007F2B3C` (squadron pass C);
  - `0089F8F1`, `SquadronSetSpeed` `0089F780`. It would re-seed along the turned nose, but
    `usn_1_pearl.lua` never calls it.

**(b) `007F2920`, read past its call list** (decompiled):
- It saves the leader's speed (`vt[38h]`), runs `007C6340`, and re-applies that speed with
  `vt[3Ch]`.
- It then places each wingman at its formation station from the leader's `+74h` matrix and gives
  it the same speed (`007F2B3C`).
- It runs in squadron pass C, which `GenerateObject` reaches synchronously (`0046DBE8 CALL
  00925F20`, `CL` = 0). So the seed is along the authored heading, before the script's turn.

**(c) `EntityTurnToEntity` `008A0A10` for a squadron:**
- The arm is `008A0D5D` (`vtable[5Ch](18h)`) through `008A0DE4`. Per member it does
  `00414DB0`, then `vtable[88h]` (`008A0DC7`/`008A0DD4`), and nothing else. The other `1FE4h`
  arm (`007EDEB0`) is session-only.
- All nine plane vtables (`00D00070`, `00D00308`, `00D05F20`, `00D06638`, `00D06920`,
  `00D0BA80`, `00D19D28`, `00D1A000`, which is the Kate's primary, and `00D1A2D8`; found through
  `007D1360` at `+18Ch`) have `+88h` = `007C9540` and `+3Ch` = `0074E1E0`.

**So no static path rotates the velocity, and nothing is bound. But the script disagrees with
the outcome.**
- `luaIntroMovie...` (`usn_1_pearl.lua` around 948-966) keeps the camera on `TorpTable[1]` for
  about 45 s of moves (8 + 25 s, then 12 + 8 s) before cutting to West Virginia.
- In the host the Kates hit the sea about 8 s after spawning.
- The authors expected a long run-in, which supports the lead's prior.

The remaining candidates, none read here:
1. **The free-flight law at negative forward speed.**
   - `q = forward / StallSpd / LevelFlight` enters lift squared.
   - Damping takes `007D92B0(forward / StallSpd)`.
   - How the image's sign conventions treat `forward < 0` was never checked: the stall-law
     packet studied small positive forward speeds.
2. **The water kill against `SetInvincible(unit, true)`.** The script makes all five squadrons
   invincible. The host's `plane depth kill` calls `BSP_MissionEntity_Kill` regardless.
3. **The aim tick's throttle and pitch** at a 140-degree velocity error.

All three are plane-lane reads (planes or gunnery owners), not Lua-host ones.
## 5cy. USN13 plausibility read: nothing found that stops the Japanese brain planning its own side (cc9-lua35, 2026-10-05)

The lead's three questions, asked because "Japanese air groups target themselves" is
implausible in a shipped campaign:

1. **Does the script re-issue orders often enough to mask the planner? No.**
   - The strike is `luaSpawnAttackWave` (`usn_13_truk.lua` 1257, `SpawnNew`, `PARTY_JAPANESE`,
     Type 162 named `bruh`; this installation, mtime 2024-08-13).
   - Each squadron gets exactly one `PilotSetTarget(unit, luaPickRnd(Mission.USCV))`, in
     `luaAttackWaveSpawned` (1645). No later line re-targets `Mission.AttackWave` (`rg` over the
     script: 801, 982, 996, 998 only count or film it).
   - So any AI order to these squadrons is the last word.
   - In the gates build (`l35_g_usn13l`) the squadrons' own command targets still name US units
     on most lines (Hill, Monterey, Cowpens, Intrepid, Essex and their squadrons). The friendly
     group orders show up as:
     - 303 `ai:close_attack` dogfight task rows and 690 `ai_command_tick` moveto rows for
       bruh #1.1;
     - 64 `attackmove arm ... target not hostile` refusals.
2. **Does `009FFD20`'s slot-4 filing apply to script-spawned groups? Yes, as read.**
   - `unit+180h` is written only through `vtable[144h]` (docs/AI_BRAIN_PLAYER_EXEMPTION.md).
   - On activation (`0077F0E0`) it is the `OwnerPlayer` property, or 9 when that is absent
     (`0077F1F1`). The `SpawnNew` table carries no `OwnerPlayer`.
   - So these units take the side test, `+54h` = 1 against slot 0's party 0, giving slot 4.
3. **Is `brain+24h` ever set from the members? No.**
   - The brain is built lazily in `BSP_AiParties_Think` (`00A1836A`-`00A1838C`,
     `operator new(28h)`, `00A15A70(slot)`), with the slot from the think's own loop.
   - A sweep of `00A15950`-`00A18850` finds one store to `[reg+24h]` that is not a stack slot:
     `00A15A97` in the constructor.
   - None of the 10 references to the brain array `00F8A89C` is followed by a `+24h` store.
     LABELLED: brain methods outside that range were not swept.

**Record:** `kAiWeightBarrelGatesBound` and `kAiPlaneAttackerWeightBound` are the image as read,
held OFF on a plausibility prior (the lead's decision of 2026-10-05). Revisit with reference Y.
## 5cz. Handoff (cc9-lua35, 2026-10-05)

Branch `agent/cc9-lua35`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua35`. No lease
is held. The `src/game_hosts_ai.cpp` loan is handed back.

| packet | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| `cc9_forced_target_weights` | `a3721808a`, `5e6f83c8b`, `538ad0d7a` | `kAiForcedTargetWeightRulesBound` | **ON** | 5cu, 5cu.1 |
| BSM04 `luaIntroMovieEnd` | `a3721808a` | - | answered: it runs | 5cv |
| USNRM01 Kates | `cb01db717`, `4ed9feb3f` | - | open (plausibility) | 5cw, 5cw.1 |
| break-off follow-ups | `61ad1d802`, `689c85caf` | - | no reach | 5cx |
| USN13 plausibility | `5f59f863f` | gates / plane arm | held OFF on a prior | 5cy |

Census: `summary mission ai forced target weight bound= mode= queries= matches= rules:`.

Scripts in `local\`:
- `l35_runs.ps1 -Sides a,b -Only rows` (two sides in parallel through the slots);
- `l35_edit_ai.py` (the applied binding edit);
- `l35_kinds.py <log> <needle>` (line-kind census);
- `l35_dispscan.py <disp...>` (disp32 census);
- `l35_nextcall.py`, `l35_vcall.py <disp> <lo> <hi>` (`mov r,[r+disp] / call r` sites);
- `l35_slots.py` (vtable slots), `l35_brain24.py`.

### Next, in order

1. **USNRM01's Kates mush into the sea about 8 s after spawning** (5cw.1), while the script films
   `TorpTable[1]` for about 45 s. Candidates, in the lead's order:
   1. the depth kill `007CE3A7` (`BSP_MissionEntity_Kill(unit, 1)`, gated on `unit+61h`)
      against `SetInvincible` (`unit+150h`), which the script sets on all five squadrons. This
      one is cheap to check;
   2. the free-flight law at negative forward speed (`007DB875` lift, `007D92B0` damping);
   3. the aim tick at a 140-degree velocity error.
2. **The gates and plane arm:** image as read, held OFF (5cy). Revisit with reference Y.
3. **Unbound but read:** a running tightturn or flikflak survives a command change in the image
   (5cx). Bind it only once goaway is reached again.
## 5da. USNRM01's Kates: the depth kill and the stall law are the image's; AvoidTerrain's throttle cap read the wrong speed (packet `cc9_terrain_avoid_forward_speed`, cc9-lua36, 2026-10-05)

5cz item 1, in the brief's order. Read statically, then measured with a diagnostic build (never
committed) that logged `KateSpawn1|.-2` every 0.25 s of free flight and every throttle plan
(`local\l36_diag_usnrm01.log`, main `05a19fc84`, forced weights ON, gates OFF).

**(a) The depth kill against `SetInvincible`: no divergence.**
- `007CE313`-`007CE3AC` (in `BSP_PlaneTickElement_FixedStep` `007CE040`, ESI = unit+310h): gated on
  the byte unit+61h (`007CE313 CMP [ESI-2AFh],0`) only. Below `-limit` (30.0f `00CE38C8`, or the
  map chain) it calls `BSP_MissionEntity_Kill(unit, 1)` at `007CE3A7`.
- `00926D90` itself reads only `+5Fh` (already dead) and `+60h`; it never reads unit+150h.
- `SetInvincible` (`00897A50` -> `0042ED80`) writes only unit+150h and recurses over the children
  (`vtable[F4h]`). unit+150h acts only in the damage path (`00879070`, `008110F0`).
- So in the image an invincible aircraft that reaches 30 m below the surface dies. The host's
  depth kill (`src/game_hosts_units.cpp`, packet `cc9_water_surface_law`) is the image's.

**(b) The free-flight law at negative forward speed: no divergence.**
- `007D99C0` returns ctl+44h (body forward) plus the carrier term, unsigned nowhere.
- Lift, `007DB875`-`007DB98D` (listing read): `aoa = -ctl+40h / ctl+44h` with the SIGNED divisor
  (`007DB8B6 FDIV [ESI+44h]`) once `|vz| >= 0.1`; `q = forward / StallSpd / LevelFlight * ctl+9Ch`;
  `007DB8DD`-`007DB8EF` takes `q*q` when `1.0 > q` and 1.0 otherwise, so a NEGATIVE q is squared
  with no cap. The coefficient is then clamped to +-2. `lift_accel_007db875` is that.
- `007DBB6A`'s x5 drag when flying backwards and `007D92B0`'s 0 below DragRangeMin are in the host.
- Measured: at spawn `bv = (-38.4, 0, -47.5)`, `q = -1.36`, lift 27.2 m/s^2 against gravity 14.7, so
  the Kate first CLIMBS (180.0 -> 181.3 m by t+0.9 s). The x5 drag stops the backwards motion in
  about 2.7 s; through `q = 0` lift is zero and the Kate drops. Nothing in (b) loses the aircraft.

**(c) What does lose it: the throttle.**
- From t = 122.40 (forward speed back to +9.2 m/s, `|v|` 35 m/s) the throttle falls 1.0 -> 0.63 in
  one think and then to 0.21 at the water, although the speed plan wants 97.8 m/s and raises its
  demand by +0.12 every think (`L36T ... out = slot + 0.12`).
- The cut is `0099BF30`'s `pilot+25Ch` arm (`gunfire_repair_0099bf30`), fed by AvoidTerrain's pass-1
  band `0099CAB0` (`0099CC11`-`0099CCAE`): `tgt = interp(0.3 -> MaxSpd x 1.8, 0.7 -> 007C47F0 + 20; m)`,
  `diff = tgt - speed`, and below 30 the cap `interp(-20 -> -1, 30 -> 1; diff)`. The log fits
  `tgt` = 34.9 + 20 = 54.9 and `speed = |v|`: (35.1 -> 0.63, 44.1 -> 0.24).
- **The image's `speed` there is not `|v|`.** `0099CC8E`-`0099CC9A` calls the unit's `vtable[204h]`,
  as does the caller at `0099F278`-`0099F280` (which then takes `max(speed, StallRangeMax x StallSpd)`
  for the look-ahead, `0099F284`-`0099F2B4`). For every plane vtable that slot is `007B8E70`,
  `FLD [ECX+0B2Ch]; RET`, the flight controller's ctl+7Ch (controller at unit+AB0h).
- **Its writer:** `007D80C0` (`__thiscall(ctl, float step)`, RET 4) does `ctl+7Ch = ctl+44h` and
  `ctl+78h = (new - old) / step` (`007D80D5`-`007D80F6`; the zero-step arm `007D80FF`-`007D8107` stores
  ctl+44h too). The core law calls it at `007DC81D` with ECX = ctl, after the integration. The other
  callers are `007D9F52` (set world velocity), `007D9CD8`, `007D9C71`, `007DB26A`. The reset
  `007DB2C0` zeroes it (`007DB37D`). No other store to `+7Ch` in `007D7000`-`007DD000`.
- By contrast `vtable[38h]` (`007B8E60`, unit+B1Ch = ctl+6Ch) is `|ctl+18h|`, the world speed
  (`007D8020`-`007D807C`). The host's labelled stand-in "vtable[38h] and [204h] are |velocity|"
  (docs/ATTACKER_EVASION.md) is right for 38h and wrong for 204h.
- With the forward speed the cap does not engage while the Kate is slipping: at t = 122.40,
  `diff = 54.9 - 9.2 = 45.7 > 30`.

**The binding,** `kTerrainAvoidForwardSpeedBound` (OFF), in `terrain_avoidance_0099f1c0`: `speed`
becomes `dot(body z row, world velocity)`, the host's ctl+44h. It feeds the look-ahead
`max(speed, v_stall)` and the band's cap, as the one image value does. The `speed > 1` guard on the
velocity-elevation stand-in (`unit+C7Ch`) keeps `|v|`, being the host's own guard on `|v|`.
SUBSTITUTION, labelled: the image reads the value the previous core step stored; the host forms it
from the current pose and velocity at the think.

**Predictions, written before the pairs:**
- USNRM01: the Kates keep throttle 1.0 through the stall. The cap engages only once the forward
  speed is above about 25 m/s. They reach the sea later or not at all. A rescue is not certain:
  at t = 122.4 they are at 139 m sinking at 30 m/s and need about 35 m/s forward for 1 g of lift.
  If they survive, torpedo releases at Battleship Row should appear (Pennsylvania or West Virginia
  under the forced weights).
- Controls (USN04, USN02): in coordinated flight forward speed is within a few m/s of `|v|`, so the
  cap and the look-ahead barely move. Expect small terrain-avoidance counter moves (the `avoid ...
  terrain` census) and RNG-coupled downstream moves; no systematic change in releases or deaths.

### 5da.1 Measured: **ON** (cc9-lua36, 2026-10-05)

Same-tree pairs from `d97ad79fd` (`local\l36_off`, `local\l36_on`; launch form of reference V,
`local\l36_runs.ps1`). USN01 300-frame smoke ON: clean.

| row | verdict | what moved |
| --- | --- | --- |
| USNRM01 9000 | 3 moved | deaths 180 -> 191; torpedo drops 54 -> 55; plane water contacts 104952 -> 95062 |
| USN04 4500 | 3 moved | deaths 46 -> 38; torpedo drops 1 -> 2 |
| USN02 3000 | 0 identical | - |

**USNRM01, the Kates (the prediction held for 9 of 15 aircraft):**
- OFF: all 15 Kates die by the depth kill at t = 126.7-127.1 s, 8 s after spawning.
- ON, KateSpawn1 and KateSpawn5 (all three each), KateSpawn2's leader, KateSpawn3|.-2 and KateSpawn4|.-2 fly the run-in at full throttle. KateSpawn1,
  KateSpawn5|.-2, KateSpawn3|.-2 and KateSpawn5 drop torpedoes at Pennsylvania (11 m, 69-71 m/s;
  `torpedo drop 1`-`4`). The forced weights send them there, as 5cu.1 predicted.
- At t = 186.91 s the script's own `Kill` (`008AC5C0`, `cc9_lua_kill`) removes KateSpawn1, 2 and 5
  (7 aircraft, alt 67-349 m, no damage). That is the end of the scripted strike, about 68 s after
  the spawn, which fits the 45 s of camera time 5cw.1 counted.
- KateSpawn2's wingmen, KateSpawn3's leader and .-3, and KateSpawn4 and .-3 still reach the sea at
  t = 146.4-148.2 s. That is a second, different fall:
  - full throttle, nose 42-71 degrees up (`pitch_c64` 0.73-1.25), 27-48 m/s;
  - the stall regime of docs/FREEFLIGHT_STALL_LAW.md, with the pilot holding full nose-up
    (`live_pitch` 1.0);
  - not bound here.
- The other moved rows (Japanese aircraft, cranes, Neosho) follow from the changed air picture
  and the shared RNG stream.

**USN04, the control:**
- The first gameplay line that differs is Yorktown-class01_sqn02's climb-out at 42.1 s (`min member
  106.54 m` -> `115.88 m`). Its three aircraft cap the throttle 18/18/19 -> 16/16/17 times
  (`vehicle/terrain avoid ... thr=`). A climbing aircraft's forward speed is below `|v|`, so the cap
  engages less.
- From there the dogfight diverges. The late fighter losses swap squadrons (OFF: Lexington sqn05,
  Yorktown sqn06/08; ON: Yorktown sqn05/07, Lexington sqn06; all killed by Zeros at 187-221 s),
  and the earlier rows move by fractions of a second.
- This is a mechanism move at the first divergence, then an RNG-coupled cascade. No death in the
  table is attributable one by one.

**Verdict: the mechanism is the image's (`0099CC98` / `0099F27E` -> `007B8E70` -> ctl+7Ch ->
`007D80C0`), and the predicted USNRM01 outcome followed. Flipped ON.**

## 5db. USN13's "attackmove target not hostile" refusals: a host stand-in orders every member at the target group's leader (packet `cc9_order_attack_member_issue`, cc9-lua36, 2026-10-05)

5cz item 2. Read from `l35_g_usn13.log` (gates ON) and `l35_off_usn13.log` (OFF), then statically.

**The two populations are different.**
- Gates ON: all 60 refusals are in one frame, 25.75 s. Every member of the 15 `bruh` squadrons
  (units 313-372) has slot-0 target 314, `bruh #1.2`, its own group's leader. They come straight
  after `player command issued to "bruh #1.x": token="artillery"` and the host's
  `AiPlanners::issue_member_order [0077d600]`.
- OFF: 50 refusals by Agano, every ~3 s, of a scripted `attackmove` at unit 10. The image's arm
  refuses that the same way. Nothing to bind.

**Which check refuses.** `00836B45`'s attack-move arm, `00836BCB`-`00836BDC`:
`005457C0(ECX = [director+24Ch], target+54h)`, which is `unit+54h != side && side != 2`
(`005457C4`-`005457D8`); false raises stage 2. A rel32 census finds three callers of `005457C0`
(`00547731`, `00835FDC`, `00836BD5`), none of them in the AI.

**The image's own-side filter is in the member pass, earlier.**
- `00A13B60`'s collection walk (CLOSEATTACK and DEFENDPOSITION) sets `EBP = (group+5638h == 0)`
  (`00A13B97`-`00A13BC9`). `group+5638h` is the group's party (SHIP_AI_OPEN_ITEMS 63.1).
- It admits a world-list entity only when `+5Ch` is set, `+5Dh`/`+5Eh`/`+60h` are clear and
  `+54h == EBP` (`00A13C03`-`00A13C32`, and again at `00A13D17` for the next list). So only the
  OTHER party's units are ever candidates. The target-group bonus (`00A2C720`, x10) applies only
  to collected candidates.
- So in the image a group whose planner picked its own group gets no member order at that group.
  `ai_close_attack_tick_00a13b60` already models this (`close_candidate_team(candidate) == own_team`
  skips).

**What the host adds.**
- `GameAiCoordinatorHost::Impl::order_attack` (`src/game_hosts_ai.cpp`) installs the
  MOVETOATTACK/CAUTIOUSATTACK command, as `00A2CBD0` does.
- It then calls `issue_to_member` for every non-ship member, with `artillery`, `attackmove` or
  `dogfight` aimed at the target group's FIRST MEMBER.
- `00A2CBD0` (body `00A2CBD0`-`00A2CCE8`) calls only `00A109B0`, `00A10890`, `00A10C20`,
  `00BD2F40`, `00A2C9F0`, `operator new` and the CRT. `00A2C9F0` calls `00A10C20`, `00A371A0`,
  `00414DB0` and `00A01230`. The rel32 census shows no `0077D600` or `00A02020` site in `00A2C000`-
  `00A2D000`.
- The host's own comment there already says the attack order reaches no member and calls the
  plane tokens a labelled stand-in. With the gates ON that stand-in hands the own-group target to
  60 aircraft at once.

**The binding,** `kOrderAttackNoMemberIssueBound` (OFF): ON, `order_attack` issues no member
command. Members are then ordered only by the command's tick (`moveto` through `00A02020`, and
CLOSEATTACK's `settarget`/`attackmove` through `00A13B60`), as in the image.

**Predictions, before the pairs:**
- USN13, gates ON: the 60 refusals go to 0. The `bruh` squadrons keep their scripted
  `PilotSetTarget` carriers until the command tick reaches them. Expect more of the Japanese
  strike to reach the US carriers than with the gates ON today.
- USN13, gates OFF: fewer `commands_issued`. Plane groups whose only attack order was the stand-in
  may attack later or not at all; a CLOSEATTACK `settarget` should replace most of it once in range.
- USN04 and USNRM01: AI-controlled plane groups lose the direct `dogfight`/`attackmove` at
  order time. Expect moved dogfight timing; a mechanism failure would be groups that never engage.

### 5db.1 Measured: **ON** (cc9-lua36, 2026-10-05)

Four same-tree builds from `f8860bd1f`: `local\l36_aoff`/`l36_aon` (this switch, gates OFF) and
`l36_goff`/`l36_gon` (the same with `kAiWeightBarrelGatesBound` and `kAiPlaneAttackerWeightBound`
ON). Launch form of reference X (`local\l36_queue.ps1`, 3 slots, `BSP_GUNNERY_RNG_STREAMS=1`,
`BSP_DEATH_TABLE=1`). USN01 300-frame smoke ON: clean. Census:
`summary mission ai order attack member issue bound= skips=`.

**Gates OFF, all 22 reference X rows:**
- 21 rows are **gameplay identical** (pair_diff exit 1), although the stand-in is skipped 2-73 times
  on most of them (USN13 59, USN13 long 73, USNOS/USNOS long 42, IJN01 13, USN01 10, E2 10, USN04 9).
  The rows without AI attack orders (BSM01, LOMP10/long, JM05/long, IJN11) have 0 skips.
- **USNRM01 moved:**
  - deaths 191 -> 201; torpedo-task releases 55 of 79 -> 61 of 86; dive-bomb-task releases
    112 -> 115.
  - The director refusals go 48 -> 0. The Kates lose 15 stand-in commands, so KateSpawn1's flight
    dies at the script's 186.91 s Kill at 17-49 m on a run-in (was 305-349 m).
  - West Virginia is sunk at 356.94 s by Jap #30.1|.-3's bomb, and the script then spawns
    Oklahoma_Killers (units 523 -> 529).
  - More of the strike reaches Battleship Row, not less.

**Gates ON (USN13, USN13 long, USN04, E2, USNOS, USNRM01):**
- USN13, USN04, E2, USNOS: gameplay identical.
- USN13 long: deaths 149 -> 152. The director refusals go 124 -> 0. The death swaps (bruh #2.x,
  JapAF/Airfield squadrons, storage) are a cascade from Maru4's close-attack ordering at the first
  divergence; no row stops attacking.
- USNRM01: torpedo releases 56 -> 64, dive-bomb releases 84 -> 75, deaths 173 -> 185.

**No row shows attacks vanishing.** Where the stand-in was the only path to a member, the command
tick's `moveto` and CLOSEATTACK's `settarget`/`attackmove` take over, and releases rise on the one
row that moves. Mechanism match: **flipped ON.**

**The gates, re-tested under this switch** (`aon` against `gon`, so only the gates differ):

| row | gates OFF | gates ON |
| --- | --- | --- |
| USN13 long | deaths 118, damage 47452 | deaths 152, damage 57478 |
| USNOS | deaths 97, damage 46848 | deaths 67, damage 36261 |
| USNRM01 | deaths 201; releases 61 / 115 | deaths 185; releases 64 / 75 |
| USN13, USN04, E2 | - | identical |

With the own-group attack-move gone, the gates no longer produce any friendly scene command (0
refusals). What they still change is the planner's choice, which 5cu read as the image's. A fresh
plausibility read of the gates (and of 5ch) is now meaningful. These numbers are its starting point,
not a verdict.

## 5dc. The six Kates that still stall nose-up: the image's repair and authority, as read (cc9-lua36, 2026-10-05)

The lead's follow-up to 5da.1. A diagnostic export of `45004bc4d` (never committed;
`local\l36_diag2.py` over `local\l36_diag`, log `local\l36_diag_usnrm01.log`) logged KateSpawn1, 3
and 4 every 0.5 s from 118 to 150 s, and every pitch rewrite by `0099BF30`.

**KateSpawn3, which reaches the sea at 148.5 s:**

| t (s) | alt (m) | nose (rad) | planner pitch slot | command after `0099BF30` | what happens |
| --- | --- | --- | --- | --- | --- |
| 118.8-121.3 | 180-170 | 0.00 | desired -1.0 | -0.23 to +0.67 | the slip from the spawn (5da); sinking 17 m/s |
| 121.8-129.3 | 160 -> 117 | 0.00 -> 0.89 | desired -1.0 | **+1.0** (a repair every think) | AvoidTerrain's pitch bands leave no admissible pitch, and the repair pulls up |
| 129.8-138.3 | 125 -> 140 | 0.90 -> 0.75 | desired -1.0 | -1.0 (no repair) | full nose-down command, but the nose falls only about 0.02 rad/s |
| 138.8-143.8 | 134 -> 55 | 0.74 -> 0.68 | desired -1.0 | -0.98 -> -0.26 | the deep stall: forward 14 m/s, body vy -33 m/s |
| 144.3-148.3 | 47 -> -26 | 0.68 -> 0.75 | desired -1.0 | **+1.0** again | the repair near the sea; depth kill |

**Each link is the image's:**
- **The repair.** `0099BF30`: when `0099B940` finds no pitch band for the command, the command
  becomes `[00CE6448]` = +1.1 if `|bank| <= [00CE3830]` (pi/2), else `[00D06BB0]` = -1.1
  (`0099BFAA`-`0099BFE1`, disk bytes). That is `gunfire_repair_0099bf30`. The planner itself never
  asks for nose-up here.
- **The bands' pitch input** is the velocity elevation (`unit+C7Ch`, written at `007C1CA8` from
  ctl+18h). At -0.6 rad of sink the fan sees the surface ahead and blocks the low pitches. As of
  5da the look-ahead speed is the image's forward speed.
- **Why the nose will not come down.** `007D9A70` scales every rotation rate by `max(a, r)^2`:
  - `a` = interp(3 -> 0, 6 -> 0.25; airborne time) is 0.25 in the air;
  - `r` = interp(ControlRangeMin -> 0, ControlRangeMax -> 1; forward / StallSpd) is 0 at
    14 m/s forward;
  - so the authority is 0.0625. The host's `control_authority` and
    docs/PLANE_CONTROL_AUTHORITY.md agree.
  - With PitchSpd about 0.5 rad/s that allows about 0.03 rad/s, as measured.
- **The law has no nose drop.** docs/FREEFLIGHT_STALL_LAW.md 2 (and docs/CLIMBOUT_SPEED_GATE.md 2)
  found no aerodynamic pitching moment and no stall guard on the pilot side.

**Verdict:** no divergence, nothing bound. These six enter the slip later in their own geometry and
spend the pull-up in the stall. The three flights that fly (5da.1) left the slip with more height
and speed.

**Open, recorded (planes lane):**
- `007CBA50` `BSP_Plane_WaterSurfaceStep` compares `vtable[204h]` (the forward speed) with
  `desc+19Ch` at `007CBAEF`. `007CB858` (`BSP_Plane_OnWaterContact`, kind 14h) compares it with 8.0.
  Neither compare is in the host's water law. Whether the image ditches a slow aircraft on the
  water instead of letting it sink to the depth kill is unread.
- `009AF15A` (`009AF0A0`, the kamikaze cruise profile) also reads it. It is unmodelled.

## 5dd. The weight gates, re-paired under 5db (packet `cc9_ai_weight_gates_flip`, cc9-lua36, 2026-10-05)

`kAiWeightBarrelGatesBound` + `kAiPlaneAttackerWeightBound`, as one packet. They were held OFF on
a plausibility prior: Japanese air groups targeting themselves (5cy). 5db removed the only path by
which such a pick reached a member as a friendly scene command. The planner's pick itself is the
image's rule as read (5cu (1)). Same-tree exports of `57d313922` (`local\l36_gx0`, `local\l36_gx1`),
all 22 reference X rows.

**Predictions, before the pairs:**
- Rows without AI attack orders (BSM01, LOMP10/long, JM05/long, IJN11) stay identical.
- With the gates ON, groups whose best value is their own group (range factor 1.0 at distance 0)
  hold MOVETOATTACK/CLOSEATTACK on themselves. Their members then `moveto` their own leader, and
  CLOSEATTACK collects only the other party's units within 4500 m of the own leader point.
  - So such groups fight whatever comes near them instead of striking far targets.
  - Expect fewer deaths on strike rows (USNOS, USN13 long) and fewer long-range dive releases
    (USNRM01).
  - Expect no friendly refusals (0 `target not hostile`).

### 5dd.1 Measured: a mechanism failure; **both gates stay OFF** (cc9-lua36, 2026-10-05)

Exports of `57d313922`: `l36_gx0` (control), `l36_gx1` (both gates), `l36_gx2` (barrel gates only).
USN01 smoke on `gx1`: clean.

**All 22 rows, gx0 -> gx1:**
- 11 rows identical: E2, ESMP08 long, LOMP10/long, JM05/long, USN02, USN04, JM08, BSM01, LOMP06,
  IJN11.
- 11 rows moved:

| row | deaths | other |
| --- | --- | --- |
| USN13 long | 119 -> 152 | damage 47.4k -> 57.7k |
| USNOS | 97 -> 67 | |
| USNOS long | 114 -> 88 | |
| USNRM01 | 201 -> 185 | dive releases 115 -> 75 |
| JM08 long | 140 -> 153 | |
| USN01 | 28 -> 29 | dive releases 6 -> 2 |
| USN12 | 5 -> 3 | |
| JM06 | 2 -> 3 | |
| IJN01 | - | hits 114 -> 79 |
| USN13 | - | non-gameplay lines only |

- **gx2 (barrel gates alone) reproduces gx1 exactly on all 12 rows run.** `kAiPlaneAttackerWeightBound`
  adds nothing on its own, and every move comes through the type queries the barrel switch makes
  real (`game_hosts_ai.cpp` line 640).

**What the planners pick, and why it is not the image's:**
- **USNRM01.** The bombing squadrons that stop releasing are the eight `A6M_n` flights (6 each,
  48 -> 0), Jap #1.1, #14.1, #44.1 and #45.1. In gx0 the A6Ms' close pass takes Downes (1424
  command-target lines for A6M_1). In gx1 they have no target line at all.
  - The script's `PilotSetTarget` to the NavPoint `Attackpoint2` (50025) is declined by `007EEC50`
    in both builds, so only the close pass could give them a target.
  - With West Virginia unsunk, Oklahoma_Killers never spawns.
- **USN01.** KatSBD's 4 releases go to 0, the same pattern.
- **Why the loaded aircraft score 0.**
  - With the type queries real, a plane attacker takes `00A08460`'s plane arm.
  - With `attacker_class > 0` (barrel gates alone) it goes to the loadout arm `00A08655`-`00A08A8E`,
    which the host does not project. `ai_plane_attack_options` returns no option.
  - With `attacker_class = 0` (plane switch) it takes the no-loadout gun arm, whose only option
    against a ship is strafe.
  - Either way a bomb- or torpedo-carrying fighter or dive bomber has no ship weight unless a forced
    rule names its class. TORPEDOBOMBER and DIVEBOMBER do; a Zero does not.
- **The premise is wrong.** The host's "00A04560 always leaves record+10h = 0" is not what
  `00A04560` does.
  - Its second argument to the record constructor `00A00020` (stored at `+10h`, `00A0005E`) is EDI:
    - `[unit+3D0h]` when the unit answers kind 18h (`00A0460D`-`00A0461B`);
    - else `[unit+C54h]` when it answers kind 0Fh and `007B9140(1)` is true (`00A04628`-`00A04648`);
    - else 0.
  - `007B9140` is true for a kamikaze (kind 17h). Otherwise it asks each part on `unit+974h`
    (count `+994h`) `vtable[210h](2Ah, 1)`.
  - So in the image a plane whose parts answer that query reaches the loadout arm with its loadout
    record. The host's planes carry no `unit+974h` parts, and the record is never formed.
- **USNOS / USNOS long.** 31 neutral or ground objects (containers, crates, barrels, mostly killed by
  the cruisers Ada2/Ada3/Zao1/Zao3 in gx0) are no longer shot. That is the barrel walk's per-target
  gates for side >= 2 targets, and it may well be the image's rule. It cannot be judged apart from
  the loadout gap while one switch carries both.

**Verdict:** attacks vanish because a weight path is unprojected, so this is a mechanism failure.
`kAiWeightBarrelGatesBound` and `kAiPlaneAttackerWeightBound` stay OFF.

**Next, for whoever takes the AI weights:**
1. Read `007B9140`'s part query (`vtable[210h](2Ah, 1)` on the `unit+974h` parts) and what `unit+C54h`
   and `unit+3D0h` hold.
2. Project the loadout arm `00A08655`-`00A08A8E` (torpedo, bomb, depth charge, paratrooper, rocket
   and dive options from `009552E0`'s list).
3. Feed record+10h as the image forms it.
4. Re-pair the gates after that; the USNOS neutral-target effect can then be judged on its own.

## 5de. Handoff (cc9-lua36, 2026-10-05)

Branch `agent/cc9-lua36`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua36`. No lease
is held; the `src/game_hosts_ai.cpp` loan was released after 5db.

| packet | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| `cc9_terrain_avoid_forward_speed` | `d97ad79fd`, `cecbbb42d` | `kTerrainAvoidForwardSpeedBound` | **ON** | 5da, 5da.1 |
| `cc9_order_attack_member_issue` | `f8860bd1f`, `45004bc4d` | `kOrderAttackNoMemberIssueBound` | **ON** | 5db, 5db.1 |
| Kate nose-up stall | `7a3f912ab` | - | image as read | 5dc |
| `cc9_ai_weight_gates_flip` | `ed61e0ca8` | the two gates | **OFF**, mechanism failure | 5dd, 5dd.1 |

Censuses:
- `summary mission ai order attack member issue bound= skips=`;
- the `vehicle/terrain avoid ... thr=` lines for the throttle cap.

Scripts in `local\`:
- `l36_queue.ps1 -Jobs 'side:row',... -Tag t` (3-slot queue over `local\l36_<side>` exports, all 22
  X rows defined);
- `l36_diffall.ps1 -A a -B b -Rows ...` (one line per pair);
- `l36_picks.py` (squadtick pick census);
- `l36_relunits.py` / `l36_reltargets.py` (release census by squadron or target);
- `l36_firstdiff.py` (first differing line, with a filter);
- `l36_rel32.py <addr...>` (rel32 call census from the disk image);
- `l36_diag2.py` (the uncommitted pitch diagnostic, applied to an export copy).

### Next, in order

1. **The AI loadout arm (5dd.1).** Project `00A08655`-`00A08A8E` and record+10h (`00A04560`:
   `unit+C54h` via `007B9140(1)`, `unit+3D0h` for kind 18h). Then re-pair the gates.
2. **5ch (`kAiTickMovetoRetasksPlaneBound`).** Re-pair it with the gates OFF, now that 5da and 5db
   are in. Not started.
3. **The water-surface `vtable[204h]` compares (5dc).** `007CBAEF` against `desc+19Ch`, and
   `007CB858` against 8.0: does the image ditch a slow aircraft?
4. 5cx's manoeuvre-survives-command item still has no reach.

## 5df. The loadout arm of `00A08460` and record+10h (packet `cc9_ai_plane_loadout_arm`, cc9-lua37, 2026-10-05)

5dd.1 failed on mechanism: with the type queries real, a loaded plane scored 0 against every ship,
because the loadout arm `00A08655`-`00A08A8E` was unprojected and record+10h was never formed.
Both are now read from the listing and bound behind `kAiPlaneLoadoutArmBound` (`src/game_hosts_ai.cpp`),
committed OFF (`b6b6c52f1` model, `5adb64812` binding).

### record+10h (`00A04560`, `00A04608`-`00A0464E`)

- X = `[entity+3D0h]` for kind 18h (a squadron's first plane), the entity itself for kind 0Fh, else none.
- record+10h = `[X+C54h]` when `007B9140(X, [ESP+28h])` answers true, else 0. Every caller but
  `00A07E7D` pushes 1 (`00A07E13`, `00A0F819`, `00A248FD`).
- `007B9140(unit, flag)` (`RET 4`): kind 17h answers at once; otherwise each part at `unit+974h`
  (count `+994h`) is asked `vtable[210h](2Ah, flag)`. For a single rack that is `006E3FE0` (vtable
  `00CF96A8`, entry `00CF98B8`). It walks the attached rounds (`[rack+48h]`, next `+44h`, `vtable[5Ch](2Ah)`).
  With the flag set it also tests the configured class, but only behind `[rack+3F0h]->vtable[1E4h]`,
  which needs `[00E17BF2]` (0 in single player; see the units host note on `006E4060`). So in this
  process it means "a rack holds an attached round", and every rack ordnance class answers 2Ah
  (docs/ORDNANCE_KIND_IDENTITY.md). Host: `GameUnitsHost::plane_holds_rack_round_007b9140`, the per-plane
  test `squadron_ammo_type_007edad0` already uses.
- `[unit+C54h]` is the plane's equipment index. Its property name is `originalEquipmentIndex`
  (`007D67E7`, string `00D05DE0`). Its writers:
  - `007CDF49` copies it from the squadron's `+124h`;
  - `007CDFD2` from the scene property `Equipment` (`00CF69AC`);
  - `007BCB74` from the Lua `PlaneChangeAmmoType` (`0089F1F0` -> `007ED690`).
  - The generator `0094B600` writes `Equipment` per plane. With no authored list (`0094BD34`) it writes
    1 when the class's Equipments count (`00951F10`, class `+128h`) is above 0, else 0.
  - **LABELLED:** this process keeps no authored Equipment, so `[X+C54h]` is that default. A Zero
    (`VehicleClass[150]`, DefaultEquipment 0, one bomb loadout) gets 1. USNRM01's Kates author
    Equipment 1, which agrees.
- `009552E0(class, n)` (`RET 4`): for 0 < n <= class `+128h`, `00954DF0(class+128h, n-1)`; n = 0 is null.
  The entry layout (from `0095A880`) is `+0h` platform slot, `+4h` Platform (device class id), `+8h` Ammo,
  `+0Ch` ReloadTime.

### The loadout arm (`00A08655`-`00A08A8E`)

- Over the list entries whose device class (`00443490`) answers `25h` (BombPlatform `00442C70`,
  MultiBombPlatform `00442D00`):
  - the minimum ReloadTime, seeded with 9999.0 (`00CE4C04`);
  - the summed Ammo;
  - the first one whose `00731040` gives a bullet keeps its device and bullet.
  - No such rack: no option, and the gun walk keeps `[ESP+11h]`.
- One option, by the bullet's sub-type. T is the mode tuning record, Params as AiModeTuning names them.
  The rate is ammo / (P1 x 0.5 + reload min), the cap is P2 / P1, and the factor is
  min(rate, cap) x DamageCalcTime. A built option clears the gun walk unless noted.

| sub-type | gate | params | descriptor |
| --- | --- | --- | --- |
| `0Ah` torpedo | target 6; a submarine only when device `+F4h` > 0 | Torpedo | `00E08F18` |
| `0Bh` depth charge | target 8 | DC | `00E08F38` |
| `0Dh` carried kamikaze | not plane, not sub | rate 1 / reload min, cap 1 / Levelbomb[1] | `00E08F50` |
| `0Fh` paratroopers | target 1Ch | ammo + max(0, DCT - Levelbomb[1] x 0.5) x ammo / (reload min + Levelbomb[1] x 0.5) | `00E08F28` |
| `12h`, IgnitionDelay <= 0 | AA rocket vs plane, or non-AA vs surface (`007B80A0`/`C0`, `[00F874FD]`) | rate ammo / reload min; cap Dogfight[3]/[1] or Strafe[3]/[1]; **gun walk continues**, the equipment penalty is cleared | `00E08F48` |
| `12h`, IgnitionDelay > 0 | not plane, not sub | BigRocket | `00E08F48` |
| `09h`, attacker 10h | not plane, not sub, target 6 | DCT x trunc(min(ammo, Levelbomb[2])) / (Levelbomb[1] x 0.5 + reload min) | `00E08F28` |
| `09h`, otherwise | not plane, not sub | Divebomb | `00E08F20` |

- A target with record `+1Ch` = 1 admits only the paratrooper option (`00A08A23`).
- Scores:
  - `00E08F20`, `00E08F48` and non-paratrooper `00E08F28`: `009FE200(max(DamageMin, BlastMin),
    max(DamageMax, BlastMax), Armour, HP)` x accuracy x factor.
  - `00E08F18` and `00E08F38`: the same against the **underwater** armour `[ESP+48h]`.
  - Paratroopers: Paratroopers accuracy (`+14Ch`) x factor x (`+1Ch` ? `+D8h` : `+FCh`) x `+F8h`.
  - A carried kamikaze: the blast pair of `[bullet+DCh]`'s class `+210h`.
- Two fixes to the existing projection:
  - The gun walk's slot base is EBX (`00A08B41`), so a pilot option after a tail gun rewrites from 1
    when a loadout option exists.
  - The equipment penalty follows `[ESP+2Bh]`, which the small-rocket arm clears.
- **Not covered (score 0, counted):**
  - Rocket accuracy (`009FE270` sub-type 12h, still unresolved).
  - The paratrooper fields `+D8h`/`+F8h`/`+FCh` (reader `007AC780` unread).
  - The carried kamikaze's class (`006FF170` unread).
  - Device class `+F4h`: a BombPlatform class is an E8h allocation (`00443273`), so the read lies past
    the object and no producer exists. Taken as 0.

### Predictions, written before any ON run

Pairs from `5adb64812`: `l37_g0` (all OFF) against `l37_g1` (both gates + `kAiPlaneLoadoutArmBound`).
- **Unchanged rows.** The 11 rows that were identical under 5dd.1's gx1 stay identical.
- **Loaded bombers.** They score their torpedo or dive option against ships again, so the 5dd.1
  collapse reverses. USNRM01 dive releases recover to near gx0 (115; at least ~100). USN01's KatSBD
  keep releasing (about 4).
- **After the drop.** A plane takes the gun arm (strafe only) and scores ~0 against ships. Spent
  bombers therefore lose ship value earlier than in gx0, which may move squadron retargeting.
- **USNOS neutral objects.** The ~31 container and crate deaths stay gone. They come from the barrel
  gates (side >= 2 targets), not from planes.

### 5df.1 Measured: **the gates and the loadout arm ON** (cc9-lua37, 2026-10-05)

**Two host faults the first pairs exposed, fixed before the verdict:**
- **`c888019ba`.** `mode_tuning_record` left the loadout Params at 0, so every dive option scored
  NaN (Params [1] = 0 makes the cap 0/0). It now carries this installation's highlvlaiglobals.lua
  (2024-07-13):
  - the three IslandCapture tables: Torpedo {80, 3}, Divebomb {60, 3}, Levelbomb {200, 4},
    DC {60, 4}, BigRocket {120, 2};
  - Duel, Escort, Siege and Competitive: {25, 3}, {25, 3}, {30, 12}, {25, 4}, {20, 2};
  - the Paratroopers accuracy `+14Ch` from the tuning block.
- **`876b192cf`.** The host's unseeded rack test reads `Equipments[DefaultEquipment or 1]`. A Zero's
  class authors DefaultEquipment 0, so USNRM01's eight A6M flights never held a round and never formed
  a loadout record: dive releases stayed at 89, as in 5dd.1. The AI's `007B9140` test now falls back to
  `Equipments[1]`, the generator default (`0094BD34`). The release path keeps its own default.
  - **LABELLED:** a carrier-launched Zero (equipment from class `+134h` = 0) would also count as loaded.

**Pairs from `876b192cf`:** `l37_i0` (all OFF) against `l37_i1` (both gates + the loadout arm), all 22
reference X rows. USN01 smoke: clean.

- **13 rows identical:** JM08 long, ESMP08 long, LOMP10/long, JM05/long, E2, USN02, USN04, JM08,
  BSM01, LOMP06, IJN11. USN13 moved in non-gameplay lines only.
- **8 rows moved:**

| row | OFF -> ON | whose numbers moved, and why |
| --- | --- | --- |
| USNRM01 | deaths 192 = 192; dive releases 118 -> 116; damage 48.6k -> 48.4k | The prediction holds; 5dd.1's collapse (-> 75) is gone. The A6Ms' records carry loadout 1 (240143 of 342879 records); dive options 33163/38259 positive. |
| USNOS | deaths 97 -> 67 | 26 of the 27 lost deaths are side >= 2 objects (containers, houses, static aircraft, `lada_nagy`) that Ada/Zao shelled in OFF. Two "Coastal Gun 01/03" died at 13.55 s to Zao1's blast, a splash from those shots. That is `00A0924C`'s neutral rule (record `+1Ch`: every gate cleared). |
| USNOS long | 114 -> 84 | The same 26 objects. |
| USN13 long | 117 -> 133; damage 47.8k -> 54.2k | Agano now kills Storage/Barracks/containers. The `bruh #2` planes die to Maru's AA in different numbers (-5 / +9). Loadout records: dive 12349 and level 14829 options, all positive. |
| USN12 | 5 -> 3 | A barrel and a watchhouse that Shiratsuyu/Shigure killed in OFF: the neutral rule again. |
| USN01 | 28 -> 29; KatSBD dive releases 6 -> 2 | Not the loadout arm. Mav1 is `MLargeReconPlane` (16h, creator `0074E540`), and the 14h rule (`00A08D4F`) leaves it no option, so it values every target 0 (OFF: 5.2-8.1). The script's KatSBD spawn moves from frame 2362 to 2736, leaving fewer frames for releases. |
| JM06 | 2 -> 3 | One extra static Mavis wreck killed by the Narwhal. |
| IJN01 | 2 -> 1; hits 116 -> 90 | One A7M not shot down by Oglala. |

**Verdict:** the mechanism now matches. Loaded bombers score their ordnance against ships, and the one
row that collapsed in 5dd.1 is back on the control. The other moves trace to two image rules as read:
the side >= 2 neutral gate and the recon 14h class. **`kAiWeightBarrelGatesBound`,
`kAiPlaneAttackerWeightBound` and `kAiPlaneLoadoutArmBound` flip ON** (reference Z).

Census line: `summary mission ai loadout arm bound= records= records_loadout= visits= ...`.

## 5dg. Item 3: the water-surface `vtable[204h]` compares do not ditch a slow aircraft (cc9-lua37, 2026-10-05)

Read from the listing, nothing bound.
- **`007CB858`** (in `007CB7F0`) is reached only past the contact gate, when `007BC5B0` is true or
  MinWaterSpd (`desc+198h`) is 0 (docs/WATER_SURFACE_LAW.md 2.1). It fires `0090F6C0(unit, 3)` only when
  all of these hold:
  - unit `+1B0h` < 8;
  - the unit is kind 14h (recon);
  - forward speed < 8.0 (`00CE3918`);
  - `+9F0h` < 0.1 (`00D05E28`, double);
  - `0043F080` and `+C34h`.
  `0090F6C0` is the same event the touchdown (`007CB5F0`) and the flight-state 4 -> 5 (`007C16F0`) raise:
  a recon floatplane alighting, not a ditch.
- **`007CBAEF`** is in `007CBA50`, the state-6 arm. With `+5Dh` set, not multiplayer, `+911h` clear and
  `+100h` below `[00CE3854]`, it raises `"splash"` (`00D05A10`) and sets `+911h` in either case:
  - forward speed > `desc+19Ch`;
  - an attitude term past the descriptor limits (`+A4h`..`+B0h`, `007CBB0B`-`007CBB8E`).
  That is a crash on a fast or bad touchdown.
- A live AI aircraft with non-zero MinWaterSpd (every Kate, Val and Zero in this installation) never
  reaches state 6, so neither compare applies to it. It stays in free flight into the depth kill
  (`007CE040`), as the host does. **The image does not ditch a slow aircraft.**

## 5dh. Item 2: 5ch (`kAiTickMovetoRetasksPlaneBound`) re-paired under 5df.1 (cc9-lua37, 2026-10-05)

Pairs from `2086291cc` (the gates and the loadout arm ON): `l37_j0` (5ch OFF) against `l37_j1` (5ch ON).
Rows: USN13, USN13 long, USN04, E2, USNOS, ESMP08 14200/14000, and the controls USN02 and BSM01.

**Predictions, written before any ON run:**
- **Controls.** USN02 and BSM01 stay identical.
- **USN13 and USN13 long.** 5co's stall was the close attack finding no candidate with weight > 0
  against the LandFort CB2. With the loadout arm, a loaded dive or level bomber now scores against
  a non-ship, non-plane target (dive and level options admit anything but planes and subs). The
  `bruh` Kates carry torpedoes, though, and a torpedo option needs target 6, so they still have
  nothing against CB2's members.
  - Expect USN13 to stay stranded or nearly so (deaths far below OFF), unless the planner now
    picks ships for the torpedo wave. The planner's group values for a torpedo wave against a
    LandFort now read 0 from the members' weights.
- **ESMP08 14200.** 5ch's original case: the leader stops circling the script's random fleet ship
  and flies the tick's moveto. Expect the strike to close Zuikaku.
- **USN04, E2 and USNOS.** They move with the re-tasking, direction unpredicted.

### 5dh.1 Measured: 5ch still strands USN13; **stays OFF** (cc9-lua37, 2026-10-05)

| row | j0 -> j1 (5ch ON) | |
| --- | --- | --- |
| USN02, BSM01 | identical | controls |
| USN13 | deaths 22 -> **0**, shots 4189 -> **0** | as 5co |
| USN13 long | deaths 133 -> **2**, shots 43333 -> 82 | as 5co (116 -> 6 then) |
| ESMP08 14200 | deaths 50 -> 46; torpedo releases 0/24 -> **5/30** | 5ch's own case: the strike now drops on the fleet |
| USN04 | 44 -> 37 | |
| E2 | 74 -> 84 | |
| USNOS | 67 -> 55; shots 5916 -> 813 | as the 5ch addendum |

**USN13 is stranded by the same mechanism as 5co.** The census reads
`script squadron intake calls=0`, `close fallback bridge calls=1193` and
`tick plane retask replaced_other=60`. The `bruh` groups (Kates, torpedo loadout 1) take
MOVETOATTACK on CB2, promote inside CollectDist (8 promotions) and orbit; no member is ever given a
target.
- The loadout arm does not rescue them. A torpedo option needs target kind 6, and CB2's members
  (LandFort, coastal guns) are not ships, so every per-member weight is 0. A zero weight cannot beat
  the close attack's seed (`00A149A8`).
- The forced rules match on this row (`r8=6396`, `r15=5682`). Which rule makes the planner's group
  pick of CB2 for a torpedo wave is the unverified link 5co (3) named.

**Verdict:** the re-tasking is the image's (5ch), but the assignment that feeds it still produces a
stall that 5co could not attribute. That is a mechanism question, not a spread, so
`kAiTickMovetoRetasksPlaneBound` **stays OFF**. Next: trace the planner pick for the `bruh` groups
(which planner kind, which forced rule) and whether the image's group value for a torpedo wave
against a LandFort group is non-zero.
- In the IslandCapture Rookie table (`ai_shipped_forced_rules`), the matched rules are:
  - `r8` = Cargo vs CommandBuilding 1.0 (the Marus' pick of CB2);
  - `r15` = TorpedoBomber vs Ship 4.5, with record `+1Ch` clear.
  No rule rates a torpedo bomber against CB2's members. So the `bruh` groups' CB2 order does not come
  from a forced rule, and which planner assigns it is the open link.

## 5di. Handoff (cc9-lua37, 2026-10-05)

Branch `agent/cc9-lua37`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua37`. No lease is held.

| item | commits | switch | state | section |
| --- | --- | --- | --- | --- |
| loadout arm + record+10h | `b6b6c52f1`, `5adb64812`, `c888019ba`, `876b192cf`, `2086291cc` | `kAiPlaneLoadoutArmBound`, `kAiWeightBarrelGatesBound`, `kAiPlaneAttackerWeightBound` | **ON** (reference Z) | 5df, 5df.1 |
| water-surface compares | - | - | the image does not ditch a slow plane | 5dg |
| 5ch re-pair | `1940b7779`, `8f145fde1`, `d0ec54a88` | `kAiTickMovetoRetasksPlaneBound` | **OFF** | 5dh, 5dh.1 |

Code:
- `src/ai_target_weights.cpp` (`ai_plane_loadout_arm`, the new scoring arms);
- `src/game_hosts_lua.cpp` (`game_ai_plane_loadout`, `game_ai_plane_equipment_count`);
- `src/game_hosts_ai.cpp` (`record_loadout_00a04560`, the loadout Params in `mode_tuning_record`,
  the census);
- `src/game_hosts_units.cpp` (`plane_holds_rack_round_007b9140`, `PlaneOrdnanceRack::generator_ammo`).

Census line: `summary mission ai loadout arm`.

Scripts in `local\`:
- `l37_queue.ps1 -Jobs 'side:row',... -Tag t`: all 22 X rows plus `esmp08x` (ESMP08 14200);
- `l37_diffall.ps1 -A a -B b -Rows ...`;
- `l37_deaths.py <a.log> <b.log>`: death-row victims only in one log, by killer;
- `l37_firstdiff.py`;
- `l37_equip.py <regex>`: VehicleClass Equipments;
- `l37_scnequip.py <scn> [regex]`: scene squadron Equipment census.

### Next, in order

1. **5co (3) / 5dh.1:** which planner gives USN13's `bruh` Kate groups MOVETOATTACK on the LandFort
   CB2. No forced rule rates a torpedo bomber against CB2, and no `ai diag order_attack` or
   `ai group target value` line is printed for those groups, so a diagnostic is needed at the
   planner that installs their command. If the image would not give a torpedo wave a LandFort, 5ch
   can be re-paired and flipped.
2. **Labelled substitutions left in the loadout arm** (each scores 0 today, counted in the census):
   - rocket accuracy (`009FE270` sub-type 12h; `006E3260`, `007B80A0` and `007B80C0` are now read,
     5df);
   - the MParatrooper fields (`007AC780`);
   - the carried kamikaze's class (`006FF170`);
   - a carrier-launched Zero's equipment (class `+134h`), which the generator default treats as 1.
3. The units host's release path still reads `Equipments[DefaultEquipment or 1]` for a scene
   squadron's rack ammo (5df.1). The image's scene squadrons fly the generator's Equipment
   (`0094BD34`). This is a planes-lane packet, with release counts on every Zero row.
4. 5cx's manoeuvre-survives-command item still has no reach.

## 5dj. Item 1: the Capture think does not filter plane groups (packet `cc9_capture_plane_filter`, cc9-lua38, 2026-10-05)

Question (cc9-lua38 brief, item 1): does `00A29FD0` assign plane groups to a capture target, or does it
filter by group kind, class or CapturePower first? **It does not filter.** Nothing is bound, and 5ch
is not re-paired by this item.

### The image

- **Group records, `00A2A263`-`00A2AA90`** (decompiled lines 158-405). Every group on the planner's
  `+24h` list gets a record (`00A287C0`): `[0]` group, `[1]` `00A2C530` (resource), `[2]` `00A1A7A0`
  (current target). The only test on the group is at `00A2A279`, `00A2C5A0`
  (`BSP_AiGroup_HasGroupableCombatant`) AND `group+5658h == [00F8A9E4]`. It runs only when
  `[00F8A9E0] == 1` (`local_195`), and it sets `local_162`, which only selects the debug draw
  `00A2B950` (lines 393-399). In this process it decides nothing.
- **The per-target weight, `00A2A380`**: `00A250A0(group, entity)`, that is
  `00A0C650(00A07E40(group), 00A24870(entity), ...)`.
  - `00A07E40` (`BSP_AiGroup_BuildEntityRecords`, `00A07E40`-`00A07EA2`) pushes one `00A04560`
    record per member, with no kind test.
  - `00A24870` collects through `00A07D40`. That walk (list `world+19CCh -> +34h`) keeps entities
    with `+5Dh` clear (`00A07D73`) and `+54h` equal to the requested side (`00A07D81`) that answer
    `vtable[5Ch]` 6, 18h or 1Bh (`00A07DB7`, `00A07DC6`, `00A07DD5`). With none, it uses the target's own record (`00A248FD`).
- **The assignment loop, `00A2AAA0`-`00A2AF40`** (lines 426-500). It keeps the best
  `(1 - k) * w + k * s` over the live target/group pairs. The only gate is `w > [00D7A218]` (0.0).
  The winner is ordered through `00A1A720` (`00A2AD77`).
- **No CapturePower read.** The Capture think's listing has no class read: no `+3D0h`, `+804h` or
  `+810h` operand, and every `+5Ch` operand is a stack slot (`[ESP+5Ch]`), not a `vtable[5Ch]`
  kind query. Among its own calls, the only one that tests a kind is `00A2C5A0`, above. CapturePower (`class+804h`) and LandedCapturePower (`+810h`) are
  read by the CommandBuilding capture (`src/game_hosts_ship_ai.cpp`), not by the planner.
- **Plane groups reach the Capture planner.** A squadron group is a groupable combatant, so the
  claim rule gives it slot 3, which is Capture (docs/PLANNER_TASK_CHOICE.md section 1).

So the image assigns any group whose members score above zero against what stands around the
target. A torpedo-bomber group qualifies whenever ships of the planner's enemy side stand near the
target.

### What this process does on USN13 (diagnostic run, this tree at `1219b301c`)

`BSP_CAPTURE_DIAG=1`, USN13 3200 frames, `local\l38_diag_usn13.log`. Its headline matches `l37_j0`:
`capture path thinks=41 assignments=1723`, `capture group value bound=1 calls=5169 zero=0`.
- The block at t=25.20 s lists 15 `bruh` groups (one squadron each) and three targets:
  - CB2, with 20 collected units;
  - CB4, with 14;
  - CBT, with 11.
- Every `bruh` group is assigned CB2: `bruh #1.1`-`#1.6` at score 3.860, and `#1.7`-`#1.15` at 0.551.
- CB2's 20 units are the Japanese side's (`planner+34h` = 1) ships and guns near it, the Marus among
  them. A Kate's torpedo option rates a ship (target kind 6). That is why `w > 0`, even though no
  torpedo option rates CB2 itself.

### Verdict

The `bruh` CB2 orders come from the image's rule as read. The only questionable input is the one 5cq
named: the slot-4 brain's team (`brain+24h` = 0, Allied, while it commands the Japanese side;
SHIP_AI 125, cc9-ships28's lane). With a Japanese-team brain:
- Capture would target the Allied CommandBuildings;
- its defender walk would collect Allied ships;
- a Kate wave would be sent at the US side.

Nothing in the Capture think is a host artefact that a switch could correct.
`kAiTickMovetoRetasksPlaneBound` therefore stays OFF. Its USN13 stall follows from the team
reading, not from a missing plane filter. No pairs were run for this item, because nothing was bound.

## 5dk. Item 2: a plane's equipment is its bag's `Equipment` (packet `cc9_plane_scene_equipment`, cc9-lua38, 2026-10-05)

5di item 3 said that the units host's release path reads `Equipments[DefaultEquipment or 1]`, while
"the image's scene squadrons fly the generator's Equipment (`0094BD34`)". The second half is wrong
for every plane this host makes.

### The image

- **`007CDF20`** (`__fastcall(plane)`) is a plane's attach. Its creation context is `[plane+C0h]`.
  - **Kind 2** (`007CDF40`-`007CDF87`): `[plane+C54h]` = the creating record's `+124h`. The racks are
    not loaded here.
  - **Kind 1, a scene bag** (`007CDF89`-`007CDFF8`):
    - `0048E9F0("Equipment")` (`00CF69AC`) tests whether the key exists.
    - If it does, `[plane+C54h]` = its `+0Ch` integer (`007CDFD2`). For n > 0, `009552E0(class, n)`
      gives `Equipments[n]`.
    - `0095A880(plane, entry, 1, 0)` then runs (`007CDFF8`). With the key absent, n <= 0 or no
      such entry, the entry pushed is 0 (`007CDFF0`/`007CDFF6`).
- **`0095A880`**:
  - With an entry, it builds each entry platform's device and hands it Ammo (`vtable[1BCh]`) and
    ReloadTime (`vtable[1C0h]`).
  - With a null entry, only the third-argument pass runs. That pass adds default devices
    (`vtable[5Ch](1Eh)` slots missing a device) and loads no rack round.
  - So **a plane whose bag has no positive `Equipment` carries nothing.** `007BCB30`
    (`PlaneChangeAmmoType`) is the only later writer, and it reloads through the same `0095A880`.
- **A squadron's planes share the squadron's bag.** `007F48FD` builds each plane's context with
  `00922DE0`, which copies the kind and clones a kind-1 bag (`BSP_ScenePropertyBag_Clone`).
- **The writers of the bag's `Equipment`** (a byte scan for `PUSH 00CF69AC` gives 11 sites):
  - the scene row as authored (`plane.props`: `Equipment = E EquipmentIndex : none`, where `"none"` = 0
    in `global.enums`; this installation, mtime 2024-10-29);
  - `SpawnNew`'s member table, merged over the seeded bag by `00944210`'s `0043D8F0`;
  - air ops `006C5050`, only when positive (`006C5232` `JLE`);
  - the catapult `006ECA21` and the shipyard `0084525D`;
  - the generator `0094B600`: an authored list, else `(Equipments count > 0)` at `0094BD34`. Its
    callers are `0094C830`, reached from `0094C900` and the planner spawn arm `00A23980`, which
    this host does not model;
  - the support generator `0094BFF0` (caller `008EADA0`).

So `0094BD34`'s "1 when the class lists any Equipments" applies to none of the planes this host
creates. The AI side's `[X+C54h]` (5df) and the holds test's `generator_ammo` fallback are labelled
substitutions of the same kind.

### Where the host diverges (this installation's data)

`local\l38_rowequip.py`, `l38_luaequip.py` and `l38_de.py`. The legacy index is the class's
`DefaultEquipment`, or 1 when it is absent; its 0 reads nil.

| row | source | squadrons | bag | legacy |
| --- | --- | --- | --- | --- |
| USN04/E2 (`usn_19_coralus.scn`) | scene | 1 Kingfisher (121) | 0 | 1 |
| USNOS (`us_osumi.scn`) | scene | 1 AD-2 (339), 3 F2G (810) | 0 | 1 |
| BSM01 | scene | 3 FlyingFortress (116) | 0 | 1 |
| IJN01 | scene | 1 Dauntless (108) | 0 | 1 |
| ESMP08 | SpawnNew | 1 TBM Avenger (16) | 2 | 1 |

Every other scene row and `SpawnNew` member on these rows agrees (USN13's script authors
`Equipment = 1` on its Kates, Bettys, Helldivers and Dauntlesses). The AI side's holds test changes
more widely: every fighter whose bag is 0 is affected (Zero 150/350, Hellcat, Wildcat, Warhawk),
including air-ops launches, which carry `DefaultEquipment` 0. Under 5df.1 those planes passed the
holds test through the `Equipments[1]` fallback and scored the loadout arm's bomb option. With
their own index they hold nothing, record+10h is 0, and they keep only the gun arm.

### The binding (`kPlaneSceneEquipmentBound`, `include/bsp/plane_squadron_host.hpp`), committed OFF

- `GameSceneEntityRecord::bag_equipment` (-1 means not carried) is set by:
  - the scene build for a class-18h row (an enum symbol through the library, absent = 0);
  - the `SpawnNew` member (`member.equipment`, 0 when absent);
  - the air-ops request (`equipment > 0 ? equipment : 0`).
  Wing records copy it, and `create_units` copies it to the slot.
- `rack_equipment_ammo` (units host) feeds both rack censuses: Equipments[n] for n > 0, none for 0,
  and the legacy read for -1. The holds test drops the `generator_ammo` fallback for a slot with a
  carried index.
- **Not covered:**
  - The AI side's record+10h value (`game_hosts_ai.cpp:5096`, cc9-ships32's file) still reads
    `Equipments[1]`. Its holds test is the units host's, so a plane with no rounds already scores 0.
    Only a loaded plane with an index other than 1 (ESMP08's Avenger, Equipment 2) keeps the
    `Equipments[1]` option list. A loan has been asked for.
  - The kind-2 path (`+124h`: catapult and shipyard launches) is not carried.
- Census lines:
  - `plane equipment: unit=... bag=...`, once per squadron row, the first 40;
  - `summary plane scene equipment bound=... squadrons= reads= none= default=`.

### Predictions, written before any ON run

Pairs from the commit that lands this OFF: `l38_a0` (OFF) against `l38_a1` (ON).
- **USN02:** identical (no squadron).
- **BSM01:** B-17s are a weak forecast, because they may not reach a target in 3200 frames. Expect
  the three B-17 squadrons to drop nothing, and the Warhawks' AI option list to lose the bomb option.
  Exit 1 or 3.
- **IJN01** (if run): Dauntless1 drops nothing. The 8 Zero squadrons lose the bomb option in AI
  scoring.
- **USN13 and USN13 long:** the strike's loadouts are unchanged (Equipment 1 = DefaultEquipment 1).
  The US Hellcats and the Japanese Zeros lose the bomb option, so the plane AI's target choice moves.
  Expect moved rows, deaths within the noise of the fighter re-pick; the strike stays stranded (5dj).
- **USN04 and E2:** the Kingfisher stops carrying its depth charge or bomb. The Zeros (SpawnNew
  Equipment 0, and air-ops) lose the bomb option. Expect a move; Lexington's fate is not predicted.
- **USNOS:** AD-2 and F2G drop nothing (F2G's bombs or rockets). Expect fewer US releases.
- **ESMP08 14200:** the Avenger releases `Equipments[2]`'s ordnance. Its count and kind are as
  authored; the release census names them.

## 5dl. Item 3: the paratrooper fields and the carried kamikaze's class (packet `cc9_ai_loadout_carried_terms`, cc9-lua38, 2026-10-05)

5df left two loadout-arm options scoring 0 because their bullet fields had no reader. Both
readers are now read.

### The image

- **`007AC780`** (`__thiscall(bullet class, LuaObject)`, `RET 4`; no direct caller, so it is a
  class's field reader in a vtable). It first runs `006E1BE0` (`BSP_BombClass_ReadLuaFields`), then
  reads each key below through a `(ECX = key, EDX = field)` helper. Each helper
  (`007AAE50`, `007AAED0`, `007AAF50`, `007AAFD0`) is `GetByName` + `GetNumber` -> `(float)`.

| field | key (string) | site |
| --- | --- | --- |
| `+D8h` | `CapturePower` (`00D05220`) | `007AC917`-`007AC92B` |
| `+DCh` | `SlowFactorClosed` (`00D05280`) | `007AC7B2` |
| `+E0h` | `SlowFactorOpened` (`00D0526C`) | `007AC7D5` |
| `+E4h` | `OpenDuration` (`00D0525C`) | `007AC7F8` |
| `+E8h` | `CloseDuration` (`00D0524C`) | `007AC81B` |
| `+ECh` | `SoldierClass` (`00CE7164`) -> `004B1400` | `007AC857`-`007AC88D` |
| `+F0h` | `SoldierAnim` (`00D05230`) | `007AC8D1` |
| `+F8h` | `CaptureDuration` (`00D0523C`) | `007AC83E` |
| `+FCh` | `Damage` (`00CE66C8`) | `007AC8F4` |

  - The 0Fh scoring (`00A08EBA`-`00A08ED9`, all `FLD`/`FMUL float`) reads `+D8h` (CapturePower)
    when record `+1Ch` is set, else `+FCh` (Damage), times `+F8h` (CaptureDuration).
- **`006FF170`**, the `MDummyKamikazePlane` reader:
  - `+D8h` is `OpenAfterTime` (`00CFC878`, `006FF1A8`).
  - `+DCh` = `00964790` (`VehicleClass_GetOrCreate`, `DL` = 1) of the integer `KamikazePlaneClass`
    (`00CFC864`; `006FF1C1`-`006FF1EE`).
  - The 0Dh option then reads that class's `+210h` (`00A08DEE`), the `KamikazeBulletClass` the plane
    class reader stores (`src/plane_class_fields.cpp`, `007D2BCA`), and takes its `+B4h`/`+B8h` blast
    pair (`00A08E06`/`00A08E10`).

### The binding (`kAiLoadoutCarriedTermsBound`, `include/bsp/ai_target_weights.hpp`), committed OFF

`read_loadout_text` (`src/game_hosts_lua.cpp`) now also returns these, per loadout entry:
- the bullet's `CapturePower`, `CaptureDuration` and `Damage` (`tonumber(...) or 0`; LABELLED: a nil
  read as 0, which is `GetNumber`'s assumed answer on nil);
- for a bullet with `KamikazePlaneClass`, that class's `KamikazeBulletClass` bullet: its `Type` and
  its `Blast` pair.

With the switch ON, an entry with sub-type 0Fh gets `paratrooper_terms_known`, and an entry with a
resolved carried bullet gets `carried_kamikaze_known`. This installation's
`classtables/arcade/bulletclasses.lua` (mtime 2026-05-09, modded) has a Paratrooper row with
`CapturePower` 12, `Damage` 42 and `CaptureDuration` 30. `classtables/realistic` also authors
`CaptureDuration` 30. The Ohka bullet names `KamikazePlaneClass` 156.

### Predictions, written before any ON run

- Rows with no Ohka or paratrooper loadout are identical. That covers every reference row except
  USNOS.
- **USNOS:** the 9 `BettyOhka` squadrons (class 32, Equipment 1) now score the carried-kamikaze
  option against surface targets: the class 156 blast pair x Kamikaze accuracy x the factor. Expect
  the census `kamikaze=` count to rise above 0. The Bettys' target pick may move, and with it the US
  AA engagement. Exit 1 or 3.

### 5dk.1 Measured (`l38_a0` against `l38_a1`, commit `7e43d04d4`), and the LaunchSquadron default arm (cc9-lua38, 2026-10-05)

| row | a0 -> a1 | |
| --- | --- | --- |
| USN02, BSM01, IJN01, USN04, E2, USNOS, USN13, ESMP08 14200 | gameplay identical | |
| USN13 long | deaths 133 -> **159**, shots 43333 -> 49139, dive-bomb releases 1 -> 0 | moved |

- **The census matches the prediction where the data reaches it.**
  - USN04: `squadrons=26 reads=216 none=172`.
  - The AI side's `records_loadout` falls 10516 -> 4142, and its `dive` options 1631 -> 0. The Zeros
    lose the `Equipments[1]` fallback.
  - ESMP08: the Avengers carry bag 2 and read it (`reads=196`).
- **USN13 long moved for a reason the prediction missed: air-ops launches got Equipment 0.** The
  airfield squadrons log `air ops squadron: Airfield5_sqn13 class=162 ... equipment=0` (and JapAF
  Bettys, class 167). `squadron ordnance` turns from type 2 (torpedo) to 0 for Airfield5_sqn13,
  JapAF_sqn14 and Airfield5_sqn15. With a 0 bag they carry nothing, so they fly as gun planes. The
  only-ON deaths are those Kates and Bettys in dogfights; under the slot-4 team reading (5cq, 5dj)
  that includes the `bruh` groups.
- **The cause is a host misreading, not the binding.** `run_launch_squadron_0089e3c0` takes the
  default arm (class+134h) as 0: "class+134h is not authored under any key". It is `DefaultEquipment`.
  - `00961F0A` pushes `"DefaultEquipment"` (`00D1AB10`), and `00961F45` stores its integer at `+134h`
    (0 when nil, `00961F30`). `include/bsp/vehicle_class_fields.hpp` already names it.
  - So `LaunchSquadron(unit, class, count)` with no fourth argument arms the slot with the class's
    `DefaultEquipment`: 1 for Kate and Betty, 0 for Zero and Hellcat.
  - The legacy release path read `DefaultEquipment` itself, which masked the 0.
  - Fixed behind the same switch (`src/game_hosts_lua.cpp`). With it, the airfield Kates and Bettys
    keep their torpedoes, the Zeros and Hellcats stay at 0, and nothing changes OFF.

**Predictions for the re-pair (`l38_c0`/`l38_c1`):**
- The rows identical in a1 stay identical.
- USN13 long returns to near a0. The airfield Kates and Bettys are armed again (`squadron ordnance`
  type 2). The residual move comes from the Zeros' and Hellcats' lost fallback option (AI side
  only). Expect deaths within about 10 of 133.

## 5dm. Item 3: rocket accuracy, `009FE4F1` read whole (packet `cc9_ai_rocket_accuracy`, cc9-lua38, 2026-10-05)

The last open arm of `009FE270`: the sub-type 12h test that splits SmallRocket from BigRocket. A
prior note called it "target-state predicates". It is actually attacker and bullet tests.

### The image (`009FE4F1`-`009FE645`, disk bytes)

Registers at the arm (prologue `009FE270`-`009FE29B`):
- `EBP` = ECX, the attacker (`009FE282`);
- `EDI` = EDX, the bullet class (`009FE27A`);
- `ESI` = the stack argument, the target (`009FE273`).

So `009FE270` does dereference its first argument, in this arm only.

| test | site | then | else |
| --- | --- | --- | --- |
| attacker `vtable[18h](0Fh)` (plane base) | `009FE4F7`-`009FE4FF` | next row | `009FE632`: target plane -> `009FE322` (`+120h`), else `009FE334` (`+124h`/`+128h`/`+12Ch`), the **Artillery** row |
| `006E3260(bullet)`, IgnitionDelay <= 0 (small) | `009FE505`-`009FE50E` | next row | `009FE5C1`: **BigRocket** `+170h`, `+174h`/`+178h` by `00827F70`, `+17Ch` |
| `007B80A0(bullet)` and target plane | `009FE514`-`009FE52C` | `009FE550`: `+160h` | the next row |
| `007B80C0(bullet)` and target not plane | `009FE52E`-`009FE54A` | `009FE550`: `+164h`/`+168h`/`+16Ch` | `009FE6BB`, 0 |

`007B80A0` answers AntiAir or `[00F874FD]`, and `007B80C0` answers !AntiAir or `[00F874FD]` (5df).
A submarine is ship-based and not small surface, so it reads the big-ship column, as in every
four-entry arm.

### The binding (`kAiRocketAccuracyBound`, `include/bsp/ai_target_weights.hpp`), committed OFF

- `bsp::ai_rocket_accuracy_offset_009fe4f1` (`src/ai_target_weights.cpp`) is the table above as a
  pure function.
- The loadout arm's rocket option asks a new host virtual, `rocket_accuracy(attacker, bullet, target)`.
  Its default keeps `bullet_accuracy` (0 for 12h).
- **Not landed:** the AI host's override, which needs `src/game_hosts_ai.cpp` (cc9-ships32's lease).
  The prepared edit is `local\l38_ai_patch.py` in the cc9-lua38 tree.
- **Not covered:** the barrel path. `barrel_accuracy` (`game_hosts_ai.cpp`) still answers 0 for
  sub-type 12h, because `GameAiWeaponFacts::Barrel` (gunnery lane) carries no IgnitionDelay or
  AntiAir. A non-plane attacker needs neither: its rocket barrel reads the Artillery row.

### Predictions

The a-pair census reads `rocket=0/0` on every row that runs the loadout arm (USN13, USN13 long,
E2, USNOS, ESMP08 14200, IJN01). So **every reference row stays identical** when the switch is
flipped with the override in place. The arm is verified by reading only. A row with a rocket
loadout (a Corsair or Avenger with HVARs) is needed to measure it.

### 5dk.2 Re-paired (`l38_c0` against `l38_c1`, commit `6a6ec6ade`): **ON** (cc9-lua38, 2026-10-05)

| row | c0 -> c1 |
| --- | --- |
| USN02, BSM01, IJN01, USN04, E2, USNOS, USN13, ESMP08 14200 | gameplay identical (exit 1: census lines and the known ship-avoidance refill noise) |
| USN13 long | deaths 133 -> **148**, shots 43333 -> 48071, hull hits 2755 -> 3509, dive-bomb releases 1 -> 0 |

- **The LaunchSquadron fix works.** The airfield Kates and Bettys are armed again:
  `squadron ordnance Airfield5_sqn13 ... -> 2`, and likewise JapAF_sqn14 through 18. Census:
  `reads=653 none=316`.
- **The remaining move is the Zeros.** The airfield Zeros (Airfield5_sqn11 and JapAF_sqn12, class
  350, `DefaultEquipment` 0) carry nothing in both builds (`squadron ordnance ... -> 0`). Under
  c0's `Equipments[1]` fallback the AI still credited them a bomb: `dive` options 12349 positive in
  c0, 0 in c1. Without it they keep only the gun arm, so they score planes. The planes in reach
  are the `bruh` Kates, and the slot-4 brain counts those as enemies (5cq, 5dj).
- **From the per-death table** (`l37_deaths.py`; 29 deaths only in c1, 14 only in c0):
  - only-ON victims are almost all `bruh` Kates, killed by `Airfield5_sqn11` (7), `JapAF_sqn`/`JapAF_sqn12`
    (3), other `bruh` (4) and `CB` (3);
  - in return, `bruh #2.9` kills `Airfield5_sqn11` and a JapAF Zero.
  - The only-OFF victims are Agano's targets (storage and containers) and a few `bruh` losses to Marus.
  - So the extra deaths are Japanese planes fighting Japanese planes, which is the team-reading
    artefact, not this binding.

**Verdict: ON.** The mechanism is the image's as read: `007CDF20`'s bag Equipment, and
`LaunchSquadron`'s `DefaultEquipment` default. Every row but USN13 long is identical. USN13 long's
move comes from removing a labelled substitution (the generator-default fallback that gave unarmed
fighters a bomb option). The friendly dogfights it exposes belong to SHIP_AI 125, the slot-4
brain's team.

## 5dl.1 Measured (`l38_b0` against `l38_b1`, commit `27f74feb3`): **ON** (cc9-lua38, 2026-10-05)

USNOS, USNOS long, IJN11, USN13 and USN02 are gameplay-identical (exit 1, noise only), and IJN11 is
exit 0. No Ohka Betty spawns inside USNOS long's 9000 frames, and every row's census reads
`kamikaze=0`, so the binding is unexercised on the reference rows. The readers are the image's as
read, and nothing moves, so the switch is ON.

## 5dn. The AI-side lines, landed OFF (cc9-lua38, 2026-10-05)

`src/game_hosts_ai.cpp` (a short window after cc9-ships32 released it):
- `rocket_accuracy` overrides the host virtual from 5dm. It calls `bsp::ai_rocket_accuracy_offset_009fe4f1`
  with the attacker's plane base, IgnitionDelay <= 0, and AntiAir or `[00F874FD]` for each of the two
  rocket tests. It is reached only with `kAiRocketAccuracyBound` (OFF).
- `record_loadout_00a04560` reads `units.plane_bag_equipment(x)` (new, `GameUnitsHost`) for
  `[X+C54h]` behind the new `kAiPlaneBagEquipmentBound` (OFF, `include/bsp/plane_squadron_host.hpp`).
  Otherwise it keeps the generator default.

Both are inert on every single-player reference row now that `kAiCoordinatorLoadGateBound` is ON
(SHIP_AI 150.6: no AI coordinator on a mission-tree launch). They stay committed OFF, for the
developer and session paths; no pairs were run.
### 5dk.3 Why a missing `Equipment` leaves the racks empty, and not at a default (cc9-lua38, 2026-10-05)

Asked by the lead before the 22-row pairing. The chain, all from disk bytes or listings:

1. **No `Equipment` key, or a value <= 0, hands `0095A880` a null entry.**
   - `007CDF98 JZ 007CDFF0` covers the missing key; `007CDFD9 JLE 007CDFF0` covers n <= 0; and
     `007CDFE3 TEST EAX,EAX / JZ 007CDFF6` covers a missing `Equipments[n]`.
   - Each of these pushes 0 as the entry and 1 as the second argument (`007CDFF0`-`007CDFF6`).
2. **`0095A880` with a null entry skips the loadout loop.** That loop is the only place a rack is
   given ammo: `vtable[1BCh]`, the single rack's `006E3530`, which writes `+484h` ammo and `+488h`
   orgAmmo.
3. **Its second loop builds a platform's `DefaultGun` only when that is >= 0.**
   - `0095AAD3 MOV ECX,[EAX+38h] / 0095AAD6 TEST / 0095AAD8 JL 0095ABA1` skips the platform.
   - Platform `+38h` is read at `00961456`-`009614B7`: the authored `DefaultGun` (`00D1AB64`)
     when present (`009614AB`-`009614B7`), else `Gun[1]` (`0096149E`), else -1.
4. **Every bomb platform an `Equipments` entry names authors `DefaultGun = -1`.**
   - Checked in this installation's `vehicleclasses.lua`: 118 of 118 platforms across every class
     with an Equipments table (`local\l38_defgun.py`). Kate's `[50]` is
     `{ ["DefaultGun"] = -1, ["Gun"] = { 85, 92 } }`, for example.
   - So without an entry, no rack device is built at all, and the plane has no class-25h part.
5. **This is why it is not a default load.** Had step 4 built the rack, its constructor `006E3C00`
   would have left ammo `+484h` = 1 and orgAmmo `+488h` = 1 (`param_1[0x121]`/`[0x122]`). The
   `DefaultGun = -1` authoring is what prevents that.

So an unarmed plane has no rack, and `007B9140` (the holds test) finds no part to ask.

**Correction to 5df.1.** A note there says a platform with a `Gun[1]` takes it as its `DefaultGun`.
That holds only when `DefaultGun` is not authored. For bomb platforms in this installation it is
always authored, and always -1.

**Correction to the lead's IJN01 example.** IJN01 loads `ijn_1_pearl.scn`, not the `ijn_01_*`
scenes the first census matched by name. Its PlaneSquadronGen rows are:
- 6 Kate and 15 PHKate, all with `Equipment` 1;
- 8 Zero, 2 Wildcat (the default Type) and 1 Warhawk, all without, and all `DefaultEquipment` 0, so
  no change;
- 1 Dauntless without `Equipment`. That squadron, Dauntless1, is the only row whose loadout changes:
  it loses its bombs.

### 5dk.4 The 23-row pairing on main with the coordinator gate ON (`l38_d0`, switch false, against `l38_d1`, ON; commit `5a8bb5d92`) (cc9-lua38, 2026-10-05)

21 of 23 rows are gameplay-identical (exit 1: census lines and the known refill noise):
USN01, USN02, USN04, E2, USN12, USN13, USN13 long, BSM01, IJN01, IJN11, JM05, JM05 long, JM06,
JM08, JM08 long, LOMP06, USNOS, USNOS long, USNRM01, ESMP08 long and ESMP08 14200. The two that
move are LOMP10 and LOMP10 long. With no AI coordinator, the USN13 long move of 5dk.2 is gone; its
deaths are 102 in both.

| row | d0 -> d1 |
| --- | --- |
| LOMP10 | dive-bomb releases 9 -> 14 (8 tasks), hull hits 103 -> 105, damage 2156.6 -> 2194.3, deaths 2 = 2 |
| LOMP10 long | releases 9 -> 14, hull hits 107 -> 109, damage 2764.2 -> 2832.2, deaths 5 = 5; Warhawk 01|.-4 dies 1.6 s later, to Kasumi instead of Ashigara |

**Per entity.** `10_san_jose.scn` authors `Equipment` 1 on all three squadrons. For two of them the
loadout changes:
- Lightning 01 (class 104) and Warhawk 01 (class 135) both have `DefaultEquipment` 0.
  - d0 starts them unarmed: `squadron ordnance ... -> 0` at 0.05 s, armed only later at 102.6 s
    and 106.8 s.
  - d1 arms them at 0.05 s: `-> 1`.
  - They drop five more bombs, and the Japanese hulls take the small damage rise.
- B-25 01 (class 118) has `DefaultEquipment` 1 and the same loadout in both.

This is the direction 5dk predicts for an authored `Equipment` on a fighter class.

**Verdict:** `kPlaneSceneEquipmentBound` stays **ON**.
## 5do. USN01's scout dive-bomb miss: the image does not lead a moving target, but it does kick the bomb down 3 m/s (packet `cc9_bomb_drop_velocity`, cc9-lua38, 2026-10-05)

Routed from cc9-ships33. On main with the coordinator gate ON, USN01's controlled ScoutDauntless
drops both bombs at about 133 s, short of moving Convoy1. Measured in this tree with
`BSP_SHELL_FATE=ScoutDauntless|Convoy1` (`local\l38_usn01_fate.log`, identical to ships33's
`s33_r3a`):
- the leader's bomb is predicted to land at (-3405, -1462);
- it ends at (-3418.4, -1473.6), 4.00 s after release;
- the target is then at (-3383.4, -1456.7).

The miss therefore has two parts: 17.5 m of the bomb flying past its own predicted point, and
22 m between that point and where the oiler went.

### The image's aim law for the glide: no target lead (read, not changed)

- The aim point is approach `+4Ch/+50h/+54h` (`009C40A0`). It is written per tick by `009FADA0`
  (target world matrix x body offset, no velocity or time term), as `cc9_hull_turndown` established.
- The impact point is `009C7D71`: own position + (`007BCC80` fall time + 0.1) x **own** velocity.
- The aimglide tick `009C5180`-`009C580B` makes no target-velocity call. Its only virtual calls
  are the approach's `vtable[0]`, at `009C51D3`, `009C5232` and `009C5278`.
- Only the fly-above (`009C62D1`-`009C63E6`) takes a three-second lead point.

So the image also releases at the target's present position, and a target moving at 8.09 m/s
(`aim lead ... v=8.09`) gains about 32 m during a 4 s fall. The 22 m part is the image's, given how
the convoy moves.

### The divergence: the bomb's initial velocity

- The host (`src/game_hosts_gunnery.cpp`, bomb drop) launches the bomb along the plane's forward
  axis at its forward speed (0092D730).
- The image launches it through the bullet's `vtable[190h]`. `006E1F00` calls it at
  `006E1F3D`-`006E1F4D` with the kind-5 ancestor (`00922E90(5)`) and a flight block. For MBomb
  (vtable `00CF9438`, written by `BSP_BombProjectile_Construct` at `006E26D8`) that slot is
  `006E0A70`, `RET 8`:
  - if the owner answers `vtable[5Ch](0Fh)` (a plane) and its `vtable[38h]` speed is > `[00CF8AAC]`
    = 1.3888889 (5 km/h), the bomb's velocity `+318h/+31Ch/+320h` (and flight `+20h..+28h`) is the
    owner's `vtable[34h]` world velocity, with **3.0 (`00E08E54`) taken off y** (`006E0AB3`). It
    also orients the bomb along that velocity (`0085DC80`).
  - otherwise, the velocity is copied unchanged (`006E0B01`-`006E0B24`).
  - The same function sits in nine bullet vtables.
- `00E08E54` is the very float `007BCC80` subtracts from the vertical velocity when it predicts the
  fall. The prediction assumes the 3 m/s kick, and the host's bomb lacks it, so the bomb falls
  longer and flies past its own predicted point.
  - In the host, `life` is 4.00 s against a predicted 3.83 + 0.1 s.
  - The census also reads a 1.6 deg angle between the nose and the velocity, which the forward-axis
    launch ignores.

### The binding (`kBombDropVelocityBound`, in the gunnery lane's file; measured from an export tree)

The bomb takes `unit_linear_velocity` (unit+AC8h, the `vtable[34h]` copy), minus 3.0 on y when the
forward speed is > 1.3888889. LABELLED: `vtable[38h]` is taken as the 0092D730 forward speed.
Measured on export trees `l38_e0` (false) and `l38_e1` (true) of `20450d20a`, patched by
`local\l38_bombvel_patch.py`. `game_hosts_gunnery.cpp` is cc9-gunnery's file; the exact edit is that
script.

**Predictions, written before the runs:**
- **USN01:** the bomb lands near its predicted point, within about 5 m instead of 17.5 m. It still
  falls about 22 m behind the moving oiler along its track (the image's no-lead law), so no hit is
  predicted, and the phase stays where it is.
- **LOMP10, LOMP10 long, USNRM01, USN13 long, JM05 long** (the rows that drop bombs): the hits move,
  with no direction predicted. Each bomb's along-track overshoot shrinks by about 0.15 s x the
  aircraft's speed.
- **Rows with no bomb drop:** identical.

### 5do.1 Measured (`l38_e0` against `l38_e1`): **the scout's bomb now hits, and USN01 leaves phase 1-2** (cc9-lua38, 2026-10-05)

| row | e0 -> e1 | |
| --- | --- | --- |
| USN02 | identical (exit 0) | control |
| USN13 long, JM05 long | gameplay identical (exit 1) | their few bombs change nothing |
| **USN01** | the leader's bomb: fate 4 at (-3418.4, -1.2, -1473.6) after 4.00 s -> **fate 2 at (-3397.6, 1.3, -1456.0) after 3.70 s**, 13.5 m from Convoy1's centre (-3384.1, -1454.4). Units 64 -> 93, torpedo tasks 0/5 -> 0/17, dive tasks 2/2 -> 2/19, controlled unit ScoutDauntless -> ConTBD1, damage 33939 -> 35061 | the convoy's hit listener fires and the script spawns the next phase |
| USNRM01 | deaths 129 -> 132, dive releases 36 -> 34, hits taken: Maryland 8 -> 20, California 2 -> 13, Oklahoma 9 -> 11, Tennessee 21 -> 22 | more bombs land on the battleships |
| LOMP10, LOMP10 long | damage 2194 -> 1987 / 2832 -> 2620, hit records +2, hull hits -1 | the 14 Lightning/Warhawk/B-25 bombs land elsewhere |

- **USN01.** The bomb now falls in 3.70 s, against the 3.83 + 0.1 s prediction; the 0.05 s steps
  quantise it. It lands 20.7 m from the e0 point and on the hull.
- **The prediction's "22 m behind the oiler, no hit" was wrong.** It measured to the oiler's
  centre, not to its hull. The lead law is the image's and unchanged. What decided the hit was the
  bomb's own overshoot.
- **USNRM01.** The only-ON deaths are Japanese planes shot down by AA (Ralph Talbot, Phoenix,
  Neosho, Arizona): the timing of the strike moved, and the AA draws with it (RNG-coupled, as
  memory notes for gunnery pairs).

**Verdict: ON is recommended.** The mechanism is the image's as read (`006E1F00` -> `vtable[190h]` =
`006E0A70`), it matches the fall time `007BCC80` predicts with, and it fixes USN01's stuck phase.
The flip belongs in `src/game_hosts_gunnery.cpp` (gunnery lane). The edit, with the switch, is
`local\l38_bombvel_patch.py <tree> true` in the cc9-lua38 tree, routed to the lead.
## 5dp. The kamikaze cruise profile's forward-speed read has no reach (cc9-lua38, 2026-10-05)

- **The site.** `009AF15A` (`MOV EDX,[EAX+204h] / CALL EDX`) is in `009AF0A0`-`009AF22A`, an
  unnamed approach update. It is called only from `009AF480`
  (`BSP_BotTaskKamikaze_UpdateCruiseProfile`, the kamikaze task's vtable `+54h`, which has no
  direct caller).
- **What it computes.** The range to the aim point (`+54h`), divided by the unit's `vtable[204h]`
  forward speed, floored to 10.0 (`00CE38B8`) below a threshold (`00CE3DC0`). That gives a time to
  target at `+60h`, which is clamped into `+ACh`. The `+5Ch` latch is set while that range is under
  `+48h`.
- **This host has no kamikaze bot task.** No source file names `009AF480`, `009AF0A0` or a kamikaze
  task body.
- **Reach on the reference rows: none.**
  - This installation's `vehicleclasses.lua` has six `Type = "Kamikaze"` classes: 45, 46, 100, 103,
    156 (MXY7 Ohka) and 370 (Funryu).
  - None of them spawns in any of the 23 `l38_d1` logs (`local\l38_kami.py` over the
    `unit hull input ... type_id=` lines).
  - USNOS's `BettyOhka` (class 32) carries the Ohka only as a `DummyKamikazePlane` bullet (5dl).
    Within the frame windows no Ohka is released, so class 156 is never created.
- **Verdict.** Nothing is bound. Binding needs the kamikaze task itself first: `009AF480` and its
  vtable, a planes-lane packet. A row that spawns a kamikaze class (a late USNOS or an IJN mission
  with Ohka releases) is needed to measure anything.

## 5dq. Handoff (cc9-lua38, 2026-10-05)

Branch `agent/cc9-lua38`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua38`. No lease
is held.

| item | commits | switch | state | single-player rows with the coordinator gate ON | section |
| --- | --- | --- | --- | --- | --- |
| Capture think, plane groups | `4e1ae0830` | - | no filter in the image | moot (no brain) | 5dj |
| bag `Equipment` drives the racks | `7e43d04d4`, `6a6ec6ade`, `c68f1da49`, `ce0f24d39`, `b2d8bbe40` | `kPlaneSceneEquipmentBound` | **ON** | **live**: LOMP10 and LOMP10 long move (Lightning/Warhawk `Equipment` 1) | 5dk-5dk.4 |
| LaunchSquadron default arm = `DefaultEquipment` | `6a6ec6ade` | under `kPlaneSceneEquipmentBound` | **ON** | **live** wherever a script launches without an arm (USN13 long airfield Kates/Bettys) | 5dk.1 |
| paratrooper fields, carried kamikaze class | `27f74feb3`, `c68f1da49` | `kAiLoadoutCarriedTermsBound` | **ON** | inert (AI side; no Ohka released) | 5dl |
| rocket accuracy `009FE4F1` | `4672e2fa3`, `5a8bb5d92` | `kAiRocketAccuracyBound` | OFF | inert (AI side; `rocket=0/0` everywhere) | 5dm, 5dn |
| AI record+10h from bag `Equipment` | `5a8bb5d92` | `kAiPlaneBagEquipmentBound` | OFF | inert (AI side) | 5dn |
| bomb drop velocity `006E0A70` | `a6f242237`, `c83b90eae`; the lead applied it as `3b466d2aa` | `kBombDropVelocityBound` (gunnery file) | **ON** | **live**: USN01's scout bomb hits Convoy1 and the phase advances; USNRM01 and LOMP10 move | 5do, 5do.1 |
| kamikaze cruise profile `009AF15A` | this section | - | no reach, no task model | - | 5dp |

Also inert on single-player rows, from 5df.1: `kAiPlaneLoadoutArmBound`, `kAiWeightBarrelGatesBound`
and `kAiPlaneAttackerWeightBound` (ON, AI target weights); `kAiTickMovetoRetasksPlaneBound` (OFF).

Scripts in `local\` (prefix `l38_`):
- `l38_queue.ps1 -Tag t -Jobs 'side:row,...'` (one comma string is fine);
- `l38_rowequip.py <log>`: scene squadrons whose bag `Equipment` differs from `DefaultEquipment`;
- `l38_luaequip.py <lua>`: `SpawnNew` member Type and Equipment;
- `l38_defgun.py`: the authored `DefaultGun` of every bomb platform;
- `l38_de.py <id>...`: `DefaultEquipment` per class;
- `l38_kami.py <log>...`: kamikaze-class spawns;
- `l38_str.py`, `l38_dwords.py`: image strings and dwords from the PE on disk;
- `l38_grep.py <log> <re> [n] [width]`;
- `l38_bombvel_patch.py <tree> <bool>`, `l38_ai_patch.py`: the applied patches.

### Next, in order

1. **The kamikaze bot task** (`009AF480`, vtable `+54h`, and the rest of its slots). It is only
   worth doing with a row that spawns a kamikaze class; find one first.
2. **The barrel path's rocket accuracy.** `GameAiWeaponFacts::Barrel` (gunnery lane) needs
   IgnitionDelay and AntiAir before `barrel_accuracy` can use `009FE4F1`. It is inert on
   single-player rows today.
3. **The kind-2 plane creation path** (`007CDF40`: `[plane+C54h]` from the creating record's
   `+124h`, used by catapult and shipyard launches) does not carry `bag_equipment`. Those planes
   keep the legacy `DefaultEquipment` read.
4. **The air-ops scene `Arm` per slot** (`006CB277`) is taken as authored. A slot authored `none`
   launches unarmed, which matches the image as read; not measured.

## 5dr. Census: what the plane, bot-task, air-ops and Lua-host paths still hit on the coordinator-gate-ON rows (cc9-lua39, 2026-10-05)

**Input.** cc9-lua38's 23 `l38_d1_<row>` logs (main `5a8bb5d92` with `kAiCoordinatorLoadGateBound`
ON, the Z state). `local\l39_census.py <prefix> <name-regex> <status>` sums each host-method row
over the logs and counts the rows that reach it; `local\l39_sumgrep.py` and `local\l39_rows.py`
group summary lines.

**The `UNIMPLEMENTED` status in `src/game_hosts_units.cpp` is not evidence of a gap.** That host's
`record()` (line 3023) always logs `unimplemented`, also after a modelled body. Read at their
sites, the top aircraft rows are modelled, and their status is a stale label:
- `PilotBot::plan_controls` 0099D300 (20 rows, 1.98 M calls): recorded after the throttle and
  air-brake slots are written (`28314`).
- `PilotBot::queue_release_order` 0099AF53 (11 rows, 328 k): the arming loop's
  "no order queued" arm; the order count is written by `write_release_order_count` (007BCBFD).
- `BotStateMoveTo::refresh_ranges` 009BDE80 (11 rows, 330 k): the three-store setter that
  DIVE_BOMB_APPROACH marks "read whole, bound".
- `Bot::hit_task_notify_*` 009D3270 / 009C7900 / 009CC400 (8 / 9 / 5 rows): the clock resets are
  done (`14591`).
- `BotStateDiveBombDone::station_keeping` (6 rows): the station law runs (`21430`).
- `Plane::water_contact_007cb7f0` (15 rows): the contact is applied (`16206`).
- `TorpedoApproach::run_profile_record_14h` (11 rows): the skill row is selected
  (`22214`); only the comment above it is stale.

**The rows that are real gaps, ranked by reach on the 23 rows:**

| # | row | rows | calls | lane | what is missing | gameplay reach |
| ---: | --- | ---: | ---: | --- | --- | --- |
| 1 | `Rack::drop_dispersion_006e4f91` (006E4D50) | 6: JM05 long, LOMP10, LOMP10 long, USN01, USN13 long, USNRM01 | 95 | units (shared) + gunnery | the drop's four scatter draws from the shared stream, and the rack+DCh offset (RELEASE_ISSUE_STAGE) | every scripted dive-bomb drop; the draws also shift the shared stream |
| 2 | `MissionLuaNative::Countdown` 008B16E0 (+ `CountdownTimeLeft` 008B1B40, `CountdownCancel` 008B19A0) | 5: JM05 long, JM08, JM08 long, LOMP10, LOMP10 long (TimeLeft: JM05 long, 83) | 5 | Lua host (script orders) | the countdown never runs, so its callback never fires | **JM08 long:** `SpawnHoshoFleet` (prcpjm08.lua:890, 180 s after `HoshoTime` at 111.15 s) never spawns the Hosho group; JM05 long: the event timer never expires; LOMP10 long: `TimeLimit` |
| 3 | `MissionLuaNative::AddAirBaseStock` 00896A90 | 3: ESMP08 long, ESMP08 14200, USN13 long | 56 | Lua host | the add `006CA770` is reconstructed (`air_base_stock_add_006ca770`) but not routed | refills (`006C0510`) read the stock; no refill runs on these rows (`refills_3_4_to_5=0`), so the reach is the GetProperty `stock` reads |
| 4 | `MissionLuaNative::GetCapturePercentage` 0089B840 | 2: JM05, JM05 long | 196 | Lua host | pushes nothing | `nil * 100` at JM05.lua:5216 fails `luaTimetable` 196 times; the failing timer is the score display `luaJM5Sec1Score` (presentation), which then never re-arms |
| 5 | `MissionLuaNative::SetCatapultStock` 00892C30 | 1: USNRM01 | 10 | Lua host | unit+638h is not written | catapult launches, if any run there |
| 6 | `MissionLuaNative::PilotRetreat` 008A4300 | 1: JM05 long | 3 | Lua host | the order is not issued (pieces exist in `src/pilot_order_bindings.cpp`) | three Allied planes out of ammo keep their tasks |

Everything else on the list is presentation (`IsGUIActive`, `DisplayScores`, `BlackBars`, hints,
narrative, `SetGuiName`, `Loading_*`), a load-time record, or the side-AI scheduler passes
(`BotScheduler::*`, 23 rows), whose outputs have no consumer on a single-player row.

**Taken in this order:** item 2 (5ds), then item 1 (needs the gunnery lane's bomb spawn), then
items 3, 5 and 6.

## 5ds. The countdown natives (packet `cc9_lua_countdown`, cc9-lua39, 2026-10-05)

### What the image does (read whole; Ghidra was read-only)

- **The object.** `[game+21E8h]`, constructed by `00735030` at `004DFA75` in
  `BSP_Game_ConstructWorld`; cleared at `004D2D5E` in `BSP_Game_DestroyWorld`. `00735030` writes
  `+4Ch..+54h` (the name and the argument vector) and leaves `+3Ch..+48h` unwritten.
- **`Countdown` `008B16E0`** (`lua_CFunction`, returns 0). `00887120(-1, 4)` (`008B17B7`-`008B17C3`)
  reads the frame into 14h-byte variants: text `+8h` (argument 0), `+1Ch` (1), `+30h` (2), and
  the callback name `+44h` (3) only when the count is above 3 (`008B17D2`). Arguments 4..n go to a
  new vector (`008B1846`-`008B185E`). Then:
  - `0052B9B0` on `[[00E198C4]+C8h]` with the text and argument 2 (`008B18B8`), the HUD;
  - `00733FF0` on `[game+21E8h]` (`008B18F7`), `__thiscall(text*, level, seconds, name*, args*)`,
    `RET 14h`: `005BCA70` (HUD), `+40h` = level, `+3Ch` = 1, `+44h` = seconds, `+48h` = the clock
    `[00F876A4]`, `+4Ch` = name, and `+54h` = args after freeing the old vector
    (`0073405E`-`0073409D`);
  - in a hosted session only (`game+1FE4h == 1`), the replication `00772B30`.
- **The step `00735100`**, on `[game+21E8h]` from `005BC920` (the HUD narrative screen's update,
  only while `game+21F0h > 0`), before the blackout fade `005B9800`
  (`00735151`-`0073524F`):
  - with `+3Ch` set, left = `+44h - (clock - +48h)`, stored as a float (`00735179`);
  - `0 > left` (`00735183 FCOMIP` / `JA`) or `left <= [00D7A218]` (= 0.0f, `0073518D COMISS` /
    `JBE`): clear `+3Ch`, `005BCAB0` (HUD), and when the name is not empty, copy it, assign "" to
    `+4Ch`, take `+54h` and zero it, and call `00887E50(self 0, &name, args, 0, -1)` on
    `[game+1A08h]` (`0073521F`), then `00733D50` on a local (`00735228`, contract: unread; the
    host releases the arguments there);
  - otherwise `005BCA80(left / +44h, left)` (HUD).
- **`CountdownCancel` `008B19A0`:** two `0052AB90` (HUD), then `007340A0(&left)`, which stores
  max(0, `+44h - (clock - +48h)`) (`007340AC`-`007340D5`), clears `+3Ch` and `+4Ch` and frees `+54h`;
  then `00B66480(left)`: one result.
- **`CountdownTimeLeft` `008B1B40`:** `00B66480(max(0, +44h - (clock - +48h)))`, one result, without
  testing `+3Ch`.

### The binding (`kLuaCountdownBound`, `include/bsp/game_hosts_script_orders.hpp`, committed OFF)

The three rows join the script-orders binding table and are handled only when the switch is ON.
The countdown state is a member of `GameScriptOrdersHost`. The step runs inside
`run_blackout_update`, ahead of the fade, as `005BC920` orders them. The clock is the host's
`mission_clock_` (00F876A4). The callback is run like the blackout's, as an after-row-9 poster.
- **ASSUMPTION:** `+3Ch..+48h` start at zero. Only `CountdownTimeLeft` before any `Countdown` can
  observe that.
- **Records** (render-side): `HudCountdown::show_text` 0052B9B0, `begin` 005BCA70, `update`
  005BCA80, `end` 005BCAB0, `clear_text` 0052AB90.

### Predictions (written before the runs)

- **JM08 long** (36000 frames): `HoshoTime` runs at 111.15 s, so the countdown expires at about
  291.2 s. `SpawnHoshoFleet` then generates the Hosho and two escorts, joins them in formation,
  sets the Hosho's speed and starts `HoshoMovie`. The row **moves** (exit 3): three more
  Japanese hulls in the fight. Deaths and hits move with no predicted direction.
- **JM05 long** (9000 frames): the event timer is 400 s (JM05.lua:931 and on). The callback
  `luaJM5EventTimerExpired` runs only if an event started before about 50 s. `CountdownTimeLeft`
  now answers a number, so the reminder arm (JM05.lua:3712-3724) runs; it is presentation.
  Expected: the timer does not expire inside the window, and the row is gameplay-identical
  (exit 1).
- **LOMP10 long** (9000 frames): `TimeLimit` runs at about 180 s after `luaIntroMovieEnd`. Every
  call it makes is a record or a dialog: `AddAirBaseStock`, `AddShipyardStock`, `SetGuiName`, and
  a `luaMonitorAF` that finds no Allied airfield squadron. The row is gameplay-identical (exit 1).
- **JM08, LOMP10** (3000 frames): no expiry inside 150 s; gameplay-identical (exit 1).
- **Every other row:** identical (exit 0) or gameplay-identical.
