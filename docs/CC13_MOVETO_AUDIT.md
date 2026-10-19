# cc13-movaudit: census row 15 (torpedo moveto SetRanges)

This packet audits a labelled substitution from cc13-stubcensus (docs/CC13_STUB_CENSUS.md
at 1e2d7a2e9, row 15). Row 14 was dropped and is noted at the end. The packet was
read-only: no code changed and there were no game runs. Names are hypotheses, not recovered
symbols. The ship heights come from recent cc9 logs (l70r_vb9_vcon_ref, p24h_on_u21f_def,
p24_off_j8, p24r_on_u04).

| Row | Site | Verdict |
|---|---|---|
| 15 | 009BDE80 SetRanges at 009D48CF (torpedo) | FAITHFUL in effect. Not urgent. |
| 14 | 009F9E40 at 009C1B23 (dive-bomb moveto steer) | Resolved: covered by the existing switch `kDiveBombMoveToOriginBound`. Not audited. |

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

### Verdict: FAITHFUL in effect (superseded)

Superseded by docs/CC13_TORPRANGES.md: current-main logs put torpedo targets' origins above 0
(USN21 up to +1.3 m), so difference 1 is live. The switch `kTorpedoMoveToRangesBound` (OFF)
models the stores and the lift.

The skipped stores have one reader, and the host feeds that reader the same values from the
same tick. This is a torpedo class row, but it is not a mismatch, so there is nothing urgent.
The census's "unproven (torpedo)" can become "faithful in effect".

## Row 14: 009F9E40 at 009C1B23 (resolved: existing switch, not audited)

This row was dropped from the packet. The census note "body unread" is stale. 009F9E40 is
reconstructed as `bsp::heading_command_009f9e40` (src/plane_flight.cpp:81), and the
dive-bomb moveto site is gated by `kDiveBombMoveToOriginBound` (OFF). Flipping that switch
is cc9's call.

Note from before the scope change: with the switch OFF, the host steers at the 009FADA0
hull aim point instead of the target origin. Four `hull_aim inrange` samples put that at
0.1-1.8 degrees, with the separation 30-61 m short.
