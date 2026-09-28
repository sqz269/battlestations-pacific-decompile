# John1's torpedoes at Houston: the image's friendly-crossing gate lets them through (packet `cc9_torpedo_friendly_crossing`, a read)

Worker cc9-ships3. Ghidra was read-only. No code changed. Descriptive names are hypotheses.

## 1. Answer

**The image's gate passes this launch too, so nothing is bound.**
- `008FFF20`'s friendly scan (`0090058A..009007F6`) tests one straight line only. That line
  runs 1000 m from the gun toward the aim point. The test crosses it against each same-party
  ship's keel line, which is the ship's position ±1000 m along its own forward axis.
- It has no hull width, no hull length, and no allowance for the torpedo's turn out of the
  tube.
- Houston, 87 m along and 51 m across the run, steers slightly **away** from it. Her keel line
  therefore meets the run line about 764 m **behind** John1's gun, outside the run segment.
- `004F3730` rejects that crossing. The image answers "no crossing" and launches, exactly as
  the host logged `crossed=0`.

## 2. The image

The friendly scan, from the listing:

```
0090058A  CALL 008053C0 / MOV EBX,[EAX+0DDCh]   ; the own party's list
009005A8  [vt+5Ch](6) / JE skip                  ; ships only
009005BA  CMP EDI,[ESP+5Ch] / JE skip            ; not the shooter
00900607  CALL 00427E30 ; 0090060C FLD qword [00D09FE8] (4.0e6) ; JA skip   ; within 2000 m
00900630  FLD [EDI+94h] ... 00900647 FLD [EDI+9Ch]                ; the friendly's forward x, z
0090065E  FLD qword [00CE47A0] (1000.0) ; FMUL                     ; +-1000 m along it
009006EE  CALL 004F3730 ; TEST AL,AL ; JE skip                    ; segment against segment
00900730  CALL 00414C60                                          ; run distance to the crossing
00900742  FDIV [ESP+74h]                                         ; / water speed = time
00900754  CALL EDX (velocity) ... 009007C7 CALL 00414C60          ; the friendly's miss then
009007D4  FMUL qword [00CE3CA8] (300.0) ; FDIV qword [00CE47A0] (1000.0) ; FADD qword [00CE4D70] (200.0)
009007F2  FCOMIP ; JA -> hold                                    ; threshold > miss holds the launch
```

**`004F3730`** (`004F3730..004F3801`) accepts only when both segment parameters lie in [0, 1]
(docs/GUN_BOT_REMAINDER.md section 6). The run segment is the gun to `run_end`. `run_end` is
the gun plus 1000 m along the normalized lead direction, built at `00900476..00900583`. That
same line is the torpedo's gyro heading (`kTorpedoGyroHeadingBound`).

**A hold cancels this frame's fire** (`0090096D`, `vtable[1E8h](0)`). The bot retries on later
frames.

## 3. The host, term by term

| term | image | host (`src/gun_bot_remainder.cpp`, `src/game_hosts_gunnery.cpp`) |
| --- | --- | --- |
| candidate list | `[008053C0(party)+0DDCh]`, kind 6, not self | same side, alive, kind 6, not self |
| range gate | squared distance < 4.0e6 | same |
| friendly line | position ± 1000 × `[+94h]`/`[+9Ch]` forward | position ± 1000 × pose forward |
| run segment | gun to gun + 1000 × lead direction | `torpedo_run_end_008fff20`, same |
| crossing | `004F3730` | the same body, a naked-asm port (`src/avoid_zone_clearance.cpp`) |
| hold rule | miss < run × 0.3 + 200 | same |

The host's gate is the image's. It is not narrower and not shorter.

## 4. The launch, measured

The source is `local\g4_sy_plain_usn02.log` in cc9-gunnery4's tree, at `t = 273.71 s`.

| fact | value |
| --- | --- |
| run direction | (-0.052, 0.999) |
| Houston's heading | +0.455 deg (controlled frame 5470) |
| Houston's speed | 8.357 m/s |
| Houston's offset | along 87, across +51 |
| Houston's speed across the run | +0.5 m/s |

Houston's forward in the run frame is along 0.998 and across +0.060. Her keel line, followed
back to across 0, reaches the run line at along 87 - 51 / 0.060 × 0.998, about -764 m. That is
behind the gun, so the run parameter is below 0 and `004F3730` answers false. **The image gives
the same answer from the same inputs.**

**Why the torpedoes still hit.** The two torpedoes that struck Houston ran only 2.45 s and
2.85 s, about 125 m and 145 m at 51.4 m/s. They did not follow the gate's line.
- The torpedo leaves along the snapped tube heading (`torpedo_snap_radians` is up to pi/4 off
  the wanted heading, `0090043D`). It then turns onto the gyro heading, and the jitter
  (`00900830..00900876`) is added to that heading.
- Houston's hull, about 180 m long, lies 41..61 m off the line, beside the first 100 m of the
  run.
- So the hit is the turn out of the tube, plus the beam and length of a ship that sits along
  the run. The image's gate models neither.
- **The tube heading at this launch was not logged.** This explanation is therefore the
  likely one, not an established one.

## 5. Verdict

The image lets this launch through, so the packet records it and stops. Whether the host's
torpedo turn out of the tube matches the image's is a gunnery-side question: the
`007311B0`/`00856637` gyro record and the torpedo turn rate. It is not the gate.

## 6. The torpedo out of the tube (packet `cc9_torpedo_tube_turn`, worker cc9-gunnery4, switch `kTorpedoSwimThrustBound`)

Section 4 left one question: whether the host's torpedo leaves the tube and joins the gyro line the
way the image's does. The answer is **no, and the difference is not the turn rate. It is the swim
itself.** The image's torpedo is a thrust-and-drag body whose velocity follows its nose. The
host's swims at a fixed speed with its velocity rotated directly.

### 6.1 The image, term by term

| term | image | evidence |
| --- | --- | --- |
| launch heading | the gun's trained angles, the tube heading `bot+60h` snapped up to pi/4 (`0090043D`); the round leaves along it at `V0` | USN02_SAMESIDE_TORPEDOES 3, `006E8430` |
| gyro heading | `007311B0`'s `heading` = the gate's run line + AngleErr jitter, installed at `record+46Ch` by `00856637` | USN02_SAMESIDE_TORPEDOES 3 |
| water entry | `008568E0` breaks the round up above `MaxWaterHitVel`, otherwise `006E6450` sets `+44h` and swaps the trails. **It sets no speed.** | TORPEDO_TICK, `008568E0` |
| the swim step | `00857480`, per step while in the water | below |
| turn | `00857061` yaws the **nose** (the local matrix) toward `+46Ch` by `clamp(e, +-HeadingTurn * pi/180) * dt`; HeadingTurn is 10 in every row | TORPEDO_TICK "Heading" |
| drag | `00857536..00857673`: `par = f (v.f)`, `perp = v - par`, `v = par (1 - [+474h] dt) + perp (1 - [+478h] dt)` | listing below |
| thrust | `008576C6..0085775D`: `v += f * [+470h] * k * dt`, `k = 008E6430(0Eh)`, 1 with no modifier | listing below |
| the three fields | `+470h = WaterTravelSpeed * 0.5999994` (`0085785D`, `00D0C5E0`); `+474h = 0.5999994` (`00D0C5EC`); `+478h = 3.0000007` (`00D0C5E8`) | `00857827..0085788B` |
| arming | `+45Ch` is seeded `-1.0` (`00D7A260`) by `006E2670`, which the torpedo constructor `00856050` calls. The scan of `5C 04 00 00` finds no other projectile writer, so **a torpedo is armed from launch**, as in the host | `006E270C`, scan |
| hit test | the projectile's segment sweep, as for every round | PROJECTILE_IMPACT |

```
00857603: fld [ebx+474h] ; fmul st1 (dt) ; fld1 ; fsubrp        ; 1 - 0.6 dt, the axial keep
008575C4: fld [ebx+478h] ; fld [esp+80h] (dt) ... fld1 ; fsubrp ; 1 - 3.0 dt, the lateral keep
008576C6: fld [ebx+470h]                                        ; T = 0.6 * WaterTravelSpeed
00857733: fld [ebx+318h] ; fadd [esp+24h] ; fstp [ebx+318h]     ; v.x += f.x * T * k * dt (and y, z)
```

**Consequences in the image:**
- The round enters the water at its launch speed, about 13 m/s for classes 62 and 67 and 23 m/s
  for class 61. It accelerates toward `T / 0.6 = WaterTravelSpeed` with a time constant of
  1/0.6 = 1.67 s.
- Its path lags its nose by about 1/3 s, from the lateral drag.

### 6.2 The host, term by term

| term | host (`src/game_hosts_gunnery.cpp`, `run_projectiles`) | same? |
| --- | --- | --- |
| launch heading | the gun's angles, `projectile_launch_velocity_006e8430` | yes |
| gyro heading | `kTorpedoGyroHeadingBound`, the same `+46Ch` | yes |
| arming | none | yes |
| water entry | **the velocity is set to `WaterTravelSpeed * 0.6` along the entry heading** | **no** |
| the swim | **constant speed; the velocity vector itself is turned at HeadingTurn** | **no** |
| steady speed | **0.6 x WaterTravelSpeed**: class 62 30.9 m/s, class 67 102.3 m/s | **no**: the image tends to WaterTravelSpeed (51.4 and 170.4 m/s) |

### 6.3 The binding (committed OFF)

`kTorpedoSwimThrustBound`:
- **At the water crossing** the round keeps its horizontal entry velocity.
- **On every swim step** the nose yaws by 00857061's rule, then the drag split and the thrust run
  with the three image constants. `GameBulletClassRow::swim_speed` is exactly `+470h`.
- **Labelled:** the vertical stays the host's surface plane, as with the OFF swim.
- **The diagnostic** `BSP_TORPEDO_TUBE_TRACE=1` logs every gyro launch as
  `gunnery: torpedo tube t= ... tube_deg gyro_deg off_deg speed`.
- **The census** is `summary mission gunnery torpedo swim thrust ...`.

### 6.4 The launches this head makes (OFF, `local\TS_OFF_<m>.log`)

- **USN02:** 227 gyro launches: 168 class 67, 24 class 62 and 35 class 61. The mean tube-to-gyro
  offset is 24.7 degrees. Category 7 has 227 shots and 76 hits, which dealt 27243.
  - Houston is sunk at 33.80 s by a Minegumo Long Lance from the opening spread (launched
    1.45..4.95 s, 2719 m). The mission fails at 34.70 s.
  - **John1's 273.71 s launch of section 4 does not happen on this head.** Since
    `kGunImmediateFireSlotBound` (GUN_SHOT_CADENCE 10.6), Houston is dead by then.
- **USN04, USN01, USN13:** no gyro launch and no torpedo drop in their frames.

### 6.5 Predictions, written before any ON run

Distance run from the water:
- **Class 67, image:** `170.4 t - 262 (1 - e^(-0.6 t))`. Against the host's `102.3 t`, the image
  is behind until about 3.3 s (about 340 m) and ahead after that. A 2719 m run takes about 17.5 s
  against 26.6 s.
- **Class 62:** the image catches up at about 2.7 s and is ahead after that.

| row | prediction |
| --- | --- |
| USN04, USN01, USN13 | **identity**: pair_diff exit 0 or 1, thrust steps 0 |
| USN02 thrust census | steps above 0, `kept_entry` equal to the swims started, mean entry speed between 12 and 24 m/s |
| USN02 torpedo hits (category 7, 76 OFF) | **up**: the faster long runs leave targets less time to turn away |
| USN02 Houston | still lost to an opening-spread Long Lance, and **earlier than 33.80 s**, so the failure comes earlier than 34.70 s |
| USN02 John1's hits on Houston at 276-277 s | **vacuous on this head**: Houston is dead long before |
| USN02 pair_diff | exit 3 |

### 6.6 The pairs, and the flip

OFF is `local\TS_OFF_<m>.log`: this tree at `c1315b447`, copied to `local\ts_off_bin`. ON is
`local\TS_ON_<m>.log`: `pair_export --commit c1315b447 --flip kTorpedoSwimThrustBound=true`
(`local\ts_on`). Streams, the death table and `BSP_TORPEDO_TUBE_TRACE=1` were on. OFF equals the
previous head: `pair_diff` against cc9-gunnery4's `local\IF_FLIP_usn04.log` exits 1.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN04, USN01, USN13 | - | pair_diff 1 on each, thrust steps 0 | identity | held |
| USN02 census | - | steps 218440, kept_entry 223, mean entry speed 15.2 m/s | steps > 0, entry 12..24 m/s | held |
| USN02 torpedo hits (category 7) | 227 shots, 76 hits | 229 shots, 90 hits | up | held |
| USN02 Houston | sunk 33.80 s, Minegumo gun 287, 2719 m | sunk 22.75 s, Minegumo gun 287, 2818 m | same spread, earlier | held |
| USN02 failure | 34.70 s | 29.75 s | earlier | held |
| USN02 totals | 9 deaths, 4597 hit records, 3666 shots, first hit 33.35 s | 11 / 5322 / 3910, first hit 19.20 s | exit 3 | held |
| John1's hits on Houston | - | - | vacuous on this head | held (Houston dies first) |

**John1's launch, reproduced with `kGunImmediateFireSlotBound` off.** Two more exports of
`c1315b447` were built: `local\ts_jo_off` (that switch off) and `local\ts_jo_on` (that switch off,
this one on). Their logs are `local\TS_JO_{OFF,ON}_usn02.log`.
- **With both switches off, the section 4 run returns.** The failure is at 212.91 s, Houston is
  sunk at 295.95 s, and John1's hits land at 276.11 and 277.01 s. The new trace line gives the
  launch that section 4 lacked:

```
gunnery: torpedo tube t=273.71 shooter=John1 plat=11 class=62 tube_deg=44.7 gyro_deg=-2.9 off_deg=-47.7 speed=13.0
```

  **The tube points 47.7 degrees off the gyro line, toward Houston's side.** Section 4's
  explanation is now established.
- **With the image swim, that battle diverges within 20 s,** so the 273.71 s launch never happens.
  Houston is sunk at 22.80 s. The answer therefore comes from the launch itself.
  `local\g4_john1_sim.py` swims both laws from this launch against Houston's logged position and
  motion (section 4's table), with the hull taken as 180 m by 20 m:

| swim | first point inside Houston's hull (t, along, across) |
| --- | --- |
| host (OFF) | 2.15 s, 52.9 m, 39.4 m |
| image (ON) | 2.15 s, 51.4 m, 39.3 m |

  Under the image's law the path even swings wider: its greatest excursion off the run line is
  75.5 m at 9.7 s. **So the image would hit Houston too.** John1's hits are the tube's 47.7-degree
  snap and the turn onto the gyro line, which the gate does not model (section 1). They are not a
  host defect.
- **Friendly direct torpedo hits per run:**

| run | friendly direct hits |
| --- | --- |
| `TS_OFF_usn02` | 6 |
| `TS_ON_usn02` | 7 |
| `TS_JO_OFF_usn02` | 4 |
| `TS_JO_ON_usn02` | 11 |

  The faster, lagging image swim spreads wider off the tube line. Recorded, not bound further.

**Decision: `kTorpedoSwimThrustBound` is ON.**
- The swim is `00857480`'s, read from the listing, and every recorded prediction held.
- The image arms torpedoes from launch, and turns them only while they swim, as the host does.

## no_ghidra_function

None. Every address named lies inside a Ghidra function.
