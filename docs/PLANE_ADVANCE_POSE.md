# What turns a plane - `007C6500 BSP_PlaneTickElement_AdvancePose` (packet `cc7_plane_advance_pose`)

Read-only analysis of `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. No Ghidra
mutation, no ledger record, no write lock. Every descriptive name is a hypothesis, not a recovered
symbol.

## The answer, in one paragraph

A plane turns because `007D9F60` rotates its pose matrix by the flight controller's **angular
velocity times the step**, as an axis-angle rotation: the axis is `normalize(w * dt)`, the angle is
`|w * dt|`, and `w` is the three floats at `ctl+24h..2Ch` where `ctl` is the flight controller at
`unit+AB0h`. The rotation is realised by `0085E4D0` as `in * L * RotZ(-|w*dt|) * transpose(L)`,
where `L` is a look-at frame built about the axis. A **second** rotation follows it, a yaw about
world `+Y` whose angle is `-(blend * sin(unit+C68h) * cos(unit+C64h) * classDesc+1C8h * step)` -
a bank-driven turn. Then, when `ctl+8Ch > 0`, row 1 is lerped toward world up and the matrix is
re-orthonormalised up-first. Finally the position is advanced and the whole thing is written to
**`unit+74h`**.

## The finding that changes the integration plan

**`007C6500` does not write `unit+674h`.** It reads `unit+674h` and writes `unit+74h`.

That is not a miss in the reading; it is the design, and `docs/TICK_ELEMENT_OVERRIDES.md:99-108`
already records it for the generic unit. `007C6500` is the plane's tick-element slot `+4h` and is
the exact analogue of `00811AB0 BSP_UnitInstance_RunShiftedControllerUpdate`, which "copies the
matrix at `unit+674h` into `unit+74h` and then advances the live matrix by `t`". The commit in the
other direction is a **different slot**: `+0Ch`, `006D1FC0`, which "copies `unit+74h` back into
`unit+674h`". And the same doc records why there are two:

> Wave 1 replays the step from the committed pose and commits the result; the interpolation wave
> replays it with the frame's leftover and does **not** commit, so the rendered pose is
> interpolated while the simulation pose stays on the step grid.

So the loop is: `006D1FC0` commits `74h -> 674h`; the fixed-step tick `007CE040` mutates `674h`
through its three arms and orthonormalises it with `0085DC80` (`docs/PLANE_POSE_COMMIT.md`);
`007C6500` reads `674h`, advances by the step, and writes `74h`.

**For the host this means the advance is not idempotent-safe to call twice against the same
committed pose unless the commit runs between**, and it means a host that keeps one pose per unit
is modelling both `74h` and `674h` in the same field. That is a design decision for the
integrator, not something this packet can settle.

## `007C6500`, the dispatch

Raw block `007C6500`-`007C675B`, `RET 4`, no Ghidra function. `__thiscall(node, float step)` where
`ECX` is `unit+310h`, the scene node, **not** the unit: `007C6538 LEA EBP,[ESI-310h]` recovers the
unit and `007C6716` repeats it. Every offset below is stated against the unit. The bias is the same
one `007CE040` uses and is cross-checked against `docs/PLANE_UNIT_TICK.md`'s `unit+9E0h` and
`unit+908h`.

```
007c6509  XMM0 = [unit+340h] ; COMISS 0.0f ; JBE 007c6526
007c651a  step *= [unit+340h]                      ; a per-unit time scale, only when > 0
007c6528  CMP byte [unit+5Ch],0 / JNZ 007c657f     ; clear -> the early exit below
007c657f  CMP byte [unit+61h],0 / JZ 007c65ce      ; set -> latch four control axes
007c6587..007c65cc   unit+9E8h = [unit+8ACh]  unit+9F4h = 0.0f   unit+9F0h = [unit+8DCh]
                     unit+9FAh = ([unit+9B0h] > 0) unit+9ECh = [unit+8C4h]  unit+9E4h = [unit+894h]
007c65ce  CMP byte [unit+C48h],0 / JZ 007c66ae     ; the control-surface block
007c66ae  EAX = [unit+900h] ; FLD step
007c66bb  if (state == 1 || state == 0) skip 007de540/007de580 on unit+C98h
007c66dd  EAX = [unit+9D4h] ; if (EAX && ([unit+61h] || [EAX+61h])) goto 007c6714   ; blocked
007c66f4  CMP byte [unit+520h],0
007c66fe  LEA ECX,[unit+AB0h]                      ; the flight controller
007c6706  CALL 007d8230   (unit+520h set)          ; translate only
007c670d  CALL 007d9f60   (unit+520h clear)        ; rotate and translate
007c6714  FSTP ST0                                 ; blocked: the step is discarded
007c6716  unit+0C8h = 0 ; unit+10Ch = 0            ; the world-pose cache is invalidated
          walk [unit+48h] through +44h calling 0042ED50 BSP_SceneNode_InvalidateSubtreePose
007c6748  unit->vtable[+1E0h](unit)
007c6754  unit->vtable[+0D8h](unit)
```

The invalidate tail at `007C6716` runs on **every** path that got past `007C6528`, including the
blocked one, so a blocked plane still has its cached world pose thrown away.

`007C6528`'s early exit (`unit+5Ch` clear) does the same invalidate at `007C653E`, then returns
after `unit->vtable[+0D8h](unit)` if `unit+4A4h` is non-zero, or immediately if it is zero.

### The four latched axes (`007C6587`-`007C65CC`)

Gated on `unit+61h` being set. Sources `unit+894h`/`8ACh`/`8C4h`/`8DCh` are copied into the live
pilot control block at `unit+9E4h` (roll), `9E8h` (pitch), `9ECh` (yaw), `9F0h` (throttle), with
`9F4h` zeroed and the byte at `9FAh` set from `[unit+9B0h] > 0`. This is a **third writer** of the
control block, alongside the two `include/bsp/plane_flight.hpp:12-15` names as candidates
(`007CA509` and `007D1333`). It is a copy from a parallel block at `unit+894h + 18h*k`, not an
input source; where that block comes from was not read.

### The control-surface block (`007C65DA`-`007C66AE`)

Gated on `unit+C48h`. Four arrays of node pointers, each with a count, each element handed to
`007DE2E0` with one signed axis:

| count | array | value | listing |
| --- | --- | --- | --- |
| `unit+DE8h` | `unit+DCCh` | `+unit+9E4h` roll | `007C65E4`-`007C6607` |
| `unit+DE4h` | `unit+DBCh` | `-unit+9E8h` pitch | `007C6613`-`007C6643` |
| `unit+DDCh` | `unit+D9Ch` | `-unit+9ECh` yaw | `007C664F`-`007C6683` |
| `unit+DE0h` | `unit+DACh` | `+unit+9ECh` yaw | `007C668F`-`007C66AC` |

The negations are `SUBSS` from the `-0.0f` at `00D7A208`. Two yaw groups with opposite signs is
what a split rudder or a pair of mirrored surfaces needs. **This is cosmetic**: it drives animated
nodes, it does not touch the pose. `007DE2E0` was not read.

### The arm selector, `unit+520h`

`007C66F4`/`007C6704`: set takes `007D8230`, clear takes `007D9F60`. The only documented writer is
`00953CC0` (`docs/TICK_ELEMENT_OVERRIDES.md:110-116`), which **clears** it when
`00927F10(unit, unit+1ACh)` answers false, and calls `unit->vtable[+1D8h](step)` while it is set.
**What the flag means was not established.** What is certain is the consequence: while it is set a
plane does not rotate at all.

## `007D8230`, the translate-only arm

`__thiscall(ctl, float step)`, `RET 4`, body `007D8230`-`007D8327`.

```
007d8239  UCOMISS step, [00D7A218] ; LAHF ; TEST AH,44h ; JNP 007d8323   ; step == 0.0f -> return
007d824d  unit = ctl->+8h
007d8252  temp = *(4x4*)(unit+674h)                     ; MOVSD.REP, ECX = 10h, 64 bytes
007d826b  004134F0(unit+74h, temp)                      ; dead, see below
007d827b  v  = ctl->+18h..20h * step
007d829a  p  = temp.row3 + v
007d82be  p += unit->+810h..818h * step
007d8304  temp.row3 = p
007d831c  004134F0(unit+74h, temp)
```

**No rotation of any kind.** The basis rows go from `unit+674h` to `unit+74h` untouched. The write
at `007D826B` is dead: nothing reads `unit+74h` between it and `007D831C`, which writes the same
64 bytes again. That is the same redundant-double-pass shape `docs/ENTITY_LOCAL_MATRIX.md:60-66`
records for `00904AEF`.

## `007D9F60`, the rotate-and-translate arm

`__thiscall(ctl, float step)`, `RET 4`, Ghidra body `007D9F60`-`007DA377`. `EBX` = `ctl`,
`ctl->+8h` = the unit, `ctl->+0Ch` = the class descriptor.

```
007d9f6e  temp = *(4x4*)(unit+674h)                    ; the same 64-byte copy
007d9f87  004134F0(unit+74h, temp)
007d9f95  if (step == 0.0f) return                      ; the same UCOMISS/JNP idiom

; --- rotation 1, the angular-velocity integration ---
007d9fa6  w = (ctl+24h, ctl+28h, ctl+2Ch) * step ; the 4th stack slot gets FLD1's 1.0f
007d9ff6  0085E4D0(temp, unit+674h, w, 1.0f)

; --- rotation 2, the bank-driven yaw ---
007da00c  c = cos(unit+C64h)                            ; FCOS, on a float spill
007da037  s = sin(unit+C68h)                            ; FSIN
007da03d  r = ([00E0C978] && [[00F88C30]+124h]) ? 008E6430(0Dh, unit) : 1.0f
007da0a9  f = InterpolateClamped([00CE3854], 0.0f, [00CE3850], 1.0f, ctl+7Ch)
007da0ae  angle = -(f * s * c * classDesc+1C8h * r * step)
007da0ed  00B646E0 BuildRotationY(rotY, &angle)
007da10d  temp = temp * rotY                            ; 00413920, ECX = left

; --- the up levelling ---
007da11c  if (ctl+8Ch > 0.0f) {
007da148     a  = 0042D0D0(ctl+80h, temp, normalize = false)
007da179     s2 = min(ctl+8Ch * step, 1.0f)
007da14d     temp.row1 += ((0,1,0) - a) * s2
007da20c     0085DAD0(temp)
             }

; --- the position, identical to 007D8230's ---
007da218  p  = (unit+6A4h,+6A8h,+6ACh) + ctl+18h..20h * step
007da26b  p += unit+810h..818h * step
007da2b1  if (ctl+FCh == 1 && unit+BF4h && unit+3Ch) { the attached branch, unread }
007da33a  temp.row3 = p
007da369  004134F0(unit+74h, temp)
```

`unit+6A4h` is `unit+674h + 30h`: row 3 of the committed pose. So both arms advance the position
from the **committed** translation, not from whatever the working copy currently holds.

The bank-yaw shape - yaw rate proportional to `sin(bank) * cos(pitch)` - is the textbook
coordinated turn, which is the direct evidence that in this game a plane turns **because it
banks**, not because yaw input is applied to the heading. `unit+C64h` and `unit+C68h` are read as
pitch and bank respectively from that shape and from `docs/PLANE_FLIGHT.md`'s use of `unit+0C64h`
in the rate law; the identification is a hypothesis, not proven here.

## `0085E4D0`, the axis-angle rotation

`__fastcall(float* out /*ECX*/, const float* in /*EDX*/, float w[3], float scale)`, `RET 10h`,
body `0085E4D0`-`0085E87D`. Two exits, `0085E86E` and `0085E87D`.

```
0085e4f0  len2 = (w.x*w.x + w.y*w.y) + w.z*w.z
0085e50a  len  = (len2 > 1e-10) ? sqrt(len2) : 0.0f            ; 00CE3820, double 1e-10
0085e53c  if (1e-8 > len) goto 0085e871                        ; 00D7A350, double 1e-8; writes nothing
0085e54a  w /= len                                             ; FDIV, in place on the caller's stack
0085e58f..0085e6dd   up = (1,0,0) or (0,0,1)                   ; y is always 0
0085e6f6  L = 00B63F10 BuildLookAt(eye = [00F87574], target = &w, up)
0085e708..0085e7c4   Lt = transpose(L)                         ; six element swaps, the full 4x4
0085e719  theta = -(len * scale)                               ; 0085E750 FCHS
0085e7ca  Rz = 00B64780 BuildRotationZ(theta)
0085e818  out = ((in * L) * Rz) * Lt                           ; three chained 00413920
0085e833  out.row3 = in.row3                                   ; restored from in's own copy
0085e85f  0085D3D0(out)                                        ; not read
```

Three things settle the composition. `00413920`'s convention is `ECX` = left, stack `(dst, right)`
(`docs/ENTITY_LOCAL_MATRIX.md:82`), and `0085E81D`/`0085E824 MOV ECX,EAX` chain each result into
the next left. The six swaps at `0085E730`-`0085E7C4` are `(1,4) (2,8) (3,12) (6,9) (7,13) (11,14)`,
which is exactly a 4x4 transpose. And the pushes resolve the three buffers as `L` at `S+9Ch`,
`Rz` at `S+DCh` and `Lt` at `S+1Ch`.

`0085E4D0` is called from three sites (`007C18B0`, `007D9F60`, `00901C20
BSP_GunBot_InterceptSolution`), so it is a general rotate-matrix-about-axis, not plane code.

Post-multiplication in the row-vector convention means the rotation acts in **world** space, after
`in`. So `ctl+24h..2Ch` is a **world-space** angular velocity, the exact counterpart of
`ctl+18h..20h`, which `007D8230` and `007DA218` both add straight to a world position.
`docs/PLANE_FREE_FLIGHT_PHYSICS.md:44` independently calls `ctl+18h` "the **world** velocity".

### `00B64780 BuildRotationZ`, read here

```
00b647c0  m[0] = cos   00b647d5  m[1] =  sin
00b647e4  m[4] = -sin  00b647c4  m[5] = cos     (the -sin is -0.0f - sin, from 00D7A208)
00b647fd  m[10] = 1    00b64816  m[15] = 1      everything else 0
```

Under the row-vector convention that turns `+X` toward `+Y`, i.e. `+theta` about `+Z` in the
frame's own handedness.

## `0085DAD0`, the up-first orthonormaliser - and the convention check

Body `0085DAD0`-`0085DC7C`, `__fastcall(float* m /*ECX*/)`, plain `RET`. It ends one byte before
`0085DC80`, the routine `docs/PLANE_POSE_COMMIT.md` recovered, and it is that routine's mirror:

| | `0085DC80` | `0085DAD0` |
| --- | --- | --- |
| authority | row 2, forward | row 1, up |
| projected off it | row 1 | row 2 |
| derived last | row 0 = `row1 x row2` | row 0 = `row1 x row2` |
| degenerate branch | yes, `\|dot\| > 0.999f` | **none** |
| used after | the flight arms, which write forward | the levelling, which writes up |

```
0085dad7  EDI = m+10h ; 0085dadc 00419440 ; 0085db06..0085db3d   row1 = normalize(row1)
0085db0d  ESI = m+20h ; 0085db40..0085db54  d = dot(row2, row1)
0085db58..0085db91   row2 -= row1 * d
0085db94..0085dbf6   row2 = normalize(row2)
0085dbc0  PUSH ESI (row2) / 0085dbc5 MOV EDX,EDI (row1) / 0085dbf9 CALL 004F9B30
0085dbfe..0085dc17   row0 = row1 x row2
0085dc1c..0085dc75   row0 = normalize(row0)
```

**This does not contradict the row-2-is-forward convention, and I checked specifically because the
brief asked.** Both orthonormalisers end with `row0 = row1 x row2`, the same handedness. They
differ only in which row the caller has just written and therefore wants preserved, which is the
correct behaviour in both cases: `007CE040`'s arms write forward, so `0085DC80` holds forward;
`007DA14D` writes up, so `0085DAD0` holds up. Nothing here asks for a different convention.

## What could not be established

- **The sense of the axis-angle rotation.** The magnitude `|w * dt|` and the axis
  `normalize(w * dt)` are certain, and so is the `FCHS` at `0085E750`. Whether the net rotation is
  `+|w*dt|` or `-|w*dt|` about the axis in right-hand terms turns on **one** unread fact: whether
  `00B63F10 BuildLookAt` is the left- or the right-handed look-at. The eye is settled - the global
  at `00F87574` is the zero vector (its twelve bytes read as zero, it lies past `.data`'s raw size
  so it is BSS per `docs/COMMAND_EXECUTION.md:61`, `docs/AI_PLANNER_TAILS.md:155` names it "the
  zero vector", and `007D7C00` uses it to clear six accumulators) - so the look-at's forward is
  exactly the axis. If `00B63F10` is left-handed, which the left-handed pose convention and the
  D3D9 renderer make likely but which was **not verified**, the `FCHS` and the frame's handedness
  cancel and the net is `+|w*dt|` in the right-hand sense. `00B63F10` is already reconstructed
  (`src/camera_look_at.cpp`), so this is cheap to close and it should be closed before anyone
  trusts a turn direction.
- **`0085D3D0`**, the fixup `0085E85F` runs on the rotated matrix. Not read. The reconstruction
  stops before it.
- **`007DA2B1`-`007DA2FE`, the attached branch** (`ctl+FCh == 1` and `unit+BF4h` and `unit+3Ch`):
  it refreshes the parent's world pose, re-expresses the position through it with `00414D10`,
  calls `006BC530`, and clamps against `classDesc+1FCh` through `007D7D70`. Not read. The rule
  raises a flag instead of guessing.
- **`unit+520h`'s meaning.** Only its effect is established.
- **`unit+810h`**, the second world velocity both arms add. Not traced to a writer.
- **`008E6430(0Dh, unit)`**, the scalar that multiplies the bank-yaw angle when two globals pass.
  Not read; treated as an input defaulting to `1.0f`.
- **`007DE2E0`**, the control-surface driver. Not read; the packet establishes only what is handed
  to it.
- **The up-reference predicate in `0085E4D0`.** `0085E58F`-`0085E6BB` builds the absolute values of
  the axis components and compares them; only the first two stores (`|x|` at `[ESP+14h]`, `|y|` at
  `[ESP+10h]`) were read. It is proven that the result is `(1,0,0)` or `(0,0,1)` with `y` always
  zero, because `XMM6` is zeroed at `0085E55E` and never written again. It cannot change the
  result: two look-at frames about one axis differ by a rotation about their own `Z`, and that
  commutes with the `RotZ` between them. It is a degeneracy guard.
- **`unit+C64h`/`unit+C68h` as pitch and bank.** A hypothesis from the `sin`/`cos` shape, not
  proven.
- **Exact float reproduction.** The reconstruction models the x87 spills where the native spills
  and uses `double` where the native keeps a sum on the stack and rounds once. `007DA0AE`'s gain
  chain stays on the x87 stack for five multiplies and the reconstruction rounds at each, so that
  term can differ by a few ulp. No bit-exact differential test was run.

## Verification

| claim | status |
| --- | --- |
| `007C6500`'s full 169-instruction listing | **exported** and read, all of it |
| `007D8230`'s full 71-instruction listing | **exported** and read, all of it |
| `007D9F60`'s 272 instructions | **exported**; read except `007DA2B1`-`007DA2FE` |
| `0085E4D0`'s composition, guards, transpose and exits | **exported** and read except `0085E58F`-`0085E6BB`'s predicate |
| `0085DAD0`'s full 145-instruction listing | **exported** and read, all of it |
| `00B64780`'s matrix | **exported** and read |
| the rule in `src/plane_advance_pose.cpp` | **reconstructed** |
| `cl /W4 /WX /std:c++17` Win32, and `./scripts/build.ps1` green with `reconstructed_math` passing | **build-tested** |
| the pure functions listed below | **fixture-tested**, by a throwaway probe, not a committed test |
| the two `AdvanceMatrixOps` arms end to end | **not exercised**. They need the six native primitives bound, which this packet does not do |
| anything at run time | **not validated**. Nothing was run against `bsp_game` or the retail binary |

The probe (scratchpad only; AGENTS.md asks for as few new tests as possible and these are pure
functions with no regression surface yet) checked: all four exits of
`select_advance_arm_007c6500` plus the `unit+340h` scaling and the case where `unit+9D4h` is
present but does not block; `0085DAD0` returning an orthonormal frame that preserves row 1's
direction, satisfies `row0 = row1 x row2` and leaves row 3 alone; the `1e-10` floor rejecting a
squared length exactly at the floor (the test is strict) and the `1e-8` gate admitting `|w| = 2e-5`;
the position law; `translate_only_007d8230` moving only the translation and returning the committed
pose unchanged for a zero step; `level_blend_007da179` both below and above its clamp; and
`bank_yaw_angle_007da0ae` giving `-0.05` for a 30-degree bank at gain 2 and **exactly zero at wings
level** - the direct confirmation that in this model a plane turns only while it is banked.

The up-levelling was checked too: at blend `0.25` a 30-degree banked up axis rises from
`0.866025` to `0.923004` toward world up, and the frame stays orthonormal afterwards.

## Wiring contract

### What is pure and ready

`select_advance_arm_007c6500`, `control_surface_drive_007c65da`,
`orthonormalize_up_first_0085dad0`, `rotation_axis_angle_0085e4d0`, `advance_position_007da218`,
`bank_yaw_angle_007da0ae`, `level_blend_007da179`, `apply_up_levelling_007da14d` and
`translate_only_007d8230` are complete pure functions over explicit inputs.

### What needs binding

`rotate_about_axis_0085e4d0` and `rotate_and_translate_007d9f60` take an `AdvanceMatrixOps`
reference. Its six members are **not stubs and not inventions**: every one is a native routine
already reconstructed elsewhere in this repo, and the interface exists so this packet records the
composition order without re-deriving them.

| member | native | existing reconstruction |
| --- | --- | --- |
| `build_look_at_00b63f10` | `00B63F10` | `src/camera_look_at.cpp` |
| `build_rotation_z_00b64780` | `00B64780` | `src/native_particle_axial_loading.cpp` |
| `build_rotation_y_00b646e0` | `00B646E0` | `src/native_particle_axial_loading.cpp` |
| `multiply_00413920` | `00413920` | convention in `docs/ENTITY_LOCAL_MATRIX.md:82` |
| `transform_direction_0042d0d0` | `0042D0D0` | `src/material_effect_plane.cpp` |
| `interpolate_clamped_00419010` | `00419010` | `src/unit_rudder.cpp` |

### Where it goes in the host

**Not in the free-flight arm.** `007C6500` is a separate tick element from `007CE040`, and
`src/plane_flight.cpp`'s `PlaneMotionHost` models `007CE040` only. The advance is a new call, at
the point in the frame where the unit's tick-element slot `+4h` runs - before the draw, after the
fixed-step arms.

The minimum that unfreezes a heading, given `unit_.motion.pose_row0/1/2` and the existing
`plane_world_velocity`:

```cpp
bsp::PlaneAdvanceInputs in;
// rows into in.committed_pose.m[0..2], [4..6], [8..10]; position into m[12..14]
in.step = scaled_step;                       // select_advance_arm_007c6500 does the unit+340h scale
in.angular_velocity = /* ctl+24h..2Ch */;    // NOT YET PRODUCED BY THE HOST - see below
in.linear_velocity  = /* ctl+18h..20h */;    // this is plane_world_velocity
in.eye_00f87574 = {0, 0, 0};
const bsp::PlaneAdvanceOutput out = bsp::rotate_and_translate_007d9f60(in, ops);
// out.live_pose rows 0/1/2 back into pose_row0/1/2, row 3 into motion.position
```

### The gap this packet does not close

`ctl+24h..2Ch`, the angular velocity, is the **input** to everything above, and nothing in this
packet produces it. `docs/PLANE_FLIGHT.md` puts its writer in the rate law `007DA710` /
`007DB680`, and `docs/PLANE_FREE_FLIGHT_PHYSICS.md:52` records that `007D8470`'s integration tail
`007D8624`-`007D904E` was never read - that tail is the obvious candidate for where a body-frame
angular acceleration becomes `ctl+24h`. **Until something writes `ctl+24h..2Ch`, wiring this rule
in leaves the heading exactly as frozen as it is now**, because `w` will be zero and
`0085E4D0` will take its `0085E871` exit every step.

So the honest ordering for the integrator is: this rule is the consumer, and the next packet is
`007D8470`'s integration tail, the producer. The bank-driven yaw at `007DA0AE` is the one turn
term here that needs **no** new producer - it runs off `unit+C68h`, `unit+C64h` and
`classDesc+1C8h` - so if one turning term is wanted before the producer lands, that is the one
that works standalone.

## Follow-up

- `007D8470`'s integration tail `007D8624`-`007D904E`: the producer of `ctl+24h..2Ch`. The real
  next packet.
- `00B63F10`'s handedness, to fix the sign of the axis-angle rotation. Cheap.
- `0085D3D0`, and `007DA2B1`-`007DA2FE`'s attached branch.
- `007C6587`-`007C65CC` is a third writer of the pilot control block at `unit+9E4h`, from a block
  at `unit+894h`. `include/bsp/plane_flight.hpp:12-15` lists that contract as unread and names two
  other candidates; this is a third.
