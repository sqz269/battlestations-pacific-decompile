# Native matrix-to-animator angle extraction

Addresses: 00b630f0, 00419440, 00bf701a, 00b79bc0

`00B630F0..00B632C7` is a complete 472-byte caller, now reconstructed in
`src/native_postprocess_pose.cpp`. The descriptive Ghidra name
`BSP_Matrix_ExtractNodeAnimatorAngles` is a hypothesis, not a recovered symbol.
The native ABI is ECX matrix, one stacked float3 destination, RET4, EAX
destination. The source adds a required borrowed `CameraAxesCrtAccess` in EDX.
It is a new C++ entry, not a drop-in binary replacement.

This supplies the missing matrix-angle operation used by the non-skin branch
of `B79BC0`. That complete postprocessor remains unimplemented. Camera FOV and
aspect setters already exist in `src/camera_projection.cpp`; AX does not
duplicate or reannotate them.

## Recovered operation order

The caller obtains the three basis-row lengths through canonical `00419440`.
Each length and reciprocal is spilled to float. An ordered positive length
selects its reciprocal; zero, negative or unordered selects positive zero.
The caller reads the current row after that row's length call, allowing a CRT
diagnostic callback to change matrix storage. It computes all needed basis
components before its first output write, so the output may overlap writable
matrix words. The fourth components and translation are ignored.

Let `r0`, `r1`, `r2` denote the needed float-spilled normalized components:

- Write Y as SSE `negative_zero - float(FSIN(r0.y))`.
- Spill `FCOS(Y)` to float `c`. Compare `c > +0`; otherwise compute
  `negative_zero - c`, preserving the unordered path.
- Compare that result with the exact double bits `3f1a36e2e0000000` from
  `D7A268`. Ordered greater selects the regular path.
- Regular path: write Z from `atan2(float(r0.y/c), float(r1.y/c))`, then X
  from `atan2(float(r2.x/c), float(r2.z/c))`. Preserve both extra float spills
  between each CRT result and the destination write.
- Fallback: write positive-zero Z, then X from `atan2(r1.x, r0.x)`, again
  retaining both result spills.

`FSIN` is literal machine behavior; it is not replaced with `asin` or a
conventional Euler decomposition. The implementation preserves native x87
stack operations, SSE rounding, signed zeros, NaNs and branch behavior.

## Math provider boundary

The existing `camera_vector_length_00419440` and its recovered sqrt dispatch
are reused with the actual borrowed global slot and required diagnostic
handler. A short integer-only adapter supplies that binding without a new FP
operation or spill. The outer entry saves/restores EBX for this context.

`BF701A` selects CRT table `E15C20` and enters `__cintrindisp2`; its x87
arguments are ST0=x and ST1=y. AX links the genuine host CRT `_CIatan2`, as
existing camera-decomposition code does. It does not port CRT library code.
The original atan2 dispatch globals, diagnostics, errno/matherr/SEH behavior
and full original CRT numerical identity are not established here.

## Verification and remaining work

The original 472-byte caller and 98-byte vector-length body match saved
Ghidra and the untouched installed PE. Both constants and the atan2 entry
bytes were checked live. Seven direct-call rows pass mechanical attribution.
The compiled caller matches all 472 original bytes after normalizing verified
CALL targets and the two constant addresses; its wrapper and length adapter
are checked separately.

Strict MSVC Win32 and both existing CTests pass. One ignored native probe
compares 24 repeated caller variants: ordinary/scaled/skewed bases, zero and
negative zero, quiet/signaling NaNs, infinity, denormals and overflow; six x87
control settings, four SSE rounding modes, DAZ/FTZ, and output aliasing each
basis row. The fixture compares all storage words, EAX, x87 stack balance,
control/status words, MXCSR and diagnostic events. It exercises 28 sqrt
diagnostic calls per side, including a result-changing callback that mutates
current matrix storage. These combinations are focused cases, not an
exhaustive Cartesian test. Both sides share the documented real CRT providers.

No repository test suite or worker was added. Remaining work includes the
postprocessor's temporary arrays and camera ownership sequence, full B79BC0,
full B891A0 graph creation/admission, original exception integration and
gameplay validation. Prior Ghidra destructor-body attribution limits remain.

## Retained evidence

Source commit `227d233e57e33c63e876b96ed4663dc6dbe3cf0a` contains the verified implementation. The strict build used the same owned source changes on `ee5086e940d942ff806d41f57a968d3b81545dd2`. The immutable local manifest retains 290 inputs and 43 artifacts, including all 5 linked production objects, exact source/header/compiler/library inputs, both original bodies, comparison images and saved Ghidra annotations/exports. These establish the caller and explicit provider boundaries above, not complete original CRT behavior or gameplay.
