# Native Vector2 Math

Addresses: 004155b0, 00419210, 00419260, 00b9a7a0, 00b9a820. Descriptive names are hypotheses, not recovered symbols.
Existing project: `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
The report records each original span, live/installed SHA256 equality, calls and ABI.

| Entry | C++ routine | Original ABI |
| --- | --- | --- |
| `004155b0` | `clamp_native_float_004155b0` | ECX value pointer, EDX minimum pointer, stack maximum pointer; ST0 result; RET4 |
| `00419210` | `native_vector2_length_00419210` | ECX two-float source; ST0 result; RET |
| `00419260` | `native_vector2_reciprocal_length_00419260` | ECX two-float source; ST0 result; RET |
| `00b9a7a0` | `native_vector2_min_00b9a7a0` | ECX destination, EDX left, stack right; EAX destination; RET4 |
| `00b9a820` | `native_vector2_max_00b9a820` | ECX destination, EDX left, stack right; EAX destination; RET4 |

`004155b0`: Spill value/minimum to binary32; ordered lower compare and unordered-inclusive upper branch select native operands. Pointer alias/read order and NaN asymmetry retained.

`00419210`: Both squares and their sum cross original binary32 spill boundaries, then use actual CRT sqrt and spill result to binary32.

`00419260`: Native square/sum/sqrt spills followed by FUCOMIP/LAHF/TEST AH44/JP reciprocal-or-zero selection.

`00b9a7a0`: Preserves native x87 comparisons and selected MOVSS words including asymmetric unordered selection and destination/source aliasing.

`00b9a820`: Preserves native x87 comparisons and selected MOVSS words including asymmetric unordered selection and destination/source aliasing.

## Integration and instruction evidence

These are complete body reconstructions over actual native storage with explicit
runtime bindings. The existing `CameraAxesCrtAccess` and native CRT sqrt kernel are
shared; no replacement sqrt policy or library implementation is added. The public
419510 normalization entry forwards the already reconstructed exact private kernel.
Added CRT arguments have explicit stack cleanup; original x87 spill boundaries remain.

The current transform lookup at BAAEBA preserves volatile XMM0..5 around its added
pure, nonallocating lookup. It captures the native table after BAAEB8 and dispatches
the captured target at BAAF12. The required lookup must neither change actual state
nor the floating environment. Default D63FA0 slot34 is B6E870; its canonical scene
binding must be installed by actual owner construction/application composition.

The geometry bridge passes actual+174 to the existing NativeModelTailStorage
accessor, which reads actual+180. The element accessor indexes geometry+54. Native
B179F0 diffuse access aliases material+38. All consumed B6DA70 calls use recurse0;
the bridge implements that branch only and does not claim the recursive routine.

The native fixture caught two integration defects before acceptance: passing the
owner base instead of its+174 tail, and Ghidra's omitted x87 destination at BAB26D.
Opcode DC CA is FMUL ST2,ST0. DE CC at BAB28B is FMULP ST4,ST0; reverse-subtract
pops at BAB295/BAB29D are also checked against machine bytes. Verified immutable
constants preserve exact bit patterns, including negative zero and CE4970/CE4ADC
approximately +/-1e10 bounds sentinels.

## Validation and limits

The ignored focused fixture executes original bytes at a relocated fixture image,
with unrepresented code filled INT3. It passed 3000 original/source pairs including
864 complete update pairs across12 x87 control words. It compares full restored
actual-shaped backing at identical addresses, ring wrap/relinks, local/world and
start/stop updates, optional child/material alpha, geometry counts and captured
transform matrices. Numeric cases include selected NaNs, signed zero and aliasing.
The current-slot lookup deliberately clobbers XMM0..5. Sticky flags, TOP, CW and
MXCSR are checked, not every x87 register or condition bit. Independent review
rechecked span provenance and reran the same fixture successfully.

The combined strict MSVC Win32 build and both existing CTests passed with explicit
`--parallel 1`. The standard script was also run; parallel MSBuild failed before
C++ compilation with MSB3491 while creating temporary tlog files. No ACL or global
build configuration change was made to work around that environment issue.

This fixture borrows the existing CRT sqrt boundary and supplies a recording
transform callback. It does not establish CRT exception dispatch, actual BAD6F0
construction, geometry production, canonical lifetime/current-slot application
binding, native exception ABI or gameplay. The new C++ interfaces are not drop-in
binary replacements. Production allocates no surrogate owner or ring here.

## Follow-up packets

Recover BAD6F0 actual tracer construction and terminal lifetime, then compose the
existing pool, curve loader and update with canonical generated-model/node/mesh
owners. Wire current34 to the same actual scene object and validate rendering and
gameplay only after that concrete path runs. Preserve the independent CRT and
ownership coverage boundaries above.

## AK saved-analysis and combined-build integration

The seven-module AK batch is registered in bsp_core. Strict MSVC Win32
compilation and both seeded CTests passed with explicit `--parallel 1`; the
standard parallel script hit environment MSB3491 before compiling C++.
The report records saved Ghidra name/signature preimages, prior-comment
preservation and readback, original-byte fixture coverage and exact call checks.
Reported returning-free continuations and missing definitions are now repaired
and saved; worker-era pending-integration notes above describe the earlier snapshot.
New C++ interfaces and required real runtime bindings remain as documented.
Successful full construction, native EH compatibility and gameplay are not implied.
