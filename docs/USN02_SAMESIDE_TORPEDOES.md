# USN02's same-side torpedo kills, and 00927F10 (packet `cc9_usn02_sameside_torpedoes`)

## 1. The kills on current main

One USN02 9200/9000 run of main `c7a0d886e` with the per-launch diagnostic, RNG streams and the
death table on: `local/stB_usn02.log`, module directory `local\stB`, `window resolution override
fit: 2560x1440 -> 1600x900`. Three same-side torpedo kills, all IJN on IJN. The Tokitsukaze-on-
Jintsu kill of earlier trees is gone; Harusame now kills Jintsu instead.

| victim | time | shooter (platform) | range at death |
| --- | --- | --- | --- |
| Amatsukaze | 127.95 s | Hatsukaze (11) | 317 m |
| Jintsu | 128.45 s | Harusame (11) | 442 m |
| Haguro | 131.35 s | Harusame (12) | 795 m |

## 2. Where the victims were at launch and at impact

`torpedo launch friends` gives every same-side ship within 8 km, along and across the gate's
run line (from the gun towards the lead point). `friendly torpedo hit` gives the impact point.

**Harusame, 121.50..125.10 s, aimed at Houston.** Every launch shows
`friendly_in_2km=4 crossed=0`. At the 125.10 s launch:
- Jintsu is 348 m along the gate's line and 286 m to one side of it, moving across the line at
  1.5 m/s.
- Haguro is 529 m along and 633 m to the same side, at 2.2 m/s.
- Their heading lines run almost parallel to the gate's line, so neither crosses its first 1000 m.

The torpedoes did not run down that line:
- The Jintsu hit (128.45 s) lies 375 m, -104 m from Harusame. That is 47 degrees off the gate's
  line.
- The Haguro hit (131.35 s) lies 795 m, -215 m from Harusame, also 47 degrees off.

**Hatsukaze, 123.15..124.70 s, aimed at Encounter.** `friendly_in_2km=2 crossed=0`. At every
launch Amatsukaze is 179..206 m along the gate's line and 253..262 m to its side, 317..326 m
away. That is 53 degrees off the line, and its heading line does not cross the first 1000 m.
The hit (127.95 s) lies 54 degrees off the gate's line from Hatsukaze, on Amatsukaze's bearing.

## 3. Why: the host launches along the snapped tube heading, the image along the gate's line

The first read stopped at the command object. This read follows it to the torpedo.

**The gate's line and the launch heading are the same line in the image.**
- `0090050B`..`00900526`: `atan2(x, z)` of the gate's normalised run direction, the world heading
  of the line `009006EE` tests, stored at `[S+24h]`.
- `00900830`..`00900876`: the TorpedoBot jitter `U(AngleErrMin, AngleErrMax)` degrees, with a
  sign from bit 0 of `00BD2FC0` (`AND AL,1` at `00900843`), converted with pi/180 and added to `[S+24h]`.
- `00900911`..`00900927`: `0072C970(gun)` ends in a plain `RET`, so its pushed argument stays on
  the stack as `007311B0`'s third parameter, which `RET 0Ch` cleans. That makes `007311B0`'s
  first float `[S+24h]`, **the jittered run-line heading**, not `bot+60h`. Its second is
  `0072C970`'s run value, the distance to the target divided by WaterTravelSpeed through
  `00852410`.
- `0072AC20` parks the command at `gun+41Ch`. The fire passes `gun[+41Ch]` to the class factory
  (`0072C006`).
- `BSP_TorpedoClass_CreateProjectile` then copies `[cmd+4]` into the torpedo's `record+46Ch`
  (`00856637`..`0085663A`). With no command it stores the `10000.0` sentinel (`00856614`).
- The command's property publisher (`00731270`) names the two fields `heading` (`00CFE758`) and
  `swimdepth` (`00CFE74C`).
- The steer (`00857061`) turns the swimming torpedo toward `record+46Ch` at `HeadingTurn`
  degrees per second (`classDesc+0E8h`, 10 in every row of this installation's
  `bulletclasses.lua`). `00857480` seeds `+46Ch` from the torpedo's own yaw only while it still
  holds the sentinel (`008574A0`).

**The snapped heading only trains the tube.** `bot+60h`, snapped by `0085AB50` up to π/4
(`00900380`), goes to `0085ABA0` and the one-degree fire test. The torpedo leaves the tube
along it, then turns onto the gate's line at 10 degrees per second, about 3 to 5 s for a 30 to
45 degree snap.

**The host** launched along the gun's current angles, the snapped tube heading, and swam
straight: no commanded heading and no turn. So its torpedoes ran up to 45 degrees off the only
line the gate had cleared. That is exactly the three kills above: 47, 47 and 54 degrees.

Also against the listing, none of which is a missed term:

| question | answer | evidence |
| --- | --- | --- |
| Does a run-length or speed term extend the look-ahead? | No. The run is the fixed `1000.0` (`00CE47A0`) and the friendly radius the fixed `4000000` squared (`00D09FE8`). Speed only converts the crossing distance to a time. | `00900537`, `00900607`, `00900742` |
| Is there a second pass at spread time? | No. `00951FC0` only flips the `unit+6D4h` spread offset after the launch. | `00900951` |
| Per tube or per salvo? | Per bot tick: once per `0.2 s` think (`00CE54A0`), before the one command it issues. | `0090003B`..`00900100`, `009008F3` |
| Does the torpedo re-test friendlies in flight? | No. The steer `00856BB0` scans `[slot+0DE8h]`, the **enemy** list, for homing. | `docs/TORPEDO_TICK.md`, `00856BFC` |

**Outcome (a).** `kTorpedoGyroHeadingBound` gives each bot-launched torpedo the command heading:
the run-line heading plus the TorpedoBot jitter (per level: Stun 10..20, SPNormal 0..10,
SPVeteran 0..0.5, MPNormal 0..6, MPVeteran 0..3, Elite 0..0.5 degrees). While it swims, it turns
toward that heading at `HeadingTurn`.

Substitutions:
- The turn runs only while swimming.
- `swimdepth` is not modelled.
- The rotation is toward the commanded heading. `0085E880`'s own sense was not read.

## 3a. Predictions, written before the gyro pair

USN02 9200/9000, `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`, OFF `local\gyO` against
ON `local\gyT`, one tree, only `kTorpedoGyroHeadingBound` differs.

- **Launches.** Gyro launches equal the ship torpedo shots, about 300. The mean launch offset
  between the tube and the commanded heading is 10 to 35 degrees.
- **Same-side kills.** 3 on OFF, 0 or 1 on ON.
- **Torpedo hits on enemies** (category 7) rise from 8 to between 8 and 25, because the
  torpedoes now run at the lead point.
- **Headline.** Deaths within 5 of the OFF count. Houston and Exeter are predicted to survive,
  but they are the rows most at risk, because IJN torpedoes now run at their lead point.
- **Other rows.** Every non-torpedo row moves only by cascade from the torpedoes.
## 4. 00927F10, the AI-held test

`00927F10` (`BSP_PartySlot_IsAiHeld`) is `__stdcall(slot)`, `RET 4`:
`return byte [[[00E188A8] + 18CCh + slot*4] + 9]`. `00521E70` answers "AI-held" for a role
slot of 8 or for a slot this byte marks AI. The host had taken every non-8 slot as player-held.

`kPartySlotAiHeldBound` now answers from the party slot record the ship AI host already holds
(`SessionParticipantPools::try_ai_held_00927f10`). The gunnery host reaches it through
`GameShipAiHost::session_participants()`. An unavailable record keeps the old answer. It feeds:
- the bullet-throw role gates (`role_ai_held_00521e70`);
- the player gun seat's three tests: the fire gate, the out-of-window return and the hand-over
  (`slot_ai_held_00927f10`).

**Prediction, written before the pair** (E2 9200/9000, OFF `local\ahO` against ON
`local\ahT`): every row identical. The only non-8 slot in these missions is the idle player's,
and the player's record should answer human. If instead it answers AI, only Lexington's guns
move: the seat never hands over, and Lexington's AA and flak take the level-2 multiplier (0).

## 5. The pairs, and the decisions

### 5.1 The gyro pair (USN02)

`local/gyO_usn02.log` against `local/gyT_usn02.log`. Both are built from this tree with
`kPartySlotAiHeldBound` ON, both show `window resolution override fit: 2560x1440 -> 1600x900`,
and both exit 0.

| row | OFF | ON |
| --- | --- | --- |
| gyro launches / mean tube-to-command offset | 0 / - | 268 / 28.4 deg |
| same-side torpedo hits (`friendly torpedo hit` lines) | 8, three kills | **0** |
| torpedo (category 7) shots / hits | 305 / 8 | 268 / 20 |
| queued hits / total damage | 603 / 55441 | 757 / 80493 |
| deaths | 16 | 21 |
| mission end | none | **failed at 44.60 s** ("Game Over", entity Jupiter) |

Torpedo kills on ON, with the range at death:

| victim | time | shooter | range |
| --- | --- | --- | --- |
| DeRuyter | 30.40 s | Jintsu | 2436 m |
| Alden | 34.50 s | Minegumo | 2884 m |
| Exeter | 43.45 s | Hatsukaze | 4124 m |
| Perth | 46.65 s | Hatsukaze | 4264 m |
| Jupiter | 140.80 s | Kawakaze | 1724 m |
| Java | 151.40 s | Haguro | 326 m |
| Witte | 159.81 s | Kawakaze | 1540 m |
| John1 | 175.06 s | Tokitsukaze | 5203 m |
| John3 | 190.76 s | Haguro | 3755 m |
| Houston | 225.11 s | Jintsu | 1289 m |

Verdict per prediction:
- **Mean launch offset 10..35 deg: held** (28.4).
- **Same-side kills 0 or 1: held** (0).
- **Torpedo hits 8..25: held** (20).
- **Deaths within 5: held, at the edge** (16 to 21).
- **Houston and Exeter survive: failed.** Exeter dies at 43.45 s to Hatsukaze's opening salvo.
  With Perth, that ends the mission as failed at 44.60 s. Houston dies at 225.11 s.

**Why so strong.** The IJN destroyers' opening salvos, launched in the first seconds at 2.4 to
4.3 km, now steer onto their lead points instead of running straight down a window edge. The
arcade `bulletclasses.lua` this host loads authors that torpedo class with `WaterTravelSpeed`
170.444, which gives a swim of about 102 m/s. The realistic table's fastest class is 51.444.
Which table the image selects for this mission's settings was not read here. It sets how fast
these salvos arrive, not whether they steer.

**Decision: ON.** The steering is read from the listing end to end: the command heading at
`00900927`, its install at `00856637`, and the turn at `00857061`. It removes every same-side
kill. It also changes the USN02 reference outcome from "no end" to a failed mission at 44.6 s,
so the integrator should re-baseline USN02 on it. The table selection above is the open
question that decides how hard these salvos land.

### 5.2 The 00927F10 pair (E2)
E2 9200/9000, `local/ahO_e2.log` against `local/ahT_e2.log`, both `window resolution override
fit: 2560x1440 -> 1600x900`, both exit code 0. **Identical in every row:** no summary line, no
death row (38 on both sides) and no gun row differs (0 of 834). The prediction held. For the idle player's slot, the record either answers human or is not
available (the fallback). The run does not tell the two apart, because the answer is the same
as the substitution's.

**Decision: ON.** The switch retires the "non-8 slot is player-held" substitution in the throw
and seat code with the image's test, and it moves nothing in E2. A multiplayer or
AI-substituted slot is where it would first matter.

The per-launch diagnostic lines (`torpedo launch friends`, `friendly torpedo hit`) stay in the
build. They print only for torpedo launches and same-side torpedo hits.