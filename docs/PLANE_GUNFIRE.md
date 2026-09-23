# Plane gunfire: the plan re-seed, the forward-gun path, and the C50h finder

Addresses: 0099B450, 0099B4E8, 007D2406, 0099BEE0, 007BB6E0, 007E2090, 007B96D0.

Packet `cc9_plane_gunfire`, after `cc9_dogfight_gun` (docs/DOGFIGHT_GUN.md). Every name is a
hypothesis, not a recovered symbol.

## 1. The per-think re-seed, 0099B450

`0099B450`, `__thiscall(plan)`, `RET 0` (`0099B588`). It is called:
* at `009998D2` in `BSP_PilotBot_Update`, once per think, before `0099B740` and the task arm;
* at `0099BEC9` from the plan array constructor;
* at `0099B591` from the hold variant `0099B590`.

It seeds the five slots from the live block with `active = 0`. Its tail
(`0099B4E8`-`0099B580`) then sets:

| field | value | site | width |
| --- | --- | --- | --- |
| `+26Ch` | 0 | `0099B4F7` | byte |
| `+270h` | 0 | `0099B4FD` | dword |
| `+2B4h` | `[plan+2F4h]+190h`, which `007D2406` stores as TravelSpeed * `tuning+334h` (`NewTravelSpeedMul`, 1.6) | `0099B503`/`0099B511` | float |
| `+2BCh`, `+2C4h` | 0.0 | `0099B517`, `0099B509` | float |
| `+2E8h` | 1.0 (`00D7A24C`) | `0099B52C` | float |
| `+2B0h` | 0 | `0099B53C` | byte |
| `+2D8h` | 1 | `0099B542` | dword |
| `+2CCh` | 1 | `0099B548` | dword |
| `+2D0h` | 2 | `0099B54E` | dword |
| `+2D4h` | 1 | `0099B558` | dword |
| `+2C8h` | 20.0 (`00CE3930`) | `0099B55E` | float |
| `+2DCh`, `+2E4h`, `+2E5h` | 0 | `0099B566`-`0099B572` | byte |
| `+2ECh` | `[00E0E2EC]` | `0099B580` | float |

**Host before this packet.** The think already calls `pilot_reset_plan_0099b450`, which covers
the slots, `+2BCh`, `+2C4h`, `+2C8h`, `+2E8h`, `+2CCh` and `+2D0h`. It did **not** re-seed
`+2B4h`, `+2B0h` or `+2D8h`, which live on the slot as `plane_desired_speed_2b4`,
`plane_trg_speed_corr_off_2b0` and `plane_air_brake_mode_2d8`. A state's speed write therefore
outlived the state. That is how run F1's one-tick maneuver throttle write (`+2D8h = 0`) stalled
two fighters. `+2D4h`, `+26Ch`, `+270h` and `+2ECh` have no host field.

**Bound** (`kPilotPlanReseedBound = true`): the three fields, re-seeded right after the existing
reset and before the task arm.

## 2. Who fires the forward guns: not established

* **The request chain holds.**
  * `BSP_PilotBot_Tick` loads `EBX = [bot+58h][0]`, the current task (`0099AE8C`). It passes
    `ECX = EBX+4`, the plan base, to `0099BEE0` (`0099B07C`), and `[EBX+274h]` there is the
    plan's `+270h` target pointer.
  * So `task+2E0h` is `plan+2DCh`, which becomes `cmd+16h`, then `unit+A12h`, then
    `unit+9FAh` (`gunFire`, `007BB8BF`), latched to `+BC9h` by `007B9770` (`007B97B4`).
  * The latch writes the whole block `+BB0h`-`+BCAh` in one pass.
* **Readers searched, none a weapon:**
  * `+BC9h` has only the latch.
  * `+BC8h` has `CMP byte` readers of the `+9F8h` byte (`007C0FCF`, `007DDCB3`, `007EB277`,
    `007EB52B`, `007EC966`) and float fields of other classes.
  * `+BB0h` is taken by pointer in `007DB6DE`, the flight core law, which reads only floats.
  * `+9FAh` is read by the quantiser, the latch, `007C7800` (a camera effect in segment 45,
    `0079A3B0`/`00795D90`), the property-bag writer `007C9B2B`, and `007D6A92`.
  * `+A12h` has no `.text` hit.
  * `LEA reg,[reg+9E4h]` gives five sites. The getter `00518F70` has no reference.
* **From the gun side:**
  * Category 0 is `PLANEGUN` (`docs/GUNNERY_TABLES.md`, row `00E092C8`), with its own target
    preference row `61h`. `0072C6A0` builds no gun bot for it.
  * `BSP_Gun_FireIfReady` (`00727E30`) sits in the gun vtables at `+1DCh` (`00CFBEFC`,
    `00CFE284`, `00CFE4E4`, `00CFE724`). Its direct callers are the bot path `0084C4E3`, the
    function at `006E4C10` (no Ghidra function), and `006EBFF0` (no Ghidra function).
    `006EBFF0` is an override in the vtable at `00CFAAB8`, whose constructor is `006EC640`. It
    calls the base, then an extra step when `[[00E188A8]+1FE4h] == 1`.
  * A scan for `CALL`/`MOV reg,[reg+1DCh]` in `006E0000`-`00A00000` found no virtual caller.
* **Not bound.** The consumer of `gunFire`, or of the PLANEGUN category, is the missing link.
  `kPlaneGunfireBound` was not added, no rounds are spawned, and no gunnery hook is requested.
  Next leads:
  * the gunnery pass `00864FE0` for category 0 with preference row `61h` (whether PLANEGUN guns
    get targets and fire there);
  * the class built by `006EC640`;
  * the property bag's `gunFire` key `00D05D64`, which the player path and the replay
    serialiser use.

## 3. The unit+C50h object and its finder

**Creation.** `007D621F` stores `unit+C50h = 007E1E20(unit)`, a `0xCCh`-byte object with
vtable `00D06B88`. It is built only when `[[00E188A8]+1FE4h]` is 0 or 1 and `unit+54h` differs
from the local player's `[[00E188A8]+5FCh]+908h`, which in practice means AI planes.
`007D0032`/`007D61BA` clear it.

**Constructor `007E1E20`.** It builds three list sub-objects at `+1Ch`, `+3Ch` and `+5Ch`, with
vectors at `+30h`/`+34h`, `+50h`/`+54h` and `+70h`/`+74h`. It sets the periods `+88h = 3.0`
(`00CE3854`), `+8Ch = 2.0` (`00CE3958`) and `+90h = 1.0`. The clocks start at `U(0, P) + P`:
`+7Ch` from 3.0, `+84h` from 2.0, `+80h` from 1.0.

**Update `007E2010`** (slot `+0Ch`, no Ghidra function, `007E2010`-`007E2065`, `RET 4`). It adds
`dt` to `+7Ch`, `+84h` and `+80h`. When `+7Ch >= +88h` it calls `007E11D0` and subtracts
`+88h`.

**Refresh `007E11D0`** (body `007E11D0`-`007E1B2E`):
* `R = (+88h + 3.0) * 180.0` (`00D7A2B0`, `00CE3D20` double), which is 1080.
* The enemy radius is `max(R, 1200.0)` (`00CFD714`). The friendly radius is R, floored at 500
  (`00CE3840` double, `00CE397C`).
* It drops `+70h` members at or beyond the friendly radius, `+50h` members beyond the enemy
  radius, and `+30h` members beyond R.
* It then walks the world list `[[00E188A8]+19CCh]+58h` for entities other than the owner that
  answer `vtable[5Ch](6)` or `(0Fh)` and are not already listed, and applies the distance gates
  at `007E1AB3`-`007E1B0A`:
  * an aircraft (`0Fh`) of another party (`+54h`) within the enemy radius goes to `+50h`;
  * an aircraft of the same party within the friendly radius goes to `+70h`;
  * any candidate within R also goes to `+30h`.
* The appends use `007E0F20`, which registers observers. A 50-entry stack array only speeds up
  the "already listed" test.

**Finder `007E2090`**, `__thiscall(obj, cone, inner, range, near, threshold, squadron, noise)`,
`RET 1Ch`:
* When `+84h <= +8Ch` it returns the cached `+B0h`.
* Otherwise it subtracts `+8Ch`, stores `+B8h..+C4h`, and scores every live `+50h` member,
  optionally filtered by `candidate+9D4h == squadron`, with `007DEEC0`. It adds `U(0, noise)` to
  a positive score when `noise > 0`, keeps the maximum above `threshold`, and re-registers the
  observer pair.

**Score `007DEEC0`** (body `007DEEC0`-`007DF145`):
* It returns -999.0 (`00D049B0`) for a dead candidate and 0 outside `1 < z < +BCh` or outside the
  cone `tan^2 < +B8h^2`.
* Otherwise `interp(+B8h^2, 0, inner^2, 1, tan^2) * ramp`, where
  `inner = min(+C0h * 0.75, +B8h * 0.5)` (`00CEC9D8`, `00D7A280`).
* `ramp = interp(1, 0, +C4h, 1, z)` inside `+C4h`, else `interp(+BCh, 0.4, +C4h, 1, z)`.
* It is multiplied by `interp(0.3, 1.0, 2.0, 0.2, dy / max(1, h))` when the candidate is above.
  The sign of `dy` is read from the pseudocode, so it is labelled.

**Callers.**
* The gun `009FC90E` passes `(max(0.4, 1.25 * +40h), +38h, +30h, +34h * 0.3, -1.0, 0, 0)`.
* `009AAA80` passes `(approach+DCh, row+220h, task+344h, row+20Ch, approach+E0h, task+4C4h,
  [00CE7638])`. Its elevation test after the finder, through `007C4830` and
  `BSP_PlaneClass_LevelFlightSpeed`, is **unread**, so the early edge stays substituted.

**Bound** (`kPlaneFinderBound = true`, the dogfight fighters only):
* the enemy list with its 3 s refresh, the 2 s finder with the gun's arguments, and the score;
* its choice replaces the task-target substitute as the gun's `+74h`, still unled;
* the clocks start at their midpoints, 4.5 and 3.0.
* **Labelled.** The object's tick cadence stands in as the think interval. The `+30h` and
  `+70h` lists are not kept, and nothing else consumes the list yet.

## 5b. Predictions for the finder pair N0/N1, written before either ran

N0 is this tree with `kPlaneFinderBound = false`. N1 has it on. Both have the re-seed on and the
throttle wiring off.
1. **Census only.** The finder feeds the gun's target, and the gun only raises the census and
   the aim boredom input. The dogfight state rows should match, unless a new fire flag moves an
   aim boredom exit.
2. **Bursts.** The finder's target is any enemy aircraft inside the forward cone (tangent 0.4,
   or 1.25 × Angle_Strafe in aim) within 1500 m, not just the task target. Expect the burst
   count to rise: more bursts for the Yorktown fighters, and possibly bursts from the Lexington
   flight if any enemy aircraft passes in front of it.
3. **Other units' rows** should be identical, because no rounds are fired.

## 5. Predictions, written before runs R0/R1/T1

* R0: this tree with `kPilotPlanReseedBound = false` and `kDogfightThrottleBound = false`.
* R1: the re-seed.
* T1: the re-seed plus the throttle wiring.

All three are USN04 at the E2 parameters.

**R1 against R0:**
1. Each think's throttle arm now starts from `+2D8h = 1` and `+2B4h = TravelSpeed * 1.6`, unless
   the task arm overwrites them.
2. States that write the speed every tick do not change. That covers moveto (`009C1850`), the
   follow law (`009BFD1C`), the dive-bomb states and aim.
3. What moves are the planes whose think writes no speed:
   * the dogfight moveto leaders, whose stand-in writes no speed;
   * aircraft between tasks.
   * They now chase `TravelSpeed * 1.6` in mode 1, where R0 left whatever the last writer set
     (mode 0 and 0 m/s at spawn).
4. So expect the two dogfight leaders' speeds and paths to change, and with them the Yorktown
   engagement timing. Recon- and assignment-driven changes to other units follow, through the
   mechanism already recorded.
5. Every other plane row should match unless it hits the same no-writer case.

**T1 against R1:** the two Yorktown fighters that stalled in F1 no longer drown, because the
re-seed returns `+2D8h` to 1 on the next think. Moveto and follow then run in speed mode.
Head-on throttle applies only on head-on aim ticks, so there should be little other change.

## 6. Runs

All runs are USN04 at the E2 parameters (`--frames 9200 --press-start-frame 30 --menu-select
USN04 --mission-frames 9000 --mission-frame-seconds 0.05`). Each ran from its own copied binary
through `tools/run_game.ps1 -Exe`, and each log ends with the renderer's final COM release line.

| run | re-seed | throttle wiring | finder | binary | log |
| --- | --- | --- | --- | --- | --- |
| R0 | off | off | - | `local\binR0` | `local\R0_usn04.log` |
| R1 | on | off | - | `local\binR1` | `local\R1_usn04.log` |
| T1 | on | on | - | `local\binT1` | `local\T1_usn04.log` |
| N0 | on | off | off | `local\binN0` | `local\N0_usn04.log` |
| N1 | on | off | on | `local\binN1` | `local\N1_usn04.log` |

**R1 against R0 (the re-seed): neutral.** Every summary row, all 8 water contacts, the
killed_by table and every dogfight row are identical. The only difference is the dive-bomb
turndown row's `speed_2b4` diagnostic, 34.5 before and 106.7 after. That is the last value in
the field: turndown writes no speed, so R1 shows the re-seed value (TravelSpeed × 1.6) where R0
showed a stale write. With that field and pointer-bearing start-up lines masked, two start-up
text lines differ. Prediction items 3 and 4 were wrong: no plane changed course. Why the
re-seeded value moved nothing was not traced.

**T1 against R1 (throttle wiring on top of the re-seed): rejected.** `Yorktown-class01_sqn02` and
its `.-2` still drown, at |v| 51.02 and 51.10, the same speeds as run F1. So the re-seed does
**not** cure the stall, and `+2D8h` persisting was not its cause. The corrected hypothesis is
the head-on arm: `007B4ED0(interp(+20h, 0.3, ShootDistance, 1.0, d))` cuts the throttle to
0.3-1.0 on every head-on aim tick. This is **untested**: head-on ticks are not in the census.
`kDogfightThrottleBound` stays off.

**N1 against N0 (the finder): neutral.** Every per-entity row is identical, and the one burst
(`Yorktown-class01_sqn02|.-3` on `D3A Val #3.1|.-3` at 845.7 m) is unchanged. With the finder
bound, the gun has no target except the finder's, so that burst proves the finder produced
`D3A Val #3.1|.-3`. Each fighter did 141 list refreshes and 212 scans; the enemy lists are
empty at mission end. Prediction 2, that bursts would rise, was wrong: the forward cone and the
850 m envelope see the same single opportunity.

## 7. Decisions

1. **The re-seed lands** (`kPilotPlanReseedBound = true`): neutral, and the image's per-think
   value replaces a stale one.
2. **The throttle wiring stays off.** The stall is not a persistence problem. Its likely cause is
   the head-on throttle cut, and that needs a head-on census before it is wired again.
3. **Forward guns: not bound.** The consumer of `gunFire` or of the PLANEGUN category is not
   established (section 2), so no `kPlaneGunfireBound`, no rounds and no gunnery hook.
4. **The finder lands** (`kPlaneFinderBound = true`): neutral. It gives the gun its image
   target source, and it is ready for `009AAA80` and the probe once `007C4830` (the elevation
   test) and the `+30h` list are read and bound.

## Correction, 2026-09-23 (packet cc9_plane_gun_pass)

Section 2's "no weapon reader" of `gunFire` is **wrong**.
* The plane's fixed step `007CE040` reads it as `[ESI+8B9h]` with `ESI = unit+310h`, which is
  `unit+BC9h` (`007CE974`), right after the latch `007B9770` (`007CE96F`).
* It hands it to `SetTriggerHeld` (`vtable[1E8h]`) of every gun part (`IsKindOf 20h`) whose
  weapon group is enabled (`007CE995`-`007CE9FB`).
* The literal `C9 0B 00 00` scan could not see an `ESI+8B9h` displacement.
* The forward guns are the `MRFSGun` class. The `006EC640` lead is the catapult. See
  `docs/PLANE_GUN_PASS.md`.
