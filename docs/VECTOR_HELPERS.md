# Vector length and staged affine transform

`src/vector_helpers.cpp` reconstructs the complete game bodies at 00414C60 and
00414CD0. Names are descriptive hypotheses. The APIs reuse standard float arrays,
`CameraMatrix`, and the canonical `transform_point_004142e0` implementation in
`src/camera_affine.cpp`; they do not introduce vector or matrix owner layouts.

## Two-component length, 00414C60

Native ABI: ECX points to two floats, no stack arguments, RET, float-valued ST0
result. The complete body ends at the one-byte RET at 00414CA1. There are 47
callers in the checked snapshot and one genuine CRT callee, 00BF7030.

The assembly loads Y then X, squares X and Y in x87, adds, and stores the sum
to binary32. It compares the reloaded sum with the double at 00CE3820:
little-endian bytes `bb bd d7 d9 df 7c db 3d`, the double encoding of `1e-10`.
An ordered sum strictly greater than the cutoff calls CRT sqrt with the rounded
sum in ST0, then performs two binary32 store/reload pairs before returning.
Below, equal, and unordered comparisons return positive zero. No component is
written and no normalization is performed. For ordinary finite values this is
`sqrt(float(x*x + y*y))` above the squared-length cutoff, otherwise zero, with
the documented x87 intermediate precision and rounding boundaries.

The x87 FCOMI and stack cleanup are retained: unordered values still take the
positive-zero path while leaving the corresponding masked exception status.
The production call uses the genuine current Win32 `_CIsqrt` import, preserving
its hidden ST0 input/result convention. No double argument spill or `sqrtf`
substitution is inserted, and no CRT body is ported. The fixture imports
`_CIsqrt` from `api-ms-win-crt-math-l1-1-0.dll`. The original library's `sqrt`
name bytes at 00E154E0 were checked against the saved program and installed PE;
its existing `LIBCRT_unmatched_00bf7030` name is preserved. Current and original
CRT results, errno, matherr, exceptional diagnostics, and exception handlers
are not assumed interchangeable.

## Staged affine transform, 00414CD0

Native ABI: ECX is destination XYZ, EDX is source XYZ, the single stack argument
is the matrix, and RET4 at 00414D0B spans through 00414D0D. The decompiler misses
the EDX source argument. EAX is not a usable result: it retains the callee's
pointer to the expired local destination. The one snapshot caller is 007954A0.

The wrapper calls canonical 004142E0 with the source, a local three-float
destination, and the caller's matrix. Only after that call finishes does it
copy local X, Y, Z to the requested output with MOVSS instructions. Thus all
source/matrix reads precede any output copy. This matters when output overlaps
the matrix: directly invoking the canonical kernel with that output would
overwrite matrix entries before later components had been computed.

The C++ wrapper calls the same reconstructed affine kernel into an
`std::array<float,3>`, then copies its bits to three borrowed output floats.
The raw output pointer supports in-place source output and positions inside
the existing `CameraMatrix` without inventing nested array lifetimes. It
preserves the native bit copies without additional x87 loads or arithmetic.
The canonical kernel supplies the exact XYZ staging and product/add/float-store
order; there is no perspective divide, added validation, or guessed transform.

## Verification and limits

The two complete native spans, cutoff constant, original sqrt name, and the
entire canonical affine span matched live Ghidra and the read-only installed
PE. Project `C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe`
were verified through the repository wrappers before analysis/export batches.
No Ghidra mutation occurred in this worker; prior names and empty comments are
recorded for the integrator's later annotation/save batch.

MSVC Win32 Release `/W4 /WX /fp:strict` build and both existing CTests passed;
all eight native math seeds matched live analysis and the installed file.
One ignored local differential fixture executed relocated complete original
game bytes against the built library: 51 length and 105 transform cases passed
with exact output bits and masked x87 exception-status bits 0..6. It covers
x87 control words 007F/027F/037F, cutoff neighbors, finite extremes, quiet and
signaling NaN encodings, infinities, signed zeros, in-place XYZ, and output at
matrix float offsets 0/1/4/12/13. The length's only relocations are the cutoff
address and the call to the actual host `_CIsqrt`; the wrapper's only call
relocation targets the complete original affine bytes, which need no edits.

No permanent tests were added. These are reconstructed, build-tested and
fixture-tested interfaces, not drop-in binary replacements. Other floating
environments, unmasked exceptions, original CRT diagnostics, and game runtime
remain unverified. The neighboring 00414D10 body is outside this packet.
