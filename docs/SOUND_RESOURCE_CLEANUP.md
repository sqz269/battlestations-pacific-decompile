# Sound resource retention and teardown

`src/sound_resource_cleanup.cpp` completes the typed resource lifetime path used
by the sound cache and the required failed-owner cleanup boundary. It uses the
canonical `SoundResourceOwner`/`SoundResourceCacheRecord`, the asset worker's
`SoundOwnedResource`, pooled `NativeString`, and Windows interlocked operations.
Required hosts invoke the real FMOD releases and the independently reconstructed
cache removal. There is no default release or cache-removal implementation.

Names below are descriptive hypotheses. These are new C++ interfaces, not native
objects or binary replacements. The one resize entry is explicitly a fragment.

| Address, inclusive body end | Original ABI and reconstructed contract |
| --- | --- |
| `00a854c0..00a854d4` | ECX manager unused; one resource stack argument; `RET4`, EAX input pointer. Interlocked increment of resource `+4`, no null guard. |
| `00a854e0..00a854fe` | ECX manager unused; one resource stack argument; `RET4`. Interlocked decrement `+4`; current resource slot0 only at zero. The supported D5B118 profile reaches BD30E0, deleting destructor A85AC0 with flag1. |
| `00a818b0..00a818b3` | ECX resource, `RET`, EAX current DWORD `+28`. Size is not recomputed from FMOD or a cached host estimate. |
| `00a85790..00a858e2` | ECX resource, `RET`. Destruct the concrete resource as detailed below; caller still owns allocation. |
| `00a85ac0..00a85add` | ECX resource, flags stack, `RET4`, EAX original pointer. Run A85790; scalar-free storage only for flags bit0. A throwing destructor prevents free. |
| `00a84c90..00a84cf9` | ECX cache owner, `RET`. Subtract current tail size, release current tail resource, reload the cache, destroy/pop its current tail if nonempty, repeat. Finish with resize-zero. |
| `00a845a0..00a8466b` | ECX array header at owner+4, signed requested count stack, `RET4`. Only `0 <= requested <= current count` is reconstructed: decrement count before each reverse record-storage destruction. Native growth calls A84450 and initializes new records; that branch is excluded. |
| `00a84d00..00a84d16` | ECX array header at owner+4, `RET`. Resize-zero then free backing allocation. No resource releases. Also reached by base-owner unwind. |
| `00a85500..00a8555e` | ECX owner, `RET`. Set D5B1E8; A84C90; resize-zero; free backing array. Native pointer/capacity words are not reset by free. |
| `00a85a50..00a85abf` | ECX derived owner, `RET`. Set D5B210, nullsafe release of error resource `+14`; clear that field after release returns, then A85500. Base cleanup also runs if the final error-resource release throws. |

## Concrete resource and FMOD ownership

Live table D5B118 contains `+0=00bd30e0`, `+4=00a85ac0`, `+8=00befaf0`,
`+0C=00a818b0`. Both cache profiles D5B210 and D5B1E8 contain retain A854C0 at
`+0C` and release A854E0 at `+10`. The resource type in
`include/bsp/sound_resource_asset.hpp` supplies the actual reference word and
canonical fields. Its profile words remain evidence values, not host function
pointers. Dispatch here supports this concrete resource/profile contract only.

A85790 first writes D5B118 and arms name/base cleanup. When bank `+0C` is
nonnull, it samples `FMOD_Memory_GetStats(&current,NULL)`, calls the bank's
`FMOD::Sound::release`, handles only result `2Bh` by the existing memory
checkpoint, then samples stats again. When event project `+8` is nonnull, it
does the same around project table slot0. The native event-project call pushes
the project pointer on the stack; it is a stdcall library virtual dispatch.
The subsound at `+10` is borrowed and receives no separate release.

The native logging callee `004254b0` is exactly `RET`; its before/after delta
arguments do not produce a log. The reconstruction preserves the real stats
calls and memory checkpoint and omits that proven empty diagnostic operation.
FMOD release errors other than `2Bh` are ignored, and `2Bh` also continues after
the checkpoint. Hosts can retain raw library results without changing this policy.

After both possible releases, the native loads the CURRENT sound singleton
F8BBD8, obtains its resource owner at `+54` through A79910, and calls A85560
with the resource's canonical string at `+14`. `SoundResourceCacheRemovalHost`
must route this to the current owner and the real
`remove_sound_resource_cache_entry_00a85560`; it cannot merely clear strings.
That cache routine is independently implemented and owned in
`src/sound_resource_cache.cpp`.

Only after cache removal returns does the destructor capture size `+28`, clear
event/bank/subsound fields, subtract that size from the supplied actual F8BBE4
DWORD, release the pooled name, and write reference-base profile CEB130. Integer
subtractions use DWORD wrap. No balancing F8BBE4 increment was found in the
asset loader, constructor, cache load, or append packets; no increment is invented.
The apparent diagnostic counter's wider producer remains outside this packet.

The deleting destructor frees standard-new `SoundOwnedResource` projection
storage only after destruction succeeds and flags bit0 is set. With flag0 the
caller retains storage whose native-style string header is now dead/stale.
The object's implicit C++ destructor does not own FMOD handles or pooled strings;
running the explicit native destructor twice is outside the supported domain.

## Cache reentrancy and cleanup order

A84C90 does not retain a record pointer over release. At A84CB4 it subtracts the
size result, at A84CB7..A84CCC it reloads and releases the current last resource,
and at A84CCE it reloads the current count. If the release invoked A85790, that
destructor may already have removed a record via A85560 and changed accounting.
The loop then destroys/pops the CURRENT last record if one remains, just as the
assembly does. It does not compensate for removal or repeated size subtraction.

For the focused same-owner one-record fixture, an error-resource reference and
the cache reference share one resource. Derived destruction drops the former;
base cleanup subtracts its size and drops the final cache reference; the resource
destructor's alias removal subtracts the size again and removes the record. The
native owner total therefore wraps below zero. The reconstruction preserves this
observed sequence and does not reinterpret it as a cleaner ownership algorithm.
Other resources skipped by reentrant tail removal are not silently released by an
extra host sweep. The actual current-singleton relationship at each call remains
the composite host's responsibility.

Record cleanup calls the cache worker's A842E0 storage routine. It releases aliases
and name storage without touching opaque resource pointers or metadata. The
shrink fragment clears the current last record then pops it without allocating.
Native A84640 decrements count before A8464C destroys storage; this intermediate
count is not reproduced through native allocator callbacks, which are outside the
standard-container projection. Moving a record merely to change that order would
add a possible MSVC list-sentinel allocation during cleanup. A84C90's normal
record cleanup already precedes its count decrement. Standard vector storage
replaces the native allocator/layout; logical `capacity_0c` remains unchanged.
Null records on a path that queries a resource's size are outside the native
valid-input domain: native code also dereferences the resource without a guard.

## Unwind and analysis evidence

Resource handler CB5F73 loads FuncInfo DEC044. Unwind map DEC034 is
`{-1,CB5F60},{0,CB5F68}`: CB5F68 destroys resource name+14 through 0041DD20,
then CB5F60 routes A81880 to reference-base destructor BD30F0. The C++ guard
preserves that cleanup if a required host throws. It does not retry unvisited
FMOD releases, remove a cache entry, or subtract accounting on that unwind path.

Base-owner handler CB5F2B loads DEBFE4; map DEBFDC is `{-1,CB5F20}`. CB5F20
passes owner+4 to A84D00, destroying/freeing remaining record STORAGE if A84C90
throws. Derived-owner handler CB5FB8 loads DEC0AC; map DEC0A4 is
`{-1,CB5FB0}`, which invokes A85500. These are scoped C++ cleanup projections;
original SEH frame layout and arbitrary double-exception behavior are not claimed.

All analysis batches use `bsp.py`'s client verification of project `bsp`,
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language,
and image base 00400000. This worker performs no Ghidra writes. A818B0 was
unrecognized as a function start; its four raw bytes prove the complete body.
False no-return annotations on CRT free omit these continuations:

| Call site | Verified continuation |
| --- | --- |
| A85547 -> BF6989 | A8554C restores saved ECX; A85550 `ADD ESP,4`; A85553 pops ESI; restores FS; A8555B `ADD ESP,10h`; A8555E `RET`. |
| A84D0D -> BF6989 | A84D12 `ADD ESP,4`; A84D15 pops ESI; A84D16 `RET`. |
| A85AD0 -> BF65AC | A85AD5 `ADD ESP,4`; joins stored branch A85AD8; return at A85ADB, length3. |

The primary owns repairs, annotation/save, and refreshed exports after leases
release. CRT function names are retained. Full installed-byte comparisons and
build/fixture outcomes are in `reports/sound_resource_cleanup.json`.

## Validation boundary

The focused ignored local probe uses the actual shared resource fields and cache
append/removal/storage implementations, with explicitly recording FMOD fixture
operations. It checks reference sharing, bank/project order, the borrowed subsound,
memory error handling, reentrant cache removal/accounting, and pooled/container
storage release. It is not native differential execution or actual FMOD playback.
The primary's installed-library integration supplies separate runtime evidence.
Full resize growth, original object/allocator ABI, and gameplay remain unvalidated.
