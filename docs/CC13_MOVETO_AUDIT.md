# cc13-movaudit: census rows 14 and 15 (moveto steer, torpedo SetRanges)

This packet audits two labelled substitutions from cc13-stubcensus (docs/CC13_STUB_CENSUS.md
at 1e2d7a2e9, rows 14 and 15). It was read-only: no code changed and there were no game
runs. Names are hypotheses, not recovered symbols. The ship and aim-point numbers come from
recent cc9 logs (l70r_vb9_vcon_ref, p24h_on_u21f_def, p24_off_j8, p24r_on_u04).

| Row | Site | Verdict |
|---|---|---|
| 15 | 009BDE80 SetRanges at 009D48CF (torpedo) | FAITHFUL in effect. Not urgent. |
| 14 | 009F9E40 at 009C1B23 (dive-bomb moveto steer) | GAP, small. An OFF switch already exists: `kDiveBombMoveToOriginBound`. |

## Row 15: 009BDE80 at 009D48CF, torpedo moveto SetRanges

### The image

009BDE80 is the whole body. It stores the three stack floats at this+30h, +34h and +38h,
then returns with RET 0Ch:

```
009bde80  MOVSS XMM0,[ESP+4]   / 009bde86  MOVSS [ECX+30h],XMM0
009bde8b  MOVSS XMM0,[ESP+8]   / 009bde91  MOVSS [ECX+34h],XMM0
009bde96  MOVSS XMM0,[ESP+0Ch] / 009bde9c  MOVSS [ECX+38h],XMM0
009bdea1  RET 0Ch
```

The call site is 009D4850 BSP_BotTaskTorpedo_TickArm, with ESI = the task and EDI = the
approach block at task+3F8h:
- 009D486F calls 009D3420, the approach update.
- 009D4874-009D4895: the third argument is approach+7Ch when 15.0 (double at 00CF3F20,
  bytes `00..00 2E 40`) is at most approach+134h. Otherwise it is approach+80h.
- 009D48A6: ECX = task+544h, the moveto state.
- 009D48AC-009D48CC: the first two arguments are both approach+78h + approach+74h.
- 009D48CF calls 009BDE80.
- 009D48E7-009D48F6: the current state's vtable[0Ch] tick runs right after, through
  task+310h.

### Who reads +30h, +34h and +38h

- **The class.** The moveto vtable is 00D20AEC. Its slots are 009C3D10, 007B3DB0, 007B3DC0,
  009C18C0, 007B3DE0, 007B3DF0, 007B45E0, 009C1850, 009C2CF0 and 009BDEB0. The constructor is
  009C2AC0. Scanning each body for `+ 0x30]`, `+ 0x34]` and `+ 0x38]` finds:
  - reads only in 009C18C0, the tick: 009C19F1 `FLD [EDI+34h]`, 009C19FC `FLD [EDI+30h]`
    and 009C1B09 `FLD [EDI+38h]`;
  - writes only in the constructor: 009C2B17, 009C2B22 and 009C2B41.
- **Through the task.** The state sits at task+544h, so the fields are task+574h, +578h and
  +57Ch. A byte scan for disp32 `74 05 00 00` (178 hits), `78 05 00 00` (97) and `7C 05 00 00`
  (127) finds one hit in the 009B-009D plane-task range: 009CC906 in 009CC8F0
  BSP_BotTaskStrafe_DebugDraw. That is a different task class. The scan is not vacuous: the
  patterns occur elsewhere.
- **Other writers.** The other 009BDE80 callers belong to other tasks: 007B7C7F, 009A2B47,
  009A671A, 009AE8F7, 009B8BA7, 009C8825 and 009CD1AF.

009C18C0 uses the three values like this: `base = max(+34h + targetY, +30h)`,
`rangeLow = +38h`, and `rangeHigh` = the live planar distance, all going into 009FBA50.
docs/TORPEDO_MOVETO_TICK.md section 2 covers this.

### The host

`refresh_move_to_ranges` (src/game_hosts_units.cpp, near line 22914) only records the call.
The torpedo moveto tick `run_move_to_tick_009c18c0` (near line 33219) reads the same
approach fields directly:
- `base = ap.alt_floor_74 + ap.alt_margin_78`;
- `range_low = ap.elapsed_134 >= 15.0f ? ap.speed_late_7c : ap.speed_early_80`.

The tick is called from the arm after the approach update, just as the image fills the
three floats immediately before the state tick in the same TickArm. Each tick therefore sees
the same values.

### Differences that remain

1. **The target-height lift.** The image's base is `max(A + targetY, A)`, where
   A = +74h + +78h. The host's is A. They differ only when the target's world Y is positive.
   Ship origins in the logs sit at or just below 0: Lexington -0.0 and -0.3, Musashi -1.2,
   Missouri -0.7. So `max(A + y, A) = A` in every case, and the two agree.
2. **A NaN at approach+134h.** In the image, the FCOMIP leaves CF and ZF set and JBE takes
   +7Ch. The host takes +80h. The host's +134h is an accumulator that starts at 0, so it is
   never NaN.

### Verdict: FAITHFUL in effect

The skipped stores have one reader, and the host feeds that reader the same values from the
same tick. This is a torpedo class row, but it is not a mismatch, so there is nothing urgent.
The census's "unproven (torpedo)" can become "faithful in effect".

## Row 14: 009F9E40 at 009C1B23, dive-bomb moveto steer

### The image

009F9E40 (009F9E40-009F9ECC, `__thiscall(bot, const float* point)`, RET 4):
- If the unit's pose is stale (+C8h clear), it refreshes it with 00414DB0.
- It computes `dz = p[2] - unit+104h` and `dx = p[0] - unit+FCh`, rounding each to float.
- It computes `h = (double)pi/2 - atan2(dz, dx)` (00BF701A, double 00CE3830), rounded to
  float. If 0 > h, it adds 2 pi (double 00CE3828).
- It stores cmd+2C0h = h and cmd+2CCh = 2 (009F9EB9 and 009F9EC1).

At 009C1B1C-009C1B23 the argument is `LEA ECX,[ESP+1Ch]`, the target entity's pose origin
from step 1 (009C18EC-009C1913).

The census says "the body is unread". That is stale. The body is reconstructed as
`bsp::heading_command_009f9e40` (src/plane_flight.cpp:81), and the host already uses it in
three places:
- the torpedo moveto, under `kPilotStateHeadingWritesBound` (ON);
- the dogfight;
- the dive-bomb moveto, under `kDiveBombMoveToOriginBound` (OFF, units.cpp:6235).

### The host, with the switch OFF

`run_dive_bomb_move_to_tick_009c18c0` (near line 30010) writes
`plan_heading_2c0 = db_bearing_c0` with mode 2. db_bearing_c0 comes from
`update_dive_bomb_approach` (near line 4892, 009C7B8A-009C7BB0). It uses the same law, but
the point it steers at is the hull aim point from 009FADA0 (`tp_hull`), not the target
origin. Planar separation follows the same pattern: with the switch OFF it is db_planar_bc,
measured to the aim point. That separation feeds 009C1850's speed and the glide.

Only the value of cmd+2C0h differs. cmd+2CCh is 2 either way, and nothing else is skipped.

### The size of the effect

These are the four `hull_aim inrange` lines in recent logs, sampled where the range latch
(009C7BFB) first trips. The heading difference is roughly aim offset / range, so it grows as
the bomber closes. I did not confirm that the moveto state hands over exactly at this latch.
If it runs closer in, the difference is larger: about 10 degrees for an 86 m offset at
500 m.

| Mission log | Target | Aim offset | Range to origin | Heading difference | Planar, aim vs origin |
|---|---|---|---|---|---|
| l70r_vb9 | Lexington | 86.3 m | 1939 m | -1.83 deg | 1877.6 vs 1938.7 |
| p24h_u21f | Musashi | 43.2 m | 2252 m | +0.71 deg | 2219.3 vs 2252.3 |
| p24_j8 | Missouri | 42.8 m | 1728 m | +0.10 deg | 1684.9 vs 1727.6 |
| p24r_u04 | Lexington | 41.9 m | 2358 m | -0.71 deg | 2327.8 vs 2357.9 |

So, with the switch OFF:
- the dive bomber heads 0.1-1.8 degrees off the image's heading at the latch, and less
  further out;
- the separation fed to the speed and the glide is 30-61 m short, about 1.5-3 %.

The census found the row live in 7 of 11 WINS missions: every dive-bomb moveto tick takes
the substitution.

### Verdict: GAP, small

The proposal is to reuse the existing `kDiveBombMoveToOriginBound`, not to add a new switch.
It already carries both halves: the origin separation and the 009F9E40 heading. Its doc is
docs/DIVE_BOMB_APPROACH.md section 19. Before a flip it needs the usual OFF/ON pair on the
seven live missions, judged on dive-bomb hits and the inrange handover times.

ON-only lookups when flipped: `origin_tgt` is
`owner_.slots[command_target_plus_one - 1]`. That is the same slot the OFF path already reads
for `tp` in the same tick, so it is live whenever the ON path runs.
