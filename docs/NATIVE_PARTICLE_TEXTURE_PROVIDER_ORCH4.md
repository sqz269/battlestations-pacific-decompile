# Particle texture provider composition audit

Addresses: `00B01350`, `00B319B0`, `00B25F40`, `00B283F0`, `00B32410`,
`00BD30E0`, `00B3F590`, `00B3F410`, `00B3F430` (read-only evidence).

## Result

The actual provider already exists. Compose
`load_native_renderer_texture_00b319b0(renderer, name8h, 0, cache, &acquired.cache)`
from `native_texture_loading_cache.hpp`, using the application's existing
`NativeTextureCacheContext`. Its canonical textures belong to the existing
`NativeTextureLoadOwners` / `GuiNativeGeometryOwners` / `NativeRenderActualOwners`
domain. `native_mesh_texture_field.cpp` is an existing concrete composition of
this load and release sequence; `native_render_service_texture_construction.cpp`
already captures current renderer profile `D5F0A8`, slot `64h`, target `B319B0`.

Replace the complete B01452/B01462 decrement-and-zero-dispatch sequence with
`release_native_render_actual_owner(same_actual_owners, resource)` once. That
helper decrements actual `resource+04h`, resolves its canonical companion only
at zero, checks that the companion borrows this exact counter, then invokes
the current native destruction/pool-return path. Calling it after a separate
decrement would incorrectly decrement twice.

This packet changes documentation only. It does not implement the particle
composition or validate it at runtime.

## Publication, layout and actual targets

The live Ghidra target was `/battlestationspacific.exe` in the configured
`C:/Users/sqz269/bsp.gpr`. Read-only queries used `tools/bsp.py ghidra`, whose
client verifies the configured project/program. No Ghidra mutations occurred.

| Producer / dispatch | Evidence and result |
| --- | --- |
| `B283F0` base constructor | `B28407` saves incoming primary ECX in ESI; `B28413` computes EDI=ESI+0Ch; `B28420` calls `B25F40` with that subobject. |
| `B25F40` publication | `B25F5A` saves incoming subobject in EDI; `B25F96` computes EAX=EDI-0Ch; `B25F9B` publishes EAX to `F8D394`. The global is the primary renderer, not the singleton subobject. |
| `B32410` derived constructor | `B3242D` saves the primary receiver; `B32434` calls the base; `B3243B` stores primary profile `D5F0A8`; `B32441` stores secondary profile `D5F0A4` at +0Ch. Publication therefore precedes installation of the final derived profile. |
| Current renderer slot | Live bytes at `D5F10C = D5F0A8+64h` are `B0 19 B3 00`, selecting `B319B0`. This audit admits this profile/target only; construction-time base profiles are not substitute texture providers. |
| Cache receiver | `B319D2` captures incoming renderer ECX in EBP. `B31A4F` computes ECX=EBP+1A74h for `B30B40`. The guard independently reads current `F8D394` at `B319D6`. Preserve these separate captures. |

The complete bounded `F8D394` xref result was searched for writes: `B25F9B`
publishes and `B26053` clears. Its first 12 consumers alone do not identify
the producer. The B01350 consumer is `B013C9`.

## B01350 normal order and ABI

The full body `B01350..B015BF` was read in pseudocode and two assembly pages.
ECX is the particle type; filename is one stack argument; both returns use
`RET 4`. ESI captures the type at `B01370`, then becomes its +68h record header.
EBP is the filename, captured at `B01369`. EDI receives the returned texture at
`B013E5` and remains that identity until its release; later EDI is the frame index.

On the initial atlas miss:

1. `B013C4` constructs an actual eight-byte pooled filename header.
2. `B013C9..B013E3` captures current renderer, its current profile, and slot
   +64h, pushes zero and the actual header address, arms native string state 0,
   then calls the captured target. No companion address substitutes for ECX.
3. The provider is `B319B0` (`RET 8`). It copies/lowercases the name, forwards
   the second DWORD to `B30B40(name, word, acquire_new=0, allow_load=1)` on
   captured renderer+1A74h. Four DWORD cache arguments are consumed by `RET 10h`.
4. `B013E7..B01409` captures temporary name data, disarms state 0, then returns
   that buffer with current length+1 to the current shared string pool.
5. `B0140E..B01449` builds the default UV record `(0,0,1,1)`, converts four
   values through actual D3DX float-to-half import, and appends the record.
6. `B01452` decrements returned texture+4. Only zero reaches `B0145C..B01462`,
   which reads the then-current texture profile and calls slot zero with
   ECX=the returned raw texture and no stack arguments.

There is no null-result guard, material assignment, record-held texture pointer,
or extra resource retain in this fallback. A null provider result cannot be
turned into success. An exception before step 6 must not invent resource
release. After this branch, B01350 counts frame names, searches only the atlas
for subsequent frames, appends records, then clamps the two frame bounds.
The initial atlas-hit branch bypasses renderer loading entirely.

The sole live direct caller is `B0164F` inside `B015C0`: ECX comes from saved
particle EDI; filename is the first word of the string-reader result returned
at `B0163D`. That property branch was inspected through `B01679`; the rest of
B015C0 is outside this audit, including its independent property contracts.

## Returned object terminal and same owner domain

| Actual profile | Current slot +0 | Current slot +4 | Canonical pool |
| --- | --- | --- | --- |
| `D61948`, 2D | `BD30E0` | `B3F590` | `0108DB38`, return `B3D8D0` |
| `D61870`, cube | `BD30E0` | `B3F410` | `0108DB70`, return `B3D940` |
| `D618B0`, volume | `BD30E0` | `B3F430` | `0108DBA8`, return `B3D860` |

All table words above were read live. Complete `BD30E0..BD30ED` checks ECX,
reads current slot +4, pushes deleting flag 1, calls it, and returns with plain
`RET`. It performs no reference decrement. Each complete scalar deletion body
calls its owner destructor, then returns its actual pool slot iff flags bit 0
is set; `RET 4` consumes the flag. Underlying destructor bodies are established
dependencies, not newly re-audited here.

`NativeTextureLoadOwners::register_completed_creator` registers one stable
companion into the supplied `GuiNativeGeometryOwners::registration()` and
borrows actual +4. Registration adds no reference. Its terminal validates the
current profile words, calls the corresponding actual destructor with flag 1,
then unbinds and retires that companion. Use that SAME registration's
`NativeRenderActualOwners` for particle release. A second resolver/map or
separate private texture pool breaks the contract.

Hot cache hits increment actual +4 through `B31D80`. Fresh B319B0 results use
the loader's reference without an additional increment. Resource-record
assignment/copy/destruction does not retain/release its +28 pointer. Do not
add a cache reference to reconcile these schedules. `caller_acquired` records
the optional extra increment and is not a complete test for whether a returned
texture requires the native temporary release; fresh results also require it.

## Required contexts and failure-frame lifetime

- Borrow `NativeTextureCacheContext`, its linked `NativeTextureLoadingContext`,
  and the canonical owners already supplied by the renderer/model integration.
  `require_domains` / `require_loading_domains` require the same actual string
  pool, VFS manager/publication, streams/conversion and retained-memory owners,
  renderer publication cell, synchronization globals, and texture owner services.
- Particle raw string pool cells `01090AA8`, `01090AA4`, and `01090AA0` must be
  the same domain used by `cache.strings`. The shared renderer publication is a
  borrowed live cell; reread it at the native capture site rather than carrying
  a stale access-structure snapshot across invocations.
- The existing `bind_native_texture_vfs_name_resolution` binds real BDF4C0
  source behavior. It verifies string/VFS domain identity and creates one
  `TextureNameOperation`, owning a distinct `NativeVfsNameResolutionAcquired`,
  per resolution. Do not supply a fake callback or shared scratch frame.
- A caller-retained particle operation must own a fresh, address-stable
  `NativeTextureCacheAcquired` before invoking B319B0. The optional null acquired
  interface supports only the first-search nonnull hot hit; any name resolution,
  recursive fallback, or cold load requires retained child storage.
- Keep the parent and all recursive cache/VFS/loader children alive after
  provider failure. Cache and loader destructors terminate for unresolved
  running/failed phases. There is no general cancel/reset/retry API. Do not put
  these children in a short-lived catch-local object, mark them complete to
  suppress the guard, or replay them. External resolution of recorded native
  obligations is required before their eventual destruction.
- Acquired frames retain source stream, converted memory, COM output, pool slot,
  constructor/registration state and record publication at the corresponding
  boundaries. Released string headers contain consumed preimages, not live
  ownership. Texture owner contexts, borrowed profiles, pool domains, canonical
  registry and notification/VFS dependencies outlive every live texture.
- Once the particle's final resource decrement begins, preserve its consumed
  status across a throwing lookup failure; no automatic retry or rollback.
  The existing terminal callback is nonthrowing and terminates if its native
  destructor throws. This is a source failure boundary, not native EH parity.

## Coverage and limits

| Routine | Coverage of this audit |
| --- | --- |
| `B01350..B015BF` | Complete body read; provider branch mapped; no implementation change. |
| `B319B0..B31AB0` | Complete body and assembly read; existing source wrapper inspected. |
| `B25F40..B25FD7`, `B283F0..B2844E` | Complete publication/base bodies and assembly read. |
| `B32410..B328F7` | Partial: publication/profile prefix only; remaining constructor not re-audited. |
| `BD30E0..BD30ED`, three scalar deletion bodies | Complete body and assembly read; underlying destructor callees remain prior dependencies. |
| `B30B40..B31089`, `B2C2D0..B2C896` | Existing source/provider contracts and ownership inspected; native full bodies not re-audited. |
| `B015C0` | Partial: B0164F caller setup only. |

Normal supported-source composition is available. Native FH3/SEH identity,
allocation failures, malformed/null resource faults, full renderer construction
and teardown, unrestricted profile mutation, concurrency and full-game behavior
remain separate proof obligations. No build/test/runtime receipt is claimed for
this documentation-only packet. Mechanical call verification accompanies the
report; indirect targets additionally depend on the table bytes above.

Prior evidence: [texture loading/cache](NATIVE_TEXTURE_LOADING_CACHE.md),
[earlier file-loading audit](TEXTURE_FILE_LOADING.md),
[mesh texture composition](NATIVE_MESH_TEXTURE_FIELD_CJ.md),
[particle property baseline](NATIVE_PARTICLE_TYPE_PROPERTY.md), and
`reports/native_texture_loading_cache.json`,
`reports/native_texture_2d_owner_audit.json`,
`reports/native_cube_texture_owner_audit.json`,
`reports/native_volume_texture_owner_audit.json`.
