# The plane's "pose commit" - `0085DC80` at `007CECBA` (packet `cc7_plane_pose_commit`)

Read-only analysis of `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Every
descriptive name below is a hypothesis, not a recovered symbol. No Ghidra mutation was made and
no ledger record was written.

## The headline, first: this is not the pose integrator

The packet brief asked for "the per-step plane pose commit" on the premise that `0085DC80` is
"the step that would change [a plane's orientation]" and that wiring it would give the
reconstructed host turning flight. **That premise is wrong, and wiring `0085DC80` alone will not
move a single plane's heading.**

`0085DC80` is a general-purpose Gram-Schmidt re-orthonormalisation of the three basis rows of one
row-major 4x4. It is already named in the ledger as `BSP_Matrix_OrthonormalizeBasisRows`. It:

- takes one argument, the matrix pointer in `ECX`, and nothing else - no time step, no velocity,
  no control axis, no unit pointer (`0085DC80 SUB ESP,0x10` .. `0085DDFA RET`, no `imm16`);
- reads and writes only the nine floats at `+00h..+08h`, `+10h..+18h` and `+20h..+28h`;
- integrates nothing. Applied to a matrix that is already orthonormal it is a no-op to within
  float rounding, which the probe in *Verification* below confirms for the identity case.

It is called from **54 sites** across the binary (`bsp.py lookup 0085dc80`), so nothing in it is
plane-specific; `0042B7F0`, `0046CF40 BSP_SceneFile_ReadEntityBlock`, `0074260E` and `006E8625`
are among the others. `007CECBA` is the tail of the plane's fixed-step motion dispatch, and what
it does there is clean up whatever the arm above it left in the matrix. Where the orientation is
actually advanced is answered as far as this packet could take it in *Where the pose does
advance* below.

## The call site

`007CE040 BSP_PlaneTickElement_FixedStep`, block `007CEC30`-`007CECBA`. `EDI` is the unit and
`ESI` is `unit+310h`, the scene node, both established in `docs/PLANE_UNIT_TICK.md` and
re-confirmed here from `007CEC39 LEA ECX,[ESI+41Ch]` being the unit's own `+72Ch`.

```
007cec30  MOV EDX,[ESI+41Ch] / 007cec36 MOV EAX,[EDX+38h] / 007cec3f CALL EAX
007cec43  JZ 007cec75                   ; free-flight gate false
007cec45  CMP byte [ESI+6D0h],0         ; unit+9E0h, the airborne-clock freeze
007cec4e  FLD step / FADD [ESI+5F8h] / FSTP   ; unit+908h += step
007cec6e  CALL 007cc2f0                 ; free-flight arm, then
007cec73  JMP 007cecb4
007cec75  MOV EAX,[EDI+900h]            ; the flight state
007cec7b  CMP EAX,4 / 007cec80 CMP EAX,5
007cec92  CALL 007cbfa0                 ; ground-roll arm, then
007cec97  JMP 007cecb4
007cec99  CMP dword [ESI+5F0h],6        ; the same unit+900h through the node base
007ceca0  JNZ 007cecbf                  ; no arm ran -> the tail is skipped too
007cecaf  CALL 007cba50                 ; water-surface arm, falls through
007cecb4  LEA ECX,[ESI+364h]            ; = unit+674h
007cecba  CALL 0085dc80
```

So the single argument is `unit+674h`. `docs/TICK_ELEMENT_OVERRIDES.md:39` settles what that is:
`unit+674h` holds the last **fixed-step** pose and `unit+74h` holds the pose that is drawn. The
size is settled independently by `007D9F74 MOV ECX,0x10` / `007D9F7D MOVSD.REP`, which copies 16
dwords out of `unit+674h` - a 64-byte row-major 4x4.

The three arms and the skip are decided by one field, `unit+900h`, per `docs/PLANE_UNIT_TICK.md`:
state `7` free flight, `4`/`5` ground roll, `6` water surface, anything else no arm and no tail.
That is the "skips this commit entirely" the brief quoted, and it is confirmed by `007CECA0 JNZ`
landing at `007CECBF`, past the call.

## Original ABI

| | |
| --- | --- |
| entry | `0085DC80` |
| Ghidra body | `0085DC80` - `0085DE93` |
| convention | `__fastcall(float* m /*ECX*/)`, one register argument, no stack argument |
| return | none; `RET` with no `imm16` at `0085DDFA` and `0085DE93`, so the caller pops nothing |
| saved | `EBX`, `ESI`, `EDI` (`0085DC83`, `0085DC84`, `0085DC8A`), restored on both exits |
| frame | `SUB ESP,0x10`; `[ESP+0Ch]` one float scalar, `[ESP+10h..18h]` one vec3 temp. With the three pushes those are exactly the 16 reserved bytes |
| aliasing | `ESI` = `m` = row 0, `EBX` = `m+20h` = row 2 (`0085DC87`), `EDI` = `m+10h` = row 1 (`0085DCB9`) |
| touched | `+00h..+08h`, `+10h..+18h`, `+20h..+28h`. Elements `+0Ch`, `+1Ch`, `+2Ch` and all of row 3 (`+30h..+3Fh`) are never addressed |

**Ledger correction (not applied - this packet is read-only).** The existing
`config/names/` record for `0085DC80` says "body 0085DC80-0085DD30". That is wrong twice over:
`0085DD30` is not an instruction boundary (`0085DD2D LEA ECX,[ESP+0x10]` runs to `0085DD31`), and
the real body runs to `0085DE93`, confirmed by `bsp.py ghidra proto 0085dc80`. The truncated range
stops before **both** exits and before the entire degenerate branch. The rest of that record - the
`__fastcall`, the row 2 / row 1 aliasing, the Gram-Schmidt reading - is correct.

## The rule

`row2` is the authority, `row1` is a hint, `row0` is derived - unless `row1` is useless, in which
case the roles of `row0` and `row1` swap.

### Step 1, `0085DC8D`-`0085DCEB`: renormalise row 2

```
0085dc8b  MOV ECX,EBX / 0085dc8d CALL 00419440   ; len = Length(row2)
0085dc96  FLDZ / 0085dc9c FCOMI / 0085dca0 JBE 0085dcac
0085dca2  FLD1 / 0085dca4 FDIVRP / 0085dca6 FSTP [ESP+0Ch]   ; inv = 1/len
0085dcac  XORPS XMM0,XMM0 / 0085dcb1 MOVSS [ESP+0Ch],XMM0     ; inv = +0.0f
0085dcb7..0085dceb                                            ; row2 *= inv
```

`len <= 0`, and a NaN (`FCOMI` unordered sets `CF` and `ZF`, and `JBE` is `CF|ZF`), both take the
`XORPS` branch and set the scale to `+0.0f`. **There is no error path**: row 2 silently becomes
`(0,0,0)` and the rest of the routine runs on it. `00419510` has the identical guard at
`0041952C`. This is the one real hazard in the routine and the host needs to know about it.

The three scaled components are computed into `[ESP+10h..18h]` and copied back only afterwards
(`0085DCDA`-`0085DCEB`), so no component is read after it has been overwritten.

### Step 2, `0085DCEE`-`0085DD20`: pick the branch

```
0085dcee  FLD [EDI+4] / FMUL [EBX+4] / FLD [EDI] / FMUL [EBX] / FADDP
0085dcfa  FLD [EDI+8] / FMUL [EBX+8] / FADDP / 0085dd02 FSTP [ESP+0Ch]   ; d = dot(row1, row2)
0085dd0c  FABS
0085dd12  FLD double [00D0D0A0] / 0085dd1c FCOMIP / 0085dd20 JBE 0085ddfb
```

`00D0D0A0` holds `00 00 00 E0 CE F7 EF 3F` = `0.9990000128746033`, which is `0.999f` widened to
double. So the test is `|dot(row1, row2)| <= 0.999f`, a `2.5625` degree cone around parallel
**and** around antiparallel. A NaN `d` takes the normal path, because `JBE` is taken on an
unordered compare - the reconstruction writes the test as `!(fabs(d) > 0.999f)` for exactly that
reason.

### Normal path, `0085DDFB`-`0085DE93`: row 1 is a usable up hint

```
0085ddfb..0085de32   row1 -= row2 * d                 ; the Gram-Schmidt projection
0085de35  CALL 00419510 (EDX = EDI)                   ; row1 = normalize(row1)
0085de3c  PUSH EBX / 0085de3f MOV EDX,EDI / 0085de51 CALL 004f9b30
0085de6a..0085de73   row0 = the cross result          ; no negation on this path
0085de78  CALL 00419510 (EDX = ESI)                   ; row0 = normalize(row0)
```

`row0 = normalize(row1 x row2)`.

### Degenerate path, `0085DD26`-`0085DDFA`: row 1 is unusable, row 0 takes over

```
0085dd26  FSTP ST0                                    ; discard the leftover d
0085dd2a..0085dd42   e = dot(row2, row0)
0085dd46..0085dd7f   row0 -= row2 * e
0085dd82  CALL 00419510 (EDX = ESI)                   ; row0 = normalize(row0)
0085dd89  PUSH EBX / 0085dd8c MOV EDX,ESI / 0085dd9e CALL 004f9b30
0085dda7  MOVSS XMM0,[00D7A208]                       ; -0.0f
0085ddb9..0085ddda   row1 = -0.0f - cross             ; a sign flip, component by component
0085dddf  CALL 00419510 (EDX = EDI)                   ; row1 = normalize(row1)
```

`row1 = normalize(-(row0 x row2)) = normalize(row2 x row0)`. The negation is the proof that the
two paths agree on handedness: the normal path asserts `row0 = row1 x row2`, and the only `row1`
consistent with that is `row2 x row0`. Negating before normalising is the same as after, since
the length is unchanged.

Both paths leave row 3 and the fourth column exactly as they were.

## The three helpers

| address | ledger name | ABI | what was read |
| --- | --- | --- | --- |
| `00419440` | `BSP_Vector3f_Length` | `__fastcall(const float* v /*ECX*/)` -> `ST0` | Body re-read here. `00419461`/`0041946F`/`0041947F` square the three components, each rounded back to float through `[ESP+8]`; `0041947D` and `00419485` add on the stack; `00419491 CALL 00BF7030` is the sqrt. It is a length, not a length-squared |
| `00419510` | `BSP_Vector3f_Normalize` | `__fastcall(float* out /*ECX*/, const float* in /*EDX*/)` -> `EAX` = `out` | Body re-read here. `00419519` calls `00419440`, then the same `len > 0 ? 1/len : +0.0f` guard at `00419522`-`0041953D`, then three scaled stores |
| `004F9B30` | `BSP_Vector3f_Cross` | `__fastcall(float* out /*ECX*/, const float* a /*EDX*/, const float* b /*stack*/)`, `RET 4` | Body re-read here. The x87 stack was traced through the x and y components: `004F9B54`-`004F9B73` gives `out.x = a.y*b.z - a.z*b.y` and `004F9B75`-`004F9B97` gives `out.y = a.z*b.x - a.x*b.z`. Standard right-hand-rule formula, argument order `a x b`. `004F9B38` re-reads the argument slot into `ECX` and then reuses that slot as scratch |

## The convention agrees with the established one

The brief asked to say loudly if `0085DC80` disagreed with the project's pose convention. **It
does not disagree.** Three independent checks:

1. **Row 2 is the authority.** `0085DC87 LEA EBX,[ESI+20h]` makes row 2 the only row that is not
   rebuilt, and `0085DC8D` measures its length. That is what you want if row 2 is forward and the
   flight law writes a heading into it - which is the project's convention
   (`src/system_camera_axes.cpp:387` reads `world[8..10]`, `src/game_hosts_units.cpp:529` takes
   `atan2(row2[0], row2[2])`).
2. **Handedness.** `row0 = row1 x row2` with the standard cross formula is `right = up x forward`,
   the left-handed / Direct3D basis: `(0,1,0) x (0,0,1) = (1,0,0)`. The probe below confirms it
   numerically - a `+30` degree yaw about `+Y`, with a stale `row0`, rebuilds `row0` as
   `(cos 30, 0, -sin 30)`, which is the correct right vector for that heading.
3. **An independent caller says the same thing.** `0074260E`, reconstructed in
   `src/land_and_structures.cpp:248`, writes a **path tangent** into row 2 and the **world up
   axis** into row 1, then calls `0085DC80` to complete the frame. Row 2 = forward, row 1 = up
   hint, row 0 = derived. Same reading, from a caller this packet did not touch.

The row-major / row-vector storage itself is `docs/ENTITY_LOCAL_MATRIX.md`'s result from
`00414DB0`, not a new claim here.

## Where the pose does advance (narrowed, not closed)

This part is a by-product of chasing the brief's premise, and it is deliberately labelled short of
proof.

`007CC2F0 BSP_Plane_FreeFlightArm` does **not** write the pose rows. Its full 171-instruction
listing was scanned: no displacement in the `0x364`-`0x3B4` or `0x674`-`0x6B4` range appears
anywhere in it, and its 18 call sites go to the control chain, effects and messaging.

A byte-pattern sweep for x87 accesses `D9 ?? <disp32>` at `+674h`, `+684h`, `+694h` and the three
node-relative equivalents finds the ship and submarine equivalents
(`00825F20 BSP_UnitInstance_UpdateShipMotion` at `00826B4B`, `00855420
BSP_SubmarineUnit_UpdateMotion` at `008554F2`) but nothing in the plane's `007CBxxx`/`007CCxxx`/
`007DXxxx` range. A sweep for `8D ?? <disp32>` - taking the matrix address rather than a component
- finds `007D8252 LEA ESI,[EAX+674h]` and `007D9F6E LEA ESI,[EAX+674h]`, where `EAX` is `ctl->+8h`,
the unit. Both copy the 64-byte matrix out of `unit+674h` and then `004134F0
BSP_Matrix_Copy4x4X87` it into `unit+74h`.

Both of those have exactly one caller: **`007C6500 BSP_PlaneTickElement_AdvancePose`**. That is a
*different tick element* from `007CE040 BSP_PlaneTickElement_FixedStep` - the ledger records it as
tick-element slot `+4h` in all six plane vtables, a raw block `007C6500`-`007C675D` with no Ghidra
function. Its six callees are `0042ED50 BSP_SceneNode_InvalidateSubtreePose`, `007D8230`,
`007D9F60`, `007DE2E0`, `007DE540` and `007DE580`.

**Status: candidate, not established.** What is proven is the negative - `007CC2F0` does not write
the rows - and the pointer: `007C6500` is the only function that reaches the two routines that
move the matrix at `unit+674h`. `007C6500`'s own body was not read, and neither was `007D9F60`'s
(272 instructions). `docs/PLANE_FLIGHT.md:478` already lists `plane_pose_integration` as an open
item and `include/bsp/plane_flight.hpp:16` already lists "the matrix composition that turns the
controller's state into unit+74h / unit+674h" as named-but-not-reconstructed. This packet narrows
that open item to `007C6500` and its callees; it does not close it.

## What could not be established

- **`007C6500`'s rule.** Not read. See above. It is the next packet, and it is the one that
  actually unfreezes the heading.
- **Whether `0085DC80` ever takes the degenerate branch for a plane in practice.** That needs
  `007C6500`'s rule, since it decides how far row 1 can drift in one step.
- **The zero-length forward case at run time.** `0085DCAC` collapses the whole basis to zero
  rather than erroring. No caller was found that guards against it, but only 3 of the 54 callers
  were looked at, so "the native code never guards this" is not claimed.
- **Exact float reproduction.** The reconstruction models the x87 rounding where the native
  rounds (each scaled component reaches memory as a float before it is used) and uses `double`
  where the native keeps a sum on the x87 stack and rounds once. `double` is 53-bit and the x87
  stack is 64-bit, so the dot products, the cross terms and `00419440`'s `xx+yy+zz` can differ
  from the native by up to one ulp of the float result. No bit-exact differential test was run.
- **`00BF7030`.** Assumed to be the CRT `sqrt`. Not disassembled.
- Of `004F9B30`'s three output components, x and y were traced instruction by instruction; z
  (`004F9B9A`-`004F9BA0`) was taken to follow the pattern rather than traced.

## Verification

| claim | status |
| --- | --- |
| `0085DC80`'s listing, both branches, all 176 instructions | **exported** and read |
| the three helpers' ABI and formulas | **exported** and read (`004F9B30`'s z component inferred) |
| `007CECB4`/`007CECBA` passing `unit+674h` | **exported** and read |
| the rule in `src/plane_pose_commit.cpp` | **reconstructed** |
| `cl /W4 /WX /std:c++17` Win32, and `./scripts/build.ps1` green with the existing `reconstructed_math` test passing | **build-tested** |
| the six behaviours below | **fixture-tested**, by a throwaway probe, not a committed test |
| anything in the running game | **not validated**. Nothing was run against `bsp_game` or the retail binary |

The probe (scratchpad only, deliberately not committed - AGENTS.md asks for as few new tests as
possible and this is a pure function with no regression surface yet) checked: identity in,
identity out; a `+30` degree yaw with a stale row 0 rebuilding row 0 to `(0.866025, 0, -0.500000)`
as predicted; a drifted basis coming back orthonormal to `7.5e-09`; `row1 == row2` taking the
`0085DD26` branch and still producing an orthonormal frame; a zero row 2 collapsing rows 0 and 2
to zero with no NaN; and the 4x4 wrapper leaving `m[3]`, `m[7]`, `m[11]` and row 3 untouched.

## Wiring contract

### What to call

```cpp
#include "bsp/plane_pose_commit.hpp"

bsp::PoseBasis basis;                    // rows 0/1/2, 3 floats each
const bsp::PoseOrthonormalizeResult r =
    bsp::orthonormalize_basis_rows_0085dc80(basis);
// r.basis           the rebuilt rows
// r.branch          which 0085DD20 exit ran
// r.forward_length  00419440's value before the reciprocal; <= 0 means the
//                   native collapses the basis to zero
```

`void bsp::orthonormalize_pose_matrix_0085dc80(float m[16])` is the same rule against the native
storage shape, if the host ever holds a real row-major 4x4 instead of three rows.

### Where it belongs

**Nowhere new.** `src/plane_flight.cpp:185` already issues the call in the right place:

```cpp
if (arm != PlaneMotionArm::None) {
    host.commit_step_pose_0085dc80();  // 007CECBA, on unit+674h
}
```

and that already reproduces `007CECA0`'s skip. The only change needed is in
`src/game_hosts_units.cpp`, where the override at **line 2121** is currently an empty body:

```cpp
void commit_step_pose_0085dc80() override {}
```

It should become, in the same `PlaneMotion` host object that already owns `unit_`:

```cpp
void commit_step_pose_0085dc80() override {
    bsp::PoseBasis basis;
    for (int i = 0; i < 3; ++i) {
        basis.row0[i] = unit_.motion.pose_row0[i];
        basis.row1[i] = unit_.motion.pose_row1[i];
        basis.row2[i] = unit_.motion.pose_row2[i];
    }
    const bsp::PoseOrthonormalizeResult r =
        bsp::orthonormalize_basis_rows_0085dc80(basis);
    for (int i = 0; i < 3; ++i) {
        unit_.motion.pose_row0[i] = r.basis.row0[i];
        unit_.motion.pose_row1[i] = r.basis.row1[i];
        unit_.motion.pose_row2[i] = r.basis.row2[i];
    }
}
```

`unit_.motion.pose_row0/1/2` is the host's projection of `unit+674h`, consistent with
`src/game_hosts_units.cpp:509`-`511` assembling `slot.world[0..10]` from the same three rows.

### Three things to know before integrating it

1. **This is a no-op today, and that is correct, not a bug.** The host reads the pose once at
   spawn from the authored placement, which is already orthonormal. `0085DC80` on an orthonormal
   basis returns it unchanged. Wiring this in will not change a single observable in the current
   host. It becomes load-bearing only once something writes `pose_row2` per step - which is
   `007C6500`'s job, not this one.
2. **No guard is recommended.** The native has none: a non-positive `forward_length` collapses
   rows 0 and 2 to zero and the plane's drawn frame goes with it. Reproducing that faithfully is
   the right default, and `r.forward_length` is exposed so the host can log or assert on it if the
   lead prefers. Adding a silent early-return would be a deliberate deviation from the binary and
   should be commented as one.
3. **`CMakeLists.txt` needs one line**, and this packet did not add it because that file is
   leased elsewhere: `target_sources(bsp_core PRIVATE src/plane_pose_commit.cpp)`. Until that
   lands, `src/plane_pose_commit.cpp` is not part of the build - it was compile-tested standalone
   with the project's own `cxx_std_17` and `/W4 /WX`.

## Follow-up

- `007C6500 BSP_PlaneTickElement_AdvancePose` and its callees `007D9F60` and `007D8230`: the real
  per-step pose advance. This is the packet that unfreezes the heading, and it is a raw block with
  no Ghidra function, so expect listing work rather than pseudocode.
- The `0085DC80` ledger record's body range needs correcting to `0085DC80`-`0085DE93`. Left to
  whoever holds the Ghidra write lock.
