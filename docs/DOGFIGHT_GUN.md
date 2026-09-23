# The dogfight task's gun controller

Addresses: 009F9980, 009FAAD0, 009FA620, 009FC7C0, 009FABE0, 007BA760, 007B96D0, 007B96F0,
0099C210, 007B4ED0, 009BECD0, 0099BEE0, 007BB6E0, 007C7800.

Packet `cc9_dogfight_gun`, after `cc9_dogfight_engaged` (docs/DOGFIGHT_ENGAGED.md). Every name is a
hypothesis, not a recovered symbol.

## 1. The controller behind approach+1Ch

`009F9980` sets `approach+1Ch = task+314h`. That object is built by `009FAAD0` from
`BSP_BotTask_ConstructBase`. It has **no vtable**: `+0h` is the task. `BSP_PilotBot_Update` ticks
it through `009FC7C0` at `00999979`, with `ECX = task+314h`, **after** the task arm
(`0099995A`), and only when `unit+C24h` is set (`00999962`). That is the same flag `007EEAEC`
requires for the dogfight class. So every task has this gun controller. The dogfight states
feed it the strafe cone at `+40h`: aim writes `Pilot/AutoStrafeAngle/Angle_Strafe`
(`tuning+678h`) and attackrun writes `tuning+670h`. Attackrun also sets the desired direction
`+68h` through `009FABE0(heading, pitch)`.

| field | value | source |
| --- | --- | --- |
| `+28h` | search arm; -1.0 after every tick | `009FAB14`, the tick's tail |
| `+2Ch` | 0.4 | `00CE7804` float, `0099C87A` |
| `+30h` | ShootDistance + 650 = 1500 | `00D1F3A0` double, `0099C888` |
| `+34h` | ShootDistance (850, SPNormal) | `0099C871` |
| `+38h` | `max(AimDistortAngle1 * 3.0, 0.08)` = 0.09 | `0099C864`; `00D7A2B0`, `00CE3E20`/`00D05B50` |
| `+3Ch` | 0 (the constructor; no writer found in the dogfight states) | `009FAB3C` |
| `+40h` | the state's strafe cone; zeroed at every tick end | aim `009A7960`, attackrun `009A7459` |
| `+48h` / `+4Bh` / `+50h` | fire / burst on / burst clock | the tick |
| `+74h` | auto target | `009FC90E` |

**`009FC7C0`**, `__thiscall(gun, float dt)`, `RET 4`, body `009FC7C0`-`009FCEE0`. Read from the
pseudocode, with the gates and burst clock checked on the listing:
1. `+50h -= dt`, and `+4Ch -= dt` while it is `>= 0`. It clears `+48h`, `+49h` and `+74h`, and
   sets `+44h = 999999.0` (`00CF87D0`).
2. **Target.** When `+28h < 1.0` and `+2Ch > 0` and `+30h > 0`, which holds every tick, it calls
   `007B96F0(max(+2Ch, +40h * 1.25), +38h, +30h, +34h * 0.3, +28h, 0, 0)`. That uses `00CF87C0`
   double 1.25 and `00CE3DC8` double 0.3. `007B96F0` forwards to `007E2090` on `unit+C50h`, and
   `007E2090` is **unread**. A live result goes to `+74h`. The lead point `+5Ch` is then the
   target's `vtable[48h]` prediction at `t = d / 007C2610()`, iterated twice.
3. **Range.** It skips to the tail unless `1 < d < max(+30h, +34h + 200)`.
4. **Envelope.** It fires only when:
   * `lateral < d * +38h`, `z > 1` and `z < +34h`, where lateral is the lead's (x, y) in the
     unit frame; and
   * `lateral > +3Ch` (divided by 1.8, `00D049A8`, with an auto target) or
     `lateral > 2 * +0Ch * +08h * d`. With `+0Ch = 0` that second arm is `lateral > 0`.
   * A hold timer `+4Ch > 0` passes it unconditionally.
5. **Steer.** When `1 - cos(lead, +68h) < +40h` and `+4Ch <= 0`, it runs `009FA7E0` (the
   distortion) and `009F9FC0`, which steers toward the lead point with the distortion scaled by
   `FighterAimMulVersusAI` (`tuning+648h`) or `...VersusPlayer` (`+64Ch`). Then `+49h = 1`.
6. **Fire** (`009FCDB6`-`009FCE1F`):
   * `envelope && !007BA760(unit) && !007B96D0(unit)` is required.
   * `007BA760` is true when `unit+C3Ah` or `unit+5Dh` is set.
   * When `007B96D0` is true, the tick re-aims at +0.25 through `009F9FC0` instead of firing.
   * Then `+4Bh != 0 || +50h < 0` sets `+48h = 1` and `task+2E0h = 1`.
7. **Burst clock** (`009FCE4F`-`009FCED9`):
   * With `+4Bh` clear, `+50h < 0` and `+48h` set: `+4Bh = 1` and
     `+50h = U(row+234h, row+238h)`, which is AimShootTime {4.5, 3.0}.
   * With `+4Bh` set and `+50h < 0`: `+4Bh = 0` and `+50h = U(row+23Ch, row+240h)`, which is
     AimShootDelayTime {1.6, 0.7}.
   * The row comes from `0099C210`.

**Which guns.** The controller never names a gun. Its only output is the request byte
`task+2E0h`.

## 2. The fire path into the projectile system: not established

* `task+2E0h` is **plan+2DCh**. `0099BEE0` is called at `0099B0A0` with `ECX = EBX+4`, the plan
  base, and copies `plan+2DCh` to `cmd+16h`.
* `007BB6E0` carries that to the control byte `unit+9FAh`, whose Lua name is `gunFire`
  (`007D6A90`). It is forced to 0 when `unit+5Dh` is set. `007B9770` latches it to `+BC9h`.
* **No weapon reader was found downstream.** The census scans:
  * `+BC9h`: only the latch writer, `007B97B6`.
  * `+BC8h`: `CMP byte` readers of the bomb byte and float fields of other classes.
  * `+9FAh`: the quantiser, the latch, `007C7800` (a player-camera effect in segment 45, gated on
    `unit+C24h`), the property-bag writer at `007C9B2B`, and `BSP_Plane_ReadPropertyBag`.
  * `+A12h`: no `.text` hit.
  * `LEA reg,[reg+9E4h]`: five sites, none a weapon.
* Category-0 guns (the fighters' `0:6`) get **no gun bot** in `0072C6A0`, which handles
  sub-types 1, 5/6, 2/3/4/6 and 7.
* So either the forward guns read `gunFire` through a pointer this census cannot see, or a
  weapon path outside the bot fires them. **Neither is established**, so no rounds are spawned.

## 3. Host against image (kDogfightGunBound = true)

**Bound:**
* `009FC7C0`'s range, envelope, fire and burst logic, as the pure rule `dogfight_gun_tick_009fc7c0`.
* Its `+48h` output feeds the aim tick's boredom rate (-4/s while firing).
* `007B4ED0` as the rule `dogfight_throttle_007b4ed0`. Its wiring into the head-on arm and the
  maneuver tail (`009A9270`) sits behind `kDogfightThrottleBound`, which is **off** after run F1
  (section 6).

**Labelled substitutions:**
* `+74h` is the task's own target, not the `007E2090` finder's choice, unled.
* `007BA760` and `007B96D0` are false.
* Draws are at their midpoints: burst 3.75 s, gap 1.15 s.
* The steering `009F9FC0` and the distortion `009FA7E0` are not bound.

**Not bound:**
* Rounds. No hook in `src/game_hosts_gunnery.cpp` is needed or requested until the gunFire
  consumer is found.

**Also read:**
* `009BECD0`, `__thiscall(state, a, b, sep)`, `RET 0Ch`. It returns
  `b + (a - b) * max(interp(tuning+5CCh, 0, tuning+5D0h, 0.5, sep), 007EF2C0())` when
  `approach+0Ch` and `+8h` are non-null, else `a`. `007EF2C0` is unread.
* The moveto stand-in stays: its tick `009C18C0`'s separation input is not modelled in the
  dogfight arm.

**Still substituted:** `009AAA80`'s early edge, because it needs `007E2090`.

## 5. Predictions, written before runs F0/F1

F0 is this tree with `kDogfightGunBound = false` (main's engaged binding). F1 turns on the gun
controller as a census, the head-on throttle (`007B4ED0`) and the maneuver tail's full throttle.
No rounds are spawned (section 3).

1. **Bursts** come only from the three `Yorktown-class01_sqn02` fighters, while they are in aim
   on a Val inside 850 m and within `0.09 * d` laterally. That gate is tighter than E1's
   `gun_window_ticks`, which used a 0.25 tangent. Expect one to three bursts per fighter. The
   Lexington flight has none.
2. **No hits and no kills by fighters**, because nothing is fired into the projectile system.
3. **Behaviour changes are small and confined to the Yorktown flight.**
   * Aim boredom now runs down at -4/s while the fire flag is up. No aim exit was boredom in
     E1, so the state sequence should not change.
   * Maneuver flies full throttle. Only `.-3` spent real time in maneuver (28 ticks in E1).
   * Head-on throttle applies only on head-on aim ticks, and E1 had no too-close entries.
4. **Downstream.** Any change in other units' rows comes through the same recon and assignment
   mechanism as E1. Dive-bomb rows and water contacts should be unchanged.

## 6. Runs

All runs are USN04 at the E2 parameters (`--frames 9200 --press-start-frame 30 --menu-select
USN04 --mission-frames 9000 --mission-frame-seconds 0.05`). Each ran from its own copied binary
through `tools/run_game.ps1 -Exe`, and each log ends with the renderer's final COM release line.

| run | configuration | binary | log |
| --- | --- | --- | --- |
| F0 | this tree (main `80f19575f` plus this packet), `kDogfightGunBound = false` | `local\binF0` | `local\F0_usn04.log` |
| F1 | gun census, plus `007B4ED0` wired into the head-on arm and the maneuver tail | `local\binF1` | `local\F1_usn04.log` |
| F2 | gun census only (`kDogfightThrottleBound = false`) | `local\binF2` | `local\F2_usn04.log` |

**F1 is rejected.** Two new water contacts appear: `Yorktown-class01_sqn02` and its `.-2`, both
at |v| 51. Each had spent exactly one tick in maneuver, whose tail leaves the throttle slot
active in direct mode (`+2D8h = 0`). Both then stalled and drowned after aim handed them back to
moveto and follow. `.-3` spent 20 ticks in maneuver and survived. The image rebuilds the command
block every think (`0099B4E8`) and this host does not, so the write outlives the state. The
throttle wiring is now behind `kDogfightThrottleBound`, which is off. `dogfight_throttle_007b4ed0`
stays as a reconstructed rule.

**F2 against F0, term by term:**
* **Fighter gun.** One burst, from `Yorktown-class01_sqn02|.-3` on `D3A Val #3.1|.-3`, in aim at
  845.7 m with a lateral offset of 59.07 m. The envelope limit there is 0.09 × 845.7 = 76.1 m.
  The burst gave 5 fire ticks: the Val stayed under 850 m for only 5 ticks before the range
  opened. The other five fighters raise no request. The prediction of one to three bursts per
  Yorktown fighter was wrong: the Vals pull away before most chases close inside 850 m.
* **Everything else is identical.** All `dogfight` rows (states, latches, targets) match. Water
  contacts are 8 and 8, the killed_by table matches, and the dive-bomb rows match (364 lines
  each). The only differing summary row is `fighter gun`. The one input the census feeds back,
  the aim boredom rate while `+48h` is up, did not change any state sequence.
* **Rounds: 0.** No fighter damages anything, so there is nothing to trace into the dive-bomb
  rows.

## 7. Decision

**The census lands on this branch** (`kDogfightGunBound = true`, `kDogfightThrottleBound =
false`). F2 matches F0 on every per-entity row, and the image's fire decision and burst clock are
now visible per fighter.

**Fighter gunfire is not bound.** Two things block it, and both need reading:
1. **The consumer of `gunFire`.** `plan+2DCh` reaches `unit+9FAh` and is latched to `+BC9h`, and
   no weapon reader of either was found by the literal scans (section 2). Candidates are a
   pointer read of the control block, or the plane's weapon update reached through the unit
   vtable.
2. **The finder `007E2090` on `unit+C50h`.** It supplies the auto target `+74h`, and through
   `007B96F0` it also gates `009AAA80`'s early edge.

**Also open:**
* The host needs a per-think re-seed of the plan slots (`0099B450` / `0099B4E8`) before
  `007B4ED0` can be wired.
* `009F9FC0` (the aim assist) and `009FA7E0` (the distortion) are unread.
