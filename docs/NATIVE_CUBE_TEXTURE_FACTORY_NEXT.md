# Native cube-texture factory: dependency discovery

`00B2A380..00B2A589` is the complete 522-byte cube-creation factory.
Fresh guarded Ghidra bytes match the installed PE across 16 spans / 882 bytes,
including its switch table, FH3 records, the full pointer-array reserve
dependency and existing allocation/guard adapters. The executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Each query verifies `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. This packet changes only discovery documentation;
it does not implement, annotate or game-validate the factory.

## Exact entry and native storage

Native ECX is the actual renderer. Four stack DWORDs are cube edge length,
mip levels, D3DFORMAT and original engine flags, in that order. The function
preserves EBX/ESI/EDI/EBP, returns the wrapper pointer in EAX and uses `RET10h`.
`D5F134` contains the exact factory address; no direct caller is currently
recorded. The containing renderer-vtable extent is outside this packet.

The actual renderer supplies device pointer `+1A10h` and the three-DWORD
generic resource registry at `+1B00h`: table, current count and capacity.
The optional-guard byte is actual `0108D6DC`. Entry tests current mode before
saving the renderer and returned AL in an otherwise uninitialized eight-byte
guard record. State0 begins after that entry sequence. Normal and exceptional
exit use the existing current-mode guard behavior; no skipped-entry repair or
new synchronization policy is justified.

## Native flag translation and D3D call

The installed Windows SDK 10.0.26100.0 `shared/d3d9.h` declaration order gives
`IDirect3DDevice9::CreateCubeTexture` index25, Win32 byte offset `64h`.
Both native calls load that slot. The exact invocation is
`CreateCubeTexture(device, edge, levels, usage, format, pool, &out, nullptr)`;
the COM call consumes eight stack DWORDs including `this`. The out pointer is
initialized to null before the first call.

| Engine flag condition | Native D3D value |
|---|---|
| Low nibble 0 / 1 / 2 / 3 | DEFAULT / MANAGED / SYSTEMMEM / SCRATCH pool |
| Bit `10h` set | Usage `1`, RENDERTARGET |
| `(flags & F00h) == 100h` | Usage `2`, DEPTHSTENCIL |
| Same field equals `200h` | Usage `4000h`, DMAP |
| Same field equals `300h` | Usage `40h`, POINTS |
| Same field equals `400h` | Usage `100h`, NPATCHES |
| Same field equals `500h` | Usage `80h`, RTPATCHES |
| `(flags & F000h) == 1000h` | Usage `200h`, DYNAMIC |
| `(flags & FF000000h) == 01000000h` | Usage `400h`, AUTOGENMIPMAP |

Usage starts at zero and the listed values are ORed. Unlisted usage-field
values contribute nothing. Low pool nibbles 4..15 leave the local pool DWORD
uninitialized: no default value, validation or equivalence for those inputs
is established. The four-entry table is exactly `B2A58C..B2A59B`; alignment
bytes `B2A58A..B2A58B` are outside both function and table.

Retry occurs only when the first output pointer is null **and** HRESULT is
nonzero **and** differs from `8876017Ch` (`D3DERR_OUTOFVIDEOMEMORY`) and
`8007000Eh` (`E_OUTOFMEMORY`). This is equality testing, not `FAILED(hr)`.
The factory calls full `B29670` device recreation once, reloads the current
renderer device, and repeats the same creation request. It ignores recreation
and retry results. A nonnull output bypasses retry even with nonzero HRESULT;
zero HRESULT with null output also bypasses retry. It proceeds to wrapper
allocation in all of these cases, with no final output/HRESULT rejection.

## Wrapper, registry and failure ownership

`B2A4EB` calls `B3F2C0`, the real canonical `0108DB70` cube-pool allocator.
The preceding ECX=`30h` is discarded by that adapter; the actual slot is 34h
bytes, comprising 30h owner bytes and the slab token at +30h. The factory
stores the raw result over the original format stack argument and arms state1.
A nonnull result calls `B3D650` with ECX=raw slot and stack arguments actual
COM output, then original unmodified engine flags. Its EAX becomes the result.
A null raw result becomes a null result and still follows registry append.

The sibling owner/base discovery owns `B3D650`, `B3EAD0`, `B3F410`, `B34020`
and their base-cleanup route. Those bodies are deliberately not duplicated
here. Its constructor contract is actual COM ownership transfer without
AddRef, followed by metadata queries; it remains a dependency to implement
and independently verify before this factory can be complete.

After construction, native selects the current registry at renderer+1B00h.
It disarms raw-slot cleanup to state0 **before** capacity growth. When current
count equals current capacity it doubles capacity with DWORD wrapping; if the
signed doubled value is <=1, the request is one. It calls `735FF0`, then
reloads current table and count, computes the wrapped next-slot address, stores
the result only if that computed address is nonzero, and always increments
current count. There is no AddRef, null-result filter or rollback.

The factory has no COM AddRef or Release call and no COM-output cleanup state.
If raw-slot allocation throws after creation, only the guard unwinds. If owner
construction throws, the raw slot is returned and the guard unwinds. If array
growth throws after construction, the factory does not destroy or return the
constructed owner; registration has not yet happened. Do not silently replace
these native failure paths with RAII cleanup. Downstream constructor cleanup
belongs to its separate evidence packet; this is not a claim that failed COM
creation is safe to pass into that constructor.

## Complete factory FH3 map

| Record | Exact role |
|---|---|
| `CBD380..CBD387` | `LEA ECX,[EBP-14h]`; tailcall complete `B21110` guard destruction |
| `CBD388..CBD38F` | Load saved raw slot from `[EBP+0Ch]`; tailcall complete `B3DCE0` |
| `CBD390..CBD399` | EAX=`DF5B90`; tailcall original FH3 at `BF6B43` |
| `DF5B80..DF5B8F` | State0 -> -1 / `CBD380`; state1 -> 0 / `CBD388` |
| `DF5B90..DF5BB3` | Magic `19930522`, maxState2, map `DF5B80`, no try blocks, flags1 |

The normal state sequence is -1 -> 0 -> 1 -> 0 -> -1. There is no omitted
returning-call tail in the main factory; all 522 bytes decode through RET.
The decompiler's misbalanced COM arguments and register/local aliases must
not be used as the ABI contract. The registry helper does have a missing
returning-free continuation, described below.

## Dependency review and ready work

| Dependency | Current evidence boundary |
|---|---|
| `B33AD0`, `B33B00`, unwind `B21110` | Complete actual-storage Win32 guard implementations in `native_renderer_synchronization_actual.cpp` |
| `B3F2C0`, unwind `B3DCE0` | Committed actual cube allocation5 implementations and original-body fixture; shared annotation integration may still be pending |
| `B3D650` | Native cube owner constructor, sibling discovery; no complete implementation yet |
| `735FF0` | Full 95-byte pointer-array reserve newly verified; ready independent implementation |
| `B29670` | Reviewed device-recreation name; complete actual renderer routine remains unported |
| `B2C2D0` | Existing retained-2D file-loading fragments; neither this factory nor complete native cube loading |

Live `B29670` analysis reports 24 direct callees: 22 internal addresses and
actual Enter/LeaveCriticalSection imports. Its dependencies include incomplete
release/restore resources and vertex-layout creation, ten unnamed helpers,
two XLive imports, and host D3D-state projections. Complete actual hardware
tree iteration and optional guards cover only part of that graph. The report
records every bounded lookup and its exact status. A name, semantic fragment
or device probe does not close the full actual-storage recreation dependency.
Do not implement this factory by substituting an empty recreation callback.

Ready packet `native_cube_texture_owner_array_reserve1` owns only `735FF0`
and four new `native_cube_texture_owner_array_reserve` header/source/doc/audit
files. Native ECX is the actual 12-byte header, stack argument is signed
requested capacity, and return is `RET4` with no semantic result. Clamp the
request to at least one; compare capacity signed; allocate wrapped
`requested*4` bytes through the existing actual singleton CRT boundary. Copy
one DWORD at a time while rereading current count and current table. Preserve
the per-destination-pointer null test, then free the **current** old table.
The nine-byte continuation `736041..736049` performs caller stack cleanup,
publishes replacement table, publishes requested capacity, and restores EBX.
It does not change count. No FH3 frame or rollback exists in this helper.
Use full 95-byte original instructions and the actual primary library for the
smallest focused comparison, including service-boundary current-field changes
where needed; do not add a substitute renderer or owner.

The later factory packet should own `B2A380` plus its three original FH3
entries and four `native_cube_texture_factory` files. It remains blocked until
the owner/base, array reserve and full device-recreation dependencies are
concrete. A separate bounded recreation discovery is needed before assigning
that larger implementation. Ghidra renames, returning-free repair at
`73603C`, exports, ledger and packet edits remain integrator-owned.

The JSON report pins native bytes, complete instruction decodes, SDK source
definitions and local evidence scripts. No C++ changed, so no build or new
test was run for this read-only discovery. Original ABI replacement and game
validation remain unproved.
