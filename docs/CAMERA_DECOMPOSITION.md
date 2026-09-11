# Camera basis normalization and matrix angles

Packet `orch3_camera_decomposition_m` reconstructs complete bodies
`0042B260..0042B2E9` and `0042D2E0..0042D43F`, then replaces the arbitrary
`CameraPositionHost::extract_matrix_angles_0042d2e0` boundary in `006E47A0`.
The `007954A0` kind-5 path now reaches the canonical decomposition. This
supersedes the unresolved-decomposition statements in the earlier
`CAMERA_POSITION_MODES.md` and its report; path sampling `007B04C0` remains
external. Descriptive names and angle-axis labels are hypotheses.

Every live query/export used `bsp.py ghidra`, which verifies `bsp.gpr` and
`/battlestationspacific.exe` through `Client.verify` before access. Both complete
bodies match the configured disk image. The 47- and 95-instruction listings have
no missing starts or flow gaps. Exact hashes, boundaries and prior names/comments
are in `reports/camera_decomposition.json`. Ghidra annotation/save/export after
renaming belongs to the parent integrator; this worker performed read-only work.

## Normalization: `0042B260`

Native ECX points to three writable floats; the function has no stack arguments
and ends with plain RET. It captures Y, X, Z through separate x87 float stores
before any component output. It computes `(X*X + Y*Y) + Z*Z` with the products and
sum retained in x87 until one binary32 squared-length spill. The comparison
against the binary64 constant at `00CE3820` selects the fallback for ordered
`<= 1e-10` **and unordered** inputs (`FCOMI` / `JBE`). Otherwise it calls native
CRT sqrt `00BF7030`, then spills/reloads the result. The fallback loads the
binary64 `1e-5` at `00CE3C70`; both paths float-store the divisor. It performs
three divisions with the captured components and writes X, Y, Z in that order.
This is not the reciprocal-multiply normalization used by `00419440` consumers.

## Decomposition: `0042D2E0`

Native ECX is the matrix, EDX points to output X, and two stack pointers select
Y and Z; both exits use RET8. The source is the actual 16-float `CameraMatrix`.
The routine captures the nine upper-left basis floats before output writes.
Row2.Y (matrix index 9) is captured through `FLD/FSTP`; the other eight captures
use `MOVSS`. All three row vectors are normalized, including components that
subsequent angle formulas do not consume. Translation and matrix indices 3, 7,
11, 15 are not read. No orthogonality repair, scale sign inference, handedness
change, or camera-state write is added.

Let `r0/r1/r2` denote those captured, normalized rows. The routine calls canonical
`camera_asin_clamped_0042cf10(r2.y)`, negates and float-spills it, then writes X.
It independently preserves the same angle on its stack before applying native
`FCOS` and float-spilling cosine `c`. The rounded absolute cosine is compared to
the exact binary64 value `9.99999974737875163555145263671875e-5` at `00D7A268`
(bits `3f1a36e2e0000000`, a promoted binary32 value).

For ordered `abs(c) > cutoff`, Z uses CRT atan2 with ST1=`float(r0.y/c)` and
ST0=`float(r1.y/c)`; Y then uses ST1=`float(r2.x/c)` and ST0=`float(r2.z/c)`.
Each division and each atan2 result has its native binary32 spill/reload. The
fallback discards cosine, loads ST1=`r1.x`, ST0=`r0.x`, writes **positive zero**
to Z with `MOVSS`, then calls atan2 and float-spills/stores Y. The machine always
writes X, then Z, then Y. Outputs can alias each other and writable matrix
elements because every matrix input was captured first. The code retains the
native output stores rather than gathering three result values before copying.

`00BF701A` loads descriptor `00E15C20` (verified `atan2` string) and jumps to
the original CRT dispatcher. The reconstruction calls genuine current-host
`_CIatan2` and `_CIsqrt`, as the existing staged asin uses genuine `_CIsqrt` and
`_CIatan`. It does not port original CRT dispatch, diagnostics, or exception
policy, and does not substitute `std::atan2`, `std::asin` or `std::cos`.

## Validation and limits

`scripts/build.ps1` passed Release Win32 and both existing CTests. Both changed
translation units separately compiled with `/O2 /W4 /WX /fp:strict`; source
registration is reserved for parent integration. No permanent test was added.
Ignored `local/decomposition_fixture.cpp` embeds its manifest and compares copied,
complete verified native bodies with the new code. Both sides bind the same
genuine CRT; the native side includes original copied normalization and asin.
The full game entry point was never run.

All 132 normalization, 1,080 decomposition, and 360 wrapper comparisons pass
byte-for-byte. Three x87 precision settings crossed with four rounding modes
cover normal/scaled bases, zero and singular bases, the normalization cutoff,
tiny/huge components, signed zero, quiet/signaling NaNs, infinity, disjoint
outputs, pairwise/all output aliases, and several matrix/output overlaps. The
normalization/decomposition comparisons also check x87 exception flags and stack
top. The wrapper retains output pointer identity. These are bounded fixtures;
they do not prove original CRT policy, unmasked trap behavior, malformed pointers,
asynchronous mutation, exhaustive float inputs, native object ownership, binary
replacement compatibility, or gameplay/render correctness.
