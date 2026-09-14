# The orthogonal-scaled affine inverse - `00B63D50`

Addresses: `00B63D50` (body `00B63D50..00B63F09`, `RET`, 145 instructions, leaf, branch-free);
constant `00D7A24C` (float `1.0`); the 71 direct call sites listed below; read for the contract
but not annotated: `00414E10`, `00414DB0`, `0042D0D0`, `00521370`, `0042CF10`, `00D7A250`,
`00CE3C64`, `00CE3CCC`.

Report: `reports/cc7_matrix_orthogonal_inverse.json`. Packet `cc7_matrix_orthogonal_inverse`,
read-only Ghidra pass (no renames, comments, prototypes, function creation or saves).

**This packet publishes no new module.** `00B63D50` is already reconstructed as
`derive_pose_affine_inverse_00b63d50` (`include/bsp/pose_derived.hpp`, `src/pose_derived.cpp`,
packet `orch3_pose_derived_i`), and an independent re-reading of the listing agrees with that
contract in every particular - see "Agreement with the existing reconstruction". The new artefacts
are this document, the report, and one regression case in `tests/math_tests.cpp` that pins a claim
which had no standing automated coverage.

`BSP_Matrix_BuildOrthogonalScaledAffineInverse` is a hypothesis, not a recovered symbol. Every
other descriptive name used below is the existing ledger name for that address and carries the
same caveat.

## Verdict

**`00B63D50` is a true inverse, and the ledger name is correct.** It computes the transpose of the
source basis with each column divided by the squared length of the source row it came from, which
is exactly `M^-1` whenever the three source basis rows are mutually orthogonal and non-zero -
including a per-axis scale and a mirrored, negative-determinant basis. `M * N` and `N * M` are
both the identity to `8.9e-7` for an orthonormal basis and `4.9e-6` for a per-axis-scaled one
(section "Numerical verification").

Three qualifications, all of which matter to the callers:

1. It is **branch-free**: there is no degenerate-scale guard, no early out, and no separate path
   for a mirrored basis. A sheared (non-orthogonal) basis silently produces a non-inverse - the
   round trip deviates by `3.0e-1` for a shear of 0.3 - and a zero-length row divides by zero.
2. It is **not safe in place**: the native writes `destination+10h` at `00B63E52` and then reads
   `source+10h` at `00B63E5E`. All 71 direct call sites pass disjoint operands.
3. The name's "Scaled" is **per-axis**, not uniform: each row is divided by its own squared
   length, so a basis with three different axis scales inverts correctly. A uniform-scale-only
   routine would divide the whole transpose by one scalar, and a plain transpose would divide by
   nothing; `00B63D50` is neither.

**This does not make the gun gravity arc's local-frame path unconditionally sound.** It is sound
only for an orthonormal mount basis, which is a strictly stronger condition than the one
`docs/GUN_GRAVITY_ARC.md` assumed - and stronger, too, than the "mutually orthogonal nonzero rows"
that `00B63D50`'s own contract requires, because a uniform scale satisfies the routine and still
biases the pitch. See "The gravity arc's round trip", the correction the integrator should fold
into that doc, and "Does the precondition hold where the arc uses it?" for how far the mount side
could be traced: no producer read to date puts scale into a pose local, one unread link remains,
and the arc's call site does not alias.

## Original ABI

| Evidence | Reading |
| --- | --- |
| `00B63D50 SUB ESP,18h` / `00B63F06 ADD ESP,18h`, then `00B63F09 RET` with **no immediate** | `__fastcall`, no stack arguments. The `18h` is scratch for the float32 spills only; nothing is read from it before it is written. |
| First use of `EDX` is `00B63D53 FLD float ptr [EDX+4]`, first use of `ECX` is `00B63E4B FSTP float ptr [ECX]` | `EDX` = source (read only), `ECX` = destination (written only). |
| No instruction in the body writes `EAX` (filtered the whole 145-instruction listing) | returns nothing. The sibling `00B63B30` returns the destination in `EAX`; this one does not, and no call site reads a result. |
| No `Jcc`, no `CALL`, no `LOOP` anywhere in `00B63D50..00B63F09` | leaf, branch-free, straight line. `EBX`/`ESI`/`EDI`/`EBP` are never touched, so there is no prologue to save them. |
| `00B63E55 XORPS XMM0,XMM0` and `00B63EF4 MOVSS XMM0,[00D7A24C]` | `XMM0` is clobbered; no other SSE register is used. |

So: `__fastcall(float* destination /*ECX*/, const float* source /*EDX*/)`, `RET`, void.

## Matrix layout

Row-major 4x4 of `float32`, **16-byte row stride**, row-vector convention (`v' = v * M`).

| Offset | Role | Evidence |
| --- | --- | --- |
| `+00h/+04h/+08h` | basis row 0 | loaded at `00B63D53..00B63D66` |
| `+10h/+14h/+18h` | basis row 1 | loaded at `00B63DA7..00B63DB8` - the +10h step is the stride |
| `+20h/+24h/+28h` | basis row 2 | loaded at `00B63DF2..00B63E03` |
| `+30h/+34h/+38h` | translation | read at `00B63E9E`, `00B63EA1`, `00B63EA9` and only there |
| `+0Ch/+1Ch/+2Ch/+3Ch` | homogeneous column | **never read** from the source; forced on the destination |

The producer settles this independently of `00B63D50`: at `007BEB44..007BEB6B` the caller writes
the global `float3` at `00F87574` into `[ESI+30h]`, `[ESI+34h]`, `[ESI+38h]` and then passes `ESI`
as the source, so `+30h..+38h` is the translation row of the source matrix, not a consumer's
guess. `00414DB0 BSP_EntityPose_RefreshWorld` writes the same layout at owner `+0CCh`.

The destination's fourth column is written unconditionally from `XMM0`: `0.0f` at `00B63EE2`
(`+0Ch`), `00B63EEA` (`+1Ch`) and `00B63EEF` (`+2Ch`) from the `XORPS` at `00B63E55`, and the
float at `00D7A24C` at `00B63EFE` (`+3Ch`). `00D7A24C` is `00 00 80 3f` = `1.0f`, read from the
image. All 16 destination words are written.

## The arithmetic, instruction by instruction

### Squared lengths (`00B63D50..00B63E3C`)

The same 38-instruction block runs three times, once per basis row, differing only in the source
offsets and the scratch slot the result lands in. Row 0, in full:

```
00b63d53  FLD [EDX+4] ; MOVSS XMM0,[EDX] ; FSTP [ESP]   ; MOVSS [ESP+4],XMM0
00b63d63  FLD [EDX+8] ; MOVSS [ESP+0Ch],XMM0 ; FSTP [ESP+8]
          ; scratch now holds [ESP]=s1 [ESP+4]=s0 [ESP+8]=s2 [ESP+0Ch]=s0
00b63d70  FLD [ESP] ; FLD [ESP+4] ; FLD [ESP+8]         ; st0=s2 st1=s0 st2=s1
00b63d7b  FLD ST1 ; FMULP ST2                           ; st1 = s0*s0
00b63d7f  FXCH ; FSTP [ESP+8]                           ; s0*s0 rounded to float32
00b63d85  FLD [ESP+8] ; FLD ST2 ; FMULP ST3             ; st2 = s1*s1
00b63d8d  FXCH ST2 ; FSTP [ESP+8]                       ; s1*s1 rounded to float32
00b63d93  FLD [ESP+8] ; FADDP ST2,ST0                   ; st1 = fl(s0^2) + fl(s1^2), 80-bit
00b63d99  FMUL ST0 ; FSTP [ESP+8]                       ; s2*s2 rounded to float32
00b63d9f  FADD [ESP+8] ; FSTP [ESP+10h]                 ; L0^2, one final rounding
```

So `L^2 = fl( fl(s0^2) + fl(s1^2) + fl(s2^2) )`: the three products are each rounded to `float32`
through a spill, the two additions stay in the x87 register, and the sum is rounded once. The
three results land in `[ESP+10h]` (row 0, `00B63DA3`), `[ESP+14h]` (row 1, `00B63DEE`) and
`[ESP+8]` (row 2, `00B63E39`). `[ESP+0Ch]` keeps `s0` untouched from `00B63D66` until it is
reloaded at `00B63E3D`, which is why the first quotient does not re-read `[EDX]`.

### The transposed, row-normalised basis (`00B63E3D..00B63E99`)

```
00b63e3d  FLD [ESP+0Ch] ; FLD [ESP+10h] ; FLD ST0 ; FDIVP ST2,ST0 ; FXCH ; FSTP [ECX]
00b63e4d  FLD [EDX+4] ; FDIV ST0,ST1 ; FSTP [ECX+10h]
00b63e55  XORPS XMM0,XMM0
00b63e58  FDIVR [EDX+8] ; FSTP [ECX+20h]
```

`FDIVR mem` computes `mem / st0`, and `st0` is `L0^2` left on the stack, so the three stores are
`s0/L0^2`, `s1/L0^2`, `s2/L0^2` into `ECX+00h`, `ECX+10h`, `ECX+20h` - **source row 0 becomes
destination column 0**. `00B63E5E..00B63E7B` repeats it for row 1 into `ECX+04h/14h/24h` with
`[ESP+14h]`, and `00B63E7C..00B63E99` for row 2 into `ECX+08h/18h/28h` with `[ESP+8]`. Each
quotient is one x87 division rounded once by its `FSTP float ptr`.

In index form, for `i, j < 3`:

```
N[i][j] = M[j][i] / (M[j][0]^2 + M[j][1]^2 + M[j][2]^2)
```

### The translation (`00B63E9A..00B63F03`)

```
00b63e9a  FLD [ECX]    ; FCHS ; FMUL [EDX+30h]           ; -N[0][0]*tx
00b63ea1  FLD [EDX+34h] ; FMUL [ECX+10h] ; FSUBP         ; ... - ty*N[1][0]
00b63ea9  FLD [EDX+38h] ; FMUL [ECX+20h] ; FSUBP         ; ... - tz*N[2][0]
00b63eb1  FSTP [ECX+30h]
```

repeated at `00B63EB4` for `ECX+34h` with `ECX+04h/14h/24h` and at `00B63ECF` for `ECX+38h` with
`ECX+08h/18h/28h`. `FSUBP` with no operand is `ST(1) = ST(1) - ST(0)` then pop; that reading is
confirmed numerically rather than assumed - the reversed reading would put the wrong sign on two
of the three terms and the round-trip check below would not return the identity in row 3, which
it does.

The translation reads the **destination columns it has just written**, not the source rows, so it
picks up their float32 rounding. The whole three-term dot product stays on the x87 stack and is
rounded once at the `FSTP`. It is `N[3] = -(M[3] . column j of N)` - the correct row-vector
inverse translation, since `v = (v' - t) * R^-1 = v' * N - t * N`.

Three `MOVSS` stores from the zeroed `XMM0` and the `1.0f` load are interleaved into this block
(`00B63EE2`, `00B63EEA`, `00B63EEF`, `00B63EF4`, `00B63EFE`) purely as instruction scheduling;
they touch no x87 state.

## Why this is an inverse, and exactly when

Write `R` for the 3x3 basis with rows `r0, r1, r2`, and `C` for the destination basis, so
`C[i][j] = R[j][i] / |r_j|^2`. Then

```
(R C)[j][k] = sum_i R[j][i] C[i][k] = (r_j . r_k) / |r_k|^2
```

which is the identity **iff** `r_j . r_k = 0` for `j != k` and `|r_k| != 0`. Orthogonality of the
rows is the whole precondition. Nothing requires the rows to be unit length, so any per-axis
scale inverts; nothing requires a positive determinant, so a mirrored basis inverts. Shear does
not, and the routine does not detect it.

For the uniform-scale case the formula reduces to the familiar `transpose / s^2`, and for the
orthonormal case to the plain transpose - but the code always takes the general path, which is
evidence that the engine's pose matrices were expected to carry non-unit and possibly per-axis
row scales. A plain transpose would have been three quarters the size.

## Agreement with the existing reconstruction

`include/bsp/pose_derived.hpp` lines 7-14 already state the contract, from packet
`orch3_pose_derived_i`. This packet re-read the listing without consulting that header first, and
agrees with all of it: `ECX` destination / `EDX` source / `RET` / no meaningful `EAX`; "an affine
inverse only for mutually orthogonal nonzero upper basis rows"; all 16 outputs written with the
last column forced to `(0,0,0,1)`; and `destination == source` generally not an inverse. The
ledger's one-line summary - "transpose each upper source row divided by squared length, x87
negative transformed translation" - is exactly what the arithmetic above does.

**No disagreement was found.** Three things this pass adds rather than contradicts: the exhaustive
71-site call set (the earlier record did not enumerate them), the numerical round trip below, and
the consequence for the gravity arc.

## Numerical verification

`local/verify_b63d50.py` re-implements the listing in Python with an explicit `float32` rounding
at each observed spill, then multiplies. The case added to `tests/math_tests.cpp` runs the same
round trip against the **existing** `derive_pose_affine_inverse_00b63d50` - the shipped kernel,
not a second implementation - and passes at the tolerances below. Bases were built independently
of the routine, from `sin`/`cos` of a yaw/pitch/roll triple composed as `Rz * Rx * Ry`, over 18
angle triples x 4 scale regimes, each with a non-zero translation:

| Basis | `max(|M*N - I|, |N*M - I|)` over all 16 entries |
| --- | --- |
| orthonormal | `8.881e-07` |
| uniform scale 2.5 | `8.814e-07` |
| per-axis scale (0.5, 3.0, 1.75) | `4.823e-06` |
| mirrored (1.0, -1.0, 1.0) | `1.465e-07` |
| **sheared** rows (0.3 off-diagonal) | **`3.000e-01`** - not an inverse |
| zero-length row 0 | division by zero; on hardware `inf`/`nan`, no guard |

The tolerances are what `float32` storage of the product allows: `8.9e-7` is about `2^-20`, and
the `4.8e-6` of the per-axis case is the wider dynamic range of a basis whose row lengths differ
by a factor of six. The identity matrix in gives the identity out, bit-exact, except that the
translation row is `-0.0f` (the `FCHS` at `00B63E9C` applied to `+0.0`), which compares equal.

The C++ case asserts `< 1.0e-6` for the orthonormal basis, `< 1.0e-5` for the per-axis-scaled one
and `> 0.25` for the sheared one, inside the existing `reconstructed_math` test. It is one case,
and it is justified because there was no standing coverage: `tests/math_tests.cpp` and
`tests/native_math_tests.cpp` contain no reference to `00b63d50`, `pose_affine_inverse` or
`pose_derived`, and the ledger's `native_differential_fixture_passed` status refers to a one-off
fixture run during packet `orch3_pose_derived_i`, not to a standing regression test.

## Does the precondition hold where the arc uses it?

The contract says "an affine inverse **only** for mutually orthogonal nonzero upper basis rows", so
the arc's soundness turns on whether a gun mount's world basis satisfies that. Two parts, one
proven and one bounded.

### The composition rule (proven)

`00414DB0` builds `world_child = local_child * world_parent` through `00413920` and never
normalises. Writing `W` for the parent's world basis and `a_i` for a row of the child's local
basis, the child's world rows are `a_i W`, and

```
(a_i W) . (a_j W) = a_i (W W^T) a_j^T = sum_k a_i[k] a_j[k] w_k^2
```

since `W W^T = diag(w_0^2, w_1^2, w_2^2)` when `W`'s own rows are orthogonal. For that to vanish
whenever `a_i . a_j = 0`, the `w_k^2` must all be **equal**. So:

* an **orthonormal** ancestor chain preserves the precondition exactly;
* a **uniformly** scaled ancestor preserves it too - `00B63D50` still returns a true inverse -
  but the arc's pitch is then wrong by `asin(y/s)`, which is the separate defect below;
* a **per-axis** scaled ancestor breaks orthogonality for every descendant whose local rotation is
  not axis-aligned, and then `00B63D50` is not an inverse at all and both angles are wrong.

Scale therefore does not have to be on the mount itself; anywhere in its ancestor chain will do.

### Where scale could enter (bounded survey, negative result)

`entity+74h`, the local matrix, is written wholesale by the vtable slot `88h` setters `00431410`
and `006E00A0` from a caller-supplied matrix, so the question is what the producers supply.
`docs/ENTITY_LOCAL_MATRIX.md` "The producers" is that packet's survey of the slot-`88h` call sites,
and **none of the four introduces scale of any kind**:

| Producer | What reaches `+74h` | Scale? |
| --- | --- | --- |
| matrix interpolator `00904600` at `00904AE3` | `((((RotZ * RotY) * RotX) * Translate) * record.base_20h)`, four chained `00413920` - pure rotations and a translation, not an element-wise lerp (`docs/WORLD_TIMED_ATTACHMENTS.md`) | no |
| spawn/placement at `006E5D03`, no Ghidra function | a stack matrix built from the identity constant `00D7A24C` | no |
| unit attach `00925CE0` at `009259C4` | identity at `00925DCA` and `00925EA0`, then the creator's `localFrame` | none of its own |
| ship motion `00825F20` | nothing - it reads `+74h` nine times and writes it never | n/a |

`0042D700 BSP_Matrix_ReturnIdentity4x4` and `0042D770` are identity writers on the same interface.

**The one open link** is the `localFrame`: `009259C1 LEA ECX,[ESI+74h]` / `009259C4 CALL 004134F0`
inside `BSP_SceneNode_AttachToParents` (`009258F0..009259FA`) copies the matrix the caller pushed
at `[ESP+14h]` straight into `+74h`, unvalidated. Whatever an attach caller passes lands there, and
the attach callers were **not enumerated**. So the finding is "no producer read to date puts scale
into a pose local", not "no producer can".

On the balance of that evidence the arc's mount bases are orthonormal and both the inverse and the
pitch are correct in practice - but that is a bounded survey resting on another packet's table,
not a proof, and it is the first follow-up below.

### Zero-length rows

No path to the division by zero was found. `+74h` is the **identity at construction**
(`00925CE0` writes it at `00925DCA` and again at `00925EA0`, `docs/UNIT_INSTANCE_LAYOUT.md`), so a
unit's local basis is never observed all-zero, and `00955630` additionally skips the whole
local-frame path when the mount is NULL (`0095576A TEST ESI,ESI`, `0095578C JZ 009557F8`). This is
a negative search result over the unit-instance constructor, not a proof over every entity class.

### Aliasing (question (b))

The arc's call site does **not** alias: `0095579E LEA EDX,[ESI+0CCh]` and `009557A4 LEA ECX,
[ESI+110h]` are 68 bytes apart on the same object and each matrix is 64 bytes, so they do not even
partially overlap. Nor does any other caller - see "Call sites" below: all 71 direct sites pass
disjoint operands, and none passes the same expression for both. The `destination == source`
hazard the contract warns about is real in the code (`00B63E52` writes `dest+10h` before
`00B63E5E` reads `src+10h`) but is not exercised anywhere in the shipped binary.

## The gravity arc's round trip

This is the question `docs/GUN_GRAVITY_ARC.md` could not answer. Its answer is **conditionally
yes, on a condition that doc stated too weakly.**

`00955630` builds the cached inverse at `009557B1` (`ECX = mount+110h`, `EDX = mount+0CCh`), then:

* `0042D0D0`, called at `009557C7` after the setup at `009557B6..009557C3` - `PUSH 0` is the
  `normalize` flag and `ADD ESI,110h` / `PUSH ESI` the matrix just written. Its body confirms the row-vector
  transform: `00B63E...`-style reads at `0042D0F2/0042D0FD/0042D10B` are `[EAX+10h]`, `[EAX]`,
  `[EAX+20h]` against `v.y`, `v.x`, `v.z`, so `out.x = m[00h]*v.x + m[10h]*v.y + m[20h]*v.z`, and
  the `JZ 0042D162` at `0042D14E` skips the normalising call `00419510` when the flag is zero.
  With `m = mount+110h` this is `d_local = d_world * M^-1`, which is the right operation.
* `00521370` at `009557E6..009557F3`. Its body: `FLD [ESI+4]` then a compare against the **double**
  `-1.0` at `00D7A250`, `JA` keeping `dir.y` unchanged; the `FLD1` at `00521386` is compared
  against the *constant*, not against `dir.y`, so `00521370` applies only the lower bound. The
  upper bound is `0042CF10`'s own: `0042CF19 COMISS XMM0,[00D7A24C]` / `JBE` returns `+pi/2` for
  `x > 1.0f`. Composed, `pitch = asin(clamp(dir.y, -1, +1))` and `yaw = atan2(dir.x, dir.z)`.

So the pitch is `asin` of the **raw** `y` component of an **un-normalised** local vector. That is
the whole issue:

| Mount basis | local vector for a world direction at pitch 30, yaw 36.87 | pitch | yaw |
| --- | --- | --- | --- |
| identity | `(0.5196, 0.5000, 0.6928)` | `30.000` | `36.870` |
| uniform scale 2 | `(0.2598, 0.2500, 0.3464)` | **`14.478`** | `36.870` |
| uniform scale 0.5 | `(1.0392, 1.0000, 1.3856)` | **`90.000`** (saturated) | `36.870` |
| per-axis, y scaled 2 | `(0.5196, 0.2500, 0.6928)` | **`14.478`** | `36.870` |
| per-axis, x scaled 3 | `(0.1732, 0.5000, 0.6928)` | `30.000` | **`14.036`** |

For an **identity** mount the local vector is bit-identical to the world one (`1.0*v + 0 + 0` in
x87 is exactly `v`) and the angle pair is exactly what the world frame would have produced. The
lead's specific question is answered yes.

But the general condition is **orthonormal**, not "similarity". A uniform scale *is* a similarity
and it still breaks the pitch, because `0042D0D0` is called with `normalize = 0` and `00521370`
takes `asin` of the unnormalised `y`: the inverse of a uniformly scaled basis scales the local
direction by `1/s`, and `asin(y/s) != asin(y)`. `docs/GUN_GRAVITY_ARC.md` line 453 says
"`mount+110h` is a similarity transform, so that a unit world direction stays unit after
`0042D0D0`" - the premise does not imply the conclusion; only `s = 1` does. The yaw survives any
scale that treats `x` and `z` alike, and fails when they differ.

Whether a real mount basis is orthonormal is **not settled here**. `00414DB0` composes the world
matrix as `local(+74h) * parent.world(+0CCh)` through `00413920 BSP_Matrix_Multiply4x4`, so any
scale in any `+74h` along the chain reaches `+0CCh`; the pose system does not normalise. No mount
with a non-unit basis was searched for - that is the follow-up below.

## Call sites

71 direct `E8`/`E9` `rel32` references to `00B63D50` in `.text`, found by scanning the PE on disk
(`local/scan_b63d50_sites.py`) rather than by a call-graph query - Ghidra's `xrefs` answer is
capped and returns 25. The full list with the preceding argument setup is reproduced by that
script. **62 of the 71** are literally `LEA EDX,[reg+0CCh]` / `LEA ECX,[reg+110h]`, the
entity-pose pattern - `0CCh` and `110h` are 68 bytes apart and the matrix is 64, so they never
overlap. The nine that are not:

| Site | destination (`ECX`) | source (`EDX`) |
| --- | --- | --- |
| `006AC409` in `FUN_006AC370` | `[ESP+1Ch]` | `EDI` = `[ESI+0CCh]` from `006AC3E7` |
| `006E5DFE` in `FUN_006E5D70` | `[ESP+5Ch]` | `[ESP+10h]` |
| `007BB4A8`, no Ghidra function, routine `007BB310..007BB570` | `EBX` | `EDI`, the matrix `004134F0` copied into at `007BB49F` |
| `007BEB6B` in `FUN_007BE9B0` | `[ESP+0B0h]` | `ESI`, a bare matrix whose `+30h..+38h` the caller has just written |
| `007D9E37` in `FUN_007D9CE0` | `EDI` | `[EBP+0CCh]` |
| `0081E1D9` | `EBP` | `[ESI+0CCh]` |
| `00849BED` | `[ESI+110h]` | `EDI` |
| `00849C22` | `EBX` | `EDI` |
| `0099ED3F` | `EBP` | `[ESI+0CCh]` |

The two the pattern-matcher could not read back (`0070ED7C`, `007BEB6B`) were resolved from the
Ghidra listing, not from the resynced linear disassembly: `0070ED69/0070ED6F` are
`LEA EDX,[EAX+0CCh]` / `LEA ECX,[EAX+110h]` - the pose pattern - and `007BEB38/007BEB3A` are
`MOV EDX,ESI` / `LEA ECX,[ESP+0B0h]`.

`007BB4A8` has no Ghidra function. Its routine is bounded by the `INT3` padding at `007BB310` and
the `RET` at `007BB570` (inclusive end `007BB570`), and a linear disassembly anchored at that
entry tiles exactly onto the call site, so the `MOV EDX,EDI` / `MOV ECX,EBX` at `007BB4A4/A6` is
anchored rather than resynced. The remaining setup sites were read from the Ghidra listing of
their containing functions.

No site is syntactically in place. For the two register-to-register sites the operands are two
different registers, and for the six mixed sites a register against a different base; that none
of those pairs can ever hold the same address was not separately proven, so the in-place claim is
"no call site passes the same expression", not "no call site can alias".

## What is proven vs. assumed

Proven here, from the listing and the image:

* the complete 145-instruction body, read contiguously in three pages with no gap, and its
  branch-freedom and leaf status by filtering the whole listing for `Jcc`/`CALL`/`LOOP`/`EAX`;
* the ABI, from the balanced `SUB`/`ADD ESP,18h` and the bare `RET`;
* `00D7A24C = 00 00 80 3f = 1.0f` and `00D7A250 = 00 00 00 00 00 00 f0 bf = -1.0` (double), both
  read from the PE, not from a prior doc;
* the layout, from `007BEB44..007BEB6B`'s producer writes into `+30h/+34h/+38h`;
* the inverse property and its precondition, algebraically and numerically, in five regimes;
* the exhaustive call-site set, by scanning `.text` on disk for `rel32` targets;
* `0042D0D0`'s transform and `normalize = 0` branch, and `00521370` + `0042CF10`'s clamp split,
  read from their listings.

Assumed or out of scope:

* whether any mount in the shipped data actually has a non-unit or non-orthogonal basis. The
  arithmetic consequence is derived above; the data was not surveyed, and `bsp_game.exe` was not
  run against this path. **The verdict on the arc is therefore conditional, not a clean bill.**
* the caller-side x87 control word. The native inherits `PC`/`RC` from its caller; everything
  above assumes the MSVC default (64-bit mantissa, round to nearest). A caller running in 53-bit
  precision would change the two wide accumulations.
* the Python model in `local/verify_b63d50.py` uses Python floats where the native keeps x87
  80-bit - the two additions inside each squared length and the three-term translation dot
  product. Products and quotients of `float32` operands are provably identical under either width
  (`64 >= 2*24+2` and `53 >= 2*24+2`), so those are exact; the chained additions are not covered by
  that argument and can differ in the last bit at extreme exponents. The C++ case does not have
  this caveat: it calls the shipped `derive_pose_affine_inverse_00b63d50` kernel itself.
* the producer survey in "Where scale could enter" is another packet's table
  (`docs/ENTITY_LOCAL_MATRIX.md`), re-read here but not re-derived, and the attach callers that
  supply `localFrame` at `009259C4` were not enumerated at all.
* `BSP_Matrix_BuildOrthogonalScaledAffineInverse` remains a descriptive hypothesis. This packet
  confirms it describes the arithmetic correctly; it is still not a recovered symbol.

Coverage: `00B63D50` **complete**. `0042D0D0`, `00521370`, `0042CF10`, `00414DB0` read only as
far as the contract above needs - partial, and not annotated by this packet.

## Corrections for the integrator

1. `docs/GUN_GRAVITY_ARC.md` "Assumed or carried over" (around line 450): `00B63D50`'s interior is
   now read and it **is** the affine inverse - that bullet can be retired. The bullet after it,
   however, is wrong as written: a similarity transform does **not** keep a unit direction unit,
   and `00521370`'s `asin` is taken on the raw `y`. The correct statement is that the mount's
   world basis must be **orthonormal**, and that a uniform scale `s != 1` biases the pitch to
   `asin(y/s)` while leaving the yaw correct.
2. `docs/GUN_GRAVITY_ARC.md` around line 310: "The clamp is the routine's own, applied before
   `0042CF10`" attributes both bounds to `00521370`. Only the lower bound is effectively
   `00521370`'s; its `FLD1` compares against the constant `-1.0`, not against `dir.y`, so the
   upper bound comes from `0042CF10`'s `COMISS` at `0042CF19`. The composed behaviour the doc
   states, `asin(clamp(dir.y, -1, 1))`, is right; the attribution is not.

## Follow-up packets

* **`scene_attach_local_frames`** - the one open link above. Enumerate the callers of
  `BSP_SceneNode_AttachToParents` (`009258F0..009259FA`) and establish what each pushes at
  `[ESP+14h]` as the `localFrame` copied into `+74h` at `009259C4`. That is the only unread way
  scale can reach a pose local, and it decides the arc question outright. Everything else in the
  chain is already read and introduces none.
* **`b63b30_vs_b63d50`** - `00B63B30` `invert_camera_affine_00b63b30` (`include/bsp/camera_inverse.hpp`)
  is the same family with a different schedule: it copies the source first, uses SSE subtraction
  from negative zero for the translation, and preserves source indices 3/7/11/15 instead of
  forcing them. Establish whether the two are the same function compiled twice or differ in an
  observable way, and which callers depend on the preserved fourth column.
* **`b63d50_degenerate_callers`** - the routine divides by zero on a zero-length row with no
  guard. Establish whether any caller can reach it with an uninitialised or collapsed pose, and
  what a resulting `nan` does downstream in `0042D0D0` -> `00521370` -> the gun command.
