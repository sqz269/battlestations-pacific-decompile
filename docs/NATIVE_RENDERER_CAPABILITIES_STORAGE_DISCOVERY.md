# Actual renderer capability storage discovery

`B2C8E0` is not yet reconstructed against actual renderer storage. A small
source-ready next packet exists: full `B22B30[130]` and `B236B0[95]`, the native
stride-12 and DWORD array reserves. They need the existing actual shared
allocation/free service, not an unknown resource-owner profile. This discovery
does not implement them or expand into gamma, constructor or recreation bodies.

The owned gather entry is complete `[00B2C8E0,00B2D8DE)`: 4,094 bytes and 885
decoded instructions, including both jump-over padding spans. Its original ABI
is ECX actual renderer, plain `RET`, with no semantic HRESULT result. It reserves
760h stack bytes and saves four general registers. There is one local string
EH state, no outer synchronization guard and no renderer-vtable selector read.

## Existing source boundary

`renderer_capabilities.cpp` and `settings_capabilities.cpp` retain useful query
and decision evidence. Their interfaces are typed projections, not native
renderer storage. The larger API copies typed state, uses `std::vector`, rejects
failed caps/identifier queries and unterminated descriptions, and publishes at
the end. Native publishes fields and array elements incrementally, ignores the
two query HRESULTs, scans the actual returned stack bytes, and does not roll
back earlier changes. The typed format record omits native padding/payload.
It also borrows one fixed `IDirect3D9&`; native rereads renderer `+1990` for each
query. Neither typed output may be cast into an actual renderer object.

## Eight immediate dependencies

| Address | Actual role | Current closure |
| --- | --- | --- |
| `419CC0[192]` | Get current sized-storage pool singleton | Full actual owner/source service exists |
| `41DD40[164]` | Resize actual 8-byte string header | Full actual-header provider exists; borrow actual owning pool/lifetime |
| `B22B30[130]` | Reserve raw 12-byte records | Complete body decoded; ready, not implemented |
| `B236B0[95]` | Reserve raw DWORD elements | Complete body decoded; ready, not implemented |
| `B2AE20[116]` | Resize outer array of 12-byte inner headers | Missing raw source; bounded subordinate closure identified below |
| `BD1510[95]` | Return captured buffer/size to current pool | Full actual pool source exists |
| `BF7680[869]` | Original CRT `_memcpy`, including overlap handling | Existing host `memmove` boundaries are explicit adaptations; no full native optimized-provider reconstruction |
| `BF9440[134]` | Original CRT `_strstr` | Typed source uses host `strstr`; full native one-character tail reaches internal `_strchr` entry `BF86F6` |

The last two remain concrete CRT binding decisions for a future full raw gather.
A valid-buffer/NUL-terminated host CRT boundary can be proposed explicitly; it
must not be reported as full native CRT instruction/fault equivalence.
`BF7680` reads mutable `0109EEA4` and can jump to `__VEC_memcpy C0C82B` for
size/alignment-qualified copies. Its complete captured span includes embedded
jump tables; a partial linear decode is not full instruction proof. Its backward
overlap path is real. `BF9440` tail-jumps to `BF86F6` after placing the single
needle byte in AL and restoring its saved registers. That target skips the
ordinary `_strchr` argument-character load. The gather's fixed needles have
length three or four, but that does not establish a complete generic `_strstr`
source body. Correct CRT names are preserved.

## Current COM and raw publication schedule

Renderer `+1990` contains the actual current **IDirect3D9 factory interface**,
not the renderer's `+1A10` device. Each of nine static COM call sites reloads
that field and its current table. No AddRef, Release or alternate API fallback
is inserted. The reached methods are `+38 GetDeviceCaps`, `+14
GetAdapterIdentifier`, and `+28 CheckDeviceFormat`.

`B2C91A` queries caps with adapter 0/HAL into stack `+1F4`. Before ATOC's call
at `B2C998`, the body has already written gamma bits, dimensions and packed
shader versions in the observed interleaved order. ATOC probes usage 0,
resource SURFACE, X8R8G8B8; exact HRESULT zero sets `+1B55`, failure retains it.
`B2C9BB` queries a second identifier into stack `+324`, again ignoring HRESULT.
Description begins at stack `+524`; the byte scan has no fixed Description
length bound. These ignored-return and readable-output requirements cannot be
replaced with early error returns while claiming the raw body.

The description is copied to a local native string header `{length,data}` at
stack `+14/+18` using resize with preserve=true, then current length+1 bytes
from the stack through `BF7680` if the captured destination is nonnull. Searches
use captured string data and exact case-sensitive literals `8800`, `8600`,
`8200`, `ATI`, retaining the native found-pointer subtraction and `-1` test.
The result byte publishes at `+1D89`.

The reached raw renderer fields are:

| Fields | Native meaning and access |
| --- | --- |
| `1B18/1B1C/1B20` DWORDs | Texture width/height and volume extent |
| `1B24/1B28` DWORDs | Anisotropy and simultaneous textures |
| `1B2C` DWORD, `1B30` byte | 4/false for PS <=104h; 8/true only for 200h or 300h; other values retain current fields |
| `1B40/1B44` DWORDs | Low 16 bits of PS/VS version; software VP can override VS with101h |
| `1B48/1B4C` DWORDs | Unsigned PS<200h gives1 else2; maximum VS constants |
| `1B50..1B55` bytes | INST transient/final flag, clip planes, stream offsets, gamma/calibration, sticky ATOC |
| `1B5C/1B60/1B64` | Actual DWORD-array data/count/capacity header |
| `1B68/1B6C/1B70` | Actual outer data/count/capacity header containing inner 12-byte headers |
| `1B74`, `1D89` bytes | Software VP and description classification |

Shader decisions reload current `+1B40` after string operations and again
before INST after declaration-array allocations. They do not reuse a private
immutable projection of the earlier caps query. The declaration array appends
0,1,2,3,4 and selected5,8..15; it is not cleared. Every append uses current
header data/count, grows only when count equals capacity, and increments count
even when the computed destination address is null.

INST is queried only for current PS<300h. The native can store `+1B50=1`, then
always stores zero; both writes belong to the raw body. Stream offsets come
from caps `DevCaps2&1`. Gamma-bit stores here do not close the separate gamma
implementation or any device-recreation body.

All 57 format-table rows were reconstructed independently from all 285 stack
initialization stores and match the existing typed table. The table contains
8-byte `{format, four metadata bytes}` records. Resource type starts at3 and
only3 is iterated. Each row first probes usage0; success enables selected
usage1, usage2, usage200h calls in order, then always usage80001h. Only exact
HRESULT zero counts. Each reached call uses the current factory/table again.

Supported rows append to inner array **index3** of the outer header at `1B68`.
Outer resize4 occurs when signed current outer count does not contain index3.
After that call, the current outer base is read and the selected inner-header
address is captured at stack `+28`. Inner reserve operates on that captured
header, and append reloads that same captured header after reserve; it does not
re-resolve a changed outer base.

Record `+00` is the format DWORD. Record `+04` copies four initialized flag
bytes, including zero at `+07`. Scratch `+24` receives only the final support
byte, but all four bytes are copied to record `+08`; its upper three bytes are
unwritten native stack payload. A zero-initialized typed record is not the raw
representation. At direct accesses the renderer needs storage through `1D89`
(minimum span1D8Ah), with separately valid owning array buffers and current
COM interfaces. This is an access bound, not a reconstructed whole owner.

## Array helpers and exact next packet

The smallest proposed source packet is `B22B30[130] + B236B0[95]` (225 bytes),
with four new `native_renderer_capability_array_reserves` source/header/doc/audit
files. Both take ECX actual 12-byte `{data,signed count,signed capacity}` header
and a by-value request in the actual callee stack slot, returning with `RET4`
and no semantic result. No new allocator callback/context is needed: compose
the existing `singleton_lifetime_allocate({object,bytes,bytes})` and matching
free under their established shared-heap/current-new-handler/exception source
boundary. Original `BF55BE` jumps to full `BF681B`; `BF6989` forwards to `_free`.
This proposal does not claim original CRT exception-object or caller ABI.

A possible MSVC declaration is `void __fastcall
reserve_native_capability_records_00b22b30(void* actual_header,
std::uint32_t unused_edx, std::int32_t requested_capacity)` and the corresponding
`reserve_native_capability_dwords_00b236b0` declaration. The explicit ignored
EDX position keeps the actual request in the original single callee stack slot;
it is a new source-call interface, not a dependency context. The implementation
can retain each complete raw instruction sequence with fixed allocation/free
bridges. The final source names/interface remain subject to the separate lease.

Both reserves clamp the signed requested capacity to at least1, compare signed
current capacity, and allocate using native wrapping DWORD byte arithmetic.
They reread current count/base after allocation, preserve native per-element
load/store order and null destination tests, free **current** old data before
publishing fresh data and the captured capacity, and leave count unchanged.
No overflow/extent repair or rollback belongs inside the claimed native body;
all reached backing storage and copies need an explicit valid-extent contract.
`B22B30` also stores the fresh allocation into its own request argument slot
at `B22B60` (`[ESP+14]` there) and reloads it after its loop. Preserve that actual
callee-slot scratch write; do not treat the request as a pointer to owner data.

Saved analysis omits the fallthrough after returning free calls `B22B9E` and
`B236FC`: respectively `[B22BA3,B22BAD)` and `[B23701,B2370A)`. Fresh guarded
raw spans include all omitted stack balancing, data/capacity publications and
register restoration. Bounded saved listings confirm the gaps; complete PE
instruction decoding supplies the tails. Source must use all 130/95 bytes,
including those tails. The worker did not repair saved analysis; the primary
can do so after review and address ownership transfer.

The remaining nested closure is another 593 bytes:

| Entry | Full behavior required |
| --- | --- |
| `B23120[96]` | Inner resize; grow initializes format DWORD and four flags only, leaving record+8 untouched; shrink decrements current count |
| `B25E60[133]` | Inner copy assignment: resize destination0, then unconditionally reserve current source count; even an empty copy can allocate one12-byte record; preserve captured source-row pointer across destination reserve |
| `B29D40[248]` | Outer reserve: fresh header array, deep-copy each current inner header, then destroy/free old inner data and outer data before publication |
| `B2AE20[116]` | Outer resize: initialize new headers to three zero words; shrink publishes count decrement before inner resize0/current data free |

These bodies have a finite shared-heap closure once the primitive reserves
exist; no intrusive resource-owner terminal/profile is involved. They are not
implemented in this discovery. The current typed vectors do not supply them.

## EH and lifetime boundaries

Gather's actual PUSH names handler `CBD56B`, which loads FuncInfo `DF5DF8`,
map `DF5DF0={-1,CBD560}`. Action `CBD560` addresses the local header and jumps
to full `41DD20`. State0 is armed at `B2CA0A`, **after resize and the whole
description copy**. Failure before that arm does not gain an invented local
string rollback. Later failures clean the local string only; renderer fields
and previously appended arrays remain published.

Normal cleanup first tests the string pointer, disarms state, then captures
current length followed by current data and calls current `419CC0/BD1510` with
size length+1 and pool type1. The EH action instead uses full `41DD20`'s
pointer-before-length schedule. A future source must keep both schedules and
the actual owning pool/global lifetime. Arrays use the shared new/free heap;
the local string uses the sized pool.

Outer reserve's actual handler `CBD327` loads FuncInfo `DF5AFC`, map
`DF5AF4={-1,CBD310}`. Its action computes two pointer arguments and calls full
`401130`, whose body is just `RET`. It does **not** free the fresh outer buffer
or completed inner buffers if copying throws. State0 and saved scratch reads
are real, but generic vector rollback is not. Both complete EH chains were
verified from each parent's handler PUSH through the map and actual target.

## Evidence boundary

The frozen discovery contains 30 fresh guarded spans, 6,705 bytes, the complete
gather and helper sequences, exact table/store derivation, both EH chains, and
26 current source/header pins. Queries verified existing `bsp.gpr` and
`/battlestationspacific.exe`; original PE bytes were unchanged. This packet has
no build, runtime, display, device, gamma, gameplay, Ghidra or shared metadata
changes. The result is a bounded source-readiness proposal, not a completed raw
gather or lifetime proof for unrelated renderer resources.
