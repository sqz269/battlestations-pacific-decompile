# Actual online-manager storage transfers

The captured owner is the 0x3F0-byte `NativeOnlineManagerStorage` allocated at
0073DC50, initialized at A40DF0, and published at F8ABE8 by A3F530. These
functions take that owner in ECX and return with plain `RET`. The C++ interface
uses an explicit reference to the **same bytes**, not the separate
`XLiveSystemPumpContext` projection or `XLiveOwnerAllocation`. Names are
descriptive hypotheses. Live Ghidra project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; installed PE SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Native body | Source | Coverage | Original ABI / inclusive end |
| --- | --- | --- | --- |
| A3ED60 download | `download_native_online_storage_00a3ed60` | complete normal body | ECX owner, RET at A3EF1D |
| A3EF20 upload | `upload_native_online_storage_00a3ef20` | complete normal body | ECX owner, RET at A3F0FE |

## Producers and field identity

The path producer A3ED10 gates `+8C + 4*[+11C]` and calls
`XStorageBuildServerPath(user, 3, null, 0, L"DropRates", owner+154,
&size=200h)`; on success only, it writes state 1 to +12C. A40020's reset
sets +11C to 1 unless +3BD is set, zeros +358/+35C and sets +3B4/+3B8
to 1. A40DF0 initializes +3B4 through +3B8 to zero and subsequently calls
A40020. The direct writer A3F4A0 stores its two DWORD stack inputs at
+358/+35C, then calls upload for states 1/9/4/5/8/10. A3F500 calls
download for states 1/4/9/5/8 without writing those fields. These two
callers were inspected read-only and remain outside this packet. Their state
8 calls do not start a transfer in either callee, and A3F500's state 5 call
does not start a download; the callee's own state gate remains decisive. The SDK
writes path bytes to +154..+353. A3ED60/A3EF20 read the
current +11C when submitting, cache +3B4 once on entry and compare the
indexed DWORD at +8C+4*index to 2. This is a DWORD access, not a uniform
per-user state array; index 1 points into the name region at +90. A caller
must ensure the indexed word is within the owner and meaningful.

| Offset | Producer / consumer, observed meaning |
| --- | --- |
| +12C | A3ED10 successful path state 1; A3ED60 states 2,3,4,5,10; A3EF20 states 6,7,8,9 |
| +130..+14B | SDK overlapped block, 1Ch bytes; transfers clear **only** first five DWORDs through +140, preserving +144/+148 |
| +14C/+150 | owner-held buffer pointer / byte count; allocation 100h for download, 9 for upload, released/replaced only on next preparation |
| +154..+353 | UTF-16 path output of A3ED10, 200h bytes; borrowed by asynchronous SDK calls |
| +354 | latest submission result or extended error; successful completion does not clear it |
| +358/+35C | two DWORD payload fields zeroed by A40020 and written by A3F4A0's stack inputs; download replaces them only for result header first DWORD 9 |
| +370..+383 | download result output, 14h bytes, cleared at download preparation; first DWORD is payload size |

Download prepares a zeroed 100h-byte buffer at states 1/9/4, then clears
five overlapped words and five result words. The 7-argument ordinal 5345
receives the current +11C, +154 path, +150 size, +14C buffer, 14h result
size, +370 result and +130 overlap. Only 997 (`ERROR_IO_PENDING`) selects
state 3; **all other returns**, even immediate success, select state 5.
If pending and the first overlap DWORD is 997, ordinal 5307 receives a
stack-local progress output initialized to the captured manager address and
two null outputs. Its return is ignored. Once that DWORD changes, ordinal
1083 receives (+130, null, TRUE). Its zero result selects state 4 and copies
two buffer DWORDs to +358/+35C only if +370's first DWORD equals 9. Its
nonzero result selects state 5, then ordinal 1082's extended error is saved
at +354; error 8015C004 changes state to 10, otherwise it stays 5.

Upload prepares at states 1/9/4/10/5, freeing the old buffer, allocating
nine bytes, writing current +358/+35C as little-endian DWORDs and a zero
ninth byte, then clearing only the five overlapped words. Ordinal 5305
receives (+11C, +154, 9, buffer, +130). Only 997 selects state 7; every
other return selects 8. Pending uses ordinal 5304 with the same local
progress/null outputs. Completion uses ordinal 1083 with (+130, null,
TRUE), selecting 9 on zero, or 8 and saving ordinal 1082's extended error
at +354 on nonzero. A3EF20 calls diagnostic 4254B0 for preparation and
submission/completion messages; its inspected body returns immediately,
so those calls have no manager-visible effect in the C++ source.

The calls are all Win32 `__stdcall`: ordinals 5345/5305 clean 28/20 stack
bytes, progress ordinals 5307/5304 clean 16, result 1083 cleans 12 and
error 1082 cleans 4. `resolve_native_online_storage_sdk` checks these
exports from an already-loaded XLive module without loading or calling it.
The allocator pair must refer to the same CRT heap as the original
`operator new` at BF55BE and `_free` at BF6989. Both pending buffers and
path/overlap/results live in the captured manager; no callback or queue is
created here. There is no failure-time buffer release beyond the next
preparation, which preserves the game's state machine.

This is a complete normal-body raw-owner reconstruction with explicit SDK
and allocator dependencies, not a binary ABI replacement. An allocation
failure, invalid indexed field, a retired manager/DLL, reentrant mutation,
or a caller that overwrites path/overlap/buffer during an outstanding request
has no supported equivalence contract. Raw A3ED10 path building, producer
callers A3F4A0/A3F500, actual sign-in/notification dispatch, manager
lifetime wiring and a real game session remain separate prerequisites for a
full storage event. No XLive request or network mutation was made for this
reconstruction.

Validation to date: strict MSVC Win32 source compilation and one local
in-memory fixture linked against the resulting library passed; it covered
the offline guard and one pending download/progress/copy/upload/progress/
completion cycle. The report's 18 original calls were checked against live
Ghidra with zero mismatches. `cmake/startup.cmake` is leased by another
worker, so the new translation unit has **not** yet been registered or
included in the default `bsp_core` build. The integration build, two
existing CTests and any game session remain pending.

## Integrated library validation

At `d260af28` this source is registered once in the default Win32 target.
The combined `scripts/build.ps1` build and both existing CTests pass. The
packet's focused fixture was compiled and run against the integrated
`bsp_core.lib`, with saved commands, stdout, exit status, dependency headers
and exact library hashes. The four current packet reports contain 55 checked
numeric call rows and zero failures. Source/header bytes match the reviewed
worker delivery. Fourteen saved names/comments were read back and exported
with previous comments preserved. Exact evidence: `local/checkpoints/d260af28/native-online-procedural-default/validation.json`.
This supersedes earlier worker-specific build or fixture-log limitations;
original ABI/FH3 delivery, full manager/pump adoption and gameplay remain open.
