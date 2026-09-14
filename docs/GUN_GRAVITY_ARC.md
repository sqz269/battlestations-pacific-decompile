# The gun gravity arc - `00955630`

Addresses: `00955630` (body `00955630..0095581F`, `RET 10h`); callees `0042B260`, `004B4D80`,
`0042D0D0`, `00414DB0`, `00B63D50`, `00521370`, `0042CF10`, `00605070`, `00BF7030`, `00BF701A`,
`00BF8490`; call sites `006DFAD4`, `0095A0AA`, `0095A4E3`, `005476EB`, `0085B8DE`, `006DEFA8`;
constants `00CF9058`, `00D7A328`, `00CEB5A8`, `00D7A208`, `00D7A250`, `00CE3828`, `00CE3D18`,
`00CE3D28`, `00CE3820`, `00CE3C70`, `00D7A24C`, `00CE3C64`, `00D7A260`, `00CE3CCC`, `00D7A210`;
data `00F87574`.

Reconstruction: `include/bsp/gun_gravity_arc.hpp`, `src/gun_gravity_arc.cpp`. Report:
`reports/cc7_gun_gravity_arc.json`. Packet `cc7_gun_gravity_arc`, read-only Ghidra pass.

`BSP_Gun_SolveGravityArc` is a hypothesis, not a recovered symbol. Every other descriptive name
used below is the existing ledger name for that address and carries the same caveat.

## Why this document exists

`docs/GUN_BOT_TICKS.md` marks `00955630` "complete" and describes it in one line, but neither it
nor any header published the arithmetic. `docs/GAME_EXECUTABLE.md` follow-up packet 1 records the
consequence: every gun elevation in the milestone 2t run is `006DF520` step 6's *pre-estimate*
`pitch = asin(g*R/v^2)/2` clamped to `pi/4`, not the ballistic solution. This document recovers the
solution from the listing.

## ABI, from the listing and from six call sites

```
bool __fastcall BSP_Gun_SolveGravityArc(
        void*         mount,        // ECX, may be NULL
        const float3* aimPoint,     // EDX
        const float3* muzzlePos,    // [ESP+4]  at entry
        float         muzzleSpeed,  // [ESP+8]
        float*        outPitch,     // [ESP+0Ch]
        float*        outYaw);      // [ESP+10h]
```

Evidence:

* `0095581F RET 0x10` - four stack arguments, cleaned by the callee. Combined with the two
  register arguments this is `__fastcall` with six parameters. The argument count is taken from
  the `RET` immediate, not from the pushes at any one site.
* Frame: `00955630 SUB ESP,0x20` then `PUSH EBX/EBP/ESI/EDI` puts the incoming stack arguments at
  `[ESP+34h]`, `[ESP+38h]`, `[ESP+3Ch]` and `[ESP+40h]` for the rest of the body. `00955633` reads
  `[ESP+24h]` before the pushes - the same slot - into `EAX` and dereferences `[EAX]`, `[EAX+4]`,
  `[EAX+8]`, so argument 3 is a `float3*`. `009556B8 FLD float ptr [ESP+38h]` makes argument 4 a
  `float`. `009556A4 MOV EDI,[ESP+40h]` then `009556AC FSTP [EDI]` makes argument 6 a `float*`;
  `009556FC`/`00955749 MOV EBP,[ESP+3Ch]` then `FSTP [EBP]` makes argument 5 a `float*`.
* `EDX` is dereferenced at `[EDX]`, `[EDX+4]`, `[EDX+8]` at `00955637..00955652`; `ECX` is copied
  to `ESI` at `00955646` and only used as `[ESI+10Ch]`, `ESI+0CCh`, `ESI+110h` - an entity, tested
  for `NULL` at `0095576A`.
* The recorded ABI in the ledger was already correct; this pass confirms it rather than changing
  it. **No correction was needed.**

The argument slot `[ESP+34h]` (argument 3) is reused as scratch for `hsq`/`k` after `EAX` is read,
and `[ESP+40h]` (argument 6) is reused for `k+h` and the discriminant after `EDI` is read. That
reuse is why the decompiler's variable list does not line up with the parameters.

### Every call site agrees

| Site | Containing function | `ECX` mount | `EDX` aimPoint | arg3 muzzlePos | arg4 speed |
| --- | --- | --- | --- | --- | --- |
| `006DFAD4` | `006DF520 BSP_GunBot_MuzzleSolutionAimTick` | `[[bot+50h]+3Ch]` | `&[ESP+34h]`, the led aim point | `00427EB0(muzzle node)` | `[[[gun+3F8h]+34h]+50h]` |
| `0095A0AA` | `00959C20 BSP_Unit_ApplyGunAimMessage`, kind 3 | `EBP` | `&[ESP+50h]` | `ESI+0FCh` (the device's own world position) | `[[[dev+3F8h]+34h]+50h]` |
| `0095A4E3` | `00959C20`, kind 5 | `EBP` | `&[ESP+50h]` | `00427EB0(ESI)` | same |
| `005476EB` | `00547480` | `EDI` | `&[ESP+4Ch]` | `00427EB0(ESI)` | same |
| `0085B8DE` | `0085B7D0` | **`XOR ECX,ECX` at `0085B8C5` - NULL** | `&[ESP+0Ch]`, a locally built delta | the static float3 `00F87574` | `[EDI+50h]` |
| `006DEFA8` | `006DEF30`, **no Ghidra function** | `[[bot+50h]+3Ch]` | `EBX+0FCh` (target world position) | `EDI+0FCh` (gun world position) | `[muzzle+50h]` |

Six sites, not the four the call graph summary listed. The two that matter for the contract:

* `0085B8DE` passes `mount = NULL` deliberately, so the mount-frame branch is live-but-skipped
  code, not a defensive test. It also passes `00F87574` as the muzzle position; that global reads
  as `{0,0,0}` in the image (the undefined routine `006DEEF0..006DEF20` copies all three of its
  floats into `ECX+84h..8Ch`, which is what proves it is a `float3` and not a lone float), so at
  that site the "aim point" is already a relative delta.
* `006DEFA8` sits in undefined code. `FUN_006DEEC0`'s Ghidra body ends at `006DEEE9`; the routine
  that contains the call starts at `006DEF30` (`PUSH ECX; PUSH EBX; MOV EBX,[ESP+0Ch]`) and ends
  at `006DEFEA` (`RET 4`). It is `__thiscall(bot)(target)`: on a `true` result it calls
  `0085A8B0(gun, pitch, yaw)` and then `gun->vtable[1D4h](target)`.

`00427EB0 BSP_EntityPose_GetWorldPositionRefreshed` returns `entity+0FCh`, the translation row of
the world matrix based at `entity+0CCh`. That is why `ESI+0FCh` and `EBX+0FCh` appear directly at
two sites - the same pointer, without the refresh.

## The arithmetic, instruction by instruction

x87 throughout, control word `27Fh` (53-bit); every `FSTP` to a `float ptr` rounds the
intermediate to single precision, and those roundings are part of the recovered behaviour.

### 1. The delta and the horizontal distance - `00955637..0095568E`

```
00955637  FLD  [EDX]      FSUB [EAX]      FSTP [ESP+20h]   ; dx = aim.x - muzzle.x   -> frame+24h
00955643  FLD  [EDX+4]    FSUB [EAX+4]    FSTP [ESP+28h]   ; dy = aim.y - muzzle.y   -> frame+28h
0095564f  FLD  [EDX+8]    FSUB [EAX+8]    FSTP [ESP+2Ch]   ; dz = aim.z - muzzle.z   -> frame+2Ch
00955659  FLD  dz ; FLD dx ; FMUL ST0 ; FLD ST1 ; FMULP ST2 ; FADDP
00955669  FSTP [ESP+34h]                                   ; hsq = dz*dz + dx*dx
0095566d  FLD  hsq ; CALL 00BF7030 ; FSTP [ESP+10h]        ; R = (float)sqrt(hsq)
0095567a  FLD  [ESP+10h] ; FSTP [ESP+14h]                  ; R kept at frame+14h
00955686  FLD  dy        ; FSTP [ESP+18h]                  ; dy kept at frame+18h
0095567e  LEA  ECX,[ESP+24h] ; CALL 0042B260               ; normalize d IN PLACE
```

`R` and `dy` are copied out **before** `0042B260` overwrites the delta. `00BF7030` is `_CIsqrt`
(`SUB ESP,0Ch; FST double [ESP]; CALL 00C08418; CALL 00BF704D`, argument and result in `ST0`; 257
callers including `BSP_Vector3f_Length`).

`0042B260 BSP_Geometry_NormalizeVectorWithFloor` (`__fastcall(float3*)`, `RET`, body
`0042B260..0042B2E9`) computes `lenSq = x*x + y*y + z*z`, uses `sqrt(lenSq)` when
`lenSq > 1e-10` (`00CE3820`) and the fixed `1e-5` (`00CE3C70`) otherwise, then divides all three
components in place. The floor is what keeps a zero delta out of the `atan2` below.

### 2. The yaw, computed first and in the world frame - `00955693..009556AC`

```
00955693  FLD [ESP+24h]   ; d.x (normalized)
00955697  FLD [ESP+2Ch]   ; d.z (normalized)
0095569b  CALL 00BF701A   ; _CIatan2(ST1, ST0) = atan2(d.x, d.z)
009556a4  MOV EDI,[ESP+40h] ; FSTP [EDI]
```

`*outYaw` is written **before** the pitch solve and independently of it. Normalizing first is
numerically motivated only - `atan2` is invariant under positive scaling - but the floor in
`0042B260` means a zero-length delta yields `atan2(0,0) = 0` rather than a domain fault.

### 3. The gravity drop term - `009556AE..009556C4`

```
009556ae  FLD  [ESP+34h]                ; hsq
009556b2  FMUL double ptr [00CF9058]    ; * g
009556b8  FLD  [ESP+38h] ; FLD ST0 ; FADD ST0,ST1 ; FMULP   ; v * (v + v)
009556c2  FDIVP
009556c4  FSTP [ESP+34h]                ; k = (hsq * g) / (v * (v + v))
```

`00CF9058` is the **double** `9.8100004196167` - the `float` literal `9.81f` widened. It is not a
field and not a per-weapon value: it is an immediate constant in `.rdata` shared with the
projectile integrator (`006E65D6 FMUL double ptr [00CF9058]`). There is no per-mount, per-class or
per-world gravity input to this routine.

> Correction for another packet's file: `include/bsp/projectile_impact.hpp:27` declares
> `kProjectileGravity = 9.8103800773621` and attributes it to `00CF9058`. The eight bytes at
> `00CF9058` are `00 00 00 60 B8 9E 23 40` = `9.8100004196167`. `include/bsp/gun_bot_ticks.hpp:159`
> `kGunBotGravity = 9.81000042f` is the correct reading of the same address. That file is not
> leased to this packet and was not edited.

`k` has units of length. It is `g*R^2 / (2*v^2)`, the drop of a horizontally fired round over the
horizontal distance `R`.

### 4. The discriminant - `009556C8..009556EE`

```
009556c8  FLD  [ESP+14h] ; FLD ST0 ; FMUL ST0          ; R, R*R
009556d0  FLD  k ; FLD ST0 ; FADD [ESP+18h]            ; k + dy
009556da  FSTP [ESP+40h] ; FLD [ESP+40h]               ; rounded to float, reloaded
009556e2  FXCH
009556e4  FMUL double ptr [00D7A328]                   ; k * 4.0
009556ea  FMULP                                        ; (k+dy) * (k*4.0)
009556ec  FSUBP                                        ; R*R - that
009556ee  FSTP [ESP+40h]                               ; D
```

`00D7A328` is the double `4.0` (`00 00 00 00 00 00 10 40`). So

```
D = R*R - 4*k*(k + h)        with h = dy = aim.y - muzzle.y
```

Order matters: the factor `4.0` multiplies `k` first, and `k + h` is round-tripped through a
`float` slot before the product.

### 5. The branch - `009556F2..009556FA`

```
009556f2  FLD D ; FLDZ ; FCOMIP ST0,ST1 ; JBE 00955715
```

`FCOMIP` compares `0.0` against `D` and `JBE` is taken on `CF|ZF`, i.e. when `0.0 <= D`. The
fall-through - the failure path - is therefore reached on exactly one condition: `0.0 > D`, with
`CF = ZF = 0`. An unordered compare sets `ZF = CF = 1`, so a `NaN` discriminant takes the
**success** path and the routine returns `true` with a `NaN` pitch; see the degeneracy section.

### 6. The selected root - `00955715..00955750`

```
00955715  FXCH ; FSTP double ptr [ESP+18h]   ; R saved as a double (lossless, it was a float)
0095571b  CALL 00BF7030 ; FSTP [ESP+40h]     ; s = (float)sqrt(D)
00955724  FLD s ; FSUBR double ptr [ESP+18h] ; R - s
0095572c  FLD k ; FADD ST0,ST0 ; FDIVP       ; (R - s) / (k + k)
00955734  FSTP [ESP+34h]                     ; T
00955738  FLD T ; CALL 00BF8490 ; FSTP [ESP+34h]
00955745  MOV EBP,[ESP+3Ch] ; FSTP [EBP]     ; *outPitch = atan(T)
00955750  MOV BL,1
```

`00BF8490` is `_CIatan`: its inner `00BF84E8` does `FLD1; FPATAN`, which is `atan(ST1/ST0)` with
`ST0 = 1.0`, i.e. `atan(x)`. It is **not** `asin`, even though `0042CF10 BSP_Geometry_AsinClamped`
also calls it (that routine builds `asin` from `atan`).

`T` is a **tangent**. The quadratic being solved is the textbook one: with `T = tan(theta)`,

```
h = R*T - k*(1 + T*T)   =>   k*T^2 - R*T + (h + k) = 0
T = ( R +- sqrt(R^2 - 4*k*(k + h)) ) / (2*k)
```

`00955728 FSUBR` produces `R - s`, so the routine takes the **minus** root - the low, flat
trajectory - and never the lobbed one. There is no branch, no gate and no caller flag that selects
the high root; it is unreachable in this build.

Substituting `k = g*R^2/(2*v^2)` gives the equivalent standard form

```
tan(theta) = ( v^2 - sqrt( v^4 - g*(g*R^2 + 2*h*v^2) ) ) / ( g*R )
```

### 7. The failure path - `009556FC..00955713`

```
009556fc  MOV EBP,[ESP+3Ch]
00955700  FSTP ST0
00955702  MOVSS XMM0,[00CEB5A8]
0095570a  FSTP ST0
0095570c  MOVSS [EBP],XMM0        ; *outPitch = pi/4
00955711  XOR BL,BL               ; result = false
00955713  JMP 00955752            ; INTO the common tail
```

`00CEB5A8` is the float `0.78539819` = `pi/4` - the same constant `006DF520` step 6 clamps its
pre-estimate to. The two `FSTP ST0` are the x87 stack cleanup for `D` and `R`.

### No drag, no time of flight

There is no drag coefficient, no air-density term, no time-of-flight computation and no iteration
anywhere in `00955630..0095581F`. The lead is already baked into `aimPoint` by the caller
(`006DF520` step 6 pushes the point out along the target velocity before step 8 calls this). The
routine is a single closed-form evaluation of a drag-free parabola.

## The frame handling

`docs/GUN_BOT_TICKS.md` step 8 says the routine "solves the flat gravity root, converts to a world
direction and re-extracts the pair in the mount's local frame". That is **correct as far as it
goes**, and incomplete in three ways: the yaw is produced first and in the world frame, the whole
conversion is skipped when `mount` is `NULL`, and there is an unconditional negate-and-wrap after
it. The corrected step 8 line is in the follow-up section.

### `004B4D80` - pair to world direction (`00955752..00955765`)

`__fastcall(float3* out)(float pitch, float yaw) -> float3*` (`EAX`), `RET 8`, body
`004B4D80..004B4DF0`:

```
out->x = sin(yaw) * cos(pitch)
out->y = sin(pitch)
out->z = cos(yaw) * cos(pitch)
```

`FSIN`/`FCOS` on the raw arguments; the intermediates are spilled to `float` slots before the two
products. The call passes `*outPitch` first and `*outYaw` second, and the result buffer is a local.

### `00414DB0` / `00B63D50` - the cached inverse (`0095578E..009557B1`)

```
0095578e  CMP byte ptr [ESI+10Ch],0 ; JNZ 009557B6
00955797  MOV ECX,ESI ; CALL 00414DB0
0095579e  LEA EDX,[ESI+0CCh] ; LEA ECX,[ESI+110h]
009557aa  MOV byte ptr [ESI+10Ch],1
009557b1  CALL 00B63D50
```

Entity pose layout, as the producers write it (not inferred from this consumer):

| Offset | Meaning | Producer |
| --- | --- | --- |
| `+0CCh` | world matrix, 4 rows of 4 floats (X basis, Y basis, Z basis, translation) | `00414DB0` via `004134F0` |
| `+0C8h` | world-matrix-valid byte | `00414DF6` sets it to 1 |
| `+0FCh` | world position = `+0CCh` row 3 | `00427EB0` returns this pointer |
| `+10Ch` | inverse-valid byte | `00414DFD` **clears** it; `009557AA` sets it |
| `+110h` | the inverse matrix | `00B63D50` writes it |

The ordering is load-bearing: `00414DB0` sets `+0C8h` and **clears** `+10Ch` as its last two
stores, so `009557AA`'s `MOV byte ptr [ESI+10Ch],1` has to come after the call, and it does.
`00B63D50 BSP_Matrix_BuildOrthogonalScaledAffineInverse` is `__fastcall(dest, src)`, `RET`, body
`00B63D50..00B63F09`; this pass read its entry convention (`ECX` dest, `EDX` src) and its exit
stores into `ECX+0Ch/1Ch/2Ch/38h/3Ch`, and otherwise relies on the existing ledger name. Its
interior is **not** re-derived here.

### `0042D0D0` - world direction to mount-local (`009557B6..009557C7`)

`__fastcall(float3* out, const float3* v)(const float* m, int normalize) -> float3*`, `RET 8`,
body `0042D0D0..0042D188`. Called with `m = mount+110h` and `normalize = 0`, so:

```
out.x = m[00h]*v.x + m[10h]*v.y + m[20h]*v.z
out.y = m[04h]*v.x + m[14h]*v.y + m[24h]*v.z
out.z = m[08h]*v.x + m[18h]*v.y + m[28h]*v.z
```

No translation, no normalization. (`src/material_effect_plane.*` already publishes this exact
branch as `transform_effect_direction_0042d0d0_no_normalize`; this packet does not include that
header because it drags the D3D9/render-command declarations into what is otherwise a pure
ballistics rule. The rule is identical.)

### `00521370` - local direction back to a pair (`009557E6..009557F3`)

`__fastcall(const float3* dir, float* outVert)(float* outHorz)`, `RET 4`, body
`00521370..005213C2`:

```
*outVert = asin( clamp(dir.y, -1.0, 1.0) )    ; 00D7A250 = -1.0, FLD1 for the upper bound; 0042CF10
*outHorz = atan2( dir.x, dir.z )              ; 00BF701A
```

The clamp is the routine's own, applied before `0042CF10`. `EDX` receives the **vertical** angle
and the stack argument the **horizontal** one - the same order `00955630` passes `outPitch`/
`outYaw`.

`00521370`'s frame balances only because `0042CF10` cleans its own argument: the prologue is
`PUSH ECX; PUSH ESI; ...; PUSH EDI` plus the argument `PUSH ECX` at `00521398`, and the epilogue
pops three registers. `0042CF9C RET 4` is what accounts for the fourth push, and it is also what
makes `005213B9 MOV EAX,[ESP+10h]` read the stack argument rather than the return address.

### `0042CF10` - `asin` from `atan` (`005213A0`)

`BSP_Geometry_AsinClamped`, `RET 4`, body `0042CF10..0042CF9C`. `00521370` is the only caller on
this path, and `src/gun_gravity_arc.cpp` reconstructs it in full, so it is derived here rather than
carried over:

```
0042cf19  COMISS XMM0,[00D7A24C] ; JBE 0042CF2E ; FLD [00CE3C64]   ; x > 1.0f   -> +pi/2
0042cf2e  MOVSS XMM1,[00D7A260] ; COMISS XMM1,XMM0 ; JBE 0042CF47
0042cf3b  FLD [00CE3CCC]                                            ; -1.0f > x  -> -pi/2
0042cf47  FLD [ESP+0Ch] ; FST qword [ESP] ; FMUL ST0 ; FSTP [ESP+0Ch]  ; x saved WIDE, then x*x
0042cf54  FLD1 ; FSUBRP ST1 ; FSTP [ESP+0Ch]                        ; 1 - x*x, rounded to float
0042cf60  CALL 00BF7030 ; FSTP [ESP+0Ch]                            ; sqrt of it
0042cf6d  FADD qword [00D7A210] ; FDIVR qword [ESP]                 ; (double)x / (root + 1.0)
0042cf7e  CALL 00BF8490 ; FSTP [ESP+0Ch]                            ; atan of the quotient
0042cf8b  FADD ST0,ST0                                              ; doubled
```

`00D7A24C`/`00D7A260` are the floats `1.0`/`-1.0`, `00CE3C64`/`00CE3CCC` the floats `+pi/2`/
`-pi/2`, and `00D7A210` the double `1.0`. The identity is `asin(x) = 2*atan(x / (1 + sqrt(1-x*x)))`.
The numerator is the **double** copy of the argument saved by `0042CF4B`, not the float slot, which
is the one place the reconstruction had to read the width rather than assume it. A `NaN` fails both
`COMISS` tests (unordered sets `ZF=CF=1`, so both `JBE`s are taken) and propagates through the
identity. Both tests are strict, and `00521370` has already clamped the argument to `[-1, 1]`, so
**on this path neither early out is ever taken**: at exactly `+-1.0` the identity itself yields
`2*atan(1/(0+1)) = 2*(pi/4) = pi/2`. The early outs exist for `0042CF10`'s other callers.

### The negate and the wrap (`009557F8..00955811`)

```
009557f8  MOVSS XMM0,[00D7A208] ; SUBSS XMM0,[EDI] ; MOVSS [EDI],XMM0   ; *outYaw = -0.0f - *outYaw
0095580a  MOV ECX,EBP ; CALL 00605070                                    ; wrap *outPitch
0095580f  MOV ECX,EDI ; CALL 00605070                                    ; wrap *outYaw
```

`00D7A208` is the float `-0.0f`, so this is an exact negation (`-0.0 - x == -x` for every `x`,
including both zeros). It matches `docs/GUN_AIMING.md`'s statement that the gun's `horzAngle` is
`-atan2(x, z)` in the gun frame, and the sibling `008FDAF0`
(`bsp::gun_bot_angles_from_local_008fdaf0`) does the same subtraction from the same constant.

**The negate is unconditional.** It is applied even when `mount == NULL`, i.e. even to the world
frame yaw that `009556AC` wrote from `atan2` - so the `mount == NULL` sites get `-atan2(dx, dz)`,
not `atan2(dx, dz)`.

`00605070` is `__thiscall(float*)`, `RET`, body `00605070..006050BD`:
`x = fmod(x, 2pi)` (`00CE3828`), then `if (x <= -pi) x += 2pi;` (`00CE3D18`)
`else if (x > pi) x -= 2pi;` (`00CE3D28`), in place - the result lands in `(-pi, pi]`.
`src/ship_ai_bearing_rating.cpp`'s `ship_ai_firepower_wrap_angle_00605070` already reconstructs it
and this packet reuses that function rather than writing a second copy.

## The failure path, exactly

Returns `false` **only** when `D < 0.0f` at `009556F8`. On that path:

* `*outPitch` is set to `pi/4` (`00CEB5A8`) at `0095570C`;
* `*outYaw` already holds `atan2(d.x, d.z)` from `009556AC` - it is never reset;
* and then **the common tail runs unchanged**. `004B4D80` builds a direction from `(pi/4, yaw)`,
  the mount branch converts it and `00521370` re-extracts the pair, the yaw is negated and both
  angles are wrapped.

So a `false` result does not leave the outputs untouched or poisoned: both are valid, wrapped,
mount-local angles for a `pi/4` lob toward the target's bearing. Callers that ignore the boolean
still get a usable pair - which is what `006DF520` step 12 relies on, since it uses the returned
flag only to decide whether to arm the shot, after `0085ABA0` has already been given the pair.

`005476EB` and `0085B8DE` both branch on `AL` immediately (`TEST AL,AL`), and `006DEF30` does too
at `006DEFAD`; `0095A0AA` and `0095A4E3` consume `[ESP+24h]` first.

## Degeneracy - the range at which there is no solution

`D < 0` is `R^2 < 4*k*(k+h)` with `k = g*R^2/(2*v^2)`. Multiplying out:

```
D = (R^2 / v^4) * ( v^4 - g*(g*R^2 + 2*h*v^2) )
```

so for `R > 0` the sign of `D` is the sign of `v^4 - 2*g*h*v^2 - g^2*R^2`, and

```
solvable  <=>  R <= (v/g) * sqrt(v^2 - 2*g*h)
```

* Level fire (`h = 0`): maximum range `v^2 / g`, the classic 45-degree range. At `g = 9.81`, a
  `700 m/s` gun reaches `49949 m`, a `250 m/s` gun `6371 m`, a `100 m/s` gun `1019 m`.
* Target above the muzzle (`h > 0`) shortens it; when `v^2 <= 2*g*h` there is **no** `R` at all,
  not even `R = 0`, and the routine always returns `false`.
* Target below the muzzle (`h < 0`) lengthens it.
* At exactly the limit, `D = 0` and the low root becomes `T = R/(2k) = v^2/(g*R)`.

Two arithmetic edges, both reachable and neither guarded:

* **`R == 0`** (the aim point directly above or below the muzzle): `hsq = 0`, `k = 0`, `D = 0 - 0
  = 0`, the `JBE` is taken, and `T = (0 - 0) / (0 + 0)` is `0/0`. x87 raises invalid and produces
  the indefinite QNaN, so `*outPitch` becomes `NaN` and the routine returns **`true`**. Every
  downstream angle is then `NaN` (the wrap's `fmod` propagates it).
* **`v == 0`** with `R > 0`: `k = +inf`, `D = -inf`, `D < 0`, so the failure path handles it and
  `*outPitch` is `pi/4`. `v == 0` **and** `R == 0` gives `k = 0/0 = NaN`, `D = NaN`, and `JBE` is
  taken on the unordered result - so `NaN` in, `NaN` out, `true` returned.

## What is proven vs. assumed

Proven from the listing at the cited addresses:

* the six-argument `__fastcall` ABI, from `RET 10h` plus the frame offsets, and confirmed
  independently at all six call sites;
* every constant, read as bytes from its address: `00CF9058 = 9.8100004196167` (double),
  `00D7A328 = 4.0` (double), `00CEB5A8 = 0.78539819` (float `pi/4`), `00D7A208 = -0.0f`,
  `00D7A250 = -1.0` (double), `00CE3828/00CE3D18/00CE3D28 = 2pi/-pi/+pi` (doubles),
  `00CE3820 = 1e-10` and `00CE3C70 = 1e-5` (doubles), `00D7A24C/00D7A260 = +-1.0` (floats),
  `00CE3C64/00CE3CCC = +-pi/2` (floats), `00D7A210 = 1.0` (double);
* the full expression and its x87 evaluation order, including which intermediates are rounded to
  `float`;
* that the **low** root is taken (`FSUBR` at `00955728`) and that the high root is unreachable;
* that `00BF8490` is `atan` and not `asin` (`FLD1; FPATAN` at `00BF84FD` inside `00BF84E8`), and
  that `00BF7030` is `sqrt`;
* the failure condition `D < 0.0f` and the fact that the tail runs on the failure path too;
* the bodies of `004B4D80`, `00521370`, `0042CF10`, `00605070`, `0042B260`, `0042D0D0`, read in
  full - the first four from the listing, the last two also cross-read against the published
  reconstructions they reuse;
* the six call sites as an **exhaustive** set, not a call-graph result: a sweep of every `E8`/`E9`
  `rel32` in `.text` for a target of `00955630` returns exactly `005476EB`, `006DEFA8`, `006DFAD4`,
  `0085B8DE`, `0095A0AA`, `0095A4E3` and nothing else. `FUN_006DEEC0`'s only recorded callee is
  `00927F10`, which independently confirms that `006DEFA8` falls outside its body;
* `00414DB0`'s two final stores (`+0C8h = 1`, `+10Ch = 0`) and the resulting ordering constraint;
* that `00F87574` is a `float3`, from the producer `006DEEF0..006DEF20`.

Assumed or carried over, not re-derived here:

* `00B63D50` really computes the affine inverse of `+0CCh`. Its entry convention and its exit
  stores were read; its 145-instruction interior was not, and the existing ledger name
  `BSP_Matrix_BuildOrthogonalScaledAffineInverse` is taken on trust.
* That `mount+110h` is a similarity transform, so that a unit world direction stays unit after
  `0042D0D0`. `0042D0D0` is called with `normalize = 0`, and `00521370` clamps only `y`; a mount
  matrix with non-uniform scale would therefore distort the recovered pitch. No mount with such a
  matrix was looked for.
* The parameter names `mount`, `aimPoint`, `muzzlePos`, `muzzleSpeed`, `outPitch`, `outYaw` are
  descriptions of use at the call sites, not recovered identifiers.
* Nothing here is run-time validated. `bsp_game.exe` does not currently reach `00955630`'s
  reconstruction (that is the point of the follow-up), so rule 6 of the verification checklist
  does not apply yet - and will apply to the wiring packet.

### Second pass

The derivation above was written in one session and re-derived from scratch in a second, from the
exported listing, from `disasm-raw` over the PE bytes and from the PE's own `.rdata` - deliberately
without consulting the first pass's conclusions while reading. **Nothing had to be corrected.** The
gravity constant, the discriminant, the choice of the minus root, the `RET 10h` argument count and
all six call sites came out identical. Two omissions were filled in: `0042CF10` had no section here
even though `src/gun_gravity_arc.cpp` reconstructs it in full, and its five constants appeared
nowhere in the address list. `tools/verify_report_calls.py reports/cc7_gun_gravity_arc.json` passes
against live Ghidra: 23 call rows checked, 0 failed.

Coverage: `00955630..0095581F`, **complete** - every instruction in the body is accounted for
above. `006DEF30..006DEFEA` is read only as a call site (`no_ghidra_function`). Every other
routine named above is read for its contract, not reconstructed by this packet.

## Reconstruction

`include/bsp/gun_gravity_arc.hpp` publishes the pure rule: `GunGravityArcQuery` (the four numeric
inputs plus an optional mount frame), `GunGravityArcSolution` (the two angles and the flag), and
`solve_gun_gravity_arc_00955630`. It takes explicit inputs, touches no globals and has no host
interface - the caller supplies the three basis rows of `mount+110h` when it wants the local-frame
conversion and leaves them out when it does not, which is the `mount == NULL` case.

`bsp::BombVector3` is reused for the float3 rather than a new type, `bsp::GunAimAngles` for the
pair, `bsp::kGunBotGravity`/`kGunBotQuarterPi` for the constants, and
`bsp::ship_ai_firepower_wrap_angle_00605070` for the wrap.

### Why three callees are re-projected here instead of reused

Four of the callees already have a reconstruction somewhere in the tree, and this packet reuses
exactly one of them. The reasons differ and are worth stating, because a later reader will
otherwise see three duplicates:

| Callee | Existing reconstruction | Reused? |
| --- | --- | --- |
| `00605070` | `ship_ai_firepower_wrap_angle_00605070`, `src/ship_ai_bearing_rating.cpp` | **yes** - a plain `float(float)` |
| `0042D0D0` | `transform_effect_direction_0042d0d0_no_normalize`, `src/material_effect_plane.cpp` | no - its header drags the D3D9/render-command declarations into a pure ballistics rule |
| `0042B260` | `normalize_camera_basis_0042b260`, `src/camera_decomposition.cpp` | no - see below |
| `0042CF10` | `camera_asin_clamped_0042cf10`, `src/camera_position_modes.cpp` | no - see below |

The two camera ones are `__declspec(naked)` inline-assembly replicas: they reproduce the native
instruction stream exactly, extern `_CIsqrt`/`_CIatan` included, and they are x86-MSVC-only. That
makes them a **different class of artifact** from this module, which publishes a portable rule and
states its one deliberate departure from the original arithmetic (64-bit intermediates instead of
the native 80-bit x87). Calling a bit-exact asm replica from inside a 64-bit projection would not
make the whole any more exact - the surrounding steps still round differently - and it would drag
an x86-asm dependency into a header that advertises explicit inputs and no host interface. So
`normalize_with_floor_0042b260` and `asin_clamped_0042cf10` here are deliberate re-projections of
the same two addresses in the same style as the rest of the file, not accidental duplicates. Both
were re-derived from the listing for this packet and agree with the asm replicas instruction for
instruction; the divergence risk is that a later correction to one copy misses the other, which is
why both carry the native address in their name. The reconstruction preserves the x87
ordering and the `float` round-trips of `k + h`, `D`, `s` and `T`; it does not preserve the 53-bit
intermediate precision, because MSVC/SSE2 on Win32 evaluates `double` expressions at 64 bits. The
difference is below the last bit of the `float` results in every case tested.

This is a **bounded implementation**: reconstructed and build-tested, not ABI-compatible and not
game-validated. It is not a drop-in binary replacement.

## Follow-up packets

1. **Wire it into `006DF520` step 8.** `include/bsp/gun_bot_ticks.hpp:430` declares
   `virtual bool solve_gravity_arc_00955630(GunAimAngles& out) = 0` and `src/gun_bot_ticks.cpp:463`
   calls it; `src/game_hosts_gunnery.cpp` implements the host. That host should now call
   `bsp::solve_gun_gravity_arc_00955630` with the muzzle position from
   `00427EB0(gun+3F8h -> +34h)`, the speed from that node's `+50h`, the led aim point from step 6,
   and the three basis rows of the mount's `+110h`. Those files belong to another packet and were
   not touched here. The same wiring serves `00959C20`'s kind 3 and kind 5 arms
   (`src/unit_gunnery_pass.cpp`).

2. **Correct `docs/GUN_BOT_TICKS.md` step 8 and its coverage row.** The step 8 line should read:
   "`00955630(unit, aimPoint, muzzlePos, muzzleSpeed, &vert, &horz)` -> `bool`; writes the world
   yaw `atan2` first, solves the **low** root of `k*T^2 - R*T + (k+h) = 0` with `k = g*R^2/(2v^2)`
   and `g = 9.81` (`00CF9058`), falls back to `pi/4` with `false` when `D = R^2 - 4k(k+h) < 0`,
   rebuilds a world direction and - only when the mount is non-NULL - re-extracts the pair in the
   mount's local frame; the horizontal half is negated and both angles wrapped into `(-pi, pi]`
   on every path." The coverage row for `00955630` stays `complete`; that file is not leased here.

3. **`00B63D50`**, body `00B63D50..00B63F09`, 145 instructions. Its interior is the one
   unverified link in this chain: if it is not a true inverse, every mount-local pair this routine
   returns is wrong by whatever it actually computes. Worth a packet of its own; it has many
   callers.

4. **`00547480` and `0085B7D0`.** Two of the six call sites have no reconstruction and no doc.
   `0085B7D0` is the only caller that passes `mount = NULL` and a static origin, so whatever it
   is, it wants world-frame angles from a relative delta; that is a distinct contract from the
   other five and deserves naming.

5. **`006DEF30..006DEFEA`** has no Ghidra function. It is a second bot aim-and-fire path
   (`0085A8B0` then `gun->vtable[1D4h]`) parallel to `006DF520` step 8, and it is currently
   invisible to the call graph and to `tools/verify_report_calls.py`.

6. **`include/bsp/projectile_impact.hpp:27`** carries the wrong value for `00CF9058` (see the
   correction above). One-line fix for whoever owns that file.
