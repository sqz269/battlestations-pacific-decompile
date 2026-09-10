# Material and texture sort metadata producers

The batch key's effect and texture fields are produced by two native construction
counters and by the root shader descriptor. `material_sort_metadata.cpp` now
provides the three corresponding metadata operations. It does not implement
effect/texture allocation, the registry, or a game-wide construction history.

This packet verifies project `bsp`, saved project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The installed executable SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live query verifies target identity. Complete bytes, hashes, original
annotations, and unapplied proposals are recorded in
`reports/material_sort_metadata_audit.json`.

## Established assignments

| Consumer field | Native producer | Meaning and width |
| --- | --- | --- |
| effect `+B0` | `00b45f69/6c`: descriptor `+8` to effect `+B0` | Signed32 `Priority`; full signed value for material ordering, low6 for index-zero key |
| effect `+AC` | `00b45f72/7b`: descriptor `+4` to effect `+AC` | Signed32 `PipeID` bits; later consumers choose their signed/unsigned interpretation |
| effect `+C0` | `00b18e60/69/6f/7b` | Full DWORD old value of counter `00f8d3a8`, followed by increment; key consumes only low8 |
| logical texture `+20` | `00b341af/b4/b7`, plus three unnamed constructor paths | Full DWORD old value of shared counter `0108d6e8`, followed by increment; key consumes only low12 |

The root shader reader `00b43b00` reads `PipeID` and `Priority` with the existing
integer/default helper. It writes descriptor `+4` at `00b43bb7` and `+8` at
`00b43bee`. Both helper calls pass default zero. This proves the correspondence
to `ShaderLuaOptions.pipe_id/priority`; it is not inferred from similar names.
The effect loader stores priority first, then PipeID, immediately after reading
the root descriptor at effect `+C4`. Per-mode descriptors do not supply these
effect fields.

Base constructor `00b18d60` and derived constructor `00b407a0` do not initialize
effect `+B0` or `+AC`. The C++ metadata therefore marks them unassigned until
`apply_material_effect_sort_descriptor_00b45ee0` runs. Zero placeholders in the
C++ representation are not native initialized values and must not be consumed
while `descriptor_assigned` is false. Once the root descriptor has been read,
zero can be a legitimate parser default. Repeated loader assignments change
these two values without creating a new serial.

Both counters are in the PE `.data` section's virtual zero-fill tail, beyond its
64KB file-backed part. Their initial process-image values are zero; saved Ghidra
bytes also read zero. These are image-initial values, not measurements of an
already-running game. The DWORD increments wrap modulo `2^32` and are not
interlocked. Effect key bytes collide every256 constructions and texture key
bits every4096; neither field is a persistent resource identifier.

## Construction and ownership boundaries

Registry `00b2ebb0` resolves a name and allocates one `0x178`-byte effect. It calls
`00b407a0` at `00b2ed43`, then loads it through `00b46950` at `00b2ed5a`.
`00b407a0` calls `00b18d60` once. The root loader iterates fourteen mode slots
starting at effect `+C8`, stores a created program at `00b46332`, and sets that
program's effect owner through `00b172b0` at `00b46334`. The loop compares14 at
`00b463a3`. One effect construction serial belongs to that common owner, not to
each `CompiledMaterialPass` or shader-mode compilation. `00b46950` can run the
loader twice on the same effect; these calls do not construct another effect.

Before taking its effect serial, base constructor `00b18d60` acquires
`error.tga`. It calls renderer virtual `+64` with the native name and flags zero
at `00b18e34`, then stores the returned logical texture at effect `+98` at
`00b18e36`. Temporary string cleanup finishes before the serial step begins at
`00b18e60`. There is no extra AddRef at the store. Base destructor `00b18eb0`
reads `+98`, decrements its intrusive count at `texture+4`, invokes virtual zero
when it reaches zero, and clears effect `+98`. The effect owns that acquired
reference. A typed effect owner must retain the actual fallback texture along
with its shared metadata across all mode passes.

Renderer primary vtable `00d5f0a8+64` points to `00b319b0`. That function copies
and ASCII-lowercases the name, then calls `00b30b40` on renderer `+1A74` with
`(name,flags,0,1)`. Correct fallback acquisition must preserve that manager's
existing logical identity and cache behavior. A fresh texture must not be
invented per effect merely to obtain a serial. The complete manager/cache route
remains an explicit integration boundary; see `docs/TEXTURE_FILE_LOADING.md`.

The named loaded-2D texture path is
`00b3f930 -> 00b34230 -> 00b34120`. The metadata step follows native name copying
and stores of the native texture/policy fields in the base. The outer constructor
then performs COM metadata queries. Its file-loader caller checks for nonnull
D3DX output at `00b2c5d4`, allocates the wrapper, checks allocation at
`00b2c5e9`, and only then calls `00b3f930` at `00b2c60d`. File/decode/null-output
and wrapper-allocation failures before this boundary consume no texture serial.
Once the base reaches the counter write, a later constructor/load failure must
not rewind the counter. Existing-wrapper recreation is not a new construction.

Static counter xrefs establish all four direct texture counter writers:

| Base entry | Stores old counter at | Increment at | Construction route |
| --- | --- | --- | --- |
| `00b33fc0` | `00b33ff5` | `00b33ff8` | Unnamed base ending with vtable `00d5f228`; caller `00b3f7b0` |
| `00b34020` | `00b34055` | `00b34058` | Unnamed base ending with vtable `00d5f280`; caller `00b3d650` |
| `00b340a0` | `00b340d5` | `00b340d8` | Unnamed base ending with vtable `00d5f2c0`; caller `00b3d720` |
| `00b34120` | `00b341b4` | `00b341b7` | Named base used by `00b34230/280/2d0` for those same three vtables |

All four share `0108d6e8`; separate per-type or per-asset counters would change
the key. The corresponding named wrappers replace the vtable after calling the
common base; they do not take another serial. The effect counter's static xrefs
contain only the read and write in `00b18d60`. This establishes the observed
direct counter operations, not the absence of any possible dynamic alias.

## Typed contract and integration

`MaterialSortMetadataCounters(effect_next,texture_next)` requires explicit
starting values. `construct_material_effect_sort_metadata_00b18d60` and
`construct_logical_texture_sort_metadata_00b34120` consume their respective next
value and return full-width serial metadata. These operations implement only
the established numeric step. They do not themselves acquire the fallback,
copy a native string, allocate a wrapper, or call COM.

`MaterialEffectSortMetadata` and `LogicalTextureSortMetadata` require explicit
serial construction; they have no default constructor. Core owner types can
hold optional metadata to distinguish an untracked identity. All passes for an
effect share its metadata owner and fallback owner. Retaining or cloning a
material that references the same effect copies that identity; it does not
consume the effect counter. Creating an actual new native effect does consume
it. Do not infer identity from a resource-name hash, COM pointer, or mode index.

The existing batch reader consumes `priority_b0` only after descriptor assignment,
low8 of `construction_serial_c0`, and the full texture serial before its low12
mask. The previous batch packet still determines texture presence from the
native signed16 material count and nonnull slot zero. Missing metadata for an
existing texture is an error at the owner adapter; it is not evidence of an
absent texture or of serial zero.

Matching IDs to a native frame additionally requires the actual next-counter
values, construction order across all four texture paths, registry/cache hits,
fallback acquisition, failed constructions that reached the metadata step,
and owner lifetime/reload behavior. Starting an isolated diagnostic domain at
zero proves only that domain's sequence. This packet closes the field producer
semantics and supplies the reusable contract; complete native registry history
and game-frame metadata parity remain unestablished.

## ABI and validation

| Entry | Original ABI |
| --- | --- |
| `00b18d60`, `00b407a0` | ECX object, RET, EAX object |
| `00b45ee0` | ECX effect; stack name, byte policy, override flag; RET0C; AL result |
| `00b34120`, `00b34230/280/2d0` | ECX object; stack native name, native texture, policy; RET0C; EAX object |
| `00b33fc0`, `00b34020`, `00b340a0` | ECX object; two stack values; RET8; EAX object |
| `00b3f930` | ECX wrapper; five stack values; RET14; EAX object |
| `00b18eb0` | ECX effect base, RET; destroys members without freeing the object |
| `00b192d0` | ECX effect base; stack deletion flags; RET4; low flag requests `_free` |

The old CG-array/vector/deleting classifications are inventory hypotheses, not
recovered symbols. The `00b192d0` export omits three bytes after `_free` because
of an existing flow annotation; live bytes `83 c4 04` prove `ADD ESP,4` there.
No Ghidra function, comment, flow override, or shared ledger was changed by this
worker. The audit preserves annotation preimages for primary integration.

The new source passes standalone MSVC Win32 compilation with
`/std:c++17 /W4 /WX /EHsc /fp:strict`. `scripts/build.ps1` also passes the existing
Win32 Release build and CTest. CMake integration of the new source belongs to the
primary integrator; the standalone compilation is the direct new-source check.
No new test corpus was added for these small metadata operations. Fourteen
complete native function ranges and bounded scalar/allocation fragments were
compared against the installed PE, with separate zero-fill verification for the
counters. This establishes source compilation and instruction/data evidence;
it does not establish native execution of this packet, original ABI
compatibility, full constructor behavior, or game validation.
