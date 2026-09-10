# Generated material clone

`clone_material_00b18b60` implements the confirmed field and ownership
transfers performed by the material constructor used by generated geometry.
It returns a distinct `MaterialCloneState`, using the existing
`MaterialTextureSlots`, `MaterialLighting` and `EffectOwner` interfaces.
The generated draw path can retain this owner and edit its diffuse quartet
through `MaterialLighting::diffuse_color_00b179f0`.

The full 401-byte function `00b18b60..00b18cf0` was refreshed from verified
Ghidra project `bsp`, program `/battlestationspacific.exe`, and compared with
the installed PE. SHA-256 is
`06200809c976f2b86146280c20861c7c649bc19c3ddfa3458bfbe9ad1beb568e`.
The precise evidence and prior annotation are recorded in
`reports/generated_material_clone_audit.json`; no Ghidra mutations were made.

## Original interface

The assembly preserves ECX destination in EBP and reads the source material
from the stack at `00b18bc6`. It returns the destination in EAX at
`00b18ce0`, with `RET 4` at `00b18cee`. Native allocation is performed by the
caller; this constructor initializes a fresh `110h`-byte material with
reference count one and vtable `00d5e520`. The typed function allocates its
host owner and does not expose the original layout or calling convention.

There are no x87 expressions, overlapping globals or CRT free calls in this
function. The lighting transfer is `REP MOVSD`, preserving every DWORD bit,
including signed zero and NaN payloads. Conditional intrusive ownership is
visible in assembly and is represented by shared ownership in the host.

## Transferred fields

| Native destination | Source or rule | Typed representation |
| --- | --- | --- |
| `+08h` | Copy source DWORD | `word08` |
| `+0Ch` | Copy pointer; retain only when copied `+10Dh` is nonzero and pointer is non-null | `pointer0c`, conditional `owner0c` |
| `+10h..+30h` | Copy and retain every slot below signed source count `+34h`, including null slots | `textures` |
| `+34h` | Starts zero; grows to copied high-water count | `textures.count()` |
| `+38h..+78h` | Copy all 17 DWORDs | `lighting.values()` |
| `+7Ch` | Copy and retain source effect, if non-null | `effect` |
| `+100h` | Initialize zero; do not copy source records | `parameters.size()` |
| `+104h` | Copy source DWORD | `word104` |
| `+108h` | Copy source DWORD | `word108` |
| `+10Ch` | Force byte to one before copying lighting | `lighting.flag_10c()` |
| `+10Dh` | Copy source byte exactly | `byte10d` |

The meaning of the words and pointer with offset-based names is unresolved.
In particular, `+0Ch` is not identified as an effect. Its generic
`std::shared_ptr<const void>` owner keeps a non-null retained identity alive;
`EffectOwner` is used specifically for the already-established effect at
`+7Ch`.

At `00b18bf3..00b18c0d`, a zero `+10Dh` skips retention even when `+0Ch` is
non-null. A nonzero byte and null pointer also skip retention. The host copies
those cases directly without dereferencing the pointer. For a nonzero byte
and non-null pointer, its supplied owner must have a control block and the
same `.get()` identity. That validation is host input policy, not a native
check. An owner attached to a borrowed source pointer is not propagated into
the clone when the byte is zero. Values such as byte `2` remain `2`.

The texture loop at `00b18c0d..00b18c77` preserves sparse slots and count;
it does not compact nulls. The existing bounded slot type starts with zero
slots and permits nine, matching the constructor's cleared pointer array.
Repeated calls to `set_material_texture_00b189f0` reproduce the native
high-water update and identity retention. Malformed negative or greater-than-
nine native counts cannot be represented by this typed source and remain
outside its supported domain.

At `00b18c79`, the clone marks the lighting flag regardless of the source
flag. The existing lighting setter performs the same flag write followed by
the bit-preserving copy. It does not recompute colors, normalize values or
retain a reference to the source lighting array. The effect remains the same
retained owner, while the parameter table is always emptied and `word104/word108`
are copied. Its shared effect metadata remains available for new registrations;
later compiled modes remain visible to both materials. The older audit's
`word100` field describes the previous projection. Current ownership and native
packing checks are in `reports/material_builder_parameter_review.json`.

## Ownership and validation boundary

The clone is independent of the source material object. Its copied words and
lighting can be changed without changing the source. Texture/effect owners
are shared, as the native constructor increments the corresponding object
references. Borrowed `+0Ch` pointers and the COM texture pointers behind
`LogicalTexture` still require external lifetime management. The clone does
not manufacture deep copies of those resources.

`make_shared` and existing vector-backed texture slots replace native object
allocation and intrusive reference storage. The native bytes `+80h..+FCh`
are not written by this constructor and are not synthesized as valid cache
state. Full material virtual dispatch, cache consumers, CPU slabs, original
destructor order, SEH and malformed-input behavior are not claimed.

The function validates required ownership before mutation and leaves output
unchanged on invalid input or allocation failure. These checked HRESULTs and
rollback are host policy. No new test suite was added; the primary integrator
owns the Win32 build and installed draw validation after merging. The packet
establishes a reconstructed semantic clone, not native ABI compatibility or
gameplay validation.
