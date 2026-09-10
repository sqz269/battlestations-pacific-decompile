# Material transform constants

`material_transform_constants.cpp` reconstructs the world-matrix fragment
`[00B42A7C,00B42E4A)`, inverse-world fragment `[00B42EF9,00B4305D)`, and
inverse-cache getter `[00B6E0D0,00B6E10A)`. These are bounded parts of
`00B42350`, not a complete material builder or a game executable.
The byte/hash audit is `reports/material_transform_constants_audit.json`.

## Shared owners and call order

Both writers take the actual `CompiledMaterialPass`, model `CameraTransform`,
and existing constant-word vectors. They read `pass.vb`/`pass.pb` in place and
never clear or resize outputs. The model transform is the same object used by
camera, model lifetime, and instance paths. No replacement transform, copied
whole-entry state, owner lookup, or absent-owner value is constructed here.

The primary builder must call these fragments in this native order:

1. Existing dynamic callbacks, material parameters, bones, skin, vertex decode.
2. `pack_material_world_constants_00b42a7c` (VS, then PS).
3. Visibility and LOD fragment `[00B42E4A,00B42EF9)`.
4. `pack_material_inverse_world_constants_00b42ef9` (VS only).
5. Diffuse-color fragment at `00B4305D`, then remaining builder work.

Resolve live model/metadata owners at the original stage after preceding
callbacks. Later writes win when register intervals overlap. The C++ interface
permits the same vector for both outputs and preserves VS-before-PS order;
it does not preflight or snapshot the complete builder. Transform and metadata
storage must remain alive, stable, and separate from output storage.

## Metadata and cache behavior

| Fragment | Enable/register | Row count | Cache |
| --- | --- | --- | --- |
| VS world | `vb.registers[0]`, native `[pass+70]+8` | `vb.counts[0]`, metadata `+3E` | model world `+F0`, flag `2` |
| PS world | `pb.registers[0]`, native `[pass+74]+8` | **`vb.counts[0]`**, not PS count | same model world/cache |
| VS inverse world | `vb.registers[1]`, metadata `+9` | `vb.counts[1]`, metadata `+3F` | model inverse `+60`, flag `8` |

Register `FF` disables that stage. Rows other than 2, 3, or 4 suppress writes,
but an enabled stage still resolves its cache first. PS reads the retained VS
row-count byte even if the VS world register is `FF`; callers must preserve
that actual byte rather than infer a PS shape from reflection.

For each world stage, rows are captured before the flag-2 refresh; the register
byte is reloaded afterward. Assembly anchors are `00B42A90..00B42AAE` and
`00B42C72..00B42C93`. The PS row read specifically loads the VS metadata pointer
at `00B42C79`. Inverse rows and register are both captured before the helper
call at `00B42F13`.

`get_transform_inverse_world_00b6e0d0` returns `transform.view` by reference
through the existing `get_camera_view_00b6fcb0`. Both native getters have
identical normalized instruction sequences:

1. If flag `8` is already set, return existing `+60`, without checking flag `2`.
2. Otherwise, refresh world through `00B6DB70` only if flag `2` is absent.
3. `00B63B30` computes the specialized orthogonal/scaled affine inverse into
   a temporary, `004134F0` copies it to `+60`, and flag `8` is ORed into `+5C`.

This reuses the existing specialized inverse and float spill schedule. It is
not replaced by the general inverse `00B632D0`; shear/singular/non-affine
inputs retain the existing helper's behavior and limitations.

## Exact writes and interface limits

For each selected row, destination word `4*r+c` reads source word `4*c+r`.
Only `4*rows` words starting at `4*register` are written.

Two-row native branches (`00B42BDF`, `00B42DC4`, `00B42FF2`) load eight words
with `MOVSS` into stack scratch and then use `REP MOVSD`. The reconstruction
uses a raw eight-word temporary and `memcpy`, preserving NaN payloads, signed
zero, and the complete two-row snapshot before output writes. Three- and
four-row native branches copy one word at a time with `FLD`/`FSTP`, in
destination order. The reconstruction uses the same x87 load/store sequence,
including signaling-NaN quieting and exception status under the caller's x87
environment. It does not reset FP controls or substitute SSE float copies.

Capacity failure is a new `false`/error-string host result. The affected
stage validates capacity after cache resolution and before its first write;
earlier stage writes and cache changes survive. Unsupported row counts do
not require output capacity. Native invalid-pointer/out-of-capacity memory
behavior is outside this typed interface. Parent links follow the existing
`CameraTransform` lifetime/acyclic-hierarchy contract. Matrix/output aliasing
is not exposed by the owning vector/array API; sequential x87 writes are
retained without claiming native arbitrary-address alias compatibility.

## Evidence and validation

The parent native ABI is `ECX=pass`, stack render-entry and override, `RET8`.
The owned fragments are interior blocks with `EBX=pass`, `EBP=entry`; world
entry reloads EBP from `[ESP+B8]`. They have no independent native call ABI.
The getter uses `ECX=transform`, no stack arguments, `RET`, and returns
`EAX=transform+60`. Ghidra's prior `undefined ... (void)` prototype omits
the actual register input/result. Descriptive names remain hypotheses.

Each analysis/byte request went through `python tools/bsp.py ghidra`, which
verified the existing `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 2,507 audited bytes matched the installed PE,
including both complete fragments, the two getters, all native dependencies
used by the probe, and their two scalar constants. No Ghidra edits were made;
the audit records prior names/comments and proposed annotations for review.

The ignored local differential probe copied both complete fragment ranges,
appended a return at each end, relocated branches/calls and output addresses,
and supplied original entry/pass/transform offsets. Calls used verified native
world-refresh, inverse, copy, and affine-compose bodies without helper stubs.
One focused fixture ran 14 fragment comparisons across row counts 1/2/3/4 and
finite/exceptional matrices. It checked all output words, world/inverse cache
bytes, flags, and x87 exception-status bits, including PS-vs-VS row mismatch,
shared output overlap, and a scalar write preceding inverse overwrite. All
comparisons passed. One host-only capacity case and shared-cache identity
check also passed. The fixture uses the default masked FP environment and
root transforms; it does not establish all FP controls or parent hierarchies.

The packet source and probe compiled as MSVC Win32 with `/W4 /WX /fp:strict`
and `/O2 /MD`. `scripts/build.ps1` passed the existing base build and
`reconstructed_math` test. At worker handoff this source is not yet in the
primary CMake target; adding it and checking the integrated build belongs to
the primary integrator. Typed API, base/isolated build, and native fragment
comparisons do not establish original object-layout/ABI compatibility or
gameplay validation. The complete builder and live owner adapters remain
separate reconstruction work.
