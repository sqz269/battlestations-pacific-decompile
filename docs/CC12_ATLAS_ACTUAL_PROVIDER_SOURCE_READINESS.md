# Actual atlas production-provider Source readiness

The genuine production atlas provider is not ready to wire. Current Source has
retained Native VFS, text/token and renderer texture services, but it lacks the
actual atlas item producer, raw descriptor parser/registration and admitted
F8C26C manager ownership composition. The smallest next bounded reconstruction
packet is the actual 30h item constructor and its cleanup contract, before
attempting `common.ats` startup wiring.

This review pins Source at `2c49c7456632e160e879190e37500f0305676b90` and selected
historical evidence separately in
`reports/cc12_atlas_actual_provider_source_readiness.json`. It uses capped Source
reads and existing metadata index queries. No new Native body/data/handler/GPR
reads or writes, exports, installed assets, C++, builds, tests, SDK probes or
runtime attempts are part of this packet. Compile, SDK, startup, FMOD, original
ABI and gameplay credit are all zero.

## Existing providers and their contracts

| Provider | Current Source | Readiness boundary |
| --- | --- | --- |
| Actual text/token primitives | `native_pooled_text.cpp/.hpp` | AF5600/AF55F0/AF5740 and AEE340/AEE3C0/AEE2A0 have genuine raw interfaces; actual 1Ch buffer, 4h pooled text and 8h string headers remain distinct. |
| Descriptor byte loading | `native_particle_text_loader.cpp/.hpp` | AF5850 borrows the same actual VFS, raw pool and retained resolver frame; caller retains failed/incomplete operations. |
| Text-buffer destruction | `native_particle_resource_loading.cpp/.hpp` | AF5620 frees captured buffer+14, then releases current raw name+C, preserving stale fields. |
| Actual VFS services | `game_native_vfs_runtime.cpp/.hpp`, `game_hosts_vfs.hpp` | Existing publication 0109CEEC, bindings, name resolver, open/conversion/date/enumeration routes, strings and memory are available as borrowed references. |
| Actual pointer-array reserve | `native_cube_texture_owner_array_reserve.cpp/.hpp` | 00735FF0 preserves current 12h header reads, allocation/free and publication; it does not implement the distinct 00735F90 helper. |
| Raw atlas lookup/consumers | `native_particle_texture_names_raw.cpp/.hpp` | AEFB20 scans manager+4/current signed+8 and returns borrowed actual 30h items; consumers require the external real manager. |
| Actual renderer texture loading/cache | `game_native_renderer_textures.inc`, `game_native_renderer_application.cpp/.hpp`, `native_texture_loading_cache.cpp/.hpp` | One retained graph borrows the application's real pools, VFS, owner registry, renderer publication and imports. `texture_cache()` exposes the existing domain. |
| Logical-texture owners | `native_texture_2d_owner.hpp`, `native_logical_texture_named_base.cpp/.hpp` | Actual 2D construction/destruction and named-prefix Source exist under their declared current-profile and lifetime contracts; the prefix alone is not a complete owner. |

The text/token raw overloads use the application's canonical 01090AA8/01090AA0
pool domain and shared F8C2C8 line scratch. A private string pool or scratch
buffer would change the contract. AF5600 leaves extent+8 untouched; line/token
helpers retain the original missing-token, aliasing and nonreentrant boundaries.

AF5850 requires a fresh retained `NativeVfsNameResolutionAcquired` and actual
text-buffer backing. It uses flags 32h and one current virtual24 read. A count
mismatch frees current data without releasing the stream. Destroying incomplete
or failed resolver frames is not a permitted rollback; the existing process
retention obligation remains. AEF280 will need its own retained parent and child
storage rather than borrowing an unrelated invocation's scratch state.

`GameNativeRendererApplication` retains `TextureLoadingGraph` through its
construction/device/drain phases. Its cache owns the existing concrete loading
services; acquisition records retain native headers and child identities after
provider failure. Ordinary returned logical-texture references must retire before
the shared renderer drain. Borrowing the cache does not authorize another
registry, COM count, serial, retained source, pool or renderer publication.

## Missing producers and exact object boundaries

The semantic `TextureAtlasItem` in `texture_atlas.hpp` explicitly uses new owning
strings/vectors and borrowed texture handles. It is not the original 30h item:

| Native item offset | Established field |
| --- | --- |
| +00 / +04 | Descriptor-filename 8h string header |
| +08 | Borrowed logical-texture pointer |
| +0C / +10 | Item-name 8h string header |
| +14 / +18 / +1C / +20 | U1 / V1 / U2 / V2 float fields |
| +24 / +26 / +28 / +2A / +2C / +2E | Packed UVs and V/U extents |

`00AEE4D0` has no reconstructed Source record. It constructs the two strings,
stores the borrowed texture and removes the final dot suffix except at offset
zero; UV and packed fields remain unwritten. A semantic record's default arrays
cannot establish those raw field lifetimes or failure/unwind effects.
`00AEE220` is also metadata-only, reached by `00AEEAD0`, manager cleanup
`00AEF090` and unload `00AEFA30`. Its exact destruction/deletion role must be
admitted from later bounded evidence, not guessed from those callers.

`00AEE620` remains a semantic finite-fixture parameter parser. A genuine raw
provider must preserve missing/repeated fields, EOF and partial stores, x87
truncate-to-zero packing and extended subtraction/multiply for the six low16
outputs. That arithmetic and the actual cleanup path remain a separate gate.

`00AEEAF0` remains a semantic fragment. The raw provider must operate on the
actual manager's item array (+4/+8/+0C) and texture array (+10/+14/+18), capture
current F8D394/table+64, load through the real admitted target with name/flag 0,
and append the returned logical texture before parsing body items. Null results,
EOF, partial publication and no rollback belong to the original contract.
The existing 00735FF0 reserve cannot stand in for the metadata-only 00735F90
callee merely because both arrays contain pointers.

`00AEF280` has no Source registration body. Its known dependency chain is actual
1Ch text-buffer allocation/AF5600, BDF4C0 resolution, AF5850 load, AEEAF0 parse,
and AF5620 cleanup. Existing helper Source supports a future reconstruction,
but the missing raw parser and original parent failure/unwind order prevent a
complete registration provider now.

Current Source has no admitted actual F8C26C manager publication/accessor/owner
composition in startup or `GameSingletonHost`. Its constructor, profile and
complete drain behavior remain open. `app_shutdown.hpp` records F8C26C release
at 007382D2; that schedule is not an implementation of the missing owner.
AEF090 and its 00737F10 deleting caller are metadata-only lifetime leads.

## Current callers and historical gates

`game_hosts_menu.cpp::load_texture_atlas` logs the requested path and
`TitleInit::load_texture_atlas` UNIMPLEMENTED at AF0060. The frontend separately
enumerates base/variant descriptors, parses semantic records, merges item lists
and loads D3D COM textures through its sprite bridge. That bridge does not
populate the actual Native manager or logical-texture owner domain.
`game_hosts.cpp` uses that frontend for GUI startup and leaves its world-effects
phase recorded; no complete actual `common.ats` startup composition was found.

The corrected current-registry callback order is already historical evidence
in `CC12_ATLAS_STARTUP_PROVIDER_READINESS.md`. AF0060 whole-body admission remains
held: 935 physical bytes/304 starts versus typed 289, with no exact AddressSet.
This review neither reopens physical evidence nor promotes the ordering slice
to a complete raw loader.

AA5E60 is internal to AA5E20. Historical AA5EA2 dispatch captures the current
renderer slot64 and then publishes manager+28 at AA5EA4. Its actual target is
conditional. The existing B319B0 cache provider is usable only after the relevant
captured profile/slot identity is admitted; a companion type, COM bridge or
similarly named family is not a substitute target.

Older `APP_INIT_WORLD_EFFECTS.md` text claiming that requested-ATS versus
texture-DDS comparison can never fire is superseded by Root's current-registry
qualification. Older `ASSET_ENTRY.md` statements about missing token/VFS helper
Source are historical. Installed file/variant counts were not rechecked here.

## Smallest concrete next packet

Reconstruct AEE4D0 and admit its AEE220/AEEAD0 cleanup/deletion dependency in a
new bounded Native evidence packet. Proposed files are
`include/bsp/native_texture_atlas_item.hpp`,
`src/native_texture_atlas_item.cpp`, `docs/NATIVE_TEXTURE_ATLAS_ITEM.md` and a
matching Source report. Shared CMake and ledger edits belong to the integrator.
Claim the specific addresses/files before obtaining new Native evidence.

The contract is caller-backed actual 30h storage, genuine raw string-pool cells,
descriptor/name aliases, borrowed logical texture, untouched UV/packed preimages
and exact original failure/unwind/cleanup effects. No mapper, semantic-record
copy, added reference or success stub is admissible. This would yield one genuine
item provider for the later parser; it would not bind startup.

Named-but-incomplete follow-ons remain raw AEE620, AEEAF0 plus 00735F90, AEF280,
the actual F8C26C manager lifetime, AF0060 raw enumeration/whole-span gate, and
the relevant captured renderer dispatch. A production code packet must wait
until these contracts can compose without invented ownership or behavior.
