# The dogfight task's engaged half

Addresses: 009AAC70, 009AA630, 009A7650, 007BBC10, 007C2610, 009A9D90, 009A9D50, 009AAFA0,
009AAA80, 009A98F0, 009A9970, 009A9BD0, 009A8560, 009A75C0 (no Ghidra function), 009A76E0,
009A84E0, 009A87F0, 009A8B20, 009A70E0, 009A71C0 (no Ghidra function), 009A71E0, 009A7DE0 (no
Ghidra function), 009A7E80 (no Ghidra function), 009A8020 (no Ghidra function), 009A80E0 (no
Ghidra function), 009A7F70 (no Ghidra function), 009A83B0, 009A6B70 (no Ghidra function).

Packet `cc9_dogfight_engaged`, after `cc9_dogfight_task` (docs/DOGFIGHT_TASK.md). Every name
below is a hypothesis, not a recovered symbol.

## 1. The approach update, 009AAC70, and target selection, 009AA630

Both were read from the listing. `approach` is `task+3F8h`, so `approach+D0h` is `task+4C8h` (the
latch) and `approach+CCh` is `task+4C4h`. `approach+CCh` is the **target squadron**: its plane
count is at `+3CCh` and its planes at `+3D0h[0..4]`, the layout `docs/BOT_TASKS.md` gives for
`unit+9D4h`. `approach+0Ch` is the unit's own squadron. `approach+14h` is the pilot robots row
(`00F8A30C + level * 248h + 0Ch`), so `row+N` is `robot_config.hpp`'s suffix `N+0Ch`.
`row+230h` is `AimShootDistance`, 850 in this installation's SPNormal pilot row.

**009AAC70**, `__thiscall(approach, float dt)`, `RET 4`:

| step | site | what | constant (address, width) |
| --- | --- | --- | --- |
| no squadron | `009AAC76` | `+D0h = 0`, `+D4h = 9999.0`, return | `00CE4C04` float |
| timers | `009AAC9D` | `+E4h -= dt`, `+E8h += dt` | |
| reselect | `009AACCF`-`009AAD10` | `009AA630` when `+B4h` is null or not live (`+5Ch` set, `+5Dh`/`+60h`/`+5Eh` clear), or when `old +D4h > row+230h + 200` **and** `+E4h < 0` | `00CE4D70` double 200.0 |
| ranges | `009AAD60`, `009AAD78` | `+D4h` = 3-D range to the aim point, `+D8h` = the same with `y = 0` | |
| latch | `009AADC5`-`009AADD7` | `d < AttackDist * approach+24h + (latched ? 150 : 0)`, strict (`FCOMIP`/`JBE`) | `tuning+644h`; `00CE3808` float 150.0 |
| flight time | `009AADDE`-`009AAE17` | latched: `+70h = clamp(d / 007C2610(), 0, 30)`; else `+D0h = 0`, `+70h = 0` | `00CE7630` double 30.0, `00CE38C8` float 30.0 |
| frame | `009AAE35`-`009AAF1E` | `009FADA0`; `+ECh..+F4h` = the aim point in the unit's frame; `+F8h = x / max(z, 1)`, `+FCh = y / max(z, 1)` | `00D7A24C` float 1.0 |

`007C2610` returns the smallest `[[part+3F8h]+34h]+50h` over the unit's parts that answer
`vtable[5Ch](21h)`. Dividing a range by it gives the `+70h` flight time, so it is read as a
muzzle speed. That reading is a hypothesis.

**009AA630**, `__fastcall(approach)`, called only from `009AAD10`:
* It re-arms `+E4h = 00BD2F10(1.0, 3.0)` (`00CE3854` float 3.0).
* With `+3CCh < 2` it takes `+3D0h[0]` unscored (`009AA66B`).
* Otherwise it scans `+3D0h[i]` for `i < 5`, **stopping at the first null**. It skips members that
  are not live or are outside the map (`0071C4F0`). Each member's score is:
  * `R = row+230h * 0.8` (`00CE3D40`, double of `0.8f`). `DC C9` at `009AA7E4` is
    `FMUL ST(1),ST`, so `R` is what `009AA7ED` stores.
  * `range = d < R ? interp(100, 0.5, 0.8R, 1.0, d) : interp(1.2R, 1.0, 3R, 0.25, d)`, using
    `00CE3D08` 100.0, `00CE3800` 0.5, `00CEC160` double 1.2, `00D7A2B0` double 3.0 and `00CE3868`
    0.25.
  * `behind = 0.1` when local `z < 0`, else 1.0 (`00D7A2F0`).
  * `angle = interp(0.25, 1.0, 1.5, 0.2, |(x, y) / max(z, 1)|)`, using `00CE380C` 1.5 and
    `00CE54A0` 0.2.
  * `score = angle * range * behind`. It is multiplied by 0.7 (`00CEFFA0`, double) once for each
    own-squadron member other than self that `007BBC10` says is already on the candidate.
    `007BBC10` compares `[unit+DF4h]+98h+[00F876B8]*1Ch` with the candidate. A candidate that is
    not the current `+B4h` is then multiplied by `00BD2F10(0.8, 1.0)`.
* The best score strictly above 0 goes to `009A7650`. With no such score, `+3D0h[0]` does.

**Target kind.** No kind test appears: the candidates are whatever planes the target squadron
holds. What makes them aircraft is the order. `007EEAEC` admits the `dogfight` class only for an
airborne target (`00922B10`), and the squadron comes from that target.

## 2. The engaged states

Vtables, from `009A94E0` (`ESI = task+3F8h`) and the two constructors: slot `+4` is enter,
`+8` is exit (`007B3DC0`, shared), `+0Ch` is the tick and `+1Ch` is the avoid weight.

| state | object | vtable | enter | tick | status |
| --- | --- | --- | --- | --- | --- |
| aim | `+67Ch` | `00D1F8D8` | `009A75C0` (no function) | `009A76E0` | **read**, except `007B4ED0` (the head-on speed) and the gun controller at `approach+1Ch` |
| maneuver | `+6A4h` | `00D1F960` (ctor `009A84E0`) | `009A87F0` | `009A8B20` | enter **read**; tick **partial**: a three-sub-mode machine (`+18h`: 0 heading servo, 1 roll to the bank at `+1Ch`, 2), read at pseudocode level to its end (`009A9270`), with the sub-mode 1/2 bank arms not checked against the listing |
| attackrun | `+6E4h` | `00D1F8B0` (ctor `009A70E0`) | `009A71C0` (no function): `+20h = 0`, `+18h = [00CE7804]` | `009A71E0` | **partial**. **Unreachable** for these fighters (below) |
| avoid_roll | `+708h` | `00D1F918` | `009A7DE0` (no function) | `009A7E80` (no function) | enter **read**. Tick **read** from disk bytes (`009A7E80`-`009A7F69`), except `009A7A50(dt)`: it sets roll rate `plan+290h = +1Ch` with `+294h = 1` and heading mode `+2CCh = 0`. While `+24h` is clear it sets yaw rate `plan+284h = -sign(+F8h)` and pitch rate `plan+29Ch = -sign(+FCh)`, both with modes 0, steering away from the target. Once `+24h` is set it sets pitch `plan+2BCh = 0.5` with mode 2. **The binding is still a stand-in**, because the host's rate channels are not wired in this seam |
| avoid_turn | `+730h` | `00D1F938` | `009A8020` (no function) | `009A80E0` (no function) | enter **read**; tick **unread** past `009A7A50(dt)`. `009A7A50` (`009A7A50`-`009A7DA8`, shared by both avoid ticks) starts with `+18h -= dt` (read), which is the exit timer `009AAFA0` tests; the rest of it is unread |
| prepare | `+5E4h` | follow | - | `009C1FD0` | read as follow. **Unreachable** for these fighters |

**Why attackrun and prepare are unreachable here.** `ENG` and `009A9D90` read `squadron+370h`,
reached through `task+404h`, which is `unit+9D4h`, the squadron. The dogfight task's
`vtable+38h` at `00D1F9E8` is `0099B710` (`MOV AL,1; RET`), the same as the dive-bomb task's. So
the flight leader's `0099B740` stores 1 every think, through `007ED3F0` at `0099B774`, and
`009AAF30` reaches `0099B740` by its tail `JMP` at `009AAF92`. With the mode at 1, `ENG` is the
latch alone and `009A9D90` always takes the latched arm, which is maneuver. Prepare needs mode 0.
Attackrun needs `ENG` without the latch, which only mode 2 gives.

**aim**:
* **Enter `009A75C0`:**
  * `+18h = 0`.
  * `+1Ch = U(0.8, 1.4) * BoringTime`, from `00CE74F8`, `00D06874` and `row+210h`.
  * `+20h = U(0.5, 0.8) * FollowDist * approach+24h`, from `row+20Ch`.
  * `+24h = +25h = 0`.
* **Tick `009A76E0`**, with a target:
  * `+25h` (head-on) is set when local `z > 1` and the dot product of the two units'
    `vtable[34h]` vectors is below 0.
  * Head-on with `d < +20h` sets `+24h`. That is `task+6A0h`, the too-close exit.
  * The boredom rate is -4.0 (`00CF1430`) when the gun controller's `+48h` byte is set. Otherwise
    it is `interp(0.25, -1.0, 2.0, 2.0, tan)` inside ShootDistance and 0 outside. Then
    `+18h = max(0, +18h + rate * dt)`.
  * Heading comes from `009F9E40` to the aim point `approach+48h` (slot 0 is `009A6B70`, no
    function). Pitch comes from `009F9ED0(aimY - ownY, approach+D8h)`, at `009A78BB`.
  * When not head-on: `plan+2B0h = 1`, `plan+2D8h = 1`, and
    `plan+2B4h = (d - FollowDist) + target vtable[38h]`.
  * When head-on: `007B4ED0(interp(+20h, 0.3, ShootDistance, 1.0, d))`, which is **unread**.
  * It writes `(approach+1Ch)+40h = tuning+678h` (`Pilot/AutoStrafeAngle/Angle_Strafe`),
    `approach+DCh = 0` and `approach+E0h = 1.0`.
  * A target-squadron plane at `(approach+1Ch)+74h` that is not the current target becomes the
    target.
* **The gun.** `approach+1Ch` is the auto-strafe gun controller. The task only hands it the strafe
  angle. **Which routine fires the guns, and on what alignment, is not in the task and was not
  read.** Nothing is bound for it, and no gunnery hook is needed yet.

**maneuver** (enter `009A87F0`):
* A heading of the target bearing plus `U(-0.8, 0.8)`. One branch adds another `U(-0.5, 0.5)`
  first, when `009A85B0(1)` (unread) is true. The heading is stored as `(+20h, +24h)`.
* `+30h = U(lo, hi)`, a pitch between the class dive and climb limits:
  * `hi = interp(100, 0, 500, class+1ECh, Ceiling - y)`.
  * `lo = interp(200, 0, 600, -class+1F0h, unit+9B4h)`.
* `+38h = U(0.8, 1.3) * ManeuverChangeTime`, from `row+21Ch`.

The read part of the tick:
* `+38h -= dt`. At 0 it calls `009A86F0` and re-enters.
* While `+3Ch` is set, or `d > +34h`, or the unit is outside the map, it pursues. The pitch point
  is `(aimY - ownY)` over `+D8h` while `+D8h <= 3 * ShootDistance`. Beyond that it is
  `min(Ceiling - (y + 50), 300)` over 500 m.
* In sub-mode 0 it is a heading servo, with heading mode 2 and pitch mode 2, while the heading
  error is below `[00D1F998]`. The large-error arm draws a bank.

**The transitions `009AAFA0`**, re-read from the listing (`009AAFA0`-`009AB1B8`, `RET 4`). They
agree with `docs/DOGFIGHT_TASK.md` section 2 except for one edge that section omits: **from
prepare, `ENG` with a non-zero mode calls `009A9D90`** (`009AB030`-`009AB03C`). Also:
* `009A98F0` is `aim+18h > aim+1Ch`.
* `009A9BD0` is local `z > 1 && |(+F8h, +FCh)| < 0.8`, using `00CE3D40` double and `00CE3820`
  double 1e-10.
* `009A8560` sets maneuver `+18h = 0` and `+34h = U(0.3, 1.1) * ShootDistance`, from `00CE69C8`
  and `00CE6448`.
* The avoid enters set `+18h = U(0.75, 1.5) * AvoidTime`, from `00CEE07C`, `00CE380C` and
  `row+214h`.
* `009AAA80`'s first gate is `approach+DCh <= 0.0` (`00D7A218`) or `approach+E0h >= 1.0`. The
  aim tick writes `+DCh = 0` and `+E0h = 1.0`, so the gate is closed after aim. **The maneuver tick
  ends (`009A9270`) by writing `+DCh = 1.5` (`00CE380C`) and `+E0h = 0.1` (`00D7A2F0`)**, which
  opens it. `009AAA80` then asks `007B96F0` (unread) for a target in the squadron. It switches to
  aim, retargeting through `009A7650`, when that target is below the unit or within an elevation
  angle set by the unit's speed. So in the image, maneuver can drop into aim early on any
  opportunity. The binding's `009AAA80 = false` removes that edge. **Labelled substitution.**
  The maneuver -> aim edge through `009A9BD0` is bound.

**Moveto.** Its speed slot `009C1BC0` writes `plan+2B4h = 009BECD0(MaxSpd, 007C47F0(),
separation)`, with `+2B0h = 0` and `+2D8h = 1`. That is `009C1850`'s shape with `classBlock+188h`
as the first argument. `009BECD0` is still unread. **Partial.** The cruise profile `009AAF30`
runs:
1. `0099B660`.
2. For the leader: when `squadron+38Dh` is clear, `+380h < 0` and `+3A9h` is clear, it sets
   `+3ADh = 1` and `squadron+394h = CruisingAlt` (`tuning+640h`). It then clears `+3A9h`.
3. `009AAC70(dt = 0)`.
4. A tail `JMP` to `0099B740`.

## 3. The order route

A scene record's command token is resolved by name through the command-type registry
`00E19A6C`. Type 13 is object `00E08F58`, whose name getter `006F8790` returns the literal
`dogfight` (`docs/SCENE_COMMAND_TYPES.md`). The same object is the class that `007EEC50` chooses
(`007EEAEC`), that `luaMW_PilotGunFire` (`008A50D0`) issues at `008A52A7` for an aircraft target,
and that `0099A170` (`BSP_Bot_InstallCommandTask`) tests at `0099A3AC` before calling the factory
`009AB570` at `0099A3D3`. So the host's `row.command == "dogfight"` trigger names **the same
class object** as the image. It is no longer a substitute for a different producer. What the host
still lacks is the director slot: it never writes `attack_command_class` from the scene path. The
arm's comment now says this.

## 4. The binding (kDogfightEngagedBound = true)

Pure rules are in `src/dogfight_task.cpp`. The host seam is the dogfight arm in
`src/game_hosts_units.cpp`.

**Bound as read:**
* The whole of `009AAFA0`, with the prepare edge.
* `009A9D90`.
* `009AA630`'s scoring.
* `009AAC70`'s reselect test, ranges, latch and tangents.
* `009A9BD0` and `009A98F0`.
* The aim enter and tick (boredom, too-close, heading, pitch, and the not-head-on speed).
* The avoid timers.

**Labelled substitutions:**
* Every `00BD2F10` draw is fixed at its midpoint: reselect 2.0, non-current 0.9, boring 1.1,
  too-close 0.65, maneuver range 0.7, avoid 1.125.
* `approach+24h` is the 1.0 floor, as in the torpedo and dive-bomb bindings.
* The robots row is the SPNormal row.
* The aim point is the target's origin instead of `009FADA0`'s hull point.
* `vtable[34h]` is replaced by the forward rows.
* The gun-lock byte is false.
* Squadron mode 1 is a constant.
* `009AAA80` returns false. That is right after aim, but it drops maneuver's early exit to aim
  (section 2).
* `009A9970` always picks avoid_roll, because its weights are only partly read.

**Labelled stand-ins:**
* **maneuver** runs the read pursue arm: heading mode 2 to the target, and pitch mode 2 to the
  point described above.
* **avoid** holds 90 degrees off the target bearing, turning the way the unit banks, level, and
  runs the timer down.
* **The head-on speed** is `interp(...) * MaxSpd` in place of `007B4ED0`.
* **Moveto** is unchanged from the skeleton.

A census row (`dogfight-engaged`) reports ticks per state, latch sets, latched ticks, reselects,
target changes, the final target, and `gun_window_ticks`. That last count is aim ticks inside
ShootDistance with the off-axis tangent below 0.25, a diagnostic for where a gun would bear.

## 5. Predictions, written before runs E0/E1

E0 is this tree with `kDogfightEngagedBound = false`, which is the predecessor's skeleton. E1 is
the binding. Both are USN04 at the E2 parameters.

1. **E0 matches the skeleton's G2 shape:** six `none -> moveto|follow` rows and nothing else.
2. **Who engages.** Every one of the six runs 009AAC70, wing members included, because follow is
   one of the two unengaged states. The latch needs the target within 2000 m (AttackDist times
   the 1.0 floor). In G2, `Yorktown-class01_sqn02` came to 104.7 m of its Val and spent 366
   ticks inside 2000 m, so its flight of three engages: a `moveto|follow -> maneuver` row through
   009A9D90, then `maneuver -> aim` once the Val is inside the 0.8 cone. `Lexington-class01_sqn01`
   never came closer than 18190 m, so its three stay on moveto/follow.
3. **Outcome.** No fighter gun fires, because the gun controller (approach+1Ch) is not bound, so
   no fighter scores a kill. The aim state's boredom timer runs down when the Val sits inside the
   cone and within 850 m, so expect aim/maneuver cycling and `too close` avoid entries on head-on
   passes.
4. **Deaths and dive-bomb rows.** Unchanged per unit, since nothing the fighters do reaches
   another unit except through observation (the recon picture). A fighter that follows a diving
   Val could reach the water; any fighter water contact is attributed to its state row.
5. **Water contacts** stay at the Vals' seven, unless item 4's risk happens.

## 6. Runs

Both runs are USN04 at the E2 parameters (`--frames 9200 --press-start-frame 30 --menu-select
USN04 --mission-frames 9000 --mission-frame-seconds 0.05`), launched through
`tools/run_game.ps1 -Exe`. Each ran from its own copied binary and ended with the renderer's
final COM release line.

| run | configuration | binary | log |
| --- | --- | --- | --- |
| E0 | this tree, `kDogfightEngagedBound = false` (the skeleton) | `local\binE0` | `local\E0_usn04.log` |
| E1 | this tree, `kDogfightEngagedBound = true` | `local\binE1` | `local\E1_usn04.log` |

**E0 matches prediction 1** and the skeleton's G2: six `none -> moveto|follow` rows, nothing
engaged, 9 gunnery kills, 7 water contacts. `Yorktown-class01_sqn02` comes to 104.7 m with 366
ticks inside 2000 m.

**Dogfight rows, E1.** The Yorktown flight engages and the Lexington flight does not, as item 2
predicted. The frame numbers are the log's frame counter.

| fighter | engage (frame, range) | ticks aim / maneuver / avoid | latch sets / latched ticks | gun-window ticks | final target |
| --- | --- | --- | --- | --- | --- |
| `Yorktown-class01_sqn02` | 1771, moveto -> maneuver at 1994.3 m; aim at 1981.8 m | 377 / 1 / 0 | 1 / 378 | 23 | `D3A Val #3.1\|.-2` |
| `Yorktown-class01_sqn02\|.-2` | 1759, follow -> maneuver at 1996.6 m; aim at 1982.7 m | 342 / 1 / 0 | 1 / 343 | 20 | `D3A Val #3.1` |
| `Yorktown-class01_sqn02\|.-3` | 1811, follow -> maneuver at 1673.4 m; aim at 1659.0 m; re-engaged at frame 2405 | 295 / 28 / 0 | 2 / 323 | 29 | `D3A Val #3.1\|.-2` |
| `Lexington-class01_sqn01` (and `.-2`, `.-3`) | never latched; the Vals stay beyond 18 km | 0 / 0 / 0 | 0 / 0 | 0 | a `D3A Val #3.1` member |

* Each engagement ends when the latch drops at 2150 m (2000 + 150), as the Vals outrun the
  chase: `aim -> moveto` at 2152.3 m, and `aim -> follow` at 2155.3 m and 2375.1 m. That is the
  `009AAFA0` `!ENG` edge.
* There were no avoid entries: no head-on pass came within `aim+20h` (195 m).
* No aim ended in boredom. Every aim exit is the latch drop. Why the boredom limit (19.8 s)
  was never reached was not checked.
* Reselects happen every 2 s: 211 for the Lexington fighters and about 202 for the Yorktown
  fighters. That is `009AAC70`'s timer arm whenever the previous range is above 1050 m. The
  score keeps the current target except in the listed changes.

**Other rows, E1 against E0, term by term:**
* **Fighters.** Shots 0, damage taken 0 and health 280 in both runs. No fighter fires, is hit
  or drowns. **Water contacts: 7 and 7**, the same seven Val rows.
* **Dive-bomb rows** (`divebomb`, `release census`, `db aim exit`, `bomb drop`): 321 lines in
  each run, **identical**.
* **The chased Vals' tail gunners** fire more: `D3A Val #3.1` 1211 -> 1245 shots and `#3.1|.-3`
  1127 -> 1161, with 0 hits. They are shooting at the fighters that now close on them.
* **Gunnery kills 9 -> 14.** All 14 are ship anti-aircraft kills of Japanese aircraft. Five die
  in E1 that survive in E0:

| unit | killed at | killed by |
| --- | --- | --- |
| `B5N Kate #6.1` | 222.51 s | `Lexington-class01` |
| `B5N Kate #6.1\|.-2` | 228.51 s | `Fletcher-class02` |
| `D3A Val #1.1\|.-2` | 384.88 s | `Northampton-class02` |
| `D3A Val #5.1\|.-2` | 384.88 s | `Northampton-class02` |
| `B5N Kate #6.1\|.-3` | 444.02 s | `York-class01` |

  `D3A Val #1.1|.-3` moves from 203.21 s to 203.31 s. The other eight kills are unchanged.
* **Where the change starts.** The first timestamped line to differ is `gunnery step 2000
  t=100.00` (`assigns` 1 -> 2). The 1800-2000 window differs only in the Yorktown formation
  geometry and one extra `set_bot_fire_target` sequence. That is after the first engagement
  (frame 1759) and before any kill. The mission-level terms that move together are
  `recon observers` 7856 -> 7665 and `identified` 5560 -> 5373, plus `gunnery contacts` and
  `bodies`. The six fighters are observers, and when they leave their moveto/follow paths the
  recon picture and the ships' anti-aircraft targeting change with them.
* **The Kate #6.1 flight has flipped before.** It died in the skeleton's G0, when four fighters
  drowned, and survived in G2, when they lived and observed (docs/DOGFIGHT_TASK.md section 6).
  The same flight's fate follows the fighters' observation paths again here.
* **Not established.** No per-kill chain was traced from a fighter's recon row to the ship gun
  that fired, so the five deaths are attributed to the engagement through the recon and
  assignment terms only.

## 7. Decision

**The binding lands on this branch** (`kDogfightEngagedBound = true`).
* The measured behaviour is the image's latch and transitions: the flight that reaches 2000 m
  engages, and the one that does not stays unengaged.
* No fighter drowns, water contacts and dive-bomb rows are identical, and no fighter fires or is
  hit.
* The five new deaths are ship anti-aircraft kills. They follow the recon change that the
  fighters' engaged flight paths cause, and there is no direct fighter-to-unit effect. The
  integrator should weigh that attribution before merging, because it is mechanism-level, not
  per-kill.

**Still owed:**
1. The gun: whichever routine reads the auto-strafe controller `approach+1Ch` (`+40h` angle,
   `+48h` lock, `+74h` attacker).
2. `007B96F0`, so that `009AAA80` can be bound. Maneuver opens its gate.
3. The rest of the maneuver tick's bank sub-modes, and the avoid ticks' rate channels
   (`plan+284h`/`+290h`/`+29Ch`).
4. `007B4ED0`, the head-on speed.
5. `009BECD0`, the moveto speed shaping.
6. The `00BD2F10` draws, which the host fixes at their midpoints.
