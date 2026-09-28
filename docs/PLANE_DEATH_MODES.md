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

## 5. Pairs

USN04 with the RNG option on both sides, on this tree (main `b17ec69e2` plus this packet).
The control is `local\dC_*.log` with `kPlaneDeathModesBound` off; the treatment is
`local\dT_*.log` with it on.

| quantity | 4500 off | 4500 on | 9000 off | 9000 on |
| --- | --- | --- | --- | --- |
| torpedo drops | 13 | 4 | 16 | 4 |
| releases from dead aircraft | 9 | 0 spawned, 2 refused | 12 | 0 spawned, 3 refused |
| Yorktown-class01 damage taken | 4599 | 0 | 4599 | 0 |
| Lexington-class01 | 7858 taken, alive | 3259 taken, alive | sunk 225.81 s | 5141 taken, alive |
| deaths | 18 | 19 | 30 | 27 |
| total damage | 16504.6 | 7306.4 | 19256.2 | 11025.9 |
| death modes (explosion / delayed / powerlost) | - | 1 / 12 / 6 | - | 2 / 14 / 11 |
| removed | - | 11 | - | 15 |
| negative halvings | 10056 | 46190 | 10879 | 283362 |

**The headline held.** Every torpedo from a dead Kate is gone.
- Yorktown's 4599 disappears at both lengths.
- At 9000, Lexington survives. In this control Fletcher-class01 was not sunk, so that
  prediction had nothing to test.
- The refusals are fewer than the dropped releases, because most dead Kates now explode 0.6 to
  1.8 s after death, before they reach a release. Removal times at 9000 were 0.00 to 1.80 s,
  15 aircraft.

**Mode mix.** At 9000 the modes were explosion 2, delayed 14 and powerlost 11 out of 27, against
the expected 20/40/40%. Two explosions against 5.4 expected is low, but within chance at n = 27.

**What else moved, and why.** The halvings and category 1 shots rise sharply, 11905 to 41697
shots at 9000.
- The cause is one live aircraft, not the death modes. At 9000, movieval|.-2 now survives the
  flak that killed it at 200.01 s in the control.
- It reaches the water alive at about 203 s. Under `kPlaneWaterContactGateBound` (packet
  `cc9_water_surface_law`) that contact is ignored, as `007CB7F0` does for a live AI aircraft
  with a non-zero MinWaterSpd.
- It then stays in free flight just below the surface and never goes below -30 m, so it remains
  a live, unhittable target until the end. It takes 0 hits, while Fletcher-class03 takes 692
  zero-damage hits around it.
- At 4500, two contacts are ignored in the same way.
- **Open item for the lead.** The water gate is faithful to the listing. The host's pilot holding
  a live aircraft at the surface is not, because the image's pilot would climb. Until the
  low-altitude flight is fixed, category 1 counts after a live contact do not measure accuracy.

**Deaths** go 30 to 27 at 9000. Lexington's survival accounts for one. The other two were not
traced aircraft by aircraft.

## 6. The death flags `+5Dh` and `+60h` (packet `cc9_plane_death_flags`)

### 6.1 The image

- **`+60h` at the destroy.** `007D0B80`'s tail (`007D12FC`) calls `vtable[70h](1)`, the unit's
  `0077D1A0`, which reaches `00926C80` (BSP_MissionEntity_Destroy). Under the `009248D0` lock it
  returns if `+60h` is already set (`00926CBF`). Otherwise it sets `+60h = 1` at `00926CD5`, sets
  the cause `+70h` (1, or the parent's), and queues the entity on the destroy list.
- **`+5Dh` at the flush.** `009273A0` dispatches `vtable[74h]` = `00926390` for every queued
  entity. When `+5Dh` is clear it stores `AL = 1` to `+5Dh` (`0092639B`) and `+60h` (`0092639E`),
  calls `00925C90`, and tail-jumps to `vtable[7Ch]`.
- **The readers.** `00A2DDE0`, the AI group's eviction pass, keeps a member only while
  `+5Ch` is set and `+5Dh`, `+5Eh` and `+60h` are clear (`00A2DE26`-`00A2DE3C`), and its party and
  team still match. The same four-byte gate is `0043F080`, which the dogfight target test uses.
  The release stage refuses on `+5Dh` alone (`007CEA29`).

### 6.2 The host before this packet

- The plane death sets `C3Ah` and removes the aircraft from its squadron, but it writes neither
  byte. `GameAiHost::unit_flags` reads `state->active`, `state->simulate` (`+5Dh`),
  `scene_destroyed_005e` and `scene_pending_destroy_0060`, so a dead aircraft passes the gate.
- cc9-dogfight-engaged found the consequence: a dead aircraft stays a group's first member, the
  group's leader point freezes on the wreck, and `00A12A90`'s promotion of the US group waits on
  a distance that never closes.

### 6.3 The binding (`kPlaneDeathFlagsBound`, default true)

- In the plane-branch head, at the step the death mode is chosen and `C3Ah` is set, the host sets
  `scene_pending_destroy_0060` (`00926CD5`) and `state->simulate` (`0092639B`).
- **Substitution, labelled:** the host has no `009273A0` flush, so `+5Dh` is set in the same step
  as `+60h`, not at the next flush. That is the same boundary the squadron removal already takes.
- Only aircraft are covered. Ships keep their own path (`kShipAiTargetReleaseBound`).

### 6.4 Predictions (written before the runs)

E2 9000, same tree, `BSP_GUNNERY_RNG_STREAMS=1`, switch OFF (`local\fl0`) against ON
(`local\fl1`). The OFF side is expected near `local/bC_9000.log` on the earlier tree: 37 deaths,
3 drops, AI coordinator `evicted=10 destroyed=14 promotions=3`, the US group promoted at step 2436
(121.80 s).

- **Dead aircraft leave their AI groups.** `evicted` rises from about 10 to at least the number of
  aircraft deaths in groups, 30 to 50. `destroyed` rises as whole groups empty.
- **Promotion time.** Within 1 s of 121.80 s if the US group's target leader is alive at 121.8 s.
  If it died earlier, the leader point moves to the next live member and the promotion moves by
  seconds, either way. It does not disappear.
- **Deaths** 37 within 34-40. **Torpedo drops** 3 within 2-5: a dead aircraft's release is already
  refused through `C3Ah`, so the new `+5Dh` refusal adds nothing.
- **Plane guns (category 0).** The dogfight target test (`0043F080`) now drops dead aircraft, so
  fighters stop firing at wrecks. Category 0 shots may fall; hits on live targets should not.
- **Ship AA.** Flat except through RNG coupling: the gunnery host already drops a dead target.
- **Lexington** takes no damage on either side.

### 6.5 Pair (E2 9000, OFF `local/fl0_9000.log`, ON `local/fl1_9000.log`)

The OFF side sits on a newer main than `bC_9000`: 35 deaths and 0 drops, not 37 and 3.

| term | OFF | ON | prediction | held? |
| --- | --- | --- | --- | --- |
| dead aircraft carrying `+5Dh`/`+60h` | 0 | 37 of 37 | all | yes |
| AI members evicted | 10 | 44 | 30-50 | yes |
| AI groups destroyed | 15 | 12 | rises | **no** |
| AI members added | 177 | 153 | not predicted | - |
| US group promotion (members 18) | 121.80 s | 121.80 s | within 1 s | yes |
| third promotion (`Lexington-class01_sqn01`, 12 members) | 235.06 s | 192.06 s | not predicted | - |
| deaths | 35 | 37 | 34-40 | yes |
| torpedo drops | 0 | 0 | flat | yes |
| category 0 shots / hits | 1111 / 34 | 1111 / 34 | may fall | flat |
| categories 1, 5, 6 hits | 128, 88, 186 | 122, 78, 193 | flat (RNG-coupled) | yes |
| Lexington damage | 0 | 0 | 0 | yes |

- **The eviction works as the gate reads.** Every dead aircraft now fails `00A2DDE0`'s gate on the
  next pass, which is why evictions rise by 34.
- **Fewer groups destroyed, fewer members added.** Dead members now leave groups on their own, so
  fewer groups empty in one go and fewer re-seeds follow. The mechanism is not traced beyond the
  counters.
- **The US group's promotion did not move.** This tree runs with the range factor off, the D0
  state, where cc9-dogfight-engaged also saw the promotion at 121.80 s. The frozen-leader case they
  found is in the range-factor-on run, which is their retry.
- **The squadron-01 group's promotion comes 43 s earlier.** The likely cause is that it no longer
  closes on a dead target leader, but its target group was not traced.
- **The two extra deaths** are US fighters: `Yorktown-class01_sqn04` at 381.18 s (powerlost) and
  `Lexington-class01_sqn01` at 424.47 s (delayed explosion). Both are late in the run, after the
  earlier promotion changed their orders. Kate and Val deaths match one to one: Vals by a mean of
  -0.5 s (one -11.0 s), Kates by -0.2 s.

### 6.6 Decision

`kPlaneDeathFlagsBound` lands ON. The bytes are the image's at the image's moment, apart from the
flush being taken in the same step. Ships are not covered by this switch.

## 7. A dead aircraft's bot stops thinking (packet `cc9_dead_plane_bot_think`, `kDeadPlaneBotThinkBound`)

Worker cc9-lua7, 2026-09-28. The question comes from cc9-lua6's handoff: does the image keep
running a dead plane's bot think and task once `007F3970` has cleared `plane+9D4h`?

### 7.1 The image (V, raw listing of `0099ACD0`)

`BSP_PilotBot_Tick` opens with five tests, all before its think accumulator at `0099AD0F`:

```
0099acd6: MOV ECX,[ESI+0x50]      ; the unit
0099acd9: TEST ECX,ECX / JE 0099B19F
0099ace1: CMP byte [ECX+0x5d],0 / JNE 0099B19F
0099aceb: CMP byte [ECX+0x60],0 / JNE 0099B19F
0099acf5: CMP byte [ECX+0x61],0 / JNE 0099B19F
0099acff: MOV EAX,[ECX+0x9d4]     ; the squadron, no null test
0099ad05: CMP byte [EAX+0x61],0 / JNE 0099B19F
```

`0099B19F` is the tick's exit. Everything else in the tick lies past those tests: the accumulator,
the task vector's arm calls (`0099AF9B`), the planner (`009998A0`) and the command publish
(`007B8C90`).

- **A dead aircraft fails the tests from its death on.** `007D0B80`'s tail (`007D12FC`) calls
  `vtable[70h](1)`, and `00926CD5` sets `+60h`. The destroy-list flush then sets `+5Dh` at
  `0092639B` (section 6.1).
- **The squadron removal comes after `+60h`.** `007BCAA0`'s `007F3970` clears `+9D4h` (`007F3A07`).
  Gate 5 dereferences `+9D4h` with no null test, and the image never faults there, because gate 3
  has already closed.
- **The commit finds nothing to take.** `007BB920` (called at `007CE865` from the plane's fixed
  step) returns when `+A14h` is clear (`007BB929`). Only `007B8C90` in the tick sets that byte. So
  from the death on, the live control axes hold whatever was last committed.
- **`007CAF10` still runs** (section 1, step 5). It zeroes the air brake of a powerlost or spinning
  aircraft, and the throttle too when `C39h` is set. That is its only control write.

**So a powerlost aircraft in the image glides on its controls as they were at the death**, with
throttle and air brake zero, until the water, the ground or its 1000 s timer removes it. An
explosion-delayed aircraft does the same for its 0.6 to 1.8 s. Section 1's phrase "glides under
pilot steering" means those frozen axes, not a live pilot.

### 7.2 The host

`pilot_think_and_commit` in `src/game_hosts_units.cpp` models only gate 6, the accumulator. A dead
aircraft keeps running its task arms, its planner and its command publish every 0.09 s. The
powerlost glide is therefore steered, and the arms' draws on the shared stream continue after the
death. That is where cc9-lua6's per-member draws came from.

### 7.3 The binding (`kDeadPlaneBotThinkBound`, committed OFF, flipped ON in 7.5)

- With the switch on, a plane whose `+5Dh` (`state->simulate`) or `+60h`
  (`scene_pending_destroy_0060`) is set skips the accumulator and the whole think.
- `007BB920`'s commit still runs, and finds no pending block.
- **Gate 4, labelled.** `+61h` has no writer in the image (`unit_flag_0061`).
- **Gate 5, labelled.** The squadron's `+61h` is not modelled. The image cannot reach it for a
  removed plane, as 7.1 shows.
- The first gated tick of each plane is logged as `dead plane bot think:` with its held controls.
  The summary line is `summary mission dead plane bot think: ticks_skipped= bound=`.

### 7.4 Predictions, written before any run

The death lists are from the current tree's OFF logs (LOMP10, USN01) and from `rb9_usn04.log`
(USN04).

- **USN01 3200/3000: moved, not identity.** Five Mavis die: Mav5 at 72.35 s (delayed), Mav4 at
  73.80 s (powerlost), Mav1 at 75.50 s (delayed), Mav2 at 76.80 s (powerlost) and Mav3 at 78.10 s
  (powerlost).
  - All five log a gate line at their death time.
  - The three powerlost glides change shape, so their water contacts and removals move in time.
    The death rows themselves do not move: the death times come before the gate.
  - Anything drawing on the shared stream after 72.35 s may move, because the dead planes' arms
    stop drawing. Releases before 72 s are identical.
- **USN04 4700/4500: moved from the first aircraft death.** The first is movieval at 26.60 s.
  - The 43 plane deaths are 10 explosion, 16 delayed and 17 powerlost. The 17 powerlost planes
    stop steering.
  - The death count stays within +-2. Kill order and later deaths can move through the shared
    stream.
  - Releases stay within +-1 of 7 of 16 torpedo and 4 of 19 dive, since a dead plane releases
    nothing either way.
- **LOMP10 3200/3000: moved from 99.40 s**, the first death, B-25 01|.-2 (delayed).
  - Warhawk 01, the squadron leader, dies at 107.00 s (delayed, 1.01 s). It holds its controls
    for that second instead of flying its task.
  - The wingmen are already out of its formation, since the removal promotes a new leader at once.
    I expect their own paths to be unchanged until a shared-stream draw diverges.
  - Lightning 01 (powerlost, 110.65 s) glides frozen.
- **The verdict rule.** The mechanism check is the gate lines: every dead plane gated at its death
  tick, and none gated before. The combat rows are shared-stream consequences, and they are
  recorded rather than predicted exactly.

### 7.5 Pairs

**The binaries.** OFF is this tree's build at `8994a7f65` (SHA-256 prefix `000E29701DE0`). ON is
`pair_export --commit 8994a7f65 --flip kDeadPlaneBotThinkBound=true` (prefix `6DBD592D11AE`). The
launch lines are the reference ones. The logs are `local/l7_dpoff_<row>.log` and
`local/l7_dpon_<row>.log` in worktree cc9-lua7.

| row | `pair_diff` | deaths, hits, damage, shots, drops | task releases (torpedo / dive) | plane water contacts | gated planes |
| --- | --- | --- | --- | --- | --- |
| USN04 4700/4500 | exit 3 | identical (40 / 749 / 13347.8 / 10512 / 5) | 8 -> 5 / 4 -> 1 | 11 -> 13 | 30 of 40 deaths, 5577 ticks skipped |
| USN01 3200/3000 | exit 3 | identical (5 / 516 / 2833.5 / 1534 / 0) | 2 -> 0 / 2 -> 2 | 3 -> 3 | 5 of 5, 785 ticks |
| LOMP10 3200/3000 | exit 3 | identical (10 / 279 / 3500.0 / 1887 / -) | - / 4 -> 3 | 0 -> 1 | 5 of 10, 293 ticks |

**The mechanism matches on every plane.**
- Every gate line falls on its plane's death step, with a lag of 0.00 s on all 40 gated planes.
- No live plane is gated.
- Every dead plane that is not gated died by `explosion`: 10 on USN04 and 5 on LOMP10. Those are
  killed and removed in the same step (section 1, step 4).

**Every lost release was a dead plane's.** `local/l7_rel.py` classifies each `release census:`
line by whether its plane had already died.
- **USN04 OFF** logs six releases after a powerlost death:
  - B5N Kate #2.1|.-3, dead at 124.40 s;
  - D3A Val #1.1, dead at 134.50 s, three times;
  - B5N Kate #6.1|.-2, dead at 204.81 s;
  - B5N Kate #6.1|.-4, dead at 204.31 s.
- **USN01 OFF** logs Mav2 (dead at 76.80 s) and Mav3 (dead at 78.10 s).
- **LOMP10 OFF** logs Lightning 01 twice (dead at 110.65 s).
- **ON** logs none of those, and every live plane's releases are the same in number and order.
  That includes the six Warhawk wingman releases on LOMP10 after Warhawk 01 dies.

The torpedo drops do not move (USN04 5 on both sides). So the dead planes' task releases never
produced a weapon. They were the task counter answering for a plane the image had already stopped
thinking for. Section 6's release-stage refusal (`dead_releases_refused=0`) never saw them.

**The other moves:**
- **Water contacts.** On USN01 the frozen glides bring Mav3 onto the water. On LOMP10 they bring
  Lightning 01 onto the water. On USN04 two more Kates reach the water. B5N Kate #2.1|.-3's contact
  and B5N Kate #4.1|.-3's contact change speed.
- **One USN01 death row.** Mav4's `killer_range` goes 772 -> 770 and its `nearest_horizontal`
  761 -> 759. Mav4 dies 1.45 s after Mav5, whose controls are now frozen. This is a 2 m move in a
  distance measured at the death.

**Against the predictions (7.4):**
- **USN01 moved, as predicted.** "Moved, not identity" holds. The move is the dead planes'
  releases and Mav4's death row, not a combat change.
- **USN04 missed on releases.** I predicted +-1 and measured -3 torpedo and -3 dive. I had not
  seen that a dead plane's task counts a release. The death count I predicted from `rb9_usn04.log`
  (43) was stale: this tree's OFF has 40, identical ON.
- **LOMP10 held.** It moved from the first dead plane, the Warhawk wingmen are unchanged, and
  Lightning 01 glides frozen.

**Verdict: `kDeadPlaneBotThinkBound = true`.** The mechanism matches: every gate falls on a death
step, no live plane is touched, and every lost release is a dead plane's. The misses are in spread,
and the mechanism explains them.

**Next, from cc9-lua6's handoff item 2.** The gate closes in the death step itself: the death
block runs before the plane's fixed step, and the gate lines show a lag of 0.00 s. So a think-driven
draw in the death tick is now skipped. Whether the own-block draw cc9-lua6 saw came from the think
is not yet checked. It wants one `kDepartedWingmanTaskBlockBound` log on this base before anyone
codes either of cc9-lua6's two fixes.
