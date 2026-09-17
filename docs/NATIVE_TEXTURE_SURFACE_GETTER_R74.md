# Native retained texture level-surface getter

Addresses: 00b3fd80

## Result

R74 reconstructs the complete `00B3FD80..00B3FE87` getter in
`native_texture_surface_getter`. It composes the existing canonical surface
pool, native surface constructor/lifetime and texture cache-storage provider.
This closes another dependency of shadow texture-holder B4E020 and the larger
render-resource initializer. Application binding and gameplay remain open.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| `00B3FD80`, 264 bytes | Complete source control flow and one-state C++ cleanup projection | ECX texture owner; mip and unused stack word; EAX retained surface owner; `RET 8` at B3FE85 |

The descriptive name `BSP_D3D9Texture2D_GetRetainedLevelSurface` is a hypothesis,
not a recovered symbol. The current Ghidra body has 97 listed instructions and
no flow gaps. Earlier discovery notes used FE88 as the range end; FE87 is the
inclusive last byte, and FE88 is the exclusive end.

## Native storage and call schedule

The entry caches the actual InterlockedIncrement IAT target at CE221C once.
It captures the signed cache count at texture+44, and, for a positive count,
the data pointer at +40. Its eight-byte records hold mip then surface owner.
A hit increments captured owner+4 before testing the owner pointer for null.
The result is an additional caller-owned intrusive reference. No extra COM
reference operation occurs on the cache-hit path.

On a miss, the current texture COM at +10 dispatches GetSurfaceLevel through
slot 48h. The output address is the incoming first stack word, whose preimage
is the mip itself. The decompiler loses this reuse and misidentifies the later
constructor input. The source explicitly preserves those bits in the acquired
frame until COM overwrites them. HRESULT is observed but ignored by the native
sequence; failed writes do not acquire a synthetic null output.

The actual static surface pool B3ED40 supplies a raw slot. State 0 is armed
after the allocation returns, including its null result path. A nonnull slot
is passed to the full existing B3F630 with current texture flags and the low
kind byte of `(flags >> 8) & FFFFFF01h`. The current COM output and its Release
target are loaded, then state -1 is stored before calling Release.

Current texture flag bit zero decides whether the result is cached. If clear,
the same captured InterlockedIncrement target retains a separate cache
reference, even before the native pointer validity assumptions. Equal current
count/capacity doubles capacity with DWORD wrap and a signed minimum of one,
through full B3D9B0. The append reloads current data/count, stores the original
captured mip and owner if its computed slot is nonzero, then increments current
count. A set bit skips retention and append; repeated requests create separate
surface owners.

| Call site | Native target | Source dependency |
| --- | --- | --- |
| B3FDCC | Captured CE221C target | Actual Win32 InterlockedIncrement on cached owner+4 |
| B3FDE5 | Current texture COM+48h | Real GetSurfaceLevel, output reuses mip word |
| B3FDEC | B3ED40 | Canonical surface pool, ECX=0108DB00 |
| B3FE16 | B3F630 | Complete existing concrete surface constructor |
| B3FE2F | Current output COM+8 | Real creator Release after disarming cleanup |
| B3FE3B | Same captured CE221C target | Actual cache-reference increment |
| B3FE57 | B3D9B0 | Actual eight-byte cache storage reserve |

All entered storage and domains must be valid under the native reads/writes.
No semantic texture/surface replacement, duplicate refcount, alternate pool,
safe-null return, current-profile switch or cache rollback is added.

## Unwind and interface limits

Handler CBEFA8 selects FuncInfo DF7894 and map DF788C. Its only state is
0 -> -1 through CBEFA0, which loads the saved raw slot and tail-jumps to the
existing B3DCC0 pool-return wrapper. No COM release or completed surface
destructor appears in this getter's cleanup map.

The source uses RAII to preserve the initial C++ exception search, returns the
slot only while state 0 is armed, and terminates on a second C++ cleanup
exception. Its explicit context/acquired frame is a new source interface.
Native FH3/SEH, stack-layout/private-frame aliases and failure execution are
not established. Existing concrete surface-constructor cleanup remains that
provider's separately documented C++ projection.

## Validation

The strict Win32 MSVC build (`/MD /W4 /WX /fp:strict`) and all three existing
CTests pass. Live Ghidra and original PE match for 330 bytes: the full getter,
18 unwind/handler bytes, 44 map/FuncInfo bytes, and the texture profile's +30h
getter slot. The mechanical report check validates all three direct call rows.

One ignored focused probe executes all 264 original getter bytes and the
source against real Direct3D9. Both use the R73 runtime factory to create real
64x32 seven-level textures. Four getter requests per lane use mips 0,1,0,2 and
distinct unused argument words:

| Mode | Original and source result |
| --- | --- |
| Flags 10h, default-pool render target | Three surfaces, one same-owner cache hit, count 3/capacity 4; caller and cache references remain distinct |
| Flags 1, managed texture | Four separate owners, no cache entries or capacity; repeated level zero produces separate owners |

All 16 calls pass. The initialized 31h surface prefix matches original/source
after normalizing only COM identity at +2C. Untouched bytes +31..33 and pool
metadata are outside that byte-comparison claim. Dimensions, flags, format,
kind zero, actual counts, ordered cache records, tracking and acquired-frame
states are checked. Caller releases leave one cache reference per cached
surface; full texture deletion then releases the cache and all surfaces through
their existing native lifetime providers. Raw singleton shutdown drains two
registrations and clears publications. Final device/API references are zero.

The original getter's direct pool/constructor/reserve calls use the full
existing source providers through x86 ABI adapters. COM calls use real current
interfaces; the captured atomic cell resolves the real Win32 export. The
original EH target is an unreached fail-fast trap, so this is normal-path
evidence. The renderer is an explicit zeroed 1D94h fixture with real device and
empty resource containers; neither native renderer construction nor application
startup is exercised. Canonical string, pool, scalar and support-manager domains
are the existing application providers.

An initial probe compilation used the Windows intrinsic spelling for the
imported atomic function; resolving the actual Win32 export corrected its
calling-convention type. No production source change was needed. The retained
compile log documents that setup error. No new repository tests were added.

Failure/null-output paths, kind-one surfaces, invalid headers/owners, concurrent
mutation, mutation of the IAT cell during a call, constructor cleanup, native
FH3/SEH, full application resource creation and gameplay remain untested.

## Next dependency

The B4E020 shadow texture-holder path can now use full B2A070/B3F7B0 runtime
creation and this full retained surface getter. Its B3D640 operation still needs
a verified function boundary and behavior; holder lifetime and resource-service
composition also remain open. Do not replace that operation with a no-op or
claim render-resource initialization merely from this getter's fixture result.
