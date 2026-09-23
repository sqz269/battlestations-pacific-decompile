# The plane's forward guns: who fires them, and the early maneuver-to-aim edge

Addresses: 00864FE0, 006EC640, 006EBFF0, 00731D50, 00730E80, 0072D2C0, 00803480, 007CE9A0,
007CE974, 007C4830, 007C47F0, 009AAA80.

Packet `cc9_plane_gun_pass`. Every name is a hypothesis, not a recovered symbol.

## 1. The three leads

* **`00864FE0`** is `BSP_UnitGunneryAi_Tick`, a vtable method (`00D0D36C`) that assigns targets to
  gun bots.
  * Its category loop starts at 0 (`00865152 XOR EAX,EAX`) and runs to `0Ch` (`0086588B`). So
    category 0 (PLANEGUN) **is** iterated whenever a unit's category-0 list (`unit+394h` count)
    is non-empty.
  * But it only feeds bots, and `0072C6A0` builds no bot for sub-type 0. It is not the forward
    guns' consumer.
* **`006EC640`** is the **catapult** class, built only by `BSP_MCatapult_Create` (`006EC8AD`),
  vtable `00CFAAB8`. `006EBFF0` is its fire-if-ready override: the catapult's launch, with a
  multiplayer step when `[[00E188A8]+1FE4h] == 1`. That lead is closed.
* **The `gunFire` property key `00D05D64`** is only written by the property bag `007C9B2B`.

## 2. The forward gun and the path from gunFire

**The class.** A plane's forward guns are the `MRFSGun` class (id `23h`, "rapid fixed slave").
* Its factory is `BSP_NodeFactory_CreateRapidFixedSlaveGun` (`00731D50`, 4D0h bytes) and its
  constructor is `BSP_RapidFixedSlaveGun_Construct` (`00730E80`), vtable `00CFE548`.
* Its slots are the turning-gun ones: `+DCh` `BSP_TurningGun_Update` `0085A270`, `+1D0h`
  `CanFire` `0085A830`, `+1D8h` `BSP_Gun_Fire` `00730160`, `+1DCh` `BSP_Gun_FireIfReady`
  `00727E30`, `+1E8h` `BSP_Gun_SetTriggerHeld` `0072D2C0`.
* Its message handler `+164h` is `00803480` (no Ghidra function). It routes opcode `0AFh` to
  `vtable[1E8h](msg+20h byte)` and everything else to `0072D830`.
* `0AFh` is what `0072D2C0` itself sends for an `MRFSGun` (`docs/GUN_PLATFORM_ARC.md`, step 4):
  the trigger replicated to peers.

**The reader of gunFire** is the plane's fixed step `007CE040` (`BSP_PlaneTickElement_FixedStep`),
with `ESI = unit+310h` and `EDI = unit`:

| site | what |
| --- | --- |
| `007CE96F` | `007B9770` latches the control block, so `unit+BC9h = unit+9FAh` (`gunFire`) |
| `007CE974` | `BL = [ESI+8B9h]`, which is `unit+BC9h` |
| `007CE97A`-`007CE98D` | `unit->vtable[1FCh](gunFire)` (not read) |
| `007CE995`-`007CE9FB` | for each part on `unit+48h` (`[ESI-2C8h]`, next at `+44h`) that answers `vtable[5Ch](20h)`: if its weapon group is enabled (`[[unit+538h]+94h][gun+38Ch]` entry `+0Ch` byte, grown by `005471B0`), `gun->vtable[1E8h](gunFire)`, which is `SetTriggerHeld` |

My earlier literal scans for `+BC9h` found only the latch because this reader addresses it as
`[ESI+8B9h]` from `unit+310h`. The claim in `docs/PLANE_GUNFIRE.md` section 2 that no weapon
reader exists is **wrong**; a dated correction is appended there.

**From the trigger to a round**, already read in `docs/GUN_PLATFORM_ARC.md` and
`docs/GUN_SHOT_CADENCE.md`, and **already modelled by the gunnery host** for bot-driven guns:
1. `SetTriggerHeld` latches `gun+454h`.
2. The fixed step `0072D130` sends one `0ADh` per step while it is latched.
3. `0072D860` calls `FireIfReady` `00727E30`, which asks `CanFire` (the `gun+478h`/`+450h`
   timers, ammunition and the barrel reload timers `+414h`).
4. `BSP_Gun_Fire` spawns the round.
There is no new spawn path to write. The host's gunnery sets `want_fire` from bot targeting
(`src/game_hosts_gunnery.cpp` around `gun_set_fire_request_0072d2c0`), and PLANEGUN guns have
no bot, so they never fire.

## 3. The binding

**`kPlaneGunfireBound` (true), the units side.**
* The dogfight gun controller's fire flag becomes the plane's latched gunFire each think
  (`task+2E0h` -> `plan+2DCh` -> `cmd+16h` -> `unit+9FAh`, forced 0 for a dead unit ->
  `unit+BC9h`).
* It is published as `GameUnitsHost::plane_gun_trigger_bc9(index)`.
* The census adds `trigger_ticks` and `trigger_rises` per fighter.

**The gunnery hook, left for the integrator** (`src/game_hosts_gunnery.cpp`, where `want_fire`
is formed, just before `gun_set_fire_request_0072d2c0`): for a gun with `gun.category == 0` on
a unit that is a plane,

```cpp
want_fire = units.plane_gun_trigger_bc9(gun.unit_index);   // 007CE9A0-007CE9F4
```

(`want_fire` is `const` there today.) The weapon-group enable byte is not modelled, so it would
be taken as enabled. Until that line lands, no fighter round is spawned and the pair below is
behaviour-neutral for the gun.

**`kDogfightEarlyEdgeBound` (true), `009AAA80`,** read from the listing:
* When `approach+DCh > 0` and `+E0h < 1` (the maneuver tail's 1.5/0.1, cleared to 0/1.0 by aim),
  it runs the shared finder with its own arguments: cone `+DCh`, inner `row+220h` (0.03), range
  `task+344h` (1500), near `row+20Ch` (300), threshold `+E0h`, the target squadron as filter,
  and noise `U(0, 0.05)` (`00CE7638`) at its midpoint.
* A target below the unit is taken. One above is taken when `atan(dy / max(1, h))` is below
  `interp(007C4830, 0, 007C47F0, 1.6, speed)`:
  * `007C4830` = `StallRangeMax` (`tuning+230h`, 1.6) × StallSpd;
  * `007C47F0` = `LevelFlight` (`tuning+24Ch`) × StallSpd;
  * 1.6 is `00D06BB4`;
  * both helpers take no arguments (plain `RET`), so the pushes at `009AAB39`-`009AAB64` are
    the interpolation's own.
* Taking it retargets through `009A7650` and switches maneuver to aim.
* The finder's cache and 2 s clock are shared with the gun, as the image's one object is.

## 5. Predictions, written before runs G0/G1/GT

G0 has both switches off. G1 has both on. GT is G0 with `kDogfightThrottleBound` on. All are
USN04 at the E2 parameters.

1. **Gun.** G1's trigger census shows one rise per burst. Expect the single burst (Yorktown
   .-3 at about 845 m, 5-6 fire ticks) or a few more if the early edge changes the chase.
   Rounds, hits and kills by fighters are 0, because the gunnery hook is not in.
2. **Early edge.** A few early edges per Yorktown fighter during maneuver: the maneuver ticks
   were 1, 1 and about 20-30, so few chances. Each ends maneuver sooner, so aim ticks rise
   slightly. Lexington fighters never engage, so they are unaffected.
3. **Other units.** Changes only through the fighters' positions: recon and AA, which is
   RNG-coupled. No dive-bomb row should change unless a fighter's path changes the recon
   picture.
4. **Stall (GT).** The Yorktown leader and `.-2` show head-on aim ticks with the throttle cut
   towards 0.3 before they drown. If they drown with no head-on ticks, the cut is not the
   cause, and the maneuver tail's throttle (1.0, direct mode) is the next suspect.

## 6. Runs

All runs are USN04 at the E2 parameters. Each ran from its own freshly copied binary. The first
G0/G1/GT set lacked the gunFire latch and census, and a second copy into the same directories
nested inside them, so those runs are superseded. Logs:

| run | early edge + gunFire | throttle wiring | log |
| --- | --- | --- | --- |
| H0 | off | off | `local\H0_9000.log` |
| H1 | on | off | `local\H1_9000.log` |
| HT | off | on | `local\HT_9000.log` |

**H1 against H0: neutral.**
* Every summary row, all 8 water contacts, the killed_by table, the 365 dive-bomb rows and every
  dogfight row are identical.
* Per Yorktown fighter (leader, `.-2`, `.-3`):

| fighter | trigger ticks | trigger rises | early-edge asks | early edges |
| --- | --- | --- | --- | --- |
| Yorktown-class01_sqn02 | 0 | 0 | 1 | 0 |
| Yorktown-class01_sqn02\|.-2 | 0 | 0 | 13 | 0 |
| Yorktown-class01_sqn02\|.-3 | 6 | 1 | 30 | 0 |

* The trigger mirrors the controller exactly: the one burst on `D3A Val #3.1|.-3` at about 845 m.
* The early edge was asked but never fired. No enemy aircraft of the target squadron scored
  above +E0h (0.1) inside the 1.5 cone at an expired finder clock.
* Rounds, hits and kills by fighters are 0, because the gunnery hook is not in.
* Prediction 2's "a few early edges" was wrong.

**The stall (HT).** The head-on census, with direct throttle on:

| fighter | head-on aim ticks | throttle minimum | outcome |
| --- | --- | --- | --- |
| Yorktown-class01_sqn02 | 213 | 0.463 | leaves aim at 2155.2 m, then drowns at \|v\| 50.94 |
| Yorktown-class01_sqn02\|.-2 | 197 | 0.458 | leaves aim at 2157.9 m, then drowns at \|v\| 51.18 |
| Yorktown-class01_sqn02\|.-3 | 181 | 0.390 | survives; it also flew 20 maneuver ticks at throttle 1.0 |

* H0 (throttle off) shows the same head-on exposure (195/183/169 ticks, minimum about 0.44). There
  the stand-in keeps speed mode 1, and nobody drowns.
* **Verdict: consistent with the throttle cut, not proven its only cause.** About 200 aim ticks
  (10 s) of direct throttle at 0.46-1.0 precede both drownings, and the fighter with the most
  full-throttle maneuver time survives.
* **But the head-on count is itself suspect.** About half the aim ticks are flagged head-on while
  chasing from behind. That points at the substitution of the forward rows for the two units'
  `vtable[34h]` vectors in the aim tick. That read comes first.
* `kDogfightThrottleBound` stays off.

## 7. Decision

**Lands** (`kPlaneGunfireBound = true`, `kDogfightEarlyEdgeBound = true`): the pair is neutral.
* The fire path is read to the gun's trigger. The units side publishes the latched gunFire.
* **The one-line gunnery hook in section 3 is left for the integrator.** With it, fighter rounds
  go through the host's existing FireIfReady / `BSP_Gun_Fire` path, and the next pair measures
  bursts, rounds, hits and kills.
* **Not read:** `unit->vtable[1FCh](gunFire)` at `007CE98D`, the weapon-group enable byte's
  producer, and `007B9740` (009AAA80's "no" arm).
