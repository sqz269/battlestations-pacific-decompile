# cc13-row21: census row 21, 0099DAC8's correction term

Packet cc13-row21, from docs/CC13_STUB_CENSUS.md row 21 (0099D300 pilot plan). Names are
hypotheses, not recovered symbols. Read-only, no game runs.

## Verdict

**The row is stale.** It says 0099DAC8's correction term (unit+B1Ch, unit+AE0h) "has no writer
in the image and is taken as 0". Both fields have writers. The term is already carried, under
`kPlaneVelocityLowpassBound` (ON, packet cc9_plane_velocity_lowpass), in
`src/game_hosts_units.cpp` at the 0099DA97-0099DAC8 block of the hosted 0099D300. The `0.0f` the
row saw is that block's OFF value. The comment above it now says so.

## The term (bytes)

ECX = [ESI+2F0h], the unit.
- **0099DA5C-0099DA91:** copies the vector unit+AE0h/+AE4h/+AE8h to [ESP+5Ch].
- **0099DA97:** calls the unit's vtable +38h. For a plane this is 007B8E60, `FLD [unit+B1Ch]`.
- **0099DAA1:** calls 0042B2F0, the 3D length of the copy.
- **0099DAA6:** FSUBR gives B1Ch - |AE0h|.
- **0099DABE:** multiplies by double [00CE3D88] (20.0).
- **0099DAC4:** multiplies by the pending value [ESP+6Ch].
- **0099DAC8:** FSUBR from the throttle error held as a double at [ESP+54h].

## Writers

**Literal-displacement scan.** local/disp_writers.py decodes every disp32 occurrence of B1Ch,
AE0h, AE4h and AE8h by a linear sweep from its function's start, and keeps the memory writes.
- No plane function writes any of them.
- The writes it does find belong to other classes: 004EBxxx, 0062F141, 0063xxxx, 0081F1E6, the
  ship AI 009E54CD-009E54D9 and 009F1215-009F1221, the joystick device 00A996C8/00A99A80, and
  zlib 00BCB891/00BCF0F0.
- Four AE8h occurrences in 00C0xxxx/00C2xxxx would not decode in sync. They lie in library code.

**The real writers go through the flight controller ctl = unit+AB0h**, with disp8 fields that a
disp32 scan cannot see:
- **unit+B1Ch = ctl+6Ch.** `MOVSS [ESI+6Ch],XMM0` at 007D807C in 007D8020
  BSP_PlaneFlightController_UpdateSpeedTerms, ESI = ctl. It is the length of ctl+18h, the
  controller's world velocity (007D8024-007D8056: float sum of squares, double 1e-10 floor at
  00CE3820, sqrt 00BF7030). Reached by tail jumps from 007D9C10 (007D9C71) and 007D9C80
  (007D9CD8).
- **unit+AE0h = ctl+30h**, the low-passed velocity.
  - Written by `FSTP [ESI+30h]/[ESI+34h]/[ESI+38h]` at 007DC7FF/007DC805/007DC80B, at the tail
    of 007DB680 BSP_PlaneFlight_CoreLaw (007DC86E in 007DC830 calls 007DB680).
  - Also copied from ctl+18h by 007D9CE0, 007DB1F0 and 007DB2C0, per the switch's existing
    comment.

**No block-copy writer is needed to explain the fields.** I did not separately scan for REP MOVS
writers of unit+B1Ch/+AE0h. The direct controller writers above are enough to refute "no
writer".

**Reach.** Both writers run on every free-flight step of every plane, so the term is LIVE. It is
already bound and ON. Nothing is new to switch.

## Left as they were (already labelled in the switch comment)

- **The filter seed.** The host seeds ctl+30h at a slot's first core step. The image seeds at
  spawn and at 007DB1F0's control-state message.
- **The third reader is not carried.** 0099D514-0099D593 recompute |B1Ch - |AE0h|| (0099D558
  FSUBR, then an absolute value by subtracting from -0.0 at [00D7A208]) into [ESP+48h].
  - No later instruction in 0099D300 reads [ESP+48h] at the frame depth of 0099D593.
  - The reads at shifted offsets after pushes (for example `LEA EDX,[ESP+50h]` at 0099E184)
    were not depth-traced.
  - So whether this value is dead is open. It is not a census row-21 item.

## Census row 21 text (for the batch)

| 21 | 0099D300 pilot plan | game_hosts_units.cpp (0099DA97-0099DAC8 block) | 9 / 2,925,028 | Stale. 0099DAC8's term has writers through the flight controller: unit+B1Ch = ctl+6Ch (007D807C) and unit+AE0h = ctl+30h (007DC7FF-007DC80B). It is carried under kPlaneVelocityLowpassBound (ON); 0 is the OFF value only. docs/CC13_ROW21.md. | kPlaneVelocityLowpassBound=ON | no |
