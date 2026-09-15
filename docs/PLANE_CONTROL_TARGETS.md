# The three control targets of `007DA710 BSP_PlaneFlight_ControlRateLaw`

`__thiscall(controller /* ECX */, float step)`. This document covers **`007DA710`-`007DAB6C`**: the
construction of the three target rates that `007DAB52`, `007DAB5F` and `007DAB6C` consume to form
the per-axis deltas. The rate law that consumes them is `docs/PLANE_CONTROL_RATE_LAW.md`; this
document does not restate it and corrects it in four places (see the last section).

Read-only analysis, no Ghidra mutation. Every descriptive name is a hypothesis, not a recovered
symbol.

## Register and slot conventions

| thing | where | evidence |
| --- | --- | --- |
| `ctl` | `ESI`, from `ECX` | `007DA715 MOV ESI,ECX` |
| `unit` | `EDI` = `[ctl+8h]` | `007DA72D MOV EDI,[ESI+8]`; `plane_flight_controller_off::kControllerUnit = 0x08` |
| `class` | `EBP` = `[ctl+0Ch]` | `007DA7BF MOV EBP,[ESI+0Ch]`; `kClassDescriptor = 0x0C` |
| `step` | `frame=4` | the single stack argument |
| `f1` | `frame=-68` | out-param 1 of `007DA380` (`007DA721 LEA EDX,[ESP+20h]`, pushed last) |
| `f2` | `frame=-52` | out-param 2 (`007DA71C LEA ECX,[ESP+2Ch]`) |
| `flag` | `frame=-81`, read to `BL` at `007DA9EF` | out-param 3 (`007DA717 LEA EAX,[ESP+0Bh]`) |

All `frame=` offsets are entry-relative, from `tools/stack_frame_walk.py 007da710`. Literal `[ESP+N]`
displacements are **not** used below: the body runs at four different depths (92, 100, 112, 120) and
the same literal names different storage at each. Ghidra's own local names map one to one onto these
offsets (`local_4c` is `frame=-76`, `local_48` is `frame=-72`, and so on), which is how the
decompiler output below can be used as a cross-check.

The three target slots, and the deltas they feed:

| target slot | consumed at | axis | delta at |
| --- | --- | --- | --- |
| `frame=-72` | `007DAB52` | `ctl+48h` pitch | `007DAB5B` |
| `frame=-8` | `007DAB5F` | `ctl+4Ch` yaw | `007DAB68` |
| `frame=-76` | `007DAB6C` | `ctl+50h` roll | `007DAB75` |

Ghidra's decompiler agrees on the pairing: `fVar10 = local_48 - *(float *)(param_1 + 0x48)`,
`fVar3 = local_8 - *(float *)(param_1 + 0x4c)`, `fVar4 = local_4c - *(float *)(param_1 + 0x50)`.

## The three expressions

```c
// ---- pitch target, frame=-72, live definition 007DA912 / 007DA922 ----
T_pitch = PitchSpd(class+1ACh) * f1 * kLatchedPitch(unit+BB4h);
if (kLatchedPitch < 0.0f)                       // 007DA8F9 / 007DA916
    T_pitch *= NegativePitchRatio(class+1D8h);

// ---- yaw target, frame=-8, live definition 007DAA97 ----
yawRaw = YawSpd(class+1B0h) * kLatchedYaw(unit+BB0h) * unit_BC4h;   // frame=-56, 007DA926..007DA953
Y      = yawRaw * f2;                                              // frame=-64, 007DA957..007DA95F
if (flag != 0)                                                     // 007DA9F5
    Y -= SlideRatio(class+1B8h) * YawSpd(class+1B0h)
         * sinf(unit+C68h) * cosf(unit+C64h);                      // 007DAA3B..007DAA58
T_yaw  = -Y;                                                       // 007DAA85..007DAA97

// ---- roll target, frame=-76, live definition 007DAA78 (or 007DA8E5 when flag == 0) ----
R = unit+838h + kLatchedRoll(unit+BB8h);                           // 007DA732..007DA74C
k = 1.0f;                                                          // 007DA73E/746
if (unit != NULL && unit->flightState(+900h) == 6)                 // 007DA730/750, 007DA752
    k = lerp_clamped(0.0f -> 1.0f, 0.10471976f -> -0.1f, fabsf(unit+C68h));   // 007DA75B..007DA7B5
R = RollSpd(class+1A8h) * f1 * k * R;                              // 007DA7C2..007DA7DA
if (unit+5Dh != 0) {                            // out of action   // 007DA7B9 / 007DA7DE
    R *= lerp_clamped(0.0f -> 1.0f,
                      DeadMeat/RollMulTime -> DeadMeat/RollMul,
                      unit+C3Ch /* lost-drag timer */);            // 007DA7E4..007DA828
    if (unit+C36h != 0) {                                          // 007DA821 / 007DA82C
        s = fminf(unit+C3Ch * 4.0, 1.0f)
            * DeadMeat/SpinRollSpd
            * fmaxf(0.35f, RollSpd(class+1A8h));                   // 007DA839..007DA8CF
        R += (unit+C37h != 0) ? +s : -s;                           // 007DA832 / 007DA85B
    }
}
if (*(int *)(ctl+FCh) == 1)   R = 0.0f;                            // 007DA8D9..007DA8E5
if (flag != 0)                                                     // 007DA9F5
    R += YawRollRatio(class+1B4h) * f1 * yawRaw;                   // 007DAA5C..007DAA78
T_roll = R;
```

`lerp_clamped` is `00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x)` - five stack floats,
`RET 14h`, result in `ST0`, clamped between the two `y` endpoints (ledger; `docs/UNIT_RUDDER_CURVE.md`).
`fmaxf` is `00415550 BSP_Math_MaxFloatByRef(ECX = &a, EDX = &b)`, `RET 0`, result in `ST0`.

## Evidence

### Prologue and the mode factors, `007DA710`-`007DA72D`

```
007da710  SUB  ESP,54h
007da717  LEA  EAX,[ESP+0Bh]      frame=-81   -> &flag
007da71c  LEA  ECX,[ESP+2Ch]      frame=-52   -> &f2
007da721  LEA  EDX,[ESP+20h]      frame=-68   -> &f1
007da726  MOV  ECX,ESI
007da728  CALL 007da380           [callee pops 12]
007da72d  MOV  EDI,[ESI+8]        ; unit
```

Push order is `&flag`, `&f2`, `&f1`, so at the call `[ESP]` is `&f1` - argument 1. Neither `frame=-68`
nor `frame=-52` is written anywhere else in the body; their only producer is the callee. In free
flight (`ctl+FCh == 0`, mode 0) `007DA6E6`-`007DA700` stores **the same scalar** to both and sets
`flag = 1` - established in `docs/PLANE_CONTROL_RATE_LAW.md`, not re-derived here. That scalar is
`007D9A70(ctl)`, which is unnamed (callees: `007D99C0 BSP_PlaneFlight_ForwardSpeed` and
`00419010`); **what it computes is not established**.

### The roll base and the flight-state-6 factor, `007DA730`-`007DA7B5`

```
007da730  TEST EDI,EDI
007da732  FLD  [EDI+838h]
007da738  FADD [EDI+0BB8h]        ; kLatchedRoll
007da73e  MOVSS XMM0,[00D7A24C]   ; 1.0f
007da746  MOVSS [ESP+1Ch],XMM0    frame=-64 = 1.0
007da74c  FSTP [ESP+10h]          frame=-76 = unit+838h + unit+BB8h
007da750  JZ   007da7b9           ; EDI == 0
007da752  CMP  [EDI+900h],6
007da759  JNZ  007da7b9
```

The `JZ` at `007DA750` consumes the flags from `007DA730`: `FLD`, `FADD`, `MOVSS` and `FSTP` do not
write `EFLAGS`. Note that the null test is scheduled **before** three unconditional dereferences of
the pointer it tests; the decompiler reproduces the same order
(`local_4c = *(float *)(iVar1 + 0x838) + *(float *)(iVar1 + 3000);` ahead of `if ((iVar1 != 0) && ...)`).

`unit+900h` is `plane_advance_off::kFlightState` / `plane_flight_off::kGroundWaterMode`. **What state
6 is, is not established** - `plane_flight.hpp:75-76` records `+900h` driving arms 4 and 5 at
`007CEC75` and a different field (`+5F0h`) for arm 6.

```
007da75b  MOVSS XMM0,[EDI+0C68h]
007da763  COMISS XMM0,[00D7A218]  ; 0.0f
007da76a  JBE  007da774
007da76c  MOVSS [ESP+1Ch],XMM0            ; > 0  -> as is
007da774  MOVSS XMM1,[00D7A208]   ; -0.0f
007da77c  SUBSS XMM1,XMM0                 ; <= 0 -> negate
007da780  MOVSS [ESP+1Ch],XMM1    frame=-64 = |unit+C68h|
...       args: [ESP]=0.0 [ESP+4]=1.0 [ESP+8]=[00D06868] [ESP+0Ch]=[00CE3CB4] [ESP+10h]=|unit+C68h|
007da7b0  CALL 00419010
007da7b5  FSTP [ESP+1Ch]          frame=-64 = the clamped interpolation
```

Constants, all in `.rdata` (`va=00CE2000`, `rawsize=00126000`, so file-backed to `00E08000`) and
therefore real: `00D7A208 = 80000000 = -0.0f`, `00D7A218 = 0.0f`, `00D7A24C = 1.0f`,
`00D06868 = 3DD67750 = 0.10471976f` (= pi/30, six degrees), `00CE3CB4 = BDCCCCCD = -0.1f`.

So in flight state 6 the roll factor ramps `1.0 -> -0.1` as `|unit+C68h|` goes `0 -> 6 degrees`,
clamped to `[-0.1, 1.0]`, and **reverses sign past roughly 5.45 degrees**. Outside that state the
factor is the `1.0` written at `007DA746`.

`unit+C68h` is `plane_advance_off::kBankAngle` (`plane_advance_pose.hpp:47`, "taken through FSIN"),
and this body uses `FSIN` on it (`007DAA12`) and `FCOS` on `unit+C64h` (`007DAA35`), matching that
header's `kPitchAngle = 0xC64` "taken through FCOS" exactly. `include/bsp/pilot_command_path.hpp:90`
gives `+C68h` a different name (`kPilotCmdUnitPitchZeroAngle`); that conflict is **not resolved here**.

### `RollSpd` and the first roll product, `007DA7BF`-`007DA7DA`

```
007da7bf  MOV  EBP,[ESI+0Ch]      ; class
007da7c2  FLD  [EBP+1A8h]         ; RollSpd
007da7c8  LEA  EBX,[EBP+1A8h]     ; kept for 00415550 below
007da7ce  FMUL [ESP+20h]          frame=-68  f1
007da7d2  FMUL [ESP+24h]          frame=-64  the state-6 factor
007da7d6  FMUL [ESP+18h]          frame=-76  the roll base
007da7da  FSTP [ESP+18h]          frame=-76
```

`007DA7CE` and `007DA7D2` read `frame=-68` and `frame=-64` through literals `20h` and `24h` at
`depth=100`, where `007DA746`/`007DA7B5` wrote `frame=-64` through literal `1Ch` at `depth=92`. This
is exactly the aliasing the frame walk exists to catch.

### The out-of-action roll ramp, `007DA7B9`/`007DA7DE`-`007DA828`

`007DA7B9 CMP byte [EDI+5Dh],0` sets the flags consumed by `007DA7DE JZ 007DA8D9`, so both this block
and the next run only when `unit+5Dh != 0`. `unit+5Dh` is recorded twice as the **out-of-action**
byte: `controlled_unit.hpp:107 kUnitOffOutOfAction = 0x5D; // 0064509B, must be clear` and
`airfield_taxi.hpp:66 kTaxiEntityOutOfAction = 0x5D`.

```
007da7e4  FLD  [EDI+0C3Ch]        ; the lost-drag timer
007da7ed  FSTP [ESP+30h]          frame=-72 used as scratch here
007da7f1  FLD  [ESP+30h]
007da7f5  FSTP [ESP+10h]          arg4  x  = unit+C3Ch
007da7f9  FLD  [00F87340] -> arg3  y1
007da803  FLD  [00F8733C] -> arg2  x1
007da80d  FLD1            -> arg1  y0 = 1.0
007da813  FLDZ            -> arg0  x0 = 0.0
007da818  CALL 00419010
007da81d  FMUL [ESP+18h]          frame=-76
007da828  FSTP [ESP+18h]          frame=-76
```

`frame=-72` is scratch at `007DA7ED`; its live definition at `007DAB52` is `007DA912`/`007DA922`,
600 bytes later. `unit+C3Ch` is `PlaneFlightInputs::lost_drag_timer` (`plane_flight.hpp:432`,
"the DeadMeat ramp's input").

**The two globals name themselves.** `00F872F0` is the base of the 312-byte plane-tuning mirror that
`007EAAD7 REP MOVSD` copies from `BSP_GameTuning_LoadFromPlaneGlobals`'s `this+210h`
(`docs/PLANE_CONTROL_RATE_LAW.md` proves the copy; the anchor is `RotationFactors/A` at
`00F872FC` = `+21Ch`). Applying the same mapping:

| global | tuning offset | key (`game_tuning_singleton.hpp`) | value in this install |
| --- | --- | --- | --- |
| `00F8733C` | `+25Ch` | `Dynamics/DeadMeat/RollMulTime` | 6 |
| `00F87340` | `+260h` | `Dynamics/DeadMeat/RollMul` | 2.0 |
| `00F87338` | `+258h` | `Dynamics/DeadMeat/SpinRollSpd` | 5.0 |

and `scripts/datatables/planeglobals.lua:303-307` carries the authors' own comments, which describe
this block: `RollMul` is *"miutan elkezd zuhanni egy repulo, a rollozasa egyre gyorsabb lesz. ez egy
szorzo, ennyiszeres lesz a rollozas 'RollMulTime' ido alatt"* - once a plane starts falling its roll
gets faster, by this multiplier, over `RollMulTime` seconds. That is exactly a `1.0 -> RollMul` ramp
over `0 -> RollMulTime` driven by the lost-drag timer.

**Caveat on the numbers**, carried over from `docs/PLANE_CONTROL_RATE_LAW.md`: this installation is
modded and `planeglobals.lua` carries the install date `2024-10-29` rather than the `2024-07-13`
bulk. The values are *this installation's*, not provably retail. The **key names and offsets** are
from the executable and do not depend on that.

### The spin term, `007DA821`/`007DA832`-`007DA8D5`

Reached only by falling through the previous block, so `frame=-72` still holds `unit+C3Ch`.
`007DA821 CMP byte [EDI+0C36h],0` gates it (`007DA82C JZ 007DA8D9`).

```
007da832  CMP  byte [EDI+0C37h],0
007da839  FLD  [ESP+1Ch]          frame=-72 = unit+C3Ch
007da83d  FMUL double [00D7A328]  ; 4.0
007da843  MOVSS XMM0,[00CF6560]   ; 0.35f
007da84b  MOVSS [ESP+2Ch],XMM0    frame=-56 = 0.35f
007da851  FSTP [ESP+24h]          frame=-64 = unit+C3Ch * 4.0
007da855  FLD1
007da857  FLD  [ESP+24h]
007da85b  JZ   007da896           ; unit+C37h == 0 -> the subtract arm
007da85d  FCOMIP ST0,ST1          ; df f1
007da861  JBE  007da86d           ; value <= 1.0 -> keep it, else 1.0
007da873  MOV  EDX,EBX            ; &class+1A8h  (RollSpd)
007da875  LEA  ECX,[ESP+2Ch]      ; &frame=-56   (0.35f)
007da87f  CALL 00415550           ; max(0.35f, RollSpd) -> ST0
007da884  FLD  [ESP+24h]          ; min(unit+C3Ch*4, 1.0)
007da888  FMUL [00F87338]         ; SpinRollSpd
007da88e  FMULP                   ; de c9 -> ST1 = max * min * SpinRollSpd
007da890  FADD [ESP+18h]          frame=-76
007da894  JMP  007da8d5
007da896  ...                     ; the mirror arm
007da8b6  FSTP double [ESP+38h]   frame=-44 = (double)frame=-76
007da8d1  FSUBR double [ESP+38h]  ; roll - max * min * SpinRollSpd
007da8d5  FSTP [ESP+18h]          frame=-76
```

Both `FMULP` are `DE C9` = `FMULP ST(1),ST(0)`; `FCOMIP ST0,ST1` is `DF F1`. The `00CF6560 = 3EB33333
= 0.35f` and `00D7A328 = 4010000000000000 = 4.0` (double) are both `.rdata`.

`planeglobals.lua:307` again describes exactly this: `SpinRollSpd` is *"SPIN meghalas eseten a maximum
rollozas ennyiszeresevel fog forogni valamelyik iranyba"* - on a spin death it rotates at this
multiple of the maximum roll, **in one direction or the other**. The "one direction or the other" is
the `unit+C37h` sign byte at `007DA832`, and "maximum roll" is the `max(0.35f, RollSpd)` at
`007DA87F`. `unit+C36h` and `unit+C37h` are **not recorded anywhere in this repository**; that they
mean "spin death" and "spin direction" is an inference from the tuning key they gate, not a trace.

### The ground gate, `007DA8D9`-`007DA8E5`

```
007da8d9  CMP  dword [ESI+0FCh],1
007da8e0  XORPS XMM2,XMM2
007da8e3  JNZ  007da8eb           ; != 1 -> SKIP the zeroing
007da8e5  MOVSS [ESP+18h],XMM2    frame=-76 = 0.0
```

The roll target is zeroed **when `ctl+FCh == 1`**, which `docs/PLANE_FLIGHT.md:79` records as the
on-the-ground mode. The decompiler agrees: `if (*(int *)(param_1 + 0xfc) == 1) { local_4c = 0.0; }`.
See the corrections section - the ledger comment on this function states the opposite.

`XMM2` stays zero from here to `007DA973`; it is the zero comparand for the pitch sign test.

### The pitch target, `007DA8EB`-`007DA922`

```
007da8eb  FLD  [EBP+1ACh]         ; PitchSpd          ST0=PitchSpd
007da8f1  MOVSS XMM0,[EDI+0BB4h]  ; kLatchedPitch
007da8f9  COMISS XMM2,XMM0        ; 0.0 vs latched
007da8fc  FLD  [ESP+20h]          ; f1               ST0=f1  ST1=PitchSpd
007da900  FLD  ST0                ; d9 c0            ST0=f1  ST1=f1  ST2=PitchSpd
007da902  MOVSS [ESP+2Ch],XMM0    frame=-56 = kLatchedPitch
007da908  FMULP ST2               ; de ca -> ST2 = PitchSpd*f1, pop
007da90a  FLD  [ESP+2Ch]          ; kLatchedPitch
007da90e  FMULP ST2               ; de ca -> ST2 = PitchSpd*f1*kLatchedPitch, pop
007da910  FXCH                    ; d9 c9
007da912  FSTP [ESP+1Ch]          frame=-72 = PitchSpd*f1*kLatchedPitch   (leaves f1 on ST0)
007da916  JBE  007da926           ; 0 <= latched -> skip
007da918  FLD  [EBP+1D8h]         ; NegativePitchRatio
007da91e  FMUL [ESP+1Ch]
007da922  FSTP [ESP+1Ch]          frame=-72
```

Every ambiguous mnemonic was settled from the bytes: `DE CA` is `FMULP ST(2),ST(0)` (`STi *= ST0`,
pop), `D9 C0` is `FLD ST(0)`, `D9 C9` is `FXCH ST(1)`. The `JBE` at `007DA916` follows
`COMISS XMM2,XMM0` with `XMM2 = 0`, so it is taken when `0 <= kLatchedPitch` (and when unordered);
the ratio therefore applies exactly when the latched pitch is **strictly negative**. The decompiler
agrees: `local_48 = *(float *)(iVar1 + 0xbb4) * local_44 * *(float *)(iVar2 + 0x1ac);` then
`if (*(float *)(iVar1 + 0xbb4) < 0.0) { local_48 = *(float *)(iVar2 + 0x1d8) * local_48; }`.

**`f1` is left on the x87 stack by `007DA912`** and stays there, through the yaw block and the
`unit+BC4h` relaxation, until `007DAA7F` pushes `PitchAccel` on top of it. That is what makes
`007DAA93 FMUL ST1` (`D8 C9`, `ST0 *= ST1`) mean `PitchAccel * f1`.

There is no mode gate and no zeroing on the pitch target.

### The yaw raw product, `007DA926`-`007DA95F`

```
007da926  FLD  [EBP+1B0h]         ; YawSpd
007da92c  MOVSS XMM0,[EDI+0BC4h]
007da934  FMUL [EDI+0BB0h]        ; kLatchedYaw
007da942  MOVSS [ESP+2Ch],XMM0    frame=-56 = unit+BC4h
007da94b  FMUL [ESP+2Ch]          ; * unit+BC4h
007da953  FSTP [ESP+2Ch]          frame=-56 = YawSpd*kLatchedYaw*unit+BC4h
007da957  FLD  [ESP+2Ch]
007da95b  FMUL [ESP+30h]          frame=-52  f2
007da95f  FSTP [ESP+24h]          frame=-64
```

Two multipliers that `docs/PLANE_CONTROL_RATE_LAW.md` does not list: `unit+BC4h` at `007DA94B` and
**`f2`, not `f1`**, at `007DA95B`. `frame=-56` keeps the pre-`f2` product and is read again at
`007DAA5C` for the roll coupling, so the two uses share one factor and differ in the last multiply.
`frame=-56` is not written again between `007DA953` and `007DAA5C`.

The `unit+BC4h` read at `007DA92C` happens **before** the relaxation block updates that field
(`007DA9AC` / `007DA9BC` / `007DA9E3`), so the yaw target uses the previous step's value.

### The `unit+BC4h` relaxation, `007DA963`-`007DA9EF` - not a target input

`007DA948 UCOMISS XMM0,XMM1` / `LAHF` / `TEST AH,44h` / `007DA967 JNP` is the MSVC equality idiom:
`AH & 44h` is `40h` only when the compare was equal-and-ordered, which is the one case with odd
parity, so the `JNP` is taken exactly when `unit+BC4h == 1.0f` and skips the block. The second guard
is `step > 0` (`007DA973 COMISS XMM0,XMM2`). The body moves the field toward `1.0` by `0.5*step`
(`00D7A280` = `0.5` as a double) and clamps on both sides. The decompiler's reading matches:
`if ((*(float *)(iVar1 + 0xbc4) != DAT_00d7a24c) && (0.0 < param_2)) { ... }`.

The block writes only `unit+BC4h` and the scratch slot `frame=-80`. It contributes nothing to the
targets in the step that runs it.

### The flag-gated block, `007DA9EF`-`007DAA78`

```
007da9ef  MOV  BL,byte [ESP+13h]  frame=-81  ; 007DA380's flag
007da9f5  JZ   007daa7c           ; flag == 0 -> skip both terms
007da9fe  FSTP ST0                ; pops f1
007daa0e  FLD  [ESP+28h]  / FSIN  / FSTP [ESP+14h]   frame=-80 = sin(unit+C68h)
007daa18  FLD  [ESP+14h]  / 007daa1f FSTP [ESP+38h]  frame=-44 = sin(unit+C68h)
007daa31  FLD  [ESP+34h]  / FCOS  / FSTP [ESP+28h]   frame=-60 = cos(unit+C64h)
007daa3b  FLD  [ESP+24h]          frame=-64  Y
007daa42  FLD  [EAX+1B8h]         ; SlideRatio
007daa48  FMUL [EAX+1B0h]         ; YawSpd
007daa4e  FMUL [ESP+38h]          ; sin(unit+C68h)
007daa52  FMUL [ESP+28h]          ; cos(unit+C64h)
007daa56  FSUBP                   ; de e9 -> ST1 = Y - product, pop
007daa58  FSTP [ESP+24h]          frame=-64
007daa5c  FLD  [ESP+2Ch]          frame=-56  yawRaw
007daa60  FLD  [ESP+20h]          frame=-68  f1
007daa64  FLD  ST0                ; d9 c0
007daa66  FMULP ST2               ; de ca -> yawRaw*f1
007daa68  FLD  [EAX+1B4h]         ; YawRollRatio
007daa6e  FMULP ST2               ; de ca -> yawRaw*f1*YawRollRatio
007daa70  FLD  [ESP+18h]          frame=-76  R
007daa74  FADDP ST2,ST0           ; de c2 -> sum
007daa76  FXCH                    ; d9 c9
007daa78  FSTP [ESP+18h]          frame=-76 = R + yawRaw*f1*YawRollRatio   (leaves f1 on ST0)
```

`DE E9` is `FSUBP ST(1),ST(0)` (`ST1 = ST1 - ST0`, pop), so the slide term is **subtracted** from the
yaw working value. `DE C2` is `FADDP ST(2),ST(0)`. The decompiler agrees on both lines:

```c
local_40 = local_40 - *(float *)(iVar1 + 0x1b8) * *(float *)(iVar1 + 0x1b0) * local_50 * local_3c;
local_4c = local_4c + *(float *)(iVar1 + 0x1b4) * local_44 * local_38;
```

`007DA9FE` pops `f1` and `007DAA60` pushes it again from `frame=-68`, so both paths reach `007DAA7C`
with the x87 stack holding exactly `f1`. Counting every push and pop from `007DA732` confirms it:
the stack is empty at `007DA8D9`, one deep (`f1`) from `007DA912` onward, and empty again after
`007DAAB9` - which independently confirms the premise of the rate-law doc's `ST4` argument, that the
stack is empty when `007DAB52` starts pushing targets.

### The yaw negation, `007DAA7F`-`007DAA97`

```
007daa7f  FLD  [EAX+1C0h]         ; PitchAccel
007daa85  MOVSS XMM0,[00D7A208]   ; -0.0f
007daa8d  SUBSS XMM0,[ESP+24h]    frame=-64
007daa93  FMUL ST1                ; d8 c9 -> ST0 = PitchAccel * f1
007daa97  MOVSS [ESP+5Ch],XMM0    frame=-8 = -(frame=-64)
```

The yaw target is the negation of everything built above it. Anything wiring this axis has to carry
that sign.

### `007DAA9D`-`007DAB4A` - built in this span, but not target inputs

For completeness of the span: `frame=-24 = PitchAccel(class+1C0h) * f1`, `frame=-20 = -f2 *
YawAccel(class+1C4h)`, `frame=-16 = f1 * RollAccel(class+1BCh)`; then `007DAABD CALL 007D99C0`
(`ForwardSpeed`) divided by `StallSpd(class+184h)`, and two `00419010` calls -
`(SpdMultipliers/StallOffPitch -> 1.0, SpdMultipliers/StallOnPitch -> 0.0)` evaluated at `unit+C64h`,
and `(SpdMultipliers/StallRangeMin -> 1.0, SpdMultipliers/StallRangeMax -> 0.0)` evaluated at the
speed ratio - whose product lands in `frame=-44` at `007DAB46`. None of these slots is read by
`007DAB52`, `007DAB5F` or `007DAB6C`; they belong to the rate, not the target. The tuning offsets are
`00F87314 = +234h`, `00F87318 = +238h`, `00F8730C = +22Ch`, `00F87310 = +230h` under the same
`00F872F0 <-> +210h` mapping.

## What is not established

* **`unit+838h`.** Added to `kLatchedRoll` at `007DA738` to form the roll base, and nothing in this
  repository names it for a plane unit. The three `+838h` hits in `include/` are a ship throttle ring,
  a submarine timer and a taxi vector capacity - different classes, and none of them applicable. Its
  provenance dead-ends here: a producer scan was not run.
* **`unit+C36h` and `unit+C37h`.** Unrecorded bytes. `C36h` gates the spin term and `C37h` chooses its
  sign. The "spin death" and "spin direction" readings are inferred from `DeadMeat/SpinRollSpd` and
  from the Lua comment's *"valamelyik iranyba"*; neither byte's writer was traced.
* **Flight state 6** at `unit+900h`, the gate on the bank-angle roll factor. `plane_flight.hpp` maps
  `+900h` to arms 4 and 5 only.
* **What `f1` and `f2` are.** `007DA380`'s mode-0 arm writes the same scalar to both, and that scalar
  is `007D9A70(ctl)`'s return - a function with no recovered name whose callees are `ForwardSpeed`
  and `InterpolateClamped`. Modes 1 and 2 (`007DA542`, `007DA3D0`) are unread, so outside free flight
  `f1 != f2` is possible and the expressions above keep them distinct for that reason.
* **The flag's value outside mode 0.** In free flight it is 1, so the slide term and the yaw-roll
  coupling both apply. On the ground (`ctl+FCh == 1`, mode 1) the flag comes from `007DA542`, unread -
  so whether the coupling term is added back onto a roll target that `007DA8E5` has just zeroed is
  **open**.
* **The authored values** of `DeadMeat/RollMulTime`, `RollMul` and `SpinRollSpd` are this
  installation's `planeglobals.lua`, not provably retail (see the caveat above). The key names and the
  offsets are from the executable.
* **Units.** No claim is made that the targets are rad/s. They are whatever `ctl+48h..50h` are, and
  `docs/PLANE_ANGULAR_VELOCITY.md` owns that question.
* **Behaviour.** Nothing was run. This is a recovered set of expressions, not a validated one.

## Corrections to published claims

**1. `docs/PLANE_CONTROL_RATE_LAW.md`, section "`unit+BC4h` scales the yaw target, not the step", is
wrong about how it does so.** It states: *"`007DAA18` reads it out and `007DAA1F` stores it to
`frame=-44`, where `007DAA4E` multiplies it into the yaw target alongside `YawSpd +1B0h` and
`cos(+0C64h)`. That is its only consumer."* Every part of that path is wrong:

* `007DAA14 FSTP [ESP+14h]` **overwrites** `frame=-80` with `sin(unit+C68h)` before `007DAA18` reads
  it, so `frame=-44` holds the sine, not the `+BC4h` value. The decompiler names the same slot
  `local_50` and assigns it from `fsin(...)`.
* The `+BC4h` chain ends at `007DA9D0`; its result goes to the **field** `unit+BC4h`, not onward.
* `007DAA4E`'s term is `SlideRatio(class+1B8h) * YawSpd * sin(bank) * cos(pitch)` and it is
  **subtracted** (`007DAA56 FSUBP`, `DE E9`), not multiplied in.
* `unit+BC4h`'s real consumer is `007DA92C`/`007DA94B`, through `frame=-56`, 280 bytes earlier.

The section's *conclusion* - that `+BC4h` scales the yaw target and not the step - survives. Its
mechanism, address, slot and arithmetic do not. Because the value is 1.0 in this host, nothing
downstream would have exposed it.

**2. `docs/PLANE_CONTROL_RATE_LAW.md`, section "The accel term": `frame=-16` is `f1 * class+1BCh`,
not `PitchAccel * f1 * class+1BCh`.** `007DAA9D FSTP` pops the `PitchAccel * f1` product, leaving
`f1` as `ST0`, and `007DAAB3 FMUL [EAX+1BCh]` multiplies that. The decompiler agrees:
`local_10 = local_44 * *(float *)(iVar1 + 0x1bc);`.

**3. The Ghidra plate comment on `007DA710` has the ground gate backwards.** It says *"the roll term
is discarded when controller+FCh is not 1 (007DA8D9)"*. `007DA8E3 JNZ 007DA8EB` **skips** the
zeroing when the mode is not 1; the roll target is zeroed when `ctl+FCh == 1`. Since `PLANE_FLIGHT.md`
records `== 1` as on-the-ground, the comment inverts a gate whose correct sense is the sensible one.
Not corrected in Ghidra - this packet is read-only.

**4. Object attribution: the latched block is on the unit, not the controller.**
`docs/PLANE_CONTROL_RATE_LAW.md` writes `kLatchedPitch ctl+BB4h` and tabulates `kLatchedYaw +BB0h`
under a `ctl+` heading. Every one of `+BB0h`, `+BB4h`, `+BB8h`, `+BC4h`, `+838h`, `+900h`, `+5Dh`,
`+C36h`, `+C37h`, `+C3Ch`, `+C64h`, `+C68h` in this function is read off `EDI = [ctl+8h]`, the unit.
`include/bsp/plane_flight.hpp` is right ("The pilot control block, **unit+9E4h**"); the doc's `ctl+`
prefix would send a reader to the wrong object. The same wording reached this packet's brief.

**5. `include/bsp/plane_advance_pose.hpp:39-41` contradicts `include/bsp/plane_flight.hpp:41-43` on
the live control axes, and this packet adds a fourth argument that `plane_flight.hpp` is the right
one.** `plane_advance_pose.hpp` has `kLiveRoll = 0x9E4 / kLivePitch = 0x9E8 / kLiveYaw = 0x9EC`;
`plane_flight.hpp` carries an explicit CORRECTION note for the opposite assignment (`+9E4h` yaw,
`+9ECh` roll) with three independent lines of evidence. The latch `007B9770` copies `+9E4h -> +BB0h`
and `+9ECh -> +BB8h`, and this function multiplies `unit+BB0h` by **`YawSpd` (class+1B0h)** and
`unit+BB8h` by **`RollSpd` (class+1A8h)**. The class constants pair with `plane_flight.hpp`'s naming
and against `plane_advance_pose.hpp`'s. Fixing that header is outside this packet's file scope.

**6. Incomplete rather than wrong.** `docs/PLANE_CONTROL_RATE_LAW.md` describes `007DA926`-`007DA95F`
as *"opens with `YawSpd +1B0h` and multiplies by `kLatchedYaw +BB0h`, landing in `frame=-64`"*. Two
further multipliers are in that span: `unit+BC4h` (`007DA94B`) and `f2` (`007DA95B`). Likewise the
roll axis is described as *"`RollSpd +1A8h` multiplied into `frame=-76` at `007DA7D6`, with
`YawRollRatio +1B4h` added into the same slot at `007DAA70`"* - true, and missing the base
`unit+838h + unit+BB8h`, the `f1` and flight-state-6 factors, the whole out-of-action ramp and spin
term, and the ground zeroing.

What the rate-law doc gets **right** and this trace confirms independently: the three target slots and
their axis pairing, the `NegativePitchRatio` branch condition, the yaw negation at `007DAA8D`, the
`f1`/`f2`/flag ABI of `007DA380`, and the emptiness of the x87 stack at `007DAB52` that its `ST4`
argument depends on.
