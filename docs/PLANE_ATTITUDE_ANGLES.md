# Plane attitude angles: `unit+C64h`, `unit+C68h`, `unit+C6Ch`

Packet: the producers of the three attitude scalars the plane flight law and the plane AI both
read. All three are written by one function, `007C18B0`, from the entity's **world** pose matrix.
Read-only Ghidra session on `bsp.gpr` / `/battlestationspacific.exe`; no annotation, no ledger edit.

Descriptive names below are hypotheses. Where a claim rests on a previously recovered contract
rather than on a listing read in this packet, it says so.

---

## The producer

`007C18B0` (`FUN_007c18b0`, body `007C18B0`-`007C1D7A`, 303 instructions).

**ABI:** `__thiscall(this = ECX)` plus **one 4-byte stack argument, a float `dt`**, callee-cleaned:
`007C1D78 RET 4`. `this` is the plane unit (`ESI`, `007C18B8 MOV ESI,ECX`); the float parameter
resolves to entry-relative `frame=+4` at all five of its references (`007C1AB2`, `007C1B12`,
`007C1B23`, `007C1B4E`, `007C1B74`, `007C1D3C`), which is what `RET 4` requires. The call site in
the per-tick path confirms both halves:

```
007cee8f  FLD  dword ptr [ESP+104h]     ; the step
007cee96  PUSH ECX                      ; reserve the 4-byte slot
007cee97  MOV  ECX,EDI                  ; this = the plane unit
007cee99  FSTP dword ptr [ESP]          ; the float argument
007cee9c  CALL 007C18B0
```

Callers (`ghidra xrefs`, 8 sites): `007CEE9C` in `BSP_PlaneTickElement_FixedStep` (`007CE040`),
`007C6224` in `BSP_Plane_PlaceOnLaunchSpotLocked`, `007D1822` and `007D19F8` in
`BSP_Plane_ApplyControlStateMessage`, `007D722D` in `BSP_Plane_ReadPropertyBag`, `007CA2DD`,
`007F29C4` and `007F2B46` in `FUN_007F2920`. So the angles are refreshed every plane step and
re-derived after any teleport/respawn/state-message that moves the plane.

### It reads the world matrix, and it is the live pose, not the fixed-step pose

```
007c18ba  CMP  byte ptr [ESI+0C8h],0
007c18ec  JNZ  007c18f3
007c18ee  CALL 00414DB0                 ; BSP_EntityPose_RefreshWorld
007c18f3  LEA  EDI,[ESI+0CCh]           ; the world pose matrix
```

`unit+C8h` is the world-pose-valid byte and `unit+CCh` the world matrix; `00414DB0`'s recovered
contract (ledger, `src/pose_refresh.cpp`) is `world(+CCh) = local(+74h) * parent.world(+CCh)`, with
the root case copying `local(+74h)`. `+74h` is `plane_advance_off::kLivePose`, the drawn pose — so
**the attitude angles track the live/drawn pose in world space, not the committed fixed-step pose at
`unit+674h`.** Every displacement below is against the matrix base `EDI = unit+CCh`.

Matrix layout: 4x4 row-major floats, row-vector convention (`v' = v*M`), translation in row 3 —
`plane_advance_off::kFixedStepPosition = kFixedStepPose + 30h` and `00413920`'s recovered row-major
product both fix this. Rows are the body axes in world: row 0 `+00h` lateral, row 1 `+10h` up,
row 2 `+20h` forward, matching `include/bsp/plane_flight.hpp:431` ("Body axes are (x lateral, y up,
z forward)"). Independent evidence for row 1 = up: `00700F8B`-`00700FB4` overwrites a copied
matrix's `+10h/+14h/+18h` with the world up global.

Naming below: `fwd = (fx, fy, fz)` = the matrix's row 2 = `(+20h, +24h, +28h)`;
`h = sqrt(fx^2 + fz^2)`.

### The world up global

`007C195F PUSH 0F8758Ch` is the second operand of `BSP_Vector3f_Cross`. The image bytes at
`00F8758C` are zero, and no instruction writes it through its own address — but
`CG_static_init_00CCFFE0` copies it at startup:

```
00cd2280  movss xmm0, dword ptr [0xe0b68c]   ; 00 00 00 00  0.0f
00cd2288  movss dword ptr [0xf8758c], xmm0
00cd2290  movss xmm0, dword ptr [0xe0b690]   ; 00 00 80 3f  1.0f
00cd2298  movss dword ptr [0xf87590], xmm0
00cd22a0  movss xmm0, dword ptr [0xe0b694]   ; 00 00 00 00  0.0f
00cd22a8  movss dword ptr [0xf87594], xmm0
```

So `00F8758C = (0, 1, 0)`, the world up vector. (The file image reads as zeros; taking that at face
value would have made the whole heading/bank path degenerate.)

---

## 1. `unit+C64h`, the pitch angle

**Written at `007C1966` `FSTP dword ptr [ESI+0C64h]`** (`D9 9E 64 0C 00 00`).

```
007c18f9  FLD  [EDI+20h]          ; fx
007c1903  FLD  [EDI+28h]          ; fz
007c1912  FMUL ST0                ; DC C8, = FMUL ST(0),ST(0): fx*fx  (i=0, so D8/DC agree)
007c191e  FMULP ST2               ; fz*fz
007c1926  FADD [ESP+0Ch]          ; fx^2 + fz^2
007c1932  CALL 00BF7030           ; sqrt  (00BF7033 FST double [ESP] / CALL 00C08418)
007c1943  FLD  [EDI+24h]          ; fy      -> ST1 after the next FLD
007c194e  FLD  [ESP+18h]          ; h       -> ST0
007c1952  CALL 00BF701A           ; LIBCRT_atan2 (MOV EDX,0E15C20h / JMP 00C07EE0 __cintrindisp2)
007c1966  FSTP [ESI+0C64h]
```

```
unit+C64h = atan2(fy, sqrt(fx^2 + fz^2))
          = atan2(m21, sqrt(m20^2 + m22^2))      // m[row][col] on the world matrix at unit+CCh
```

* **Frame:** world, live pose (`unit+CCh`, from `unit+74h`).
* **Range:** `[-pi/2, +pi/2]`. The second `atan2` operand is a `sqrt`, hence `>= 0`; no wrap is
  applied and none is needed.
* **Sign:** `fy > 0` (the nose pointing above the horizon) gives a positive angle. **Positive =
  nose up.**
* **Operand order:** `_CIatan2` consumes `ST0 = x`, `ST1 = y` and returns `atan2(y, x)` —
  `include/bsp/geometry_helpers.hpp:7`, recovered in an earlier packet from `__cintrindisp2`'s
  descriptor. The first `FLD` is therefore the `y` argument.
* Computed unconditionally: no guard can skip this store.

## 2. `unit+C6Ch`, the heading

**Written twice, at `007C1A32` and `007C1ACA`, both `FST` (`D9 96 6C 0C 00 00`, store without
pop). `007C1ACA` writes the final value**; `007C1A32` stores the *raw, unwrapped* intermediate,
which is read straight back at `007C1A9A` as the argument to the wrap. Nothing else can observe the
intermediate — every path through `007C1AD0` has already executed `007C1ACA` — but a reader of the
listing must not mistake `007C1A32` for the producer.

The derivation removes the pitch from the matrix first:

```
007c195f  PUSH 0F8758Ch                 ; up = (0,1,0)
007c1964  MOV  EDX,EBX                  ; EBX = EDI+20h = &fwd
007c196c  LEA  ECX,[ESP+40h]            ; out
007c1970  CALL 004F9B30                 ; BSP_Vector3f_Cross: out = fwd x up = (-fz, 0, fx)
007c19a5  ...                           ; |cross|^2
007c19a9  FLD  double [00CE3820]        ; 1.0842e-10 ; JBE 007C1B21 -> skip heading and bank
007c19bd  CALL 00BF7030                 ; len = |cross| (= h)
007c19d4  COMISS XMM0,[00D7A23C]        ; 0.001f     ; JBE 007C1B23 -> skip heading and bank
007c19e1..007c1a11                      ; the 16-byte argument {cross/len, pitch}
007c19f2  MOV  EDX,EDI                  ; the world matrix
007c19f8  LEA  ECX,[ESP+58h]            ; out matrix U
007c1a14  CALL 0085E4D0                 ; rotate_about_axis, RET 10h
007c1a19  FLD  [ESP+70h]                ; U+28h = U.m22   -> y
007c1a1d  FLD  [ESP+68h]                ; U+20h = U.m20   -> x
007c1a21  CALL 00BF701A                 ; atan2(U.m22, U.m20)
007c1a32  FST  [ESI+0C6Ch]              ; the raw angle, measured from +X
...
007c1aa0  FSTP [ESP+4]                  ; arg1 = the raw angle
007c1aa4  FLD  [00CE3C64]               ; 3FC90FDB = 1.5707964f = pi/2
007c1aaa  FSTP [ESP]                    ; arg0
007c1aad  CALL 00438B10                 ; BSP_Math_SubtractWrappedAngle(pi/2, raw)
007c1aca  FST  [ESI+0C6Ch]              ; the wrapped heading
```

`0085E4D0(out ECX, in EDX, {axis.xyz, angle})`, `RET 10h`, is the axis-angle pose rotation already
recovered in `include/bsp/plane_angular_velocity.hpp:89-100` as
`rotate_about_axis_equivalent`: **"a right-hand rotation of the pose by `-|w|` radians about
`normalize(w)`"**, rotating only the basis rows and copying row 3. With
`axis = normalize(fwd x up) = (-fz, 0, fx)/h` (perpendicular to both `fwd` and world up) and
`angle = pitch`, that rotation takes `fwd` exactly onto the horizontal plane:
writing `hhat = (fx, 0, fz)/h`, `fwd = h*hhat + fy*yhat` and `axis = hhat x yhat`, so a right-hand
rotation about `axis` carries `hhat` toward `yhat`; by `-pitch` it carries `fwd` back to `hhat`.
Hence `U.row2 = hhat` and

```
raw     = atan2(U.m22, U.m20) = atan2(fz, fx)                  // measured from +X toward +Z
unit+C6Ch = SubtractWrappedAngle(pi/2, raw) = wrap(pi/2 - atan2(fz, fx))
          = atan2(fwd.x, fwd.z)                                // measured from +Z toward +X
```

* **Frame:** world, live pose. The heading is the compass bearing of the matrix's forward row
  projected onto the horizontal plane.
* **Range and wrap:** `(-pi, pi]`, radians. The wrap is `00438B10
  BSP_Math_SubtractWrappedAngle(a, b) = wrap(a - b)`: `00438B10 FLD [ESP+4] / 00438B14 FSUB
  [ESP+8]`, then loops adding `00CE3828 = 6.2831855` while the result is at or below
  `00CE3D18 = -3.1415927` and subtracting it while above `00CE3D28 = +3.1415927`, float-rounding
  every iteration. `RET 8`, result in `ST0`. The constants are the float-rounded `pi`, so the bound
  is `3.14159274f`, not the double `pi`. The ledger records the range as `(-pi, pi]`.
* **Sign:** `fwd = +Z` gives `0`; `fwd = +X` gives `+pi/2`. **The heading increases turning from
  +Z toward +X**, i.e. clockwise seen from above in this y-up, z-forward frame — the usual compass
  sense. The subtraction from `pi/2` is what performs the change of reference axis *and* the sign
  flip; `raw` alone is measured the other way, from +X toward +Z.
* **Guards:** if `|fwd x up|^2 <= 1.0842e-10` (`00CE3820`) or `|fwd x up| <= 0.001f`
  (`00D7A23C`) — i.e. the plane is pointing within ~0.06 degrees of straight up or straight down —
  **`unit+C6Ch` and `unit+C68h` are not written at all and keep their previous values**, while
  `unit+C64h` has already been updated. The heading rate is skipped too; the pitch and bank rates
  still run, the bank rate against the stale bank.

## 3. `unit+C68h`, the bank angle

**Written at `007C1A94` `FSTP dword ptr [ESI+0C68h]`** (`D9 9E 68 0C 00 00`).

The heading rotation is divided out of the already-unpitched matrix `U`, and the residual is a roll
about the forward axis:

```
007c1a2e  LEA  EDX,[ESP+0Ch]            ; &raw   (007C1A3F FCHS / 007C1A41 FCHS cancel)
007c1a38  LEA  ECX,[ESP+0C8h]
007c1a47  CALL 00B646E0                 ; BSP_Matrix_BuildRotationY(raw), EAX = the matrix
007c1a4c  PUSH EAX
007c1a4d  LEA  ECX,[ESP+8Ch]            ; frame -192
007c1a54  CALL 004134F0                 ; Copy4x4: [-192] = RotationY(raw)        RET 4
007c1a60  PUSH EAX                      ; &[-192]           -> right
007c1a68  PUSH ECX                      ; &[-64]            -> destination
007c1a69  LEA  ECX,[ESP+50h]            ; frame -256 = U    -> left
007c1a6d  CALL 00413920                 ; [-64] = U * RotationY(raw)              RET 8
007c1a72  PUSH EAX
007c1a73  LEA  ECX,[ESP+4Ch]            ; frame -256
007c1a77  CALL 004134F0                 ; [-256] = [-64]                          RET 4
007c1a7c  FLD  [ESP+60h]                ; -256+18h = M.m12  -> y
007c1a80  FLD  [ESP+5Ch]                ; -256+14h = M.m11  -> x
007c1a84  CALL 00BF701A                 ; atan2(M.m12, M.m11)
007c1a94  FSTP [ESI+0C68h]
```

```
M = U * RotationY(raw)
unit+C68h = atan2(M.m12, M.m11)         // = atan2(up.z, up.y) in the de-pitched, de-yawed frame
```

* `00413920 BSP_Matrix_Multiply4x4` is `__fastcall(left ECX, stack: destination, right)`, `RET 8`,
  `EAX = destination`, row-major `dst = left * right` — `include/bsp/camera_multiply.hpp:5`. Of the
  two pushes, `[ESP]` is therefore the destination `[-64]` and `[ESP+4]` the right operand.
  `004134F0 BSP_Matrix_Copy4x4X87` is `__fastcall(dst ECX, stack: src)`, `RET 4` (shape confirmed
  at `00700F77`-`00700F93`, where `ECX` is the object subsequently written through).
* `00B646E0 BSP_Matrix_BuildRotationY(t)` is
  `[cos 0 -sin 0][0 1 0 0][sin 0 cos 0][0 0 0 1]` (`include/bsp/plane_angular_velocity.hpp:47-51`,
  read off `00B64724`/`00B64731`/`00B64753`/`00B6475D`). With `U.row2 = (cos raw, 0, sin raw)`, the
  row-vector product gives `M.row2 = (1, 0, 0)` **exactly**: the forward axis lands on +X, so the
  residual is a rotation about X and `M.m10 = 0` identically.
* **Frame:** world, live pose, with pitch and heading removed in that order.
* **Range:** `(-pi, pi]` as `atan2` returns it; no wrap is applied and none is needed. In practice
  `M.row1 = (0, cos b, sin b)` exactly (orthonormality against `M.row2 = (1,0,0)`), so the angle is
  exact rather than a small-angle approximation.
* **Sign:** with `M.row2 = (1,0,0)` and `M.row1 = (0, cos b, sin b)`, orthonormality with
  `row0 x row1 = row2` (true for any proper rotation) forces `M.row0 = (0, sin b, -cos b)`. At
  `b = 0` the lateral axis is `-Z`; for `b > 0` the lateral axis gains a **+Y** component, i.e. the
  right wing rises. **Positive bank = left wing down = rolling left.**
  This is confirmed by the consumer: `include/bsp/plane_advance_pose.hpp:234-237` records the
  bank-driven yaw fed to `BuildRotationY` at `007DA0AE` as
  `-(blend * sin(bank) * cos(pitch) * gain * difficulty * step)`. Under the `RotationY` layout above
  a positive angle turns the forward axis from +Z toward +X, i.e. increases the heading (right
  turn); the leading minus therefore makes a **positive bank produce a left turn**, which is what a
  left-wing-down aircraft does. The two independently recovered signs agree.

---

## The three rates fall out of the same function

Each is `SubtractWrappedAngle(current, previous) / dt`, with the previous value latched from the
field at entry (`007C18C1`/`007C18CF`/`007C18DD`, into entry-relative frames `-308`, `-296`, `-312`,
each consumed exactly once):

| field | value | address | guard |
|---|---|---|---|
| `unit+C70h` | `wrap(heading - prev_heading) / dt` | `007C1AE4`..`007C1B19` | `dt > 0` (`00D7A218 = 0.0f`) **and** `abs(delta) < 0.5f` (`00CE3800`), a jump reject |
| `unit+C74h` | `wrap(pitch - prev_pitch) / dt` | `007C1B49`..`007C1B58` | `dt > 0` |
| `unit+C78h` | `wrap(bank - prev_bank) / dt` | `007C1B6F`..`007C1B7B` | `dt > 0` |

`docs/PILOT_BOT_PLAN_CONTROLS.md` records `C74h`/`C78h`; `C70h` and its `0.5f` rejection window
are not recorded anywhere in the repository as far as this packet found.

The same function then derives two further elevation angles of the identical
`atan2(v, sqrt(a^2+b^2))` shape from the vectors at `unit+AC8h..AD0h` and `unit+AE0h..AE8h`, storing
them at `unit+C7Ch` (`007C1CA8`) and `unit+C84h` (`007C1D05`) with rates at `unit+C80h` and
`unit+C88h`. Those two vectors were not identified in this packet.

## No other runtime producer

Byte scans over `.text` for every store form against each displacement, with `007C1966` /
`007C1A94` / `007C1ACA` as the positive control (each scan returns the known store):

* x87 `D9 ?? <disp32>`: for `64 0C 00 00`, 33 matches, of which exactly one has a ModRM reg field of
  3 (`FSTP`) or 2 (`FST`) — `007C1966` (`D9 9E`); the other 32 are `D9 80..87` (`FLD`). For
  `68 0C 00 00`, 21 matches, one store: `007C1A94`. For `6C 0C 00 00`, 7 matches, two stores:
  `007C1A32` and `007C1ACA` (`D9 96`, `FST`), the rest `FLD` — including `0074E260
  D9 81 6C 0C 00 00`, the `FLD [ECX+0C6Ch]` getter that `include/bsp/plane_ai_control.hpp:238`
  identifies as `vtable[50h]`.
* `F3 0F 11 ?? <disp32>` (MOVSS store): one site per field, all three in
  `BSP_PlaneUnitInstance_Construct` (`007D0179`, `007D0181`, `007D0189`) — construction-time init.
* `89 ?? <disp32>` (MOV r32 store): one site per field, all three in `BSP_SceneRecord_Construct`
  (`004DA383`, `004DA389`, `004DA38F`), `MOV [ESI+...],EBX` in a run of field initialisations —
  construction-time, and a different class unless the plane unit derives from it (not checked).
* `C7 ?? <disp32>` (MOV imm32): no matches for any of the three.

So `007C18B0` is the sole runtime writer of all three.

---

## Contradiction with an existing document

**`docs/PILOT_BOT_PLAN_CONTROLS.md:115-119` states the bank formula incorrectly.** It reads:

```
007c1a7c: FLD [ESP+60h]    ; copy+14h = m11
007c1a80: FLD [ESP+5Ch]    ; copy+10h = m10
007c1a84: CALL 00BF701A    ; atan2(m10, m11)
```
> so `unit+C68h = atan2(m10, m11)` on the de-yawed matrix

Two errors:

1. **The matrix base is off by 4.** The doc takes the copy destination from `007C1A73 LEA
   ECX,[ESP+4Ch]` and subtracts that displacement from the load displacements. But `004134F0` is
   `RET 4`: `ESP` is 4 higher at `007C1A7C` than at `007C1A73`. The base at the loads is `ESP+48h`,
   so `[ESP+60h] = base+18h = m12` and `[ESP+5Ch] = base+14h = m11`, not `m11`/`m10`. The same `-4`
   correction is forced independently by the rest of the frame: only with it does `[ESP+14Ch]`
   resolve to the entry-relative `+4` demanded by `RET 4` and by the call site, and only with it do
   `[ESP+14h]`, `[ESP+20h]` and `[ESP+18h]` at `007C1AD6`, `007C1B35` and `007C1B5E` resolve to the
   three previous-value slots latched at entry.
2. **The doc's formula is identically zero.** On a matrix whose forward row has been rotated onto
   `(1,0,0)`, orthonormality makes `m10 = 0` exactly, so `atan2(m10, m11)` is `0` for every attitude
   (and the alternative misalignment `atan2(m11, m10)` is the constant `+pi/2`). `atan2(m12, m11)`
   is the only alignment of that adjacent float pair that is not a constant.

The doc's pitch formula (`unit+C64h = atan2(m21, sqrt(m20^2 + m22^2))`, line 103) agrees with this
packet. Its open items at lines 523-532 — "the two `atan2` operands were not traced ... which
operand is `y` was not checked ... recovered only as far as 'wrapped `atan2` of two orientation
terms'" — are closed above.

Nothing here contradicts `include/bsp/plane_advance_pose.hpp` or `include/bsp/plane_ai_control.hpp`;
`kPitchAngle`, `kBankAngle`, `kUnitHeading` and "the wrapped atan2 that `007C1ACA` writes" all hold.

---

## What is not established

* **`0085E4D0`'s rotation sense is taken as a contract, not re-derived.** This packet used
  `include/bsp/plane_angular_velocity.hpp:89-100` ("a right-hand rotation of the pose by `-|w|`
  about `normalize(w)`", itself read off `00B63F10`/`00B64780`/`00413920` in an earlier packet). The
  listings of the look-at and rotation-Z builders were not re-read here. What *was* checked here is
  that this sense is the self-consistent one: it makes `U.row2` horizontal, which in turn makes
  `M.row2 = (1,0,0)` exactly, which is what makes the bank extraction at `+14h`/`+18h` meaningful.
  The opposite sense would double the pitch instead of removing it and would leave `M.row2` off the
  +X axis, making `M.m10` non-zero and the recovered heading discontinuous at 45 degrees of pitch.
* **`0085D3D0`**, called at `0085E85F` at the end of `0085E4D0`, was not examined. The prior
  packet's closed form asserts the net effect is "basis rows rotated, row 3 copied"; if `0085D3D0`
  does more than normalise, the closed form is where that would surface, not here.
* **`0085E4D0` normalises the axis a second time** (`0085E54A`-`0085E588`, dividing args 0-2 by
  their own length) after `007C19FE`-`007C1A11` already did. Harmless, but it means the caller's
  `len > 0.001f` guard is the only one that matters.
* **The pose is assumed to be a proper rotation** (orthonormal, `det = +1`). The bank sign argument
  uses `row0 x row1 = row2`, which fails under a mirror. `unit+CCh` is `local * parent.world`, so a
  negatively scaled parent would break it. Uniform positive scale is harmless — every formula here
  is a ratio.
* **The store census covers direct `[base + C64h/C68h/C6Ch]` forms only.** A write through a
  pointer held at a different base with a small displacement, or a bulk `REP MOVSD` / `memcpy` over
  the region, would not appear in those scans. No such writer is known; the claim is "no direct
  store outside `007C18B0` and the two constructors", not "no write of any kind".
* **`004DA2A0 BSP_SceneRecord_Construct`'s `+C64h..+C6Ch`** were assumed to be a coincident offset
  in a different class. Whether the plane unit derives from that record was not checked; either way
  it is a constructor, not a per-frame producer.
* **The double `FCHS` at `007C1A3F`/`007C1A41`** cancels exactly and is numerically a no-op. It
  suggests the source wrote `-(-angle)`; that provenance is not established.
* **`unit+C7Ch`/`C80h`/`C84h`/`C88h`** and the vectors at `unit+AC8h`/`AE0h` they come from are out
  of this packet's scope. They share the `atan2(v, sqrt(a^2+b^2))` elevation shape with the pitch.
* **No runtime validation.** Nothing here was observed in the running game; the sign conventions are
  derived from the listings plus the two recovered matrix-primitive contracts, and cross-checked
  against the bank-driven yaw term's sign.
