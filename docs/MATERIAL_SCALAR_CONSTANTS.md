# Material visibility, LOD and diffuse constants

`write_material_visibility_lod_00b42e4a` reconstructs the native builder's
`[00b42e4a,00b42ef9)` fragment. `write_material_diffuse_00b4305d` reconstructs
`[00b4305d,00b430cf)`. They patch the existing
`MaterialEntryConstantState.vertex_words` and `.pixel_words` in place using the
selected `CompiledMaterialPass.vb`/`.pb`. They do not allocate or clear the banks.

The integrator must call the visibility/LOD fragment, then the inverse-world
fragment `[00b42ef9,00b4305d)`, then diffuse. Combining the scalar and diffuse
operations changes the final values when shader metadata selects overlapping
registers. Shader register counts do not gate these fragments; only register
byte `ff` skips a stage, independently of the shader object's presence.

## Native evidence and order

The parent `00b42350` receives ECX=pass and stack entry/override, returns with
`RET8`, and retains EBX=pass and EBP=entry in these ranges. Neither fragment is a
standalone native function. VS metadata is `[pass+70]`, PS metadata is
`[pass+74]`; semantic byte `i` is metadata `+8+i`. Each register selects a float4
at bank base plus `16*register`: VS `0108ebf4`, PS `0108dbec`.

| Order | Native sites | Metadata | Operation |
| --- | --- | --- | --- |
| 1 | `00b42e4a..00b42e76` | VS semantic 43 / byte `+33` | Copy entry visibility `+18` into x; write positive zeros to yzw. |
| 2 | `00b42e77..00b42ea3` | PS semantic 43 / byte `+33` | Reload visibility and copy the same scalar float4. |
| 3 | `00b42ea4..00b42ef8` | PS semantic 44 / byte `+34` | Compute `cLODValue`, store x through x87 and positive-zero yzw. |
| 4 | Separate fragment | VS inverse world | Execute before diffuse. |
| 5 | `00b4305d..00b43095` | PS semantic 47 / byte `+37` | Resolve material, call diffuse getter, copy four DWORDs. |
| 6 | `00b43096..00b430ce` | VS semantic 47 / byte `+37` | Resolve material again, call diffuse getter again, copy four DWORDs. |

Visibility uses `MOVSS` and receives no clamp, normalization, or x87 load. Native
XMM0 is zero at fragment entry and resets after the LOD helper. The port copies
each source word as integer bits, preserving signed zeros and signaling NaN
payloads, and writes padding with explicit positive-zero bits. Each selected
stage reads its live binding immediately before that stage. Duplicate register
selections are valid and keep native last-writer behavior.

The LOD caller performs entry `+0` `FLD/FSTP` to a local, then another
`FLD/FSTP` to the argument stack. Fraction bits `3c23d70a` at `00d7a238` are
`0.01f`. At `00b42ecf`, `00b73770` receives ECX=`entry+8`, stack input/fraction,
returns ST0 and consumes eight stack bytes. The caller stores ST0 to float,
reloads it, stores the selected PS x component, and writes positive-zero yzw.
The port explicitly retains these caller x87 boundaries and reuses
`compute_stream_threshold_fade_00b73770` through its host cdecl ABI with the
resolved threshold's bits as an additional stack argument.

The helper reads DWORD index `[owner+50]`. Actual index zero selects bits
`3f7eb852` (`0.995f`) from `00ce77dc`; otherwise the threshold is the float at
`owner+4+16*index`. It stores width=`threshold*fraction` as float, follows the
original x87 subtraction/reciprocal/product schedule, stores the result as
float, then clamps ordered values below zero or above one. Unordered results
pass through. The helper's clamp does not apply to visibility.

Diffuse's native owner is `[[entry+4]+20]`, the section material. Getter
`00b179f0` receives ECX=material, ignored stack slot zero, returns EAX=`material+38`
with `RET4`. PS and VS each call it separately. The port likewise obtains a
pointer from `MaterialLighting.diffuse_color_00b179f0(0)` for each selected stage
and performs each DWORD read immediately before the corresponding write.

## Owner and error boundary

The scalar API requires a pointer to the actual resolved stream threshold only
when PS semantic 44 is selected. It supplies no default and does not interpret
the host `GeneratedInstanceGeometry` layout as native stream-parameter storage.
The primary owner adapter must resolve the current native index/threshold at
this execution point. Only an observed zero index justifies the native default.

The diffuse API requires the actual section material's `MaterialLighting` only
when at least one stage selects semantic 47. The primary adapter retains this
owner through both copies. No guessed color or material is constructed here.
The new interfaces assume valid typed objects and stable retained storage.

A missing selected owner or insufficient destination bank returns an error.
Earlier stages' writes remain visible; the operation is not transactional. Host
bounds checks replace native out-of-bounds memory behavior with failure. Empty
selections need no owner. The first-register upload cursor is untouched.

## Validation

`reports/material_scalar_constants_audit.json` records full byte preimages and
SHA-256 identities for both assigned spans, the reused helpers and constants,
plus the preserved Ghidra annotation preimages. Each Ghidra query verified the
configured `bsp` project and `/battlestationspacific.exe`; `C:/Users/sqz269/bsp.gpr`
exists. All bytes matched the installed PE. This packet made no Ghidra changes.

The new source compiled with MSVC Win32 `/W4 /WX /fp:strict`. The repository
baseline build passed both existing tests after `verify-seeds`. The standalone
source was additionally compiled optimized and linked with the baseline library
for one local differential fixture: copied native fragments and their copied
helpers, with relocated constant/buffer addresses, matched every output word for
finite and signaling-NaN inputs, overlapping semantic registers, exact raw
visibility/diffuse copies and untouched words. A separate mutation between the
two calls checked their independent execution points. Host missing-owner and
absent-selection boundaries also passed. No new tracked test suite was added.

This verifies bounded buffer-writing behavior, not a complete builder, native
object/ABI compatibility, loader-to-draw owner integration, or gameplay.
