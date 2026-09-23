# How the image ends a shot-down aircraft

Addresses: `00877B90` (BSP_UnitInstance_SetHealth), `007CA8A0` (the plane's `vtable[1B0h]`, the
death-mode choice), `007BBFA0` (`vtable[194h]`, BSP_Plane_StartNamedEffect), `007D0B80`
(BSP_Plane_HandleStateMessageKinds), `007CAF10` (`vtable[1ECh]`, BSP_Plane_LimitAndResetControls),
`007CE040` (`007CE9FD`-`007CEA2D`, the release gate), `007BBCF0` (the plane hit response), `007CFD20`
(the constructor defaults) and `004A9BD0` (the planepartclasses global loader).

Packet `cc9_plane_death_modes`, 2026-09-23. Ghidra was read, not written. All names are
hypotheses. Nothing here is ABI-compatible or game-validated.

## 0. What this changes in the E2 reference

**Dead torpedo bombers have been scoring hits in every USN04 reference.** In
`local\wC_9000.log` (this tree before this packet, USN04, 9000 mission frames, option on),
12 of 48 releases come from B5N Kates that the gunnery host had killed 1.2 to 6.2 s earlier.
- Yorktown-class01's 4599 damage matches the four Kate #4.1 torpedoes (1150 each).
- The Lexington sinking at 225.8 s is credited to B5N Kate #6.1, which died at 213.61 s and
  released at 219.80 s.
- The Fletcher-class01 sinking at 255.5 s is credited to B5N Kate #6.1|.-3, which died at
  222.61 s and released at 226.90 s.

The `635e7b27b` reference has the same pattern, 12 of 45 releases. The image refuses every one
of those releases (section 1, step 6). Any E2 ship-damage or sinking figure taken before this
packet includes them.

## 1. The chain, from the listing

1. **Health reaches zero.** `00877B90` writes `unit+370h`, and on every change except in net mode 2
   it calls `vtable[1B0h]` (`00877C40`). For the dive bomber's vtable `00D19D28`, `+1B0h` holds
   `007CA8A0` (`00D19ED8`). The same pointer sits in eight other vtables.
2. **`007CA8A0` chooses the mode.**
   - It returns at once while `unit+370h > 0` (`007CA8E5 COMISS`, `JC`).
   - Otherwise it draws `r = 00BD2F10(ECX=1, 0.0, 1.0)` (`007CA914`), on the shared stream 1.
   - With the tuning block's `DeathModeChances` (`+10h` Explosion, `+14h` Explosion_delayed,
     `+18h` Spinning, `+1Ch` Powerloss), `c1 = +10h`, `c2 = c1 + +14h`, `c3 = c2 + +1Ch`. `+20h`
     Explodetoparts is not read here. The choice, in listing order:

     | test | mode |
     | --- | --- |
     | `byte unit+DFCh` set (`007CA91D`) | `"explosion"` |
     | `unit+800h == 0` and `r < +18h` | `"leftspinning"` |
     | `unit+800h == 1` and `r < +18h` | `"rightspinning"` |
     | `r < c1` | `"explosion"` |
     | `r < c2` | `"explosion_delayed"` |
     | `r < c3` | `"powerlost"` |
     | otherwise | `"explosion_delayed"` (`007CAA7E`) |

   - This installation's `planeglobals.lua`: Spinning 0.3, Explosion 0.2, Explosion_delayed 0.2,
     Powerloss 0.4, Explodetoparts 0.2.
   - `unit+DFCh` is set by `007BBCF0` (`007BBD13`) when the hit record's `+4h` object answers
     `IsKindOf(5)`, the unit base. That is a ram by a unit, not a projectile.
   - `unit+800h` is -1 from the constructor (`007CFD69 OR EDI,-1`, `007CFDB4`). `007BBCF0` copies
     a hull segment's side into it (`007BBDA8`) only when `hit+34h != -1`.
3. **`007BBFA0` stores the timer and routes the mode's message.**

   | mode | `unit+C10h` | message |
   | --- | --- | --- |
   | `explosion` | -1.0 (`00D7A260`) | 99 |
   | `explosion_delayed` | `00BD2F10(ECX=0, [00E18710], [00E1870C])` | 99 |
   | `powerlost` | 1000.0 (`00CE3804`) | 102 (`007BA1C0`) |
   | spins | 1000.0 | 103 (`007BA200`, side) |

   - `004A9BD0` fills `[00E18710]`/`[00E1870C]` from `planepartclasses.lua`'s
     `ExplosionExplosionDelay = {0.6, 1.8}`.
   - The same loader also fills `SpinExplosionDelay` (`[00E18708]`/`[00E18704]`),
     `PowerLostExplosionDelay` (`[00E18700]`/`[00E186FC]`) and `SpinVSPowerLostChance`
     (`[00E18720]`). A byte scan finds **no reader** of those five dwords outside the loader.
     They are dead data in this image; my earlier `docs/WATER_SURFACE_LAW.md` note that
     PowerLost explodes after 4 to 8 s was wrong.
   - The explosion arm of `007BBFA0` forces an immediate explosion when the flight state is not
     7, 6, 4 or 5, or when the free-flight gate is false and the speed is below 0.2778
     (`00CF8918`).
4. **`007D0B80` handles the message.** It dispatches through `msg->vtable[0Ch](kind)`.
   - Kind 66h (powerlost, `007D0BEB`), or kind 63h with `unit+C10h > 0` and the free-flight gate
     (`007D0BF9`-`007D0C2C`): `007DE600(unit+C98h)`, `unit+C39h = 1` (`007D12D6`-`007D12E8`).
   - Kind 63h otherwise (`007D0C37`): a point effect above the altitude `[00CFBC84]`, then
     **`BSP_MissionEntity_Kill(unit, 1)`** at `007D0CFD` and `unit+C10h = -1`.
   - Kind 67h (spin): wing-break effects and `unit+C36h = 1`.
   - The tail `007D12FC`-`007D131B`, reached by every arm above: when `unit+C3Ah` and `unit+5Dh`
     are clear, it sets `C3Ah = 1` and calls `vtable[70h](1)`, which is
     BSP_UnitInstance_DestroyAndBroadcast. The destroy list then sets `unit+5Dh` and the Lua
     `Dead` flag (`docs/ENTITY_DEAD_FLAG.md`). **The aircraft is dead from the message on.**
5. **`007CAF10`, every step of a dead aircraft.** It is called from the free-flight arm at
   `007CC322` and from the water arm's first step.
   - While `unit+5Dh` or `+5Eh` is set, it adds the step to `unit+C3Ch`, the DeadMeat timer.
   - When `0 < unit+C10h < unit+C3Ch` and `[00E186E8] <= [00E1873C]` (MaxExplosionNum), it raises
     `"explosion"`. That sets `C10h = -1`, and message 99 then reaches the Kill arm.
   - With `C39h` or `C36h` set and the free-flight gate true, it zeroes the air brake, and with
     `C39h` also the throttle (`unit+9F0h`/`+BBCh`, `+9F4h`/`+BC0h`), then returns. The pilot's
     steering is left alone.
   - The DeadMeat timer drives the free-flight drag ramp at `007DBB0E`, which the host already
     carries as `lost_drag_timer`. After `Dynamics/DeadMeat/LostDragTime`, a nose-down dead
     aircraft loses its drag.
6. **No release from a dead aircraft.** The release stage `007CE9FD` returns at `007CEA1C` when
   `unit+C3Ah` is set, and at `007CEA29` when `unit+5Dh` is set.
7. **The AA target set.** `008FFA20` drops a target whose `+5Dh` is set. The host already drops a
   gunnery-dead target (`src/game_hosts_gunnery.cpp`, the `unit_state[target].dead` test).

**So in the image, per mode, from the death:**

| mode | share with `unit+800h = -1` | what happens |
| --- | --- | --- |
| explosion | 20% | killed and removed at once |
| explosion_delayed | 40% (0.2 to 0.4, and 0.8 and above) | throttle and air brake zero, removed 0.6 to 1.8 s later |
| powerlost | 40% | throttle and air brake zero, glides under pilot steering, removed only when `C3Ch` passes 1000 s or by another path (the ground explosion in `007CE040`, or the water and depth rules in `docs/WATER_SURFACE_LAW.md`) |
| spins | 0% here | `unit+800h` is -1 without a hull-segment hit |

A dead aircraft releases nothing and is not an AA target in any mode.

## 2. The host today

Measured on `local\wC_9000.log`: this tree before this packet, USN04, 9000 mission frames, with
the RNG option on.
- **Dead aircraft fly on normally.** 29 side-1 aircraft die. The host keeps flying each one under
  its pilot, with the throttle the pilot commands, until the run ends. Only 7 of them reach the water, 7 to 114 s after
  death in the reference.
- **Dead aircraft release ordnance.** 12 of the 48 releases come from a B5N Kate the gunnery host
  had already killed, 1.2 to 6.2 s earlier. The four Kate #4.1s die at 119.00 to 128.50 s and
  release at 122.40 to 129.70 s. Those torpedoes do real damage:
  - Yorktown-class01 takes 4599, all from the four Kate #4.1 torpedoes.
  - Lexington-class01 is sunk at 225.81 s, credited to B5N Kate #6.1, which died at 213.61 s and
    released at 219.80 s.
  - Fletcher-class01 is sunk at 255.46 s by B5N Kate #6.1|.-3, which died at 222.61 s and released
    at 226.90 s.

  The same happened in the `635e7b27b` reference: 12 of 45 releases.
- **AA fire on dead aircraft is already zero**, by the dead-target test. No round or halving goes
  to a dead aircraft today, so that term does not move.

## 3. The binding (`kPlaneDeathModesBound`, default true)

The pure rules are in `include/bsp/plane_death_modes.hpp`. The units host applies them in three
places:
- **At the head of the plane branch in the motion loop.** On the first step where the gunnery
  host has the aircraft dead, it chooses the mode, sets `C10h`, `C39h`/`C36h` and `C3Ah`, and on an
  immediate explosion kills it. A killed aircraft is no longer stepped.
- **In the free-flight arm, after the pilot's commit.** This applies `007CAF10`'s death terms: the
  DeadMeat timer, the delayed explosion, and zero throttle and air brake.
- **In the release stage.** `C3Ah` now blocks the release, as it does at `007CEA1C`.

**Substitutions, labelled in the code:**
- The death is the gunnery host's, seen at the next plane step.
- The two draws come from a units-host generator, not from `00BD2F10`'s streams 1 and 0. The
  gunnery host owns the shared stream, and cc9-difficulty holds a lease on its file.
- `unit+DFCh` is clear and `unit+800h` is -1, so no spin.
- The message is delivered in the same step.
- The MaxExplosionNum budget is taken as met.
- Removal means the aircraft leaves the motion step. The scene node is not torn down.

- **At the torpedo and bomb spawn sites.** These bypass the release stage, and the lead assigned
  them to this packet. A release from an aircraft the gunnery host has dead is refused and logged
  as `dead release refused`. This is the same switch.

## 4. Predictions, written before any run

USN04 with the RNG option on, switch off against switch on, same tree.
- **Death modes.** Every plane death logs one `plane death mode` line. With 29 deaths, expect
  about 6 explosions, 12 delayed explosions and 12 power-lost.
  - Explosions are removed at the death step.
  - Delayed explosions are removed 0.6 to 1.8 s after the death.
  - Power-lost aircraft glide at zero throttle and reach the water sooner. Water contacts rise
    from 8, and every contact is a dead aircraft.
- **Releases from dead aircraft, the headline.**
  - At 9000: 12 torpedo releases are refused, and torpedo drops go 16 to 4.
  - Yorktown-class01 takes no torpedo damage instead of 4599.
  - Lexington-class01 is not sunk at 225.8 s, and Fletcher-class01 is not sunk at 255.5 s.
  - Ship deaths fall by 2. Because those two ships keep firing, aircraft deaths may rise.
  - At 4500, the older-tree log `local\l4_4500.log` has 9 of 30 releases from dead Kates. Expect
    about 9 refusals, torpedo drops falling by as many, and Yorktown's 4599 gone.
- **AA rounds and halvings on dead aircraft:** 0 before and after.
- **Hits by entity.** Category 10 hits fall by the dead aircraft's torpedoes once the refusal lands.
  With the stage gate alone, only rows downstream of the removed and power-lost aircraft's paths
  move.
