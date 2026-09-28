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

## no_ghidra_function

None. Every address named lies inside a Ghidra function.
