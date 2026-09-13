# MPAK container ownership: BA discovery

Addresses: 00bb7a20, 00bb7ba0, 00a40d60, 00bb6cd0, 00bb7590, 00bb6f00, 00bb7ad0, 00a40b90, 00bb65a0, 00bb7240, 00bb6630, 00bb7640, 00bb6180, 00543e50, 00cc44f0, 00cc4510

This is read-only discovery, with **zero reconstructed bodies**. The three parser
container calls remain STL contracts. The next useful game-owned packet is the
two MPAK record copy constructors plus the shared custom string-vector copy:
`BB65A0`, `BB6630`, `543E50`, **three bodies / 332 bytes**. Their descriptions
below are hypotheses about source intent, not recovered symbols.

The analysis used `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` through
the target-verifying `bsp.py ghidra` commands. Nine bounded spans were compared
byte-for-byte with the unchanged installed PE, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The machine-readable report carries the ranges, hashes, call sites and limits.
No saved analysis, exports, names, tags, C++ or shared metadata were changed.

## Container boundary

| Entry and inclusive end | Classification and inspected behavior | ABI / coverage |
| --- | --- | --- |
| BB7A20..BB7AC0 | STL vector of 24h records, `push_back`; spare capacity uses uninitialized fill of one element, otherwise checked end insertion | ECX vector, stack source pointer, RET4; complete body read |
| BB7BA0..BB7C40 | Same specialization for 14h directory records | ECX vector, stack source pointer, RET4; complete body read |
| A40D60..A40DE8 | Already reviewed shared STL `vector<int>::insert`; also used by the award queue | ECX vector, stack result iterator / where-container / where-pointer / value-pointer, RET10h; complete body read |
| BB7590..BB7639 / BB7AD0..BB7B79 | Checked one-element insert for 24h / 14h records | Same four stack words and RET10h; complete body read |
| BB6CD0..BB6D62 / BB6F00..BB6F8E | Compiler-emitted uninitialized fill, repeatedly copy-constructing the same source; completed prefix destruction and rethrow are separate catch bodies | ECX destination, EDX count, four stack words, RET10h; main and catch listings read |
| BB7240..BB754B / BB7640..BB791A | STL `_Insert_n`: alias-protecting temporary, size checks, 1.5-times growth, allocate/copy/fill/relocate, or in-place shifts | ECX vector, stack where-container / where-pointer / count / source-pointer, RET10h; classification and selected assembly, not a complete EH audit |
| A40B90..A40D43 | Shared DWORD `_Insert_n`; scalar alias copy, same checked growth policy | Same four-word insertion ABI, RET10h; bounded classification, not a complete allocator audit |
| BB6180..BB6213 | STL DWORD-vector copy constructor; allocate exactly source size and checked byte copy | ECX destination vector, stack source vector, EAX destination, RET4; complete listing read |

The classification follows the bodies, not the existing medium-confidence
`stl_probable` tags: all three container types have allocator/debug storage at
`+0`, begin/end/capacity pointers at `+4/+8/+C`, and the matching iterator pair
`{container,pointer}`. The insertion specializations preserve an element offset
across reallocation, pass count one to `_Insert_n`, and rebuild the result pair.
Maximum counts are `071C71C7h`, `0CCCCCCCh`, `3FFFFFFFh`, respectively, matching
`UINT32_MAX / sizeof(element)`. Growth uses old capacity plus its unsigned half,
then at least size plus count. None interprets an archive field.

`BB7A20` and `BB7BA0` capture the old end before construction and publish
`captured_end + stride` only after the copy returns. The fill calls at
`BB7A85` / `BB7C05` pass ECX=end, EDX=1, then stack
`source, vector, source, tag`; only the tag's low byte is initialized to zero.
The four stack words are confirmed by the callee's RET10h. Do not recreate the
decompiler's apparently uninitialized DWORD as an application-level input.

The fill catch blocks are separate Ghidra functions `BB6D2B..BB6D4F` and
`BB6F5B..BB6F7B`. They destroy only the completed prefix and rethrow through
`BF6885`. File fill delegates each element to the allocator destroy helper
`BB5E50`; directory fill calls the established `BB6500` destructor. These catch
sites must not be attributed to the enclosing function merely because its
minimum-to-maximum range spans them.

## Actual record ownership

The existing producer `BB6870` and parser `BB7C50` establish the layouts; these
copy routines agree with `docs/NATIVE_MPAK_PROVIDER.md`:

| Record | Storage |
| --- | --- |
| File, 24h | Counted pooled name at +0/+4; DWORDs +8/+C; flag byte +10; untouched padding +11..+13; STL allocator/debug word +14; offset-vector pointers +18/+1C/+20 |
| Directory, 14h | Counted pooled name at +0/+4; **custom** 0Ch string vector at +8 with data/count/capacity at record +8/+C/+10 |

`BB65A0..BB662F` (144 bytes) is the file record copy constructor:

1. ECX is destination and the sole stack word is source; EAX returns destination,
   RET4. Zero destination name length and pointer **before** testing equality.
2. If destination differs from source, resize the destination name through
   `41DD40(source.length,1)`. Reload source length for the nonempty test, then
   copy **current destination length** bytes from current source pointer to
   current destination pointer at `BB65ED`.
3. Copy source DWORDs +8/+C and its byte +10. Preserve padding and allocator.
4. Arm name cleanup, then call `BB6180` with destination/source +14. That library
   copy constructor owns the offset-vector operation. It first calculates the
   source count, then zeroes destination +4/+8/+C, allocates exactly count,
   performs its returning invalid-parameter checks and `BF67A7` copy, and
   publishes current destination end. Its allocator word is not written.

`BB6630..BB66AC` (125 bytes) is the directory record copy constructor. Its ABI and
name-copy sequence are the same. It then arms name cleanup, explicitly zeroes
the custom member vector's three words, and calls `543E50(destination+8,
source+8)`. This is not the 10h STL vector used for the outer directory array.

Both direct byte copies call the correctly retained `_memcpy` symbol `BF7680`;
the existing provider work established its overlap-safe backward branch. A
future C++ implementation should reuse the established `memmove` behavior.
There is no self-copy repair: the early header zeroing occurs even when the
source equals destination. Preserve current-field reloads across allocation
callbacks and avoid adding a successful no-op self-assignment guard.

The exception metadata is fully bounded for these two constructors:

| Constructor | Handler / FuncInfo / unwind map | Sole state-0 action |
| --- | --- | --- |
| BB65A0 | CC44F8 / DFDE68 / DFDE60 | CC44F0..CC44F7 -> 41DD20, using saved destination |
| BB6630 | CC4518 / DFDE94 / DFDE8C | CC4510..CC4517 -> 41DD20, using saved destination |

Each FuncInfo has one unwind state, successor -1. The name is armed only just
before the nested vector copy. If initial string resizing/copying throws, this
constructor has no armed name cleanup. If the nested copy throws, it releases
only the current destination name through the already actual `41DD20` routine.
It does **not** destroy a partially built nested vector or restore the record.
Do not add that cleanup under a general RAII owner. This establishes the C++
ownership schedule; it is not validation of original FH3/SEH interoperability.

## Shared custom string-vector dependency

`543E50..543E8E` (63 bytes) has ECX destination, one source-pointer stack word,
EAX destination and RET4. It is a shared application helper, with seven callers
in the current graph, not an MPAK-only specialization. It:

1. Calls actual `427110(destination,0)` to release existing names and set count.
2. Reads the source count **after that call**, then calls actual
   `426520(destination,source.count)` to reserve.
3. Iterates with signed index `< current source.count`, reloading source data
   each iteration, and calls actual `4CDC20(destination,&source.data[index])`.
4. Returns destination without a new exception frame or rollback.

All nine call sites across its seven callers were checked. They pass one source
vector pointer and ECX destination: `54454B` copies into an existing owner+0Ch
field; `5F5470/5F5482` copy matching +98h/+A4h fields; `B80CFA` copies 0Ch
elements into new storage; `BB6692`, `BF0C2F`, `BF215B` copy into zeroed embedded
vectors; `BF25D2/BF25F9` copy stack temporaries. The body therefore supports both
replacement of existing contents and construction into an empty header. No
caller adds a different argument form or an archive-specific flag.

All three dependencies already have actual-storage implementations in
`src/native_string_vector.cpp`. The helper should reuse those functions and
`NativeStringVectorStorage`, rather than define another vector type. Self-copy
clears the source count through the same destination before reserve; it is not
a no-op. Existing reserve retains its minimum-one-capacity behavior even for an
empty source. A partial sequence remains if an append fails.

## Flow defects to preserve as pending analysis work

Read-only `ghidra flow` found **six gaps in three STL growth functions**. Disk
decoding at the known call boundaries confirms that `_free` falls through:

| Function | Missing bytes, end exclusive | Recovered continuation |
| --- | --- | --- |
| BB7240 | BB73CF..BB73D2 | ADD ESP,4; publishes replacement vector pointers |
| BB7240 | BB7405..BB7411 | ADD ESP,4; PUSH0; PUSH0; CALL BF6885 (rethrow) |
| BB7240 | BB750E..BB7511 | ADD ESP,4; continues temporary-record cleanup |
| BB7640 | BB77CE..BB77D1 | ADD ESP,4; publishes replacement vector pointers |
| BB7640 | BB7804..BB7810 | ADD ESP,4; PUSH0; PUSH0; CALL BF6885 (rethrow) |
| A40B90 | A40C91..A40C94 | ADD ESP,4; publishes replacement vector pointers |

The consumed file-element destroy helper `BB5E50` has one additional gap at
`BB5E62..BB5E65` after its free call `BB5E5D`: ADD ESP,4 continues into pointer
zeroing and current-name release. This helper is also an external contract;
its pseudocode's apparent immediate return from `_free` is not behavior.

No repair was applied. The primary may lease these functions and use the locked
flow-repair tool before any later full growth/exception audit. `BB65A0`,
`BB6630`, and `543E50` have 47/47/27 listed instructions and zero gaps, so these
library repairs do not block the proposed ownership packet.

## Bounded next implementation packet

Proposed packet: `orch2_native_mpak_record_copy_ba`.

- Own and lease **BB65A0, BB6630, 543E50**; new
  `include/bsp/native_mpak_record_copy.hpp`, `src/native_mpak_record_copy.cpp`,
  `docs/NATIVE_MPAK_RECORD_COPY.md`, `reports/native_mpak_record_copy.json`.
  The primary coordinates the CMake registration and any shared interface edit.
  These addresses and proposed code paths were unleased at discovery time;
  recheck before assigning them.
- Implement the three complete actual-storage schedules above. Reuse the native
  string/vector types and existing `41DD40`, `41DD20`, `427110`, `426520`,
  `4CDC20` implementations. Retain `BB6180` as an explicit, fallible library
  callback with native-vector storage semantics; provide no success stub.
- Keep `BB7A20`, `BB7BA0`, `A40D60`, all STL growth/copy/fill algorithms and CRT
  bodies outside reconstructed coverage. A concrete compatible library binding
  is still required to remove the parser's outer-container contracts. The three
  new game bodies alone do not make archive parsing self-contained.
- Preserve the shared `543E50` replacement semantics and all nine call sites;
  do not give it an MPAK-specific interface or assume a zeroed destination.
  Its caller argument setups were inspected, but the callers themselves were
  not audited as complete routines and remain outside this packet.
- Validate with the ordinary strict Win32 build and existing checks. If a focused
  native comparison is needed, one fixture can cover nonempty deep copy,
  preservation of padding/allocator, current-field reloads, and a nested-copy
  throw showing name-only cleanup. Do not duplicate the existing broad archive
  fixture or count original STL oracle bytes as reconstructed game code.

This packet can run independently of archive read/seek and VFS device work with
the listed files and addresses. Its explicit remaining dependency is the STL
`BB6180` binding; later integration of the new copy callbacks into compatible
outer-container bindings is a separate reviewed step. No build, runtime fixture,
installed archive, ABI compatibility or gameplay claim is made by this discovery.
