# Native texture surface cache storage

The complete reserve, resize, and storage destructor are reconstructed in
`src/native_texture_surface_cache_storage.cpp`. They operate on the actual
12-byte header embedded at native texture owner +40h, using the existing shared
allocation/free boundary. Five original-byte comparisons pass, including a real
CRT new-handler retry that changes the header while reserve is allocating.

The header has data at +00h, signed count at +04h, and signed capacity at +08h.
Each eight-byte record contains a mip DWORD and a borrowed surface-owner pointer.
These three routines never retain, release, or destroy the pointed-to surfaces.
They provide no header constructor or implicit destructor.

| Original entry | Complete bytes | Original ABI | Reconstructed operation |
| --- | ---: | --- | --- |
| `00B3D9B0..00B3DA17` | 104 | ECX header; signed capacity stack DWORD; RET4 | `reserve_native_texture_surface_cache_00b3d9b0` |
| `00B3DA20..00B3DA6F` | 80 | ECX header; signed count stack DWORD; RET4 | `resize_native_texture_surface_cache_00b3da20` |
| `00B3EC40..00B3EC56` | 23 | ECX header; RET | `destroy_native_texture_surface_cache_00b3ec40` |

EAX has no stable result contract. The descriptive names are reconstruction
names, not recovered symbols. The new C++ entry points take a reference to
`NativeTextureSurfaceCacheStorage`; they are not binary ABI replacements.

## Exact storage behavior

Reserve clamps its signed request to at least one, then returns if current signed
capacity is already sufficient. The allocation request is the wrapped DWORD
product `capacity * 8`, with equal native and host byte counts. It uses the real
`BF55BE -> BF681B` allocation boundary through `singleton_lifetime_allocate`.
Allocation failure propagates before publication; no cleanup or catch is added.

After allocation returns, reserve starts a signed loop at zero. Each iteration
uses current count as its bound, captures current data, and copies two separate
DWORDs to the next destination record. The second source DWORD is read after the
first destination DWORD is written. A zero destination address skips those
accesses. Destination address and index increments use DWORD arithmetic. Reserve
then frees current header data and publishes its captured allocation followed by
the requested capacity. It preserves count and leaves excess records unwritten.

Resize compares the signed requested count with current capacity and reserves if
necessary. It reloads count after reserve. Each newly exposed record is computed
from current data and the current loop index, tested for address zero, and cleared
with two ordered DWORD stores. Shrink repeatedly decrements the actual count and
rereads it before the next comparison. A final store writes the requested count.
Shrinking does not clear old records or release their borrowed pointers.

Destroy calls the complete resize-to-zero routine, reloads current data, and
frees it through `BF6989 -> BF65AC -> BF9DC8`. It leaves the resulting count,
dangling data pointer, and capacity in the actual header. None of the three
functions installs an exception frame or supplies an unwind cleanup.

## Assembly and saved-analysis boundaries

All eight fixture spans were freshly read through the guarded Ghidra CLI for
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and matched to the
installed executable SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The fixture rechecks every span against the installed file before execution.

The saved no-return analysis hides normal fallthrough after free. At capture,
reserve's body already included `00B3DA17`, but its instruction listing omitted
`00B3DA0A..00B3DA12`: stack adjustment, new data publication, new capacity
publication, and EBX restore. Destroy's saved body ended at `00B3EC51`; the complete
body also contains `00B3EC52..00B3EC56` (stack adjustment, ESI restore, RET).
Raw installed bytes, verified against live Ghidra bytes, establish both tails.
The worker does not mutate Ghidra; integration must repair the saved instruction
gap and destructor extent before refreshed exports and annotations.

## Validation and reproducibility

The ignored fixture is
`local/cache3_check.cpp` in worktree
`J:/PROG/battlestations-pacific-decompile-native-texture-surface-cache-storage-20260910b`.
Run `./local/build_cache3_check.ps1` there. The committed
`reports/native_texture_surface_cache_storage_audit.json` pins the local build
script, source/header, installed spans, postimages, raw traces, executable, object
providers, and build/test logs.

The fixture executes all 207 owned original bytes and the three complete original
CRT jump thunks. Only the CRT allocator and final free entries are adapted to the
unchanged shared production helpers. All owned original bytes remain unchanged.
It compares against the real new source compiled into
`native_texture_surface_cache_storage.obj`; the linker map verifies all three
owned providers plus the unchanged aliased allocation/free helper providers.

Five paired scenarios compare 1,271 normalized DWORDs:

1. Negative reserve's minimum-one clamp, growth copy, untouched excess records,
   zero-filled growth, shrink without record destruction, and stale destroy state.
2. Actual allocator `malloc` failure, new-handler return zero, and `bad_alloc`,
   leaving the original header and records unchanged.
3. Actual new-handler retry replaces data/count/capacity; reserve copies current
   records, frees current data, and publishes its captured requested capacity.
4. Signed resize to -1 preserves record bytes while changing count through repeated
   decrements. The fixture then frees its separately owned storage directly.
5. A zero-count reserve request `0x20000000` wraps its byte product to zero and
   reaches the real zero-byte allocator, followed by ordinary destruction.

All native postimages equal their verified preimages except the two declared CRT
boundary jumps. MSVC Win32 `/W4 /WX /fp:strict /O2 /EHsc` compilation passes for
the new source and fixture. `scripts/build.ps1` also passes both existing CTests
after seed verification. Shared CMake registration is left to the integrator;
the new source is separately compiled and linked by this fixture.

## Limits

The signed and wrapped cases establish arithmetic and isolated state behavior;
they do not prove that game callers issue these requests or that their resulting
nominal capacities describe usable storage. Every accessed record must be valid
and every freed pointer must belong to the shared allocator. In particular,
destroying the isolated negative-count case would resize from -1 to zero and
access record[-1], so the fixture does not claim that subsequent operation is safe.

The new-handler retry is a controlled ambient CRT callback, not an extra cache
callback. Source fields are accessed with explicit ordered scalar loads/stores.
The fixture does not provide concurrent mutation or per-instruction write-watch
proof, arbitrary invalid-header support, or an invented allocator-null-success
path. Data pointers are normalized by actual allocation identity; freed payload
snapshots are retained before the real free and never reread afterward. This is
build and original-byte fixture evidence, not in-game or rendering validation.
