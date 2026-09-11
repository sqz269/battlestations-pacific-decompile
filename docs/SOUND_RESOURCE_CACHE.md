# Sound resource cache loading and removal

Addresses: 00a84740, 00a84530, 00a842e0, 00a84680, 00a85560.

The typed implementation supplies the complete normal loader and cache record
operations over the existing `SoundResourceOwner` projection. The resolver,
creator, retain operation, resource size, platform pump and VFS metadata query
are required `SoundResourceCacheHost` services. These boundaries perform real
work; the cache provides no default or fabricated resource implementations.
The integrator connects the asset, ownership and platform/VFS packets to this
interface. This packet alone does not establish audible or game startup success.

## ABI and representations

| Native entry | Recovered ABI | Last instruction / inclusive end |
| --- | --- | --- |
| 00a84740 | ECX=owner; path, options, clone byte, load-if-missing byte; EAX=resource; RET10 | 00a84c87 RET10, 3 bytes / 00a84c89 |
| 00a84530 | ECX=owner+4 array header; source record; RET4 | 00a8459c RET4, 3 bytes / 00a8459e |
| 00a842e0 | ECX=record; no stack args; RET | 00a84357 RET, 1 byte / 00a84357 |
| 00a84680 | ECX=destination; source record; EAX=destination; RET4 | 00a8471f RET4, 3 bytes / 00a84721 |
| 00a85560 | ECX=owner; name; RET4 | 00a85763 backward JMP, 5 bytes / 00a85767; shared RET4 at 00a856d7 |

Records are 2Ch: native name +0, list subobject +8 (head +C, count +10),
five metadata words +14..24, resource pointer +28. The existing `field_08`
belongs to the list/allocator representation; no game meaning is invented.
Temporary host construction leaves its existing zero default. Record assignment
does not write that field. Standard string/list/vector storage replaces native
allocation machinery; neither native physical layout nor allocation traces are
claimed. Native path temporaries reuse `NativeString`, the supplied storage and
the recovered actual-header normalization functions.

## Loader order and non-obvious branches

00a84768 calls platform message service 00beccd0 before copying the input. The
copied input is normalized by 00bee690. All aliases of all records are searched
with a counted-length check followed by the CRT case-insensitive C-string
comparison. A matching alias with a null resource only ends that record's alias
scan: the outer search continues. Any nonnull hit invokes owner virtual +C
00a854c0, even with `clone=false` or `load_if_missing=false`.

On a miss, owner virtual +4 00a82ea0 returns an owned native string. The cache
copies it, releases that returned temporary, and normalizes its own canonical
copy. If canonical and request differ under the native C-string comparison,
the next scan compares only each record's first alias to the canonical name.
The first matching record gains the requested alias, including when its resource
is null. A live match is retained immediately. A null match falls through to
creation, without scanning later canonical records. This alias mutation occurs
even if `load_if_missing=false`, in which case the eventual result is null.

When loading is allowed, virtual +8 00a835b0 runs before record construction.
The temporary record receives canonical name and first alias, then the five-word
00bdd340 metadata result; a differing requested name becomes the second alias.
00a84530 copies the record into the cache, including null resource results. A
nonnull result's virtual +C size is queried next, then added to the current +10
accounting as unsigned DWORD arithmetic. The accounting field is read after
the callback. Only this newly created path tests `clone`; its nonnull result is
retained when that flag is true. Temporary record storage and paths are released
afterward. Null cache aliases therefore permit later reloads and duplicate null
records; there is no negative-cache shortcut.

No reference to vector storage survives a resolver, creator, metadata or retain
callback. Creation may recursively append records before the outer load commits
its own record. The caller serializes access on the owning game thread; the
native entry has no separately acquired synchronization lock.

## Record storage, exceptions and removal

00a84530 grows only at count==capacity, doubles using DWORD arithmetic, and
selects at least 64 with a signed comparison. Helper 00a84450 allocates/copies
the existing records before replacing storage/capacity. The reconstruction uses
`vector::reserve`, records explicit native capacity only after success, then
copies the new record. Record copies do not retain the resource. Overflow that
would place a native record out of bounds is rejected by the host; corrupt
header behavior is outside the valid projection.

00a842e0 clears aliases, frees their sentinel, and releases name storage. Its
tail does not release +28 or reset scalar metadata. Standard containers implement
this storage destruction; clearing host fields is not a claim that the dead
native string header was cleared. The loader EH FuncInfo DEBEE0 points to five
unwind actions at DEBF04: CB5E40/48/50 destroy paths, CB5E58 destroys a partially
constructed record name, and CB5E60 calls 00a842e0. There is no game-resource
release action for the creator's returned pointer. Exceptions propagate without
invented resource rollback: metadata failure occurs before cache adoption;
size/retain failure occurs after the record has been committed. Native raw
allocation failure and SEH equivalence remain unclaimed.

00a84680 assigns the name, clears then repopulates aliases, then copies five
metadata words and the resource pointer. It preserves the allocator word and
does not acquire or release the resource. Self-assignment skips string/list work.

00a85560 first copies the input, constructs a separately normalized copy through
00bee780, and immediately destroys that normalized result. Assembly at
00a855bc..00a855e7 and 00a8562e/00a8564b establishes that the alias search uses
the original input copy. Case comparison is insensitive, but converting slashes
or trimming is not retained. On the first alias match, the native requires a
nonnull resource, queries its size and subtracts from the current accounting.
It then reloads count/base, copies the current last record over the matched
record if different, destroys the current last storage and decrements count.
No resource release happens here. The matched pointer is captured over the
size callback and must remain valid; this matters when composing the destructor
that invokes removal. Both diagnostic paths call 004254b0, verified as a RET body.

## Analysis repair evidence and validation

Ghidra remained read only in this worker. Current target was verified through
the configured BSP bridge (`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`).
00a842e0's saved body ends at 00a84318 because call 00a84314 to 00bf65ac is treated
as no-return. Disk decoding proves the missing tail through RET 00a84357.
Supporting reserve 00a84450 has an omitted 10-byte interval 00a84510..00a84519
after call 00a8450b to 00bf6989: it restores ESP/EDI, installs the new pointer and
capacity, and pops EBP. These gaps and reviewed names are reported for parent
annotation; this worker did not repair saved analysis or claim refreshed names.

Win32 MSVC Release build and both existing CTests passed after all eight seed
byte checks matched. One ignored fixture passed callback ordering, normalized
aliases, unconditional cached retain, null hit/reload/duplicate behavior,
metadata and size exception boundaries, callback-mutated unsigned accounting,
discarded removal normalization, last-record replacement and allocator-word
preservation. Its resource objects are explicit fixture stand-ins, not production
resource providers. Commands and output are in `local/run_sound_cache_fixture.cmd`,
`local/sound-cache-fixture.log` and `local/sound-cache-build.log`.

These results establish reconstructed/build-tested/fixture-tested behavior.
Native ABI compatibility, identical pool/STL/SEH behavior, complete executable
startup, FMOD asset creation, audible playback and gameplay were not tested by
this cache fixture. See `reports/sound_resource_cache.json` for exact scope.
