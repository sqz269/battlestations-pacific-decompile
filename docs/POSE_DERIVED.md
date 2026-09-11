# Derived pose matrix at 00414E10 and 00B63D50

This packet reconstructs the complete `00414E10` derived-matrix accessor and
its leaf `00B63D50`. The new typed interface is `PoseDerivedView`, containing
only a reference to the existing `PoseRefreshView` and the actual output matrix
at owner +110h. It owns no second validity flag, world matrix, or hierarchy.
Descriptive names are hypotheses; the inverse interpretation below follows
from the recovered arithmetic and has specific mathematical preconditions.

## Math meaning and output

For a source matrix whose upper three row vectors are `r0`, `r1`, and `r2`,
define `qi = dot(ri, ri)`. Ignoring intermediate rounding for this mathematical
description, the routine writes the transpose of each row divided by that
row's squared length:

```
destination[j][i] = source[i][j] / qi       (i,j = 0..2)
destination[3][j] = -sum(source[3][k] * destination[k][j], k=0..2)
destination[0][3] = destination[1][3] = destination[2][3] = +0
destination[3][3] = 1
```

For nonzero mutually orthogonal rows, `ri dot rj / qj` is the identity matrix.
Thus this transpose-and-scale matrix is the inverse upper basis, and the
negative transformed translation completes the affine inverse. That proves
the restricted inverse interpretation. For a shear, a zero-length row, or a
projective input, the routine still applies the same formula without testing
these conditions; it is not a general 4x4 inverse. Source indices 3, 7, 11 and
15 are ignored for distinct matrices and the output always forces the affine
last column. The one constant at D7A24C is the exact float word 3F800000h.

This operation differs from existing `invert_camera_affine_00b63b30`: B63D50
does not make an initial source copy, forces the affine last column, and uses
x87 FCHS and successive subtractions for translation instead of the other
routine's SSE subtraction from negative zero. Reusing that other kernel would
change output and rounding behavior. No CRT or library routine is called.

`derive_pose_affine_inverse_00b63d50` uses a private MSVC Win32 source-assembly
kernel preserving all 145 instructions' register/stack schedule, each float32
spill after input loads/squared components/norms/divisions, source rereads,
destination-store order, and x87 translation intermediates. The 18h stack
scratch and its reused slots are retained. Only the absolute reference to the
verified one constant is replaced with the same local constant word. Production
code does not load executable bytes or call into the installed game image.

There is no epsilon, determinant check, identity fallback, NaN cleanup, FPU
control change or source snapshot. With masked exceptions, singular/exceptional
inputs propagate the native arithmetic results/status. With exact
destination == source, later source reads observe previous output writes,
including translation components written before later translation calculations.
The typed leaf preserves those native sequential results; in-place operation
generally does **not** produce an inverse. Arbitrary partial overlap is outside
the typed array contract.

## Cache accessor and original ABI

| Body | Original ABI | Bounds and final instruction |
| --- | --- | --- |
| `00414E10` | ECX = pose owner, no stack arguments, EAX = owner +110h | 49 bytes, 12 instructions; `00414E40 RET`, length 1 |
| `00B63D50` | ECX = destination float16, EDX = source float16, no stack arguments; EAX untouched | 442 bytes, 145 instructions; `00B63F09 RET`, length 1 |

The accessor tests owner byte +10Ch at `414E13`. Any nonzero value immediately
returns owner +110h, even if world-valid byte +C8h is zero. If +10Ch is zero,
it calls canonical `refresh_pose_00414db0` at `414E1C`, obtains world +CCh as
source and derived +110h as destination, **stores +10Ch = 1 at `414E2D` before
calling the leaf at `414E34`**, and returns the same output address.

The typed accessor uses `view.pose.derived_valid_10c` directly. It adds no
independent cache flag, extra world-valid gate, validity rollback, or late
validity store after the math. Therefore an interrupted/faulting leaf has
already observed the valid store; the caller does not repair the flag. This
ordering is established from assembly and the source sequence, not from a
fixture that raises unmasked FPU traps. Ordinary singular results are still
cached as native arithmetic results.

The view, output, existing pose and hierarchy must remain alive. Output +110h
must be distinct from the pose fields/matrices for this accessor, as in a
well-formed native owner. The generic leaf's exact-alias behavior is a separate
contract. No native object layout, ownership constructor, invalidation family
or adjacent 414EB0/414F50/415010 operation is reconstructed here.

## Evidence and validation

Read-only `bsp.py` wrappers verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, for the assembly/export batches. Both functions
have complete stored bodies, final instructions and flow; no missing function
definition, false-no-return gap, or truncated tail was found. Saved code ranges
used by the fixture match the installed PE bytes:

- B63D50, 442 bytes: `f796b55571445ad63c6ef518a777d8007992f29cae564fd9ed89b452a62345f8`
- 414E10, 49 bytes: `d57da3a7cf21e96d3143b214953715a9dd591221364027f35d5bf208a00027d7`

`./scripts/build.ps1` passed the strict MSVC Win32 build and both existing
CTests (`reconstructed_math`, `native_math_differential`). One ignored fixture,
`local/pose_derived_fixture.cpp`, is built/run by
`local/run_pose_derived_fixture.cmd`. It executes isolated relocated copies of
the two disk-matched native bodies. The leaf has only its absolute one-constant
address patched; no math call or operation is shared with the rebuilt leaf.
The native accessor calls this original leaf, with only its pose-refresh call
bridged to the canonical recovered refresh. That shared dependency is explicit.

Sixteen leaf comparisons check all 64 output bytes and `_statusfp()` after
distinct/in-place runs over identity, scaled orthogonal basis, shear, zero row,
signed zero, quiet NaN, infinity and subnormal cases. The cache comparison checks
the returned output address, real shared validity fields, dirty-world refresh,
nonzero derived flag bypass with an invalid world, and singular-result caching.
All passed under the fixture process's default masked FP configuration.

No broad randomized suite, arbitrary-overlap tests, unmasked FPU exception/SEH
probe, signaling-NaN payload sweep, all-rounding-mode proof, native owner lifetime
integration, graphics/input hardware, or gameplay validation is claimed. The
results establish the recovered instructions and these focused native cases,
not a generally valid inverse for unsupported matrices.
