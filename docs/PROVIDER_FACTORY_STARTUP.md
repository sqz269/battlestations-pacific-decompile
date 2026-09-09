# Provider factories and physical startup mounts

The font probe constructs physical providers using the recovered factory
policy and registers all three initial mounts through the recovered priority
and prefix code: FileStore at 300, persistent data at 99, and the loose root
at 0. It receives the installation root as a host input. FileStore population
and mounted stream opening now feed font loading. Diagnostic cache priming
does not reproduce native startup preload selection, and MPKG remains separate.

## Factory registration and selection

`00be0660` takes manager ECX and a factory pointer on the stack, RET4. It
appends a 0Ch-byte list node before the sentinel at manager+34. Node+8 stores
the supplied raw factory pointer; this path does not AddRef it. Constructor
`00beda60` registers physical singleton `00bed990`, then startup calls
`004fc150` and `00736a90` and appends their results at `0073d675/0073d688`.
The resulting initial order is physical directory, FileStore, MPKG.

`00bdb040`, manager virtual+18, takes ECX manager and system/virtual strings,
RET8, EAX provider. It starts at sentinel->next and calls each factory's
virtual+4 with the same two strings. It returns the first non-null provider;
otherwise null. Factory failures do not reorder or remove entries. The full
85-byte body is independently read because no Ghidra function existed there
before this batch. The general factory list is analyzed, not yet implemented
as a complete host provider manager.

`00be1890` takes manager ECX and five stack arguments:
system path, virtual path, signed priority, ownership byte, device id. Both
returns are RET14h. It invokes manager+18, calls the global manager error
callback+90 on null, and still returns null if that callback returns. Success
stores argument5 at provider+10, logs the paths/device id, and forwards
provider/virtual path/priority/ownership to `00be1740`. The decompiler omits
two arguments; assembly at `00be18c3/00be18f0/00be18f4` establishes the slots.
Device id, error callbacks/logging and native provider lifetime are not silently
replaced by the diagnostic constructor.

## Physical factory and persistent namespace

`00bf4df0`, physical table `00d68cfc` virtual+4, takes factory ECX and two
stack strings, RET8. It accepts a nonempty system path only if its final byte
is backslash. It performs no physical-directory existence check. Pool
allocation then calls `00bf4d30(root, flag)`. Flag is true exactly when the
virtual path compares case-insensitively to `persistent_data` through
`00425850` and the literal at `00cff208`.

The new `create_physical_directory_00bf4df0_fragment` preserves this path
gate and flag selection. It uses shared C++ ownership instead of the native
pool. Unsupported embedded-NUL/oversized strings fail, and allocation exceptions
propagate; native pool failure/SEH and factory ABI are not reproduced.

Physical existence `00bf3f70` checks empty name first, then its last-success
cache, then flag+28. With that flag true, every nonempty suffix is accepted
without querying the OS or changing the cache. It can resolve a logical name
whose subsequent file open fails. Constructor false/index-empty remains the
ordinary `GetFileAttributesA` route. No indexed-name tree is implemented.

Startup supplies current directory plus backslash for both physical mounts:

| Virtual input | Priority | Ownership byte | Device id | Physical existence mode |
|---|---:|---:|---:|---|
| `.` | 0 | 1 | -1 | OS/cache |
| `persistent_data` | 99 | 1 | -1 | Accept nonempty suffix |

The probe substitutes the supplied installation root for current directory.
`00be1740` canonicalizes `.` to empty; signed priority sorting places
`persistent_data` ahead of the loose root; the FileStore mount precedes both.
Callbacks capture shared provider owners. Both physical roots
are identical, so its mounted provider adapter removes the persistent
virtual prefix before path construction only when a nonempty suffix follows.
For a trailing-slash-only name the persistent provider rejects the empty
suffix; a later root provider may instead match an actual directory. Separate physical and
FileStore open adapters supply memory streams; neither implements MPKG.

See `VFS_MOUNT_REGISTRATION.md` for signed descending priority, equal-priority
insertion order and the native path canonicalizer. The payload byte is retained
as metadata; native teardown and deletion rules remain unknown.

## Cache and package continuation

`ARCHIVE_PROVIDER_ENTRY.md` implements FileStore insertion, membership and
memory-stream opening. One existing-probe DAT round trip verifies normalized
insertion, a retained original wrapper, cursor-zero open sharing the backing,
flag-bit0 rejection and backing survival after the store is destroyed. The
ordinary DAT decoder then consumes that opened data. The mounted-stream
integration additionally connects FileStore's priority-300 startup mount and
the flags-2 population fragment `00be7ab0` to the font stream resolver. The
probe explicitly primes its three font resources, then verifies all three
font loads open FileStore with no physical reopen. See
[MOUNTED_RESOURCE_STREAMS.md](MOUNTED_RESOURCE_STREAMS.md).

`PACKAGE_MOUNT_STARTUP.md` implements the package-name priority fragment and
records two fresh startup scans, provider-order enumeration and duplicate
handling. The installed root has no loose MPKG fixture. Package enumeration,
transformed archive parsing, compression and source-slice ownership remain
unported. A missing fixture does not prevent continuing their binary analysis.

## Verification

All 21 spans in `reports/mount_cache_audit.json` matched current installed PE
and saved Ghidra bytes after target verification. The Win32 build, both existing
CTests and full asset D3D9 probe passed. `reports/mount_cache_font_probe.txt`
records the cache round trip, unchanged resolved paths, and bilinear glyph A:
74 lit pixels, zero outside expected bounds, state restored. No new test target
was added; one focused cache-lifetime case extends the existing probe.

Signed priority ties, package overflow names, persistent missing-file behavior
and canonicalizer edge cases are assembly-backed, not native runtime-compared
by that historical probe. Full native mount/provider ownership and preload selection,
original-game resource selection and gameplay equivalence are unestablished.

The current Win32 build and full D3D9 probe also pass after mounted opening
was integrated. [Mounted font output](../reports/mounted_stream_font_probe.txt)
records `cache_opens=3 physical_opens=0 cache_entries=3`, retained atlas
recreation and the same 74-pixel glyph. The open counters describe the font
load after explicit priming, not the physical reads needed to prime it.

Twenty-one Ghidra names/comments were applied and read back, preserving prior
comments and old values in `local/ghidra-annotations-20260909T181015Z.json`.
Three missing functions were created at byte-verified starts; their prior
absence was recorded in `local/mount_cache_function_creation_before.json`.
The program was saved and the inventory/all affected exports refreshed.
The shared `00bf0fb0` received the broader descriptive name
`BSP_FileProvider_ResolveLogicalName` because both physical and FileStore
provider tables use it.
