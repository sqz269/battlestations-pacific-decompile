# The torpedo aim: lead, spread and gyro heading (packet `cc9_torpedo_spread_aim`)

Worker cc9-gunnery4, on main `6de76b925` (reference h). Ghidra was only read, and the slot bodies
were read from the disk bytes. Descriptive names are hypotheses.

## 1. Answer

**The host aims the torpedo the way the image does, term by term. Nothing is bound.**
- USN02's opening spreads come out of the image's own solution on the logged inputs, to 0.01
  degree once the run-line offset is included (section 4).
- The early failure of reference h is therefore closed **as the image's own for an idle player**.
  Houston is sunk at 20.85 s by a Yamakaze Long Lance, and the mission fails at 29.75 s.

## 2. The image, from the fire target to the gyro heading

`008FFF20`, the TorpedoBot tick (GUN_BOT_TICKS 6.5), in order:

```
00900120: call 0x427eb0          ; gun point = BSP_EntityPose_GetWorldPositionRefreshed(gun) = gun+FCh
0090014E: call 0x42d7e0          ; the target's world matrix; +30h..+38h, its origin
009001A8: call 0x42b2f0          ; the range; 009001E6 00415510 caps it, JA 009003A6 aborts beyond it
00900200: call [eax+44h] ; call [edx+34h]   ; the fire target's velocity
0090024D: xorps / movss [esp+58h]           ;   with its vertical zeroed
0090022B: fld [eax+0E4h]                    ; the speed argument: WaterTravelSpeed
0090025E: call 0x8fbb00                     ; the intercept (008FB8D0's quadratic, GUN_BOT_REMAINDER 2)
00900280: fld [edi+6D4h] ; fmul [eax+20h] / [eax+28h]   ; + unit+6D4h x the target's forward (the spread)
009002C3..009002E7                          ; lead - gun, horizontal, y = 0
00900311: call 0x415510 ; ja 0x900974       ; the lead is beyond range: abandon
00900328: call 0x42b260                     ; normalise
0090032F: call 0x414e10 (ECX = [gun+3Ch])   ; the OWNING UNIT's derived affine inverse
0090033F: call 0x42d0d0                     ; into the hull frame
0090035A: call 0x521370 ; 00900366 fchs     ; the angle pair, horizontal negated
00900380: call 0x85ab50 (limit [00E0B588] = pi/4)   ; snapped onto the firing window: the tube heading
```

Then the gyro heading:
- **The run line.** `00900476..00900583` runs from `gun + 2 x snap_degrees x tube axis` through
  the lead, 1000 m long. Its world heading is taken at `0090050B..00900526`.
- **The jitter.** `00900830..00900876` adds the TorpedoBot AngleErr, `U(min, max)` degrees with a
  random sign. SPNormal gives 0..10 degrees.
- **The install.** `007311B0` carries the result, and `00856637` installs it as `record+46Ch`
  (USN02_SAMESIDE_TORPEDOES 3).

**The spread.** After each launch `00951FC0` flips `unit+6D4h` through 0, -35, +35, -70, +70,
-105 m and back to 0 (TORPEDO_SPREAD 1). Each 0.2 s bot tick issues at most one command, so a
ship's tubes take consecutive offsets along the target's track, one per launch.

**What the image assumes of the target.** It takes the target's true present velocity from
`vtable[34h]`: no estimate, no delay and no deliberate aim-off beyond the spread and the jitter.

## 3. The host, term by term

| term | image | host (`src/game_hosts_gunnery.cpp`) | same? |
| --- | --- | --- | --- |
| gun point | `gun+FCh`, the mount's world position | `gun_muzzle_point`: the mount (`kShipPlatformAttachmentBound`) | yes |
| target point | the target's matrix origin | `unit_aim_point`: the origin, raised by Height in y only | yes: y does not reach the horizontal heading |
| target velocity | `vtable[34h]`, y zeroed | `unit_velocity`, y kept | yes for a ship (y is about 0) |
| solver speed | `WaterTravelSpeed` (`+0E4h`) | `gun.water_travel_speed` | yes |
| intercept | `008FBB00` / `008FB8D0` | `torpedo_intercept_point_008fbb00` (`src/gun_bot_remainder.cpp`) | yes |
| spread | `unit+6D4h x` target forward, flipped per launch | `kTorpedoSpreadBound`, the same flip | yes |
| frame | the owning unit's (`[gun+3Ch]`) | the hull's right and forward | yes |
| window snap | `0085AB50`, pi/4 | `gun_snap_heading_to_fire_window_007f6190`, pi/4 | yes |
| run line | from `gun + 2 x snap_deg x axis` | `torpedo_run_end_008fff20`, the same | yes |
| jitter | AngleErr by skill | `kTorpedoAngleErr` by skill | yes |
| gyro install | `007311B0` -> `+46Ch` | `kTorpedoGyroHeadingBound` | yes |

## 4. The three opening spreads, recomputed

- **The run.** `local\SA_usn02.log` is this tree at `6de76b925` with the new diagnostic line
  `gunnery: torpedo aim` (under `BSP_TORPEDO_TUBE_TRACE=1`). It is gameplay-identical to reference
  h's `local\rb8_usn02.log` (`pair_diff` exit 1, the 10 death rows identical).
- **The recompute.** `local\g4_aim_check.py` re-implements `008FB8D0` / `008FBB00` from
  GUN_BOT_REMAINDER 2's coefficient table. It runs on each launch's logged gun point, target
  origin and velocity, and `WaterTravelSpeed`.

**The first launch of each ship** (WaterTravelSpeed 170.444, the targets at 12.0 m/s):

| t | shooter | tube | target | range | intercept time | bare intercept heading | host run line | jitter | tube heading | gyro heading |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1.50 s | Minegumo | 10 | Houston | 3162 m | 18.3 s | -114.34 | -113.96 | 0.00 | -138.6 | -114.0 |
| 1.55 s | Yamakaze | 12 | Alden | 3046 m | 17.5 s | -127.45 | -127.37 | +9.02 | -138.7 | -118.3 |
| 3.20 s | Tokitsukaze | 10 | Exeter | 3175 m | 18.4 s | 113.77 | 113.04 | -5.38 | 147.9 | 107.7 |

- **The spread offsets repeat the image's pattern** on every ship: 0, -35, +35, -70, +70, -105,
  then 0.
- **The difference between the bare intercept heading and the host's run line** (0.1 to 0.7 degree)
  is the image's own run-line origin. That origin is the gun moved 2 m per snapped degree along the
  tube axis. The tubes snap 11 to 34 degrees at these launches.
- **The gyro heading is the run line plus the logged jitter**, to the 0.1 degree the line prints.
- **The tube heading is the snapped one**, 20 to 40 degrees off the gyro line. The round turns onto
  the gyro line as TORPEDO_FRIENDLY_CROSSING 6 describes.

**Why a Long Lance aimed at one ship sinks another.**
- Yamakaze's spread is aimed at **Alden**, 3.0 km away, and the round that sinks Houston at 20.85 s
  is from that spread.
- The spread and the jitter widen the fan by up to 105 m and 10 degrees. Houston, beside Alden,
  lies across it.

**Why the Long Lances arrive a little behind the intercept.**
- The solver assumes a constant `WaterTravelSpeed`. The image's round starts at `V0` (13 m/s) and
  approaches that speed with a 1/0.6 s time constant (TORPEDO_FRIENDLY_CROSSING 6.1), so it is
  about 1.6 s late, roughly 20 m behind a 12 m/s target.
- That is the image's own solver against its own swim.

## 5. Verdict

The aim is the image's. **The USN02 failure at 29.75 s stands as the image's own for an idle
player**, and this closes reference h's "USN02's outcome hinges on the opening torpedo spread"
flag. Only the diagnostic line was added to the host, and it is gameplay-neutral.
