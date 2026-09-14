# The rotation sense, and where the angular velocity comes from (packet `cc7_plane_angular_velocity`)

Read-only analysis of `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. No Ghidra
mutation, no ledger record, no write lock. Every descriptive name is a hypothesis, not a recovered
symbol. This packet closes the two items `docs/PLANE_ADVANCE_POSE.md` left open.

## The two answers

1. **The sense.** `0085E4D0` rotates the pose by **`-|w*dt|`** about `normalize(w)` - that is, by
   `|w*dt|` **about `-w`**. The pose turns *opposite* to the vector at `ctl+24h..2Ch`. This is
   proved below from four listings and then confirmed numerically two ways.
2. **The producer.** `docs/PLANE_ADVANCE_POSE.md` named `007D8470`'s tail as the candidate
   producer of `ctl+24h..2Ch`. **That was wrong.** `007D8470` integrates the *linear* body
   velocity and never touches an angular quantity; it cannot even reach `ctl`. The real producer
   is `007D9C80`, from a **body-frame** angular velocity at `ctl+48h..50h` that
   `007DA710 BSP_PlaneFlight_ControlRateLaw` writes at `007DAD01`. The chain is complete and
   nothing in it is missing.

## Part 1: the rotation sense

### The four primitives, read rather than assumed

`docs/PLANE_ADVANCE_POSE.md` took these as a contract because the existing in-repo
reconstructions are inline-asm transcriptions. All four were read from the listing here.

**`00413920 BSP_Matrix_Multiply4x4`** - `__fastcall(left ECX, stack: dst, right)`. `00413923`
takes `right` from `[ESP+48h]` and `00413927`..`0041397B` assembles `dst[0]` from
`left[0]*right[0] + left[1]*right[4] + left[2]*right[8] + left[3]*right[12]`, which is
`sum_k left[0][k] * right[k][0]`. The plain row-major product `dst = left * right`.

**`00B64780 BSP_Matrix_BuildRotationZ`** - `__fastcall(m ECX, const float* angle EDX)`.

```
00b647c0  m[0] = cos    00b647d5  m[1] =  sin
00b647e4  m[4] = -sin   00b647c4  m[5] = cos      (-sin is -0.0f - sin, from 00D7A208)
00b647fd  m[10] = 1     00b64816  m[15] = 1       everything else 0
```

**`00B646E0 BSP_Matrix_BuildRotationY`** - the same shape about Y.

```
00b64731  m[0] = cos    00b64724  m[2] = -sin
00b64753  m[8] = sin    00b6475d  m[10] = cos
00b64744  m[5] = 1      00b64776  m[15] = 1
```

Both are the **transpose** of the textbook column-vector rotation, which is what the row-vector
convention needs: `v * RotZ(t)` sends `+X` toward `+Y`, and `v * RotY(t)` sends `+Z` toward `+X`.
Each is therefore a right-hand rotation by `+t` about its axis.

**`00B63F10 BSP_Matrix_BuildLookAt`** - `__fastcall(m ECX, eye EDX, stack: target*, up.x, up.y,
up.z)`, `RET 10h`. It is **exactly `D3DXMatrixLookAtLH`**:

```
00b63f1d  up = normalize(up)                       ; the up argument, first
00b63f87  FLD [EAX] / 00b63f89 FSUB [ESP+30h]      ; target - eye, NOT eye - target
00b63fa7..00b63fef   zaxis = normalize(target - eye)
00b64117  PUSH EAX (zaxis) / 00b64118 LEA EDX (up) / 00b64120 CALL 004F9B30
          xaxis = normalize(up x zaxis)
00b6415d  PUSH ECX (xaxis) / 00b64162 LEA EDX (zaxis) / 00b64198 CALL 004F9B30
          yaxis = normalize(zaxis x xaxis)
00b641d7  m[0] = x.x   00b641e5 m[2] = z.x   00b6424d m[1] = y.x     ; COLUMNS
00b641f4  m[4] = x.y   00b64201 m[6] = z.y   00b64256 m[5] = y.y
00b6421e  m[8] = x.z   00b6423f m[10] = z.z  00b6425d m[9] = y.z
00b64290  FCHS / FSTP [ESI+30h]                    ; m[12] = -dot(xaxis, eye)
```

Two facts carry the whole result. **`zaxis = target - eye`** makes it left-handed, and the axes go
into the **columns**, which is what makes `v * L` the view transform.

### The derivation

`0085E4D0` computes `out = ((in * L) * RotZ(theta)) * transpose(L)` with `theta = -|w| * scale`
and `scale = 1.0f` from `007D9F60`, `L = LookAt(eye = [00F87574], target = normalize(w), up = ...)`.
`00F87574` is the zero vector, so **`L`'s `zaxis` is exactly `normalize(w)`**.

1. Columns are the axes, so `(u * L)_k = dot(u, axis_k)`. With `zaxis = w_hat`, the axis maps to
   view `(0,0,1)`: **view `+Z` is `+w_hat`, not `-w_hat`.** This is the step a right-handed
   look-at would have flipped.
2. `cross(xaxis, yaxis) = cross(x, cross(z, x)) = z` for an orthonormal triple, so
   `(xaxis, yaxis, zaxis)` is **right-handed** under the standard cross formula. `00B64198`'s
   operand order `(zaxis, xaxis)` is what makes this so.
3. `v * RotZ(theta)` is therefore a right-hand rotation by `+theta` about `+w_hat`.
4. `eye = 0` makes `L`'s translation row zero, so `transpose(L)` is exactly `L^-1`.
5. Post-multiplication in the row-vector convention applies the rotation in **world** space, after
   `in`. Each row of `in` - a body axis expressed in world - is rotated by it.

So the net is a right-hand rotation by `theta = -|w|` about `+w_hat`. **The pose turns opposite to
`w`.**

### Confirmed numerically, two independent ways

`src/plane_angular_velocity.cpp` implements the four primitives from the formulas above, and the
packet's probe composes them exactly as `0085E4D0` does.

- **Against a primitive read from a separate listing.** For `w = (0, t, 0)` the whole four-matrix
  composition - look-at, transpose, `RotZ`, three products - equals `RotY(-t)` with
  `max|difference| = 0.000e+00`, bit for bit. `00B646E0` was read independently of `00B63F10`, so
  this cross-validates the look-at, the transpose and the multiply order all at once. (The
  agreement is exact rather than approximate because for `w` along `+Y` the look-at comes out as a
  signed permutation matrix, so no rounding occurs.)
- **Against the closed form.** For a general axis `(0.12, -0.25, 0.07)` and a yawed starting pose,
  the composition matches a Rodrigues rotation of `-|w|` to `1.788e-07`, while the same rotation
  of `+|w|` is wrong by `3.779e-01`. The sign is not within float noise of ambiguous.
- **Stated physically.** Starting from heading `0` with `w = +0.5` about world `+Y`, the new
  heading is `-0.500000 rad`.
- **The look-at itself.** `normalize(w) * L = (0, 0, 1)` exactly, confirming the left-handed
  mapping in step 1.

### What this means for a host

Do not apply `ctl+24h..2Ch` as a right-hand angular velocity. Either negate it, or use
`rotate_about_axis_equivalent`, which bakes the proven sign in. Whether the field "is" an angular
velocity or a negated one is a naming question this packet cannot settle; the *relationship* is
exact and is what matters.

`docs/PLANE_ADVANCE_POSE.md` guessed that if `00B63F10` were left-handed "the `FCHS` and the
frame's handedness cancel and the net is `+|w*dt|`". It is left-handed **and they do not cancel** -
left-handedness is precisely what preserves the `FCHS`. That doc's speculation was wrong in both
halves and is superseded by this one; it was correctly labelled as unverified.

## Part 2: `007D8470`'s tail is not the producer

`007D8470 BSP_PlaneDynamics_IntegrateStep`, `__thiscall(dyn, step, bodyToWorld, worldToBody)`,
body `007D8470`-`007D904E`, 957 instructions. **`RET 0Ch`, not `RET 10h`** - the ledger record and
`docs/PLANE_FREE_FLIGHT_PHYSICS.md` both say `RET 0x10`; `007D903E` and `007D904C` are both
`RET 0xc`, and `007D8476 MOV ESI,[ESP+54h]` resolves to the third of three stack arguments. A
ledger correction, not applied here.

### It only ever writes `dyn`

`EDI = dyn` from `007D847B` and is never reassigned. Every non-stack store in the function was
enumerated, and every destination base resolves to `dyn`:

| base | set at | resolves to |
| --- | --- | --- |
| `EDI` | `007D847B MOV EDI,ECX` | `dyn` |
| `EBX` | `007D8491 LEA EBX,[EDI+1Ch]` | `dyn+1Ch`. Reassigned to the matrix at `007D883D`, and **no store through `EBX` occurs after that point** |
| `EDX` | `007D84E6 LEA EDX,[EDI+40h]` | `dyn+40h` |
| `EBP` | `007D84B9 LEA EBP,[EDI+4]`, then `007D8C6E LEA EBP,[EDI+4Ch]` | `dyn+04h`, `dyn+4Ch` |
| `ESI` | `007D8476` the matrix argument, then `007D85F7 LEA ESI,[EDI+64h]` | `dyn+64h`; the only stores through it are after `007D85F7` |
| `EAX` | `007D873D`/`007D8742 LEA EAX,[ESI+4h/8h]` | a loop cursor over `dyn+64h..6Ch`; one store, at `007D874B` |

`dyn` is `ctl->+10h`, a separate `operator new(0D0h)` allocation (`007D7ECA`,
`include/bsp/plane_flight.hpp` `kSubObject`). There is no back-pointer dereference anywhere in the
body, so **`007D8470` cannot write `ctl` at all**, let alone `ctl+24h`.

### What it actually does

Its only callees are `00419510` (normalize), `0042D0D0` (transform direction) and `00BF7030`
(sqrt): no cross product, no matrix builder, no torque.

```
007d8480..007d84ff   the three world-into-body folds     (already in PLANE_FREE_FLIGHT_PHYSICS.md)
007d8502..007d85a5   the 0.001f deadband                 (already documented)
007d85f7  ESI = dyn+64h, the BODY LINEAR velocity
007d85fa..007d8608   dyn+70h..78h = the previous body velocity
007d8611..007d874f   a three-iteration loop, one pass per component, integrating dyn+64h..6Ch
007d8755..007d8774   dyn+64h..6Ch += dyn+88h..90h
007d8777..007d87c2   a 0.01f deadband on the result      (00D7A238)
007d87c7..007d87f1   when dyn+C8h > 0, clamp dyn+6Ch down to dyn+CCh   (a forward-speed cap)
007d8f89..007d8ffb   dyn+58h..60h = (dyn+7Ch..84h) * worldToBody
007d8ffe..007d902f   a countdown on dyn+C0h, floored at zero
```

**It is a linear-velocity integrator.** Reported as a negative result, as asked.

## The real producer chain

### `007D9C10` and `007D9C80`, a mirrored pair

These settle what the controller's four velocity slots are. Both are `__thiscall(ctl)`, both
tail-jump to `007D8020`, and both pass `normalise = 0` to `0042D0D0`.

`007D9C10 BSP_PlaneFlightController_RefreshBodyFrame`, body `007D9C10`-`007D9C75`:

```
007d9c1a  EDI = ctl+0B0h
007d9c25  0085DEA0(ctl+0B0h, unit+74h)              ; rebuild worldToBody from the live pose
007d9c2d  ctl+3Ch..44h = 0042D0D0(ctl+18h..20h, ctl+0B0h)     ; linear  world -> body
007d9c47  ctl+48h..50h = 0042D0D0(ctl+24h..2Ch, ctl+0B0h)     ; angular world -> body
```

`007D9C80 BSP_PlaneFlightController_BodyToWorldVelocity`, body `007D9C80`-`007D9CDC`:

```
007d9c8f  ctl+18h..20h = 0042D0D0(ctl+3Ch..44h, unit+74h)     ; linear  body -> world
007d9cb2  ctl+24h..2Ch = 0042D0D0(ctl+48h..50h, unit+74h)     ; angular body -> world
```

| | body frame | world frame |
| --- | --- | --- |
| linear | `ctl+3Ch..44h` | `ctl+18h..20h` |
| **angular** | **`ctl+48h..50h`** | **`ctl+24h..2Ch`** |

The linear half corroborates `docs/PLANE_FREE_FLIGHT_PHYSICS.md:43-46`, which settled
`ctl+18h` as the world velocity and `ctl+3Ch` as the body velocity from `007D9C2D` independently.
The angular half is the same routine's other half and had not been carried forward.

### `007DA710` writes the body angular velocity

`007DA710 BSP_PlaneFlight_ControlRateLaw`, called from the core law at `007DB6A6`, writes
`ctl+48h` at `007DAD01 MOVSS [ESI+48h],XMM1` with `ESI = ctl`. The shape around it is a
rate-limited slew, one axis at a time:

```
007dab52  d_x = commanded_x - ctl+48h           ; and the same for +4Ch and +50h
007dacd1  FLD [ESI+48h] / FADDP                 ; new = ctl+48h + increment
007dace0..007dacf9                              ; clamped between an upper and a lower bound
007dad01  ctl+48h = the clamped value
```

`ctl+4Ch` and `ctl+50h` repeat it at `007DAD77` onward and `007DADE3` onward. The commanded rates
come from the latched control axes and the per-class gains `docs/PLANE_FLIGHT.md` already tabulates
(`classDesc+1A8h RollSpd` and the rest). **Only the limiter shape and the store were read here;
the derivation of the commanded rates was not.**

### The loop, end to end

```
007DB697  007D9C10   ctl+24h..2Ch  --worldToBody-->  ctl+48h..50h     (world -> body)
007DB6A6  007DA710   ctl+48h..50h slewed toward the commanded rates   (the pilot's input)
          ...force accumulation, then 007D8470 integrates the LINEAR velocity only...
          007D9C80   ctl+48h..50h  --unit+74h-->     ctl+24h..2Ch     (body -> world)
007C6500  007D9F60   0085E4D0(temp, unit+674h, ctl+24h..2Ch * step)   the pose turns
```

Nothing is missing from it. The gate `docs/PLANE_ADVANCE_POSE.md` described is open.

## The bank loop, and the lead's framing note

The framing note in the brief - that the bank-driven yaw "needs no new producer but still needs a
non-zero bank, and nothing rolls the plane until the bot commands roll" - **is correct**, and the
mechanism is now visible.

`unit+C68h` has exactly two writers in the whole image (a byte-pattern sweep for both the `MOVSS`
and the x87 store forms at that displacement): `007CFD20 BSP_PlaneUnitInstance_Construct` at
`007D0179`, the constructor, and `007C18B0` at `007C1A94`. `007C18B0` is called from
`007CE040 BSP_PlaneTickElement_FixedStep`, saves the previous `unit+C64h`/`C68h`/`C6Ch`
(`007C18C1`-`007C18E6`), refreshes the world pose through `00414DB0`, then reads `unit+0CCh` - the
**world matrix** - at `007C18F3 LEA EDI,[ESI+0CCh]` / `007C18F9 FLD [EDI+20h]`, which is world row
2, the forward axis, and uses `00438B10 BSP_Math_SubtractWrappedAngle`.

So `unit+C64h`/`C68h` are **derived from the pose**, not integrated independently. The loop is:
roll input -> `ctl+48h` roll rate -> `ctl+24h` -> `0085E4D0` rolls the pose -> `007C18B0` reads the
bank out of the pose -> the bank-yaw term turns the plane. A wings-level plane with no roll order
has `sin(unit+C68h) = 0` and the term contributes nothing. Wiring it alone is a faithful no-op, as
the brief says.

`007C18B0` is also the third caller of `0085E4D0`, and it was not otherwise read here.

## What could not be established

- **`007DA710`'s commanded rates.** The limiter and the three stores are read; how the commanded
  rate for each axis is built from the latched axes and the class gains is not. That is the
  remaining unread link between a stick input and a turn, and it is the natural next packet.
- **`0085DEA0`**, which `007D9C10` uses to rebuild `ctl+0B0h` from `unit+74h`. Not read. It is
  presumably the inverse/transpose, but that is not claimed.
- **`007D8020`**, the tail call both `007D9C10` and `007D9C80` end with. Not read, so the two
  reconstructions here model the transforms only.
- **`0085D3D0`** and **`007DA2B1`-`007DA2FE`**, still open from `docs/PLANE_ADVANCE_POSE.md`.
- **`007C18B0`'s rule.** Characterised only: it derives `unit+C64h`/`C68h`/`C6Ch` from
  `unit+0CCh`. The extraction itself was not read.
- **Whether `ctl+24h..2Ch` "is" the angular velocity or its negative.** The composition is exact
  either way; the label is not settled.
- **The up-reference predicate in `0085E4D0`**, still partially read - but now demonstrably
  irrelevant, since the probe reproduces the native composition exactly with an arbitrary valid
  reference.
- **Exact float reproduction.** The primitives model the native's float spills, but
  `multiply_00413920` accumulates in `double` where the native keeps four products on the x87
  stack, and `build_look_at_lh_00b63f10` was not compared against the native output bit for bit.
  No differential test against the binary was run.

## Verification

| claim | status |
| --- | --- |
| `00B63F10`, `00B64780`, `00B646E0`, `00413920`, `0042D0D0` listings | **exported** and read |
| `007D8470`'s 957 instructions, and the complete enumeration of its stores | **exported** and read |
| `007D9C10` and `007D9C80` in full | **exported** and read |
| `007DA710`'s limiter and its three stores | **exported** and read; the commanded-rate derivation is not |
| `unit+C68h` has exactly two writers | **byte-swept** across the image, both encodings |
| the rules in `src/plane_angular_velocity.cpp` | **reconstructed** |
| `cl /W4 /WX /std:c++17` Win32, and `./scripts/build.ps1` | **build-tested** |
| the rotation sense, five checks | **fixture-tested** by a throwaway probe, not a committed test |
| anything at run time | **not validated**. Nothing was run against `bsp_game` or the retail binary |

The probe links the real `src/plane_advance_pose.cpp`, `src/unit_rudder.cpp` and
`src/camera_projection.cpp`, not stubs, so `rotate_about_axis_0085e4d0` from the previous packet is
exercised end to end for the first time.

## Wiring contract

### The binding gap from the previous packet is closed

`docs/PLANE_ADVANCE_POSE.md` left `AdvanceMatrixOps` abstract because none of its six members had
a portable reconstruction. Five are recovered here and the sixth already had one in
`include/bsp/unit_rudder.hpp`, so `bsp::NativeAdvanceMatrixOps` is a complete, concrete binding:

```cpp
#include "bsp/plane_angular_velocity.hpp"

bsp::NativeAdvanceMatrixOps ops;                       // nothing to supply
const bsp::PlaneAdvanceOutput out =
    bsp::rotate_and_translate_007d9f60(in, ops);
```

### Prefer the closed form for the rotation

`bsp::rotate_about_axis_equivalent(out, in, w)` is one Rodrigues evaluation with the proven sign
instead of a look-at, a transpose and three 4x4 products, it reproduces both of `0085E4D0`'s
length guards, and it has no degenerate up reference to choose. The probe checks it against the
native composition.

### The angular velocity itself

The host needs `ctl+24h..2Ch`. Two honest options:

1. **Model the loop.** `world_to_body_007d9c10` and `body_to_world_007d9c80` are supplied and are
   complete. What is missing is only `007DA710`'s commanded-rate derivation, so a host can slew
   `ControllerVelocities::angular_body` toward a rate of its own choosing and the frame transforms
   around it will be faithful. That is a deliberate, labelled substitution for one unread rule, not
   a reconstruction of it.
2. **Wait for the next packet.** `007DA710`'s rate derivation is the last unread link.

Either way, **negate**: the pose turns by `|w*dt|` about `-w`.

### Registration

`src/plane_angular_velocity.cpp` needs a line in `cmake/startup.cmake`, which this packet does not
touch. It includes `bsp/plane_advance_pose.hpp` and `bsp/unit_rudder.hpp`, both already in the
build.

## Follow-up

- **`007DA710`'s commanded rates**, `007DA710`-`007DAFCD`. The last unread link between a control
  axis and a turn, and the real next packet.
- `0085DEA0`, `007D8020`, `007C18B0`'s angle extraction, `0085D3D0`.
- Two ledger corrections, neither applied: `007D8470` is `RET 0Ch`, not `RET 10h`; and
  `0085DC80`'s body range is `0085DC80`-`0085DE93`, still outstanding from
  `docs/PLANE_POSE_COMMIT.md`.
