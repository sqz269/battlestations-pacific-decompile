# The torpedo aim tick's two kind probes

Addresses: `009D15F0` (BSP_BotStateTorpedoAim_Tick, the probes at `009D1714`, `009D175F`,
`009D176E`, `009D1E08`, `009D1E17`), `0074E400` (BSP_PlaneInstance_IsKindOf, the plane's slot `5Ch`
body).

Packet `cc9_torpedo_kind`, 2026-09-22. It binds the two `vtable[5Ch]` probes in the torpedo aim
tick exactly, and measures each binding with its own confirming pair. The host binding is in
`src/game_hosts_units.cpp`, and the tick's reconstruction is `src/torpedo_aim_tick.cpp`.

## 1. The two probes, read from the listing

The tick's `ESI` is the task state, `[ESI+4]` is the approach, and `approach+4h` is the unit
(`docs/TORPEDO_AIM_TICK.md`). `approach+CCh` is the ordered target.

**The target probe, `009D16FE`-`009D174C`.**

```
009D16FE MOV EAX,[ESI+4]          ; approach
009D1701 MOV EDI,[EAX+0CCh]       ; the ordered target
009D1707 TEST EDI,EDI / JZ 009D1750
009D170D MOV EAX,[EDX+5Ch]        ; target's own slot 5Ch
009D1710 PUSH 6 / 009D1714 CALL EAX
009D1716 TEST AL,AL / JZ 009D1750
009D171C ... 009D1721 CALL [vtable+50h]  ; the target heading, into the aspect
```

The aspect `F=2Ch` is written only when the target exists and answers kind 6, the ship base.

**The own-unit probe, first use: the 0.9 tighten, `009D1750`-`009D178A`.**

```
009D1750 MOV EDX,[ESI+4]          ; approach
009D1753 MOV EDI,[EDX+4]          ; the aircraft itself
009D175B PUSH 10h / 009D175F CALL [vtable+5Ch]
009D1763 JNZ 009D1774
009D176A PUSH 16h / 009D176E CALL [vtable+5Ch]
009D1772 JZ 009D178E
009D1774 FLD  float [ESP+14h]     ; F=14h, the range
009D1778 FLD  double [00D7A390]   ; 3FECCCCCC0000000 = 0.9f widened to double
009D177E FMUL ST1                 ; DC C9, both stack slots scaled
009D1782 FSTP float [ESP+14h]     ; range *= 0.9
009D1786 FMUL float [ESP+0Ch]
009D178A FSTP float [ESP+0Ch]     ; F=0Ch, the time to target, *= 0.9
```

**The own-unit probe, second use: the pitch selector, `009D1DE6`-`009D1E2F`.** At `009D1DBE`
`ECX = [ESI+4]` and `009D1DC1` reloads `EDI = [ECX+4]`, the same aircraft. When the bank at
`unit+C64h` is negative, `009D1E04 PUSH 10h / 009D1E08 CALL` and `009D1E13 PUSH 16h / 009D1E17
CALL` choose between `[00CF87C8]` = 2.5 and `[00CE380C]` = 1.5 for the pitch denominator. EDI
is rewritten at `009D1940` (to the approach), `009D1BD8` and `009D1D41` before that, so the first
use's EDI does not survive. The reload at `009D1DC1` is `[[ESI+4]+4]` again, and nothing writes EDI
between it and `009D1E17`. So both uses probe the aircraft.

`src/torpedo_aim_tick.cpp:137-143` and `:269-274` transcribe both uses correctly. Before this
packet the host answered `false` to the own-unit probe, so neither use ever fired.

## 2. Which routine answers

Slot `5Ch` dispatches through the probed object's own vtable. For a plane it is `0074E400`
(vtable `00D05F20`). Every other class has its own compiled body. `bsp::unit_is_kind_of(class_id,
literal)` (`src/unit_kind_query.cpp:278`) models all 88 bodies from their decoded compare runs and
answers from the object's own class id. So it is the right model for either probe, whatever class
the probed object is. `src/game_hosts_units.cpp` already uses it for these same two literals on
the same aircraft, for the bank rate cap (`0047B880` and `0099D1C2`).

## 3. Step 1: the target probe

Before, the probe was `aim_target() != nullptr`, a labelled substitution. After:

```cpp
bool target_is_kind_vtable5c(int query) override {
    const GameUnitSlot* const t = aim_target();
    return t != nullptr && bsp::unit_is_kind_of(t->class_id, query);
}
```

**Prediction:** no simulation line moves in either mission, because every ordered target in the
controls is a ship. USN01 targets Northampton, Dunlap and SaltLakeCity. USN04 targets
Yorktown-class01 and Lexington-class01.

MEASUREMENT: see section 6.

## 4. Which classes are kinds 10h and 16h

`src/entity_class_ids.cpp` gives class `10h` MPlaneBomber (parent `0Fh`) and `16h`
MLargeReconPlane (parent `14h`). No class in the table has either one in its ancestry, so the
probe answers true for exactly those two classes. The host's `class_id` is the
`VehicleClassKind` of the authored `VehicleClass.Type` (`include/bsp/vehicle_class.hpp`):
`LevelBomber = 10h`, `TorpedoBomber = 11h`, `DiveBomber = 12h`, `LargeReconPlane = 16h`.

The aircraft that run the torpedo aim tick in the two reference missions:

| mission | aircraft | `VehicleClass` | `Type` in this installation | creator / vtable in the log | kind 10h or 16h |
| --- | --- | --- | --- | --- | --- |
| USN01 | Mav1-Mav5 | 174, H6K Mavis | `LargeReconPlane` | `0074E540` / `00D00308` | **yes, 16h** |
| USN04 | B5N Kate #2.1, #4.1, #6.1, #8.1 and wings, 12 | 162, B5N Kate | `TorpedoBomber` | `009564E0` | no, 11h |

The `Type` lines are from this installation's `scripts/datatables/autoload/vehicleclasses.lua`,
read only. That file was modified locally on 2026-05-09, so these are this installation's
values, not necessarily the retail ones. The creator `0074E540` and vtable `00D00308` are the ones
`src/unit_instance_layout.cpp` and `src/unit_kind_query.cpp` record for LargeReconPlane.
`summary mission torpedo task` reports `aircraft=5` for USN01 and `aircraft=12` for USN04 in the
controls, which matches the table.

## 5. Step 2 predictions, written before the step 2 build

The binding is `return bsp::unit_is_kind_of(s_.class_id, query);`.

- **USN04: nothing moves.** All twelve torpedo aircraft are kind 11h, and the probe answers false
  exactly as the stub did. Step 2 must reproduce step 1 on every simulation line.
- **USN01: the five Mavs move, and only through their own torpedo tick.** Three things change for
  each Mav on every aim tick: the range `F=14h` and the time `F=0Ch` are scaled by 0.9, and a
  banked Mav uses 2.5 instead of 1.5 in the pitch denominator.
- **The release range grows by about 1/0.9, not 0.9.** The release flag at `009D2052` opens when
  the envelope exceeds `F=14h`. With the range scaled, it opens when `0.9 * R` falls below the
  envelope, that is at a true `R` of envelope / 0.9. The controls release at 433.0-437.8 m, so
  the prediction is roughly 481-487 m for each Mav. The prediction holds if that clause is the
  last of the five to open and the release speed is about the same. Other clauses also read the
  scaled time, including the altitude gate at `009D20B4` and the cone, so a Mav more than a few
  metres off envelope / 0.9 is a finding to explain, not noise.
- **What follows from the Mavs, and only from them:** their torpedo runs, closest approaches and
  hits, and through those the damage to their targets. A change to any unit's behaviour that
  does not trace back to a Mav round is a finding.

MEASUREMENT: see section 6.

## 6. Measurements

All runs are from this worktree's root through `./tools/run_game.ps1 -WaitSeconds 2400`, with
`--press-start-frame 30 --mission-frame-seconds 0.05`. USN01 uses `--frames 3200 --menu-select
USN01 --mission-frames 3000`, and USN04 uses `--frames 5000 --menu-select USN04 --mission-frames
4800`. The controls come from this tree built unchanged from main `13dd6a438`. The predecessor's
`rel_usn01_aspect.log` and `rel_usn04_aspect.log` predate six merges and are history only. Logs are
in `local\`. "Simulation lines" means the whole log minus environment lines (module path, thread
ids, FMOD call count, message-pump count, harness slot lines) and the `ship avoidance search`
counter, which varies between identical runs (`docs/GUNNERY_HOST_LIFETIME.md` section 8).

### 6.1 The controls

| mission | round | ordered target | target moved | release range | abs cos aspect | t_cpa | miss |
| --- | --- | --- | --- | --- | --- | --- | --- |
| USN01 | Mav3 | Northampton | 0.0 m | 436.9 m | 0.478 | 11.40 s | 8.5 m |
| USN01 | Mav2 | Northampton | 0.0 m | 433.0 m | 0.520 | 11.30 s | 7.7 m |
| USN01 | Mav1 | Dunlap | 141.3 m | 434.6 m | 0.139 | 7.40 s | 210.3 m |
| USN01 | Mav5 | SaltLakeCity | 132.9 m | 437.8 m | 0.239 | 8.00 s | 182.6 m |
| USN01 | Mav4 | SaltLakeCity | 129.5 m | 433.1 m | 0.293 | 7.80 s | 175.2 m |

USN01 `torpedo_drop drops=5`, `total_damage=2914.7`, `deaths=1`. USN04 has 12 rounds from B5N Kates at
306.1-317.3 m (`|cos|` 0.956-0.989) and 436.9-439.1 m (`#8.1` wing, `|cos|` 0.31-0.36),
`drops=12`, `total_damage=16473.3`, `deaths=10`. Logs: `ctl_usn01.log`, `ctl_usn04.log`.

### 6.2 Step 1, the target probe: prediction met

| mission | control | step 1 | lines | simulation lines that differ |
| --- | --- | --- | --- | --- |
| USN01 | `ctl_usn01.log` | `kind_usn01_step1.log` | 22739 | 0 |
| USN04 | `ctl_usn04.log` | `kind_usn04_step1.log` | 51418 | 0 |

Both missions reproduce to the digit, so the bound probe is committed.

### 6.3 Step 2, the own-unit probe: prediction met on the units and the factor, missed on three release ranges

**USN04: met.** `kind_usn04_step2.log` matches `kind_usn04_step1.log` on all 51418 lines, with zero
simulation lines differing. No Kate is kind 10h or 16h.

**USN01, the factor: met exactly.** The first line that differs is Mav1's first aim tick. There
`F14` goes 2192.3 -> 1973.0 and `F0C` 3.9038 -> 3.5134, both exactly 0.9, and every other field on
the line is identical. At aim tick 151 each Mav carries `F14 = 0.9 * range_90` to four digits in
step 2 and `F14 = range_90` in step 1:

| Mav | step 1 `F14` / `range_90` | step 2 `F14` / `range_90` | ratio |
| --- | --- | --- | --- |
| Mav1 | 695.5 / 695.5 | 631.3 / 701.4 | 0.9000 |
| Mav2 | 799.4 / 799.4 | 725.4 / 806.0 | 0.9000 |
| Mav3 | 803.6 / 803.6 | 729.4 / 810.5 | 0.9000 |
| Mav4 | 684.3 / 684.3 | 620.7 / 689.7 | 0.9000 |
| Mav5 | 694.9 / 694.9 | 630.5 / 700.6 | 0.8999 |

**USN01, the release range: two met, three missed.**

| Mav | target | step 1 range | step 2 range | ratio | predicted 1/0.9 = 1.1111 | release alt, speed (step 1 -> 2) | timer first fire (aim tick) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Mav3 | Northampton, stationary | 436.9 | 485.7 | 1.1117 | met | 12.4 -> 17.5 m, 78.7 -> 81.6 m/s | 194 -> 188 |
| Mav2 | Northampton, stationary | 433.0 | 481.3 | 1.1115 | met | 12.4 -> 17.4 m, 78.7 -> 81.6 m/s | 194 -> 188 |
| Mav1 | Dunlap, moving | 434.6 | 469.7 | 1.0808 | **missed by 13 m** | 14.3 -> 25.4 m, 81.4 -> 84.2 m/s | 180 -> 176 |
| Mav5 | SaltLakeCity, moving | 437.8 | 473.8 | 1.0822 | **missed by 13 m** | 14.3 -> 25.5 m, 81.7 -> 84.5 m/s | 179 -> 175 |
| Mav4 | SaltLakeCity, moving | 433.1 | 460.5 | 1.0633 | **missed by 21 m** | 14.5 -> 25.6 m, 82.0 -> 84.6 m/s | 178 -> 175 |

The `release range` column is the drop census's centre-to-centre distance to the ordered target at
the drop. The `|cos|` aspect moves only on the three moving-target rounds: 0.139 -> 0.153,
0.239 -> 0.251 and 0.293 -> 0.301. The misses move 7.7 -> 8.8, 8.5 -> 8.5, 210.3 -> 217.0,
182.6 -> 188.6 and 175.2 -> 177.9 m.

**Where the prediction went wrong, as far as the logs show.** Section 5 named the release flag at
`009D2052` as the clause that sets the range. The aim-tick record says the drop came through the
timer path instead: `release_arm_009D2287=1`, `timer_fires=1`. That path reads the scaled time
`F0C`, not `F14`. Because `F0C` is `range_90` over the commanded speed (`009D1500`,
`docs/TORPEDO_AIM_TICK.md`), it is also proportional to `range_90`. So a gate on it still opens
at `range_90` / 0.9, and that is what the stationary target shows to four digits. For the three
moving targets, the drop census's centre-to-centre range is not `range_90` / 0.9. The logs do not
print `range_90` at the drop, so this packet cannot say whether `approach+90h` is measured to
something other than the target's centre for a moving target, or whether the timer's delay
between arm and fire closes a different distance. That is the open finding.

**Nothing else moved without a Mav cause.** The per-unit table has 9 differing rows of 26. The
five Mav rows differ, and so do the ships that shot at them or measure their distance to them:
Northampton, Dunlap, SaltLakeCity, Enterprise, Ralph, McCall and Blue, all in the
`nearest`/`shots`/`hits`/`dealt` columns. Northampton's `taken=2218` (the torpedo damage) is
unchanged. The one death that flips is **Mav2, which now survives**: SaltLakeCity's fire dealt it
434 instead of 450, leaving 16 hp. Hence `deaths` 1 -> 0 and `total_damage` 2914.7 -> 2892.0
(`queued_hits` 38 -> 35). `summary mission world` is identical.

## 7. Decisions

- **Step 1, the target probe: bound and committed.** The pair reproduces both missions to the
  digit.
- **Step 2, the own-unit probe: NOT committed, left as `return false`.** The packet's rule was to
  commit only if the pair matched the prediction written in section 5. It matched on the units
  (only the five LargeReconPlane Mavs, USN04 untouched) and on the factor (exactly 0.9 on `F14`
  and `F0C`). But three of five release ranges missed the stated 1/0.9 by 13-21 m, and section 5
  had said such a miss is a finding, not noise. The binding that was measured is:

  ```cpp
  bool unit_is_kind_vtable5c(int query) override {
      return bsp::unit_is_kind_of(s_.class_id, query);
  }
  ```

  It replaces `bool unit_is_kind_vtable5c(int) override { return false; }` in the torpedo aim
  binding of `src/game_hosts_units.cpp`, and it reproduces `kind_usn01_step2.log` and
  `kind_usn04_step2.log`. In this packet's reading the listing supports it: the probe object is
  the aircraft at both sites (section 1), and the measured factor and units are exactly the
  listing's. What is unexplained is the downstream release range against a moving target, which
  is a property of the timer path and the range source, not of the probe. Landing it is the
  integrator's call.
- **Open:** print `approach+90h` and the target's centre distance at the timer fire for the three
  moving-target Mavs, which would settle the moving-target release range. The pitch selector's
  2.5/1.5 at `009D1E1D`/`009D1E27` has no census line of its own, so its share of the change is
  not separated here.

## 8. Landed (packet `cc9_torpedo_kind_land`, 2026-09-22)

The integrator decided to land step 2, which supersedes the step 2 decision in section 7. The
probe fired for exactly the predicted units at exactly the image's factor. The miss was in this
doc's downstream release-range model. The binding is the one section 7 records.

**The pair.** On this tree at main `9b77ca812` plus the binding, USN01 with the control's exact
parameters (`local\kind_usn01_land.log`) reproduces `local\kind_usn01_step2.log` on all 22784
lines. No simulation line differs. USN04 is untouched, because no Kate is kind 10h or 16h (section
6.3 measured it identical). **The USN01 step 2 run is the reference from here on.**

**Measured reference values, not a fit.** These are the numbers the landed binding produces, as
section 6.3 read them.

| Mav | target | `F14` / `range_90` at aim tick 151 | `F0C` scaled with it | release range, step 1 -> landed | ratio |
| --- | --- | --- | --- | --- | --- |
| Mav1 | Dunlap, moving | 631.3 / 701.4 = 0.9000 | 1.2771 | 434.6 -> 469.7 m | 1.0808 |
| Mav2 | Northampton, stationary | 725.4 / 806.0 = 0.9000 | 1.4340 | 433.0 -> 481.3 m | 1.1115 |
| Mav3 | Northampton, stationary | 729.4 / 810.5 = 0.9000 | 1.4407 | 436.9 -> 485.7 m | 1.1117 |
| Mav4 | SaltLakeCity, moving | 620.7 / 689.7 = 0.9000 | 1.2595 | 433.1 -> 460.5 m | 1.0633 |
| Mav5 | SaltLakeCity, moving | 630.5 / 700.6 = 0.8999 | 1.2759 | 437.8 -> 473.8 m | 1.0822 |

USN01's landed summary: `torpedo_drop drops=5`, `queued_hits=35`, `total_damage=2892.0`,
`deaths=0`. Mav2 survives with 16 hp.
