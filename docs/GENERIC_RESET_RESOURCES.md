# Generic reset resources are concrete texture wrappers

The renderer array at `+1b00h` / count `+1b04h` / capacity `+1b08h` is populated
by concrete texture factories. It must not be implemented as arbitrary empty
callbacks merely because older handoffs called it a generic resource list.
This audit grounds three observed factory paths; it does not assert that every
possible writer has been recovered.

## Concrete registrations

| Factory | Device creation slot | Wrapper constructor / vtable | Reset release `+20h` | Restore `+24h` |
|---|---|---|---|---|
| `00b2a070` | `+5ch`, CreateTexture (2D) | `00b3f7b0` / `00d61948` | `00b3dd30` | `00b3dd90` |
| `00b2a380` | `+64h`, CreateCubeTexture | `00b3d650` / `00d61870` | `00b33f10` | `00b33f20` |
| `00b2a5a0` | `+60h`, CreateVolumeTexture | `00b3d720` / `00d618b0` | `00b33f10` | `00b33f20` |

Each factory explicitly selects renderer `+1b00h` after creating its wrapper
(around `00b2a2d4`, `00b2a520`, `00b2a740`), checks count against capacity,
grows via `00735ff0`, writes the pointer into the next slot, and increments
count. The append sequence contains no intrusive AddRef. Constructors establish
intrusive count one; registration is separate from object construction and
from the eight-byte singleton getter `00b3e730`.

The cube and volume callbacks are **actually empty native routines**:
`00b33f10` is byte `C3` (RET), and `00b33f20` is `C2 04 00` (RET4). These facts
do not justify using empty callbacks for 2D textures, inferring that every
texture kind is reset-safe, or silently adding cube/volume recreation behavior.
They identify an original limitation/specialization whose intended usage and
pool restrictions remain to be established. The observed factory registrations
are not restricted to DEFAULT pool by the append site itself.

## 2D release: cached surfaces before texture

`00b3dd30` takes the 2D wrapper in ECX, consumes no stack arguments, and has no
internal guard. It uses a signed index loop over wrapper `+40h` / count `+44h`.
Records are eight bytes: texture level/key at offset zero and surface-wrapper
pointer at offset four. Each surface receives virtual `+3ch` (existing native
surface release helper). The base is reloaded before every entry and count is
reloaded after callbacks; there is no entry-null test or temporary retain.

After nested surface release, it performs a balanced AddRef/Release access
pair on texture COM pointer `+10h`, reloads the pointer, drops its owned
reference if nonnull, and clears that field. Wrapper metadata, cached level
records, and nested surface wrappers survive. Neither this phase nor its nested
surface callback is destruction/unregistration.

## 2D restore: texture first, then reacquire cached levels

`00b3dd90` is absent from the current function snapshot. Read-only raw
disassembly recovered its full 362-byte body plus the separate four-entry
pool jump table at `00b3defc`; no Ghidra function creation was performed.
It takes ECX wrapper and one device stack argument (`RET4`), without an
internal guard.

It derives D3D usage/pool from stored flags `+1ch`, then calls CreateTexture
using width `+28h`, height `+2ch`, mip count `+14h`, and format `+18h`.
Low nibble 0..3 maps to D3D pools 0..3. The invalid-nibble path copies the
device argument into the pool variable; this malformed native behavior is not
a sensible host default. Other inspected mappings include render-target bit
`10h`, usage groups `100h..500h`, dynamic group `1000h`, and autogen-mipmap
high-byte group `01000000h`. A typed implementation must explicitly bound
unsupported flag encodings instead of pretending they were a recovered pool.

The returned texture COM pointer is installed using retain-new-before-release-
old when distinct, then its temporary creation reference is released. There
is no native HRESULT branch; its output temporary is not initialized before
the COM call in this body. Failed creation therefore cannot safely be treated
as a native-defined null result. Existing typed helpers may report failures,
but that must remain an explicit interface difference.

For every cached level record, it calls the new texture's
`GetSurfaceLevel(record.key)`, then invokes the existing nested surface wrapper's
virtual `+14h` initializer with that getter pointer and releases the temporary
COM reference. The loop reloads array base/count around the callbacks and uses
signed index comparison. It reuses the nested wrapper; it does not call generic
standalone `CreateRenderTarget` to replace texture-owned mip surfaces.

This dependency is why recreating only an `IDirect3DTexture9` is insufficient
for the native reset path. Cached level surfaces must be released before reset
and rebound to corresponding levels of the replacement texture afterward.

## Unregistration and ownership limits

`00b27d40` receives renderer ECX and a texture-wrapper stack argument, selects
renderer `+1b00h`, and invokes `00b25580` on the argument's address (`RET4`).
The latter removes the first matching pointer by replacing it with the final
entry and decrementing count. It neither retains nor releases the object and
reports whether it found a match. Its body is byte-identical to the separate
surface-list removal helper, but the renderer array owner is different.

Direct xrefs identify calls from 2D destructor `00b3f2e0` at `00b3f350` and a
second 2D-related lifecycle path within `00b3fa90` at `00b3fb2e`. The destructor
also releases cached wrapper relationships and texture COM ownership; full
string/allocator teardown should not be inferred solely from truncated
pseudocode around `_free`.

Cube and volume destructors `00b3ead0` / `00b3eb70` release their COM ownership
and nested object relationships, but the inspected bodies and common base
destructor `00b33f50` contain no call to `00b27d40`. Their renderer virtual
`+6ch` call resolves to `00b32250`, which handles a named object through
renderer `+1a74h`; it is not a visible `+1b00h` pointer-removal call. This audit
does not prove complete cube/volume array lifetime cleanup or license a stale
pointer assumption. Resolve that ownership gap before claiming a complete
polymorphic texture registry.

The proven append/removal operations do not establish an owning-reference
container. A typed pointer list needs stable object lifetimes and explicit
registration/removal. Do not add automatic retained ownership while labeling
it native behavior. Reset traversal itself retains a raw cursor while reloading
base/count after callbacks; its mutation limits remain as documented in
`SURFACE_RESET_TRAVERSAL.md`.

## Next concrete boundary

Implement the 2D wrapper reset pair with its metadata and cached level-surface
records, using actual CreateTexture/GetSurfaceLevel and existing surface
release/initialize helpers. Keep object identities and release/reacquire order.
The current logical texture binding projection alone does not carry those
records or establish native texture ownership. Ground constructor metadata and
failure semantics before treating a typed wrapper as a complete native port.

Cube/volume literal-return callbacks can eventually be represented faithfully
for their concrete types; they must not become a blanket fallback for unresolved
resource types. Full factory/recreation, owner teardown, and scheduler validation
remain independent work.

## Evidence

Each Ghidra batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`. The following bytes
matched the installed PE exactly:

| Body/data | Bytes | SHA-256 |
|---|---:|---|
| `00b3dd30..00b3dd85`, 2D release | 86 | `79b651f7ffa1321bc2e2a1652f0d397b58b9a77a9a9d4f35add970d4521815e2` |
| `00b3dd90..00b3def9`, raw 2D restore | 362 | `725508b5f5d5ffcae47674b0daca2805c95bb1ff3d60603448483708ddedb81b` |
| `00b3defc`, pool jump table | 16 | `bf6e27ad79143889b5709e8c6b17e04821abde4c73b5c53fb8468ecb6c9da448` |
| `00b33f10`, native empty release | 1 | `ae3f4619b0413d70d3004b9131c3752153074e45725be13b9a148978895e359e` |
| `00b33f20`, native empty restore | 3 | `e598d0c3ba86d917b177d7adde0556aa99bc355543c57aee0a3e50b684dd7e99` |
| `00b27d40`, unregister adapter | 19 | `e0a5fa450768b0e65e2f94ab32e7a6fd515f0af30096ff84b182466181465fd3` |
| `00b25580`, removal | 103 | `24d2aab156d7889ea7543c821483002b5f93b654671b47704e8253690ef1b8d0` |
| `00d61948`, first ten 2D vtable entries | 40 | `cef4028cda212e93b505419639808ea1c5e0f4177a10ce2ef7d4421db247d56d` |
| `00d61870`, first ten cube vtable entries | 40 | `fe465d36c788ed0f1d0f6df19161a008ae99b142c9a4c31ab9405d6df12de184` |
| `00d618b0`, first ten volume vtable entries | 40 | `ae2296999e340f76a6e0cd6913af54df71b1e2f0d2ca9861a8404e2d4b4ae93d` |

This bounded audit changes only this document. No code, shared metadata,
Ghidra annotations, builds, or runtime validation were performed.
