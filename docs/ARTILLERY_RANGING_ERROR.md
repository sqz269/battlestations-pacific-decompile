# Artillery ranging error: the per-target engagement record

Addresses: 00864880 00862CD0 00862660 00864CA0 00865078 006DF520 006DF66A 006DF694 006DF800
006DF9A5 0042AC60 00E19994 00CE3D9C 00CE7630 008FCF30

Packet `cc9_artillery_ranging_error`, Ghidra read-only. Every descriptive name is a hypothesis, not a
recovered symbol. `docs/UNIT_GUNNERY_PASS.md` (step 6, the `+B0h` list and `00864CA0`),
`docs/GUN_BOT_TICKS.md` (`006DF520`) and `include/bsp/robot_config.hpp` (the
ArtillerySubDirectorBot layout, reader `008FCF30`) are cited, not restated. The field meanings
follow the Hungarian comments above `Robots["ArtillerySubDirectorBot"]` in this installation's
`scripts/datatables/robots.lua`.

## 1. The record

The unit's `+B0h` list holds one 8Ch-byte record per target. `00864CA0` finds it through
`00863B10`, or builds and inserts one with `00864880` / `00864160`, every time the pass hands a gun
to a target. It always ends with `00862660(rec)(gun)`.

| field | meaning | writer |
| --- | --- | --- |
| `+14h` | the owner's skill row, `unit+390h` | `00864880`, `00862CD0` |
| `+18h` / `+1Ch` | owner / target, observer-paired | `00864880` |
| `+20h..+5Fh` | the target's world matrix at the last update; `+50h..+58h` its translation | `00862CD0` tail |
| `+60h..+68h` | the target's extrapolated position | `00862CD0` tail |
| `+6Ch` | the error bearing | `00864880`, `00862CD0` |
| `+70h` | the error length | same |
| `+74h..+7Ch` | the offset, `(0, 0, +70h)` rotated about the up axis by `+6Ch` | same |
| `+80h` / `+84h` | the clock at the last update / at the last update that followed a shot | same |
| `+88h` | "a gun fired since the last update" | `00862660`, cleared by `00862CD0` |

## 2. The rules

The row is `[00E19994] + 28h * level` (robots.lua ArtillerySubDirectorBot). With `dist` = owner to
target and `frac = clamp(dist / unit+490h, 0, 1)`, where `unit+490h` is the artillery max range:

* **Build, `00864880`:** `err0 = min(MaxErrorRadius, ErrorRangeMul * dist)`,
  `+70h = U(err0 * MinErrorMul, err0)`, `+6Ch = U(0, 2pi)` (`00CE3D9C`), both clocks set to now.
  The extrapolated point is the target position with `y = 0`.
* **Update, `00862CD0`,** at step 6 of every pass (`00865078..0086513C`). A record whose target
  or owner was released is deleted instead.
  * `+88h` set: `+84h = now`,
    `+70h = +70h * (ApproachMulMin + (ApproachMulMax - ApproachMulMin) * frac) + |+60h - target| * DeviationMul * frac^2`,
    and `+88h` is cleared.
  * `+88h` clear and `now - +84h > ErrorDistIncStartTime`:
    `+70h += MaxErrorRadius * frac * (now - +80h) / ErrorDistIncFullTime`.
  * Otherwise the error, bearing and offset stay as they are.
  * When either of the first two ran: cap `+70h` at `min(MaxErrorRadius, ErrorRangeMul * dist)`,
    then `+6Ch += U(-AngleChange, AngleChange)`, wrapped, and rebuild the offset.
  * Always: the target's horizontal motion since `+50h` is carried from the old matrix into the
    current one and added to the target position (`+60h`). The matrix, translation and `+80h`
    are refreshed.
* **Hand-off, `00862660`:** `gun+398h`'s bot `+84h..+8Ch = rec+74h..+7Ch`. When `+88h` is clear
  and `+84h < gun+474h` (the gun's last shot), `+88h = 1`.
* **Aim, `006DF520`:** `bot+90h` walks toward `bot+84h` by `dt * 30` per axis
  (`006DF66A..006DF6D2`, `0042AC60`, `00CE7630`). `006DF800..006DF81F` adds it to the world aim
  point. `006DF9A5..006DF9CD` zeroes it when the target is not a ship.

The rows, by skill (Lua `SKILL_*` in `global/luamw_init.lua`; `SetSkillLevel` stores the number):

| row | MaxErrorRadius | ErrorRangeMul | MinErrorMul | ApproachMul min..max | DeviationMul | AngleChange | StartTime / FullTime |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 Stun | 750 | 0.50 | 0.75 | 0.75 | 10.0 | 20 deg | 5 / 30 |
| 1 SPNormal | 40 | 0.20 | 0.50 | 0.15..0.25 | 0.80 | 15 deg | 10 / 40 |
| 2 SPVeteran | 0 | 0 | 0 | 0 | 0 | 10 deg | 60 / 120 |
| 3 MPNormal | 25 | 0.10 | 0.15 | 0.15..0.20 | 0.75 | 10 deg | 20 / 90 |
| 4 MPVeteran | 15 | 0.10 | 0.10 | 0.10..0.175 | 0.70 | 10 deg | 40 / 100 |
| 5 Elite | 0 | 0 | 0 | 0 | 0 | 10 deg | 60 / 120 |

Labelled: the host takes the level from `GameUnitsHost::skill_level` (the slot's
`unit+390h`), the extrapolation uses the pose rows, and `+50h` at build time is the target position.

## 3. Switch

`kArtilleryRangingErrorBound` in `src/game_hosts_gunnery.cpp`. Draws use the keyed stream
`Draw::ranging` (key = owner, target).

## 4. Predictions (written before any run)

USN02 is this installation's `usn/usn_2_java.lua`. It sets the DeRuyter group (DeRuyter, Java,
Kortenaer, Electra) to `SKILL_STUN`, the Houston and Exeter groups to `Mission.SkillLevelOwn`
(SPVeteran, row 2) and the Japanese to `Mission.SkillLevel` (SPNormal, row 1) at difficulty 1.
The log shows 4 units at 0, 10 at 2 and 12 at 1.

1. **Houston and Exeter groups (SPVeteran):** MaxErrorRadius 0, so no offset. Their hit counts
   move only through what the battle does to them.
2. **DeRuyter group (Stun):** at 5 to 10 km the cap is 750 m. The first error is 560 to 750 m, each
   shot update keeps 75% of it and adds 10 * frac^2 * the target's prediction miss. These four
   ships stop hitting ships almost entirely: their shell hits fall by 80% or more.
3. **Japanese (SPNormal):** the cap is 40 m. The first error is 20 to 40 m, and each shot update
   keeps 15 to 25% of it plus 0.8 * frac^2 * the prediction miss. Each new engagement opens with
   one to three missed salvos, then converges to a few metres. Japanese shell hits on Allied ships
   fall 10 to 30%.
4. **USN02 9000 frames:** fewer ship deaths or later ones on both sides. The Allied losses that
   came from Japanese gunfire come later, and the Japanese losses the DeRuyter group caused
   disappear or move to torpedoes. The failure at 44.60 s does not move.
5. **USN04:** no artillery engages a ship inside 4500 frames, so the run is identical.

## 5. Results

Pending.
