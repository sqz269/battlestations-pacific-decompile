# Native VFS plain-list and pair-vector lifetime

Addresses: `00BDAED0`, `007F8310`, `00BDB3C0`, `007F8770`, `00BDB850`,
`00BDCC60`, `00BDCD10`, `00BDEC70`, `00BE0350`.

The two independent sequence packets in
[the manager container map](NATIVE_VFS_MANAGER_CONTAINERS_PACKET_MAP.md) now
have actual-storage source in `src/native_vfs_sequence_lifetime.cpp`. All nine
logical bodies (792 bytes) were read, including the stored-listing gaps below.
This supplies bounded helpers; the full manager lifetime remains incomplete.
Names are descriptive hypotheses, not recovered symbols. Ghidra was read-only:
the proposed names are recorded in the repository ledger but were not applied.

The [report](../reports/native_vfs_sequence_lifetime.json) carries each native
CALL's site, target and containing function, original hashes, source COFF entry
sections and relocations, frozen build/probe provenance, and exception limits.

| Entry / inclusive logical end | Bytes | Original ABI | Coverage |
| --- | ---: | --- | --- |
| BDAED0-BDAF17 | 72 | ECX owner; RET0 | Complete plain-list storage behavior |
| 7F8310-7F8357 | 72 | ECX owner; RET0 | Complete plain-list storage behavior |
| BDB3C0-BDB3C4 | 5 | Tail JMP BDAED0; unchanged inputs | Complete tail thunk |
| 7F8770-7F8774 | 5 | Tail JMP 7F8310; unchanged inputs | Complete tail thunk |
| BDB850-BDB8C5 | 118 | ECX pair; RET0 | Complete normal storage behavior; native throwing-getter/FH3 cleanup outside source exception domain |
| BDCC60-BDCD0E | 175 | ECX destination; stack source; EAX destination; RET4 | Complete normal branches and source first-string cleanup; original FH3 execution untested |
| BDCD10-BDCDEC | 221 | ECX vector; signed stack capacity; RET4 | Complete normal branches; no reserve rollback, matching RET-only unwind action |
| BDEC70-BDECD4 | 101 | ECX vector; signed stack count; RET4 | Complete grow/shrink branches under existing storage exception contract |
| BE0350-BE0366 | 23 | ECX vector; RET0 | Complete normal storage behavior under existing storage exception contract |

The four list APIs use a Win32 fastcall adapter with unused EDX and no stack
arguments. The pair/vector APIs pass `NativeStringStorage&` explicitly and do
not expose the original binary ABI. Neither the adapted list entries nor the
other source interfaces are claimed as drop-in game replacements.

## Actual layouts and observable order

Plain-list owner+4 is the sentinel pointer and owner+8 is the count. Nodes have
next/previous DWORD links at +0/+4. Destruction captures the first node, writes
the current head's next and previous links to itself, and clears the live count
before freeing any nodes. Each iteration captures next before the free and
compares it against the reloaded current head afterward. It finally frees the
current head and clears owner+4. Owner+0 and node payload pointers are untouched.
There are no added null-owner or null-head guards.

The pair contains the established native eight-byte `(length,data)` headers at
+0 and +8; `include/bsp/native_string.hpp` and the actual 41DD40 producer settle
that layout. BDB850 releases the second string, then reloads and releases the
first. Release receives the captured pointer and wrapping DWORD `length+1`.
Neither header is cleared. With `ActualNativeStringPoolStorage`, each nonnull
release repeats the existing pool getter through the storage bridge.

BDCC60 writes two zero DWORDs before each string's identity guard. Self-copy
therefore clears both headers. The nonidentity branch calls actual-header
41DD40 with `(source_length,true)`, reloads the source length, and, if nonzero,
copies the current destination length from the current source data pointer.
The second construction repeats these steps independently. Callback-visible
header changes are retained, including overlapping source/destination headers.

Although Ghidra names BF7680 `_memcpy`, its BF7694-BF769A comparisons select the
BF7844 backward-copy branch for `source < destination < source+length`.
The direct pair-copy source uses `std::memmove` to preserve that behavior. It
omits a zero-byte memory operation after reading the raw pointers. The existing
41DD40 source provider still has its documented standard-CRT buffer boundary;
this packet does not broaden that provider's overlap or failure contract.

The vector is three DWORDs `(backing,signed_count,signed_capacity)` at +0/+4/+8,
with 10h-byte pairs. BDCD10 clamps a requested capacity below one to one, returns
if the current signed capacity suffices, and allocates wrapping DWORD
`capacity<<4`. Thus `0x10000000<<4` requests zero bytes without an overflow
rejection. It copies the current elements, destroys the old elements, frees
the current backing, then publishes new backing followed by capacity. Loop
conditions and backing fields are reloaded across callbacks; count is not
restored from a snapshot. Null computed placement destinations skip construction
as in the raw branch. No extra failure rollback is added.

BDEC70 reserves only when requested signed count exceeds current capacity.
Growth zeroes all four DWORDs of each new nonnull computed slot. Shrink
decrements the live count before deriving the removed pair's wrapped offset
and destroying it, then rechecks the current count. Requested count is stored
last. The negative-count path is retained: count zero resized to -1 destroys
the pair at backing-10h using 32-bit address arithmetic. BE0350 resizes to zero,
frees the reloaded backing, and retains the pointer/capacity fields.

## Native call contracts

| Caller sites | Callee / source service | ABI and stack evidence |
| --- | --- | --- |
| BDAEF3, BDAF07; 7F8333, 7F8347 | BF65AC returning free / `singleton_lifetime_free` | One cdecl pointer; ADD ESP,4 at BDAEF8, BDAF0C, 7F8338, 7F834C |
| BDB886, BDB8AA | 419CC0 pool getter / explicit storage bridge | No arguments; RET0, EAX pool. Three already-pushed words belong to following return-block call |
| BDB88D, BDB8B1 | BD1510 sized return / `destroy_native_string_header_0041dd20` | ECX pool; stack `(data,length+1,1)`; RET0Ch at BD152F/BD156C; third word unused |
| BDCC99, BDCCDC | 41DD40 / `resize_native_string_header_0041dd40` | ECX destination header; stack `(length,true)`; RET8 at 41DD7F/41DDE1 |
| BDCCAE, BDCCF1 | BF7680 actual overlapping copy / `std::memmove` | cdecl `(destination,source,length)`; ADD ESP,0Ch at BDCCB3/BDCCF6 |
| BDCD52 | BF55BE JMP BF681B allocation / `singleton_lifetime_allocate` | cdecl byte count; ADD ESP,4 at BDCD59; malloc/new-handler source service |
| BDCD8D | BDCC60 pair copy | ECX destination; source stack word; RET4 at BDCD0C |
| BDCDB4, BDECC2 | BDB850 pair destruction | ECX current pair; RET0 |
| BDCDCB, BE035D | BF6989 JMP BF65AC returning free | One cdecl pointer; ADD ESP,4 at BDCDD0/BE0362 |
| BDEC7F | BDCD10 reserve | ECX vector; signed capacity stack word; RET4 at BDCDEA |
| BE0355 | BDEC70 resize | ECX vector; zero stack word; RET4 at BDECD2 |
| CC627E in CC6270 | 401130 | Actual entire body RET; two cdecl words consumed by ADD ESP,8 at CC6283; no rollback |

## Exception states and limits

BDB850 FuncInfo E00440 / map E00438 arms state 0 while destroying +8. Its
CC60B0 action tail-jumps to 41DD20 for captured pair+0. BDCC60 FuncInfo E006A8 /
map E006A0 arms state 0 only after first-string construction succeeds; action
CC6250 tail-jumps to 41DD20 on captured destination+0. Source catch/rethrow
therefore releases only the current first string if second-string construction
throws. First-string construction failure has no pair cleanup; a partially
constructed second string is not destroyed.

BDCD10 FuncInfo E006D4 / map E006CC arms state 0 around the current copied
element. CC6270 computes the copied-index address and passes it with the current
destination to the verified RET-only 401130. Completed pairs and new backing
are not rolled back. The three source allocation-failure fixtures check that
owner publication has not occurred and only a second-string failure releases
the current first string. They do not execute the original FH3 throw/unwind.

`NativeStringStorage::release` is already `noexcept`. In
`ActualNativeStringPoolStorage`, a throwing lazy getter during release would
terminate instead of reproducing native BDB850's throwing-getter/FH3 cleanup.
The source module retains this explicit boundary; no synthetic native handlers,
CRT globals, pool, or stronger rollback guarantees were introduced. The real
service smoke uses `crt_string_storage()` and the existing lifetime allocator;
it does not validate the actual native pool's lazy-getter exception domain.

## Stored-body gaps

These six inclusive intervals remain absent from four saved Ghidra listings.
Matching full logical bytes from Ghidra memory and the installed executable
provide their evidence. No saved body or no-return annotation was repaired.

| Entry | Required raw continuation |
| --- | --- |
| BDAED0 | BDAEF8-BDAF02; BDAF0C-BDAF17 |
| 7F8310 | 7F8338-7F8342; 7F834C-7F8357 |
| BDCD10 | BDCDD0-BDCDD9: stack cleanup, POP EDI, backing/capacity publication, POP EBP |
| BE0350 | BE0362-BE0366: stack cleanup, POP ESI, RET |

BDCD6D-BDCD6F is skipped LEA alignment, not an executed continuation.
All 21 report CALL rows pass the live call checker; that result does not
establish stored-body completeness or repair the six gaps. Two list thunks
and two cleanup tail-jumps have separate transfer records, not CALL rows.

## Validation and frozen provenance

The saved `scripts/build.ps1` run compiled MSVC Win32 with `/W4 /WX /fp:strict`
and passed both existing CTests (`reconstructed_math`, `native_math_differential`)
after seed-byte verification. The ignored focused probe has 17 passing native /
source whole-arena and event-trace comparisons, three source allocation-failure
cases, and a real source CRT/lifetime-service smoke. It adds no permanent tests.

The probe reads the original installed PE and maps a private executable image.
Its 50 recorded address fixups relocate the relevant original bodies and
BF7680 code/tables; allocation/getter/free boundaries use matched fixture
callbacks. BF7680's backward copy executes original bytes. Original FH3 failure
execution and unexercised copy dispatch/CRT branches remain unclaimed. The
installed game and saved analysis are unchanged; there is no game validation.

The interrupted worker froze 59 artifacts in
`local/sequence_lifetime/frozen-final/`, including source, compiled object,
archive, original bytes, probe executable, logs and nine actual COFF entry
sections with relocations. On resumption all 59 frozen and current files
matched their sealed hashes before any build; no C++ or build input changed
and no rebuild overwrote the passing logs. The 18 captured original/support
spans were compared again through the verified read-only BSP CLI against
the installed PE. Full hashes and local provenance paths are in the report.
