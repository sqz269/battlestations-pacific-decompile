# Raw reader allocation boundary (BS discovery)

This packet establishes the actual allocator/free wrappers reached by the reader's 12-byte storage routines. It does not implement them. `BF55BE -> BF681B -> BF9F1A` is a native CRT allocation path with heap-mode selection and two levels of new-handler policy. `BF6989 -> BF65AC -> BF9DC8` is its native release path, including small-block classification, lock ownership and errno publication. The existing shared source allocator is an explicit host CRT service; it is not evidence that these native owners have been reconstructed.

Base: `e9c7792df0bc8a213b6b13c88c206575f2b1eb4d`. Worktree: `orch2-raw-reader-allocation-bs`. Live CLI batches verified project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. The installed PE SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The report contains every span, direct/tail row, indirect-site qualification, source hash and complete double-hashed local inventory.

## Evidence and inherited boundaries

There are 23 fresh PE/live-equal spans: 19 code spans covering 1,020 unique bytes / 362 instructions, plus three 28-byte scope tables and the 360-byte errno map. The nine-byte `BF9E1E` finally is also contained in the 142-byte free span and is counted once in unique coverage. All 52 direct/tail rows (49 CALLs and three thunks) have independently checked REL32 targets; seven indirect sites are qualified separately. One direct CALL is absent from the current stored listing.

Corrected storage discovery `915cf0d3c52dd0fa83fa5f5b47d365c5a8ef4780` is retained without altering its 753 artifacts. Its report SHA256 is `e515eef3c57a10066cd31d9be704d7f2256f9c1fd9b198a3faa2956c326da180`. Its original and correction evidence remain separate. PTD discovery `f56cf12b4992af1137fa3ab49685de02ce6171f3` is retained from the parent's immutable delivery: 129 artifacts and report SHA256 `fd41b5d16ed432590059212f216675a0a36bd57f1b54e97fb65af717d3219823`. Lock initializer discovery `01fc5c40538266cd29f1ab33e479401c70603be1` contributes its 80 verified artifacts. No whole PTD, SEH4 frame, C++ throw or small-block allocator reconstruction is repeated here.

## ABI and complete physical spans

| Entry | Bytes | Original interface and role |
| --- | ---: | --- |
| `BF55BE` | 5 | Tail JMP to `BF681B`; original size at entry ESP+4, EAX pointer on normal return. |
| `BF681B` | 105 | cdecl operator-new body; one unsigned 32-bit byte size; EAX storage or native exception path. |
| `BF6989` / `BF65AC` | 5 each | Tail JMP to `BF65AC` / `BF9DC8`; original pointer at ESP+4. |
| `BF9F1A` | 195 | cdecl malloc; unsigned size; EAX pointer or zero. Captures original size in EBP. |
| `BF9DC8` | 142 | cdecl free; one pointer; no semantic return value. Physical span includes `BF9E1E`. |
| `BF9E56` / `BF9E9C` | 70 / 9 | cdecl V6 small-block allocation wrapper and lock-4 finally. |
| `C055B1` | 34 | cdecl new-handler dispatcher; one original-size argument; EAX normalized to 0 or 1. |
| `BFFB8B` / `BFFB50` | 19 / 59 | cdecl errno-location accessor; cdecl unsigned Win32-error to errno mapper. |
| `C11C21` / `C11B31` | 49 / 21 | cdecl indexed lock / unlock using the actual table at `E16478`. |
| `C11B5E` / `C11C18` | 186 / 9 | cdecl lazy lock initialization; EAX Boolean; lock-10 finally. |
| `C11D3D` + `C11D4F` | 18 + 25 | One physical 43-byte cdecl descriptor search, despite split saved function ownership. Pointer argument at ESP+4; EAX descriptor or zero. |
| `C0481A` | 64 | cdecl malloc-with-current-time-budget retry; EAX pointer or zero. |

The source names remain descriptive hypotheses. These ABI facts do not establish a drop-in native replacement or asynchronous-fault identity.

## Allocation, retries and failure

`BF681B` first calls actual `BF9F1A(size)`. A null return selects `C055B1(size)`; a nonzero handler result retries malloc. A zero result selects the original cached `bad_alloc` construction path: test bit 0 of `0109DD74`, set that bit before `BF6802(0109DD68)`, register `CE1122` through `BF6FF5`, copy through `BF63A6` into its stack object, write vptr `D6923C`, and call `BF6885` with throw metadata `E03CC0`. The native constructor/copy, atexit owner, destructor action, C++ throw metadata and handler identity remain external. A host `throw std::bad_alloc()` is a source policy, not proof of that path. There is no invented initialization rollback or synchronization around the observed guard.

`C055B1` reads the current encoded handler word `0109DE44` and calls actual `C04FDE`. A null decoded result returns zero; otherwise CALL EAX at `C055C5` passes the original size and normalizes the callback result to Boolean. The body has no lock and does not catch a thrown callback exception. The existing `native_crt_decode_pointer_00c04fde` source is usable only with its actual borrowed TLS/PTD/module context. It does not provide the original handler registration owner or replace the current encoded word with a private callback.

`BF9F1A` compares the captured size unsigned against `FFFFFFE0`. An oversized request calls the handler once, ignores its normal Boolean result, obtains the actual errno location, stores 12 and returns zero. `BF681B` can consequently call the handler again. For admissible sizes, the HeapAlloc IAT at `CE20F8` is captured in EBX once before the retry loop; each iteration rechecks the current heap `0109E1BC` and heap mode `0109ED7C`.

- Mode 1 calls the captured HeapAlloc with the current heap, flags 0, and size unchanged except that zero becomes one.
- Mode 3 first calls `BF9E56(original_size)`. A nonnull result completes malloc. Its null result takes the ordinary-heap fallback.
- Other modes, and the mode-3 fallback, use `((size != 0 ? size : 1) + 15) & FFFFFFF0` as the HeapAlloc size.

On allocation failure, current `0109E314` selects the inner new-handler policy. Nonzero calls `C055B1(original_size)`; a nonzero result retries from `BF9F31`, rereading current heap/mode. A zero handler result stores errno 12 once. Zero new-mode calls `BFFB8B` and stores 12 twice, with a fresh accessor each time. These calls can create PTD and are not safely collapsed into a cached host errno pointer. The result remains zero after these normal failure paths.

A missing heap calls `C05B68`, `C059A8(1Eh)`, then `BFBA53(FFh)`. Physical continuation exists after the last call; this packet does not infer exact current no-return metadata or substitute a private abort. The original startup heap owner remains required.

## Small-block and release ownership

`BF9E56` compares size unsigned with current threshold `0109ED6C`. An out-of-range size returns zero. Otherwise it calls lock 4 before arming state 0, calls actual `C12968(size)`, saves the result, disarms to -2, calls finally `BF9E9C` to unlock 4, and returns the saved pointer. Scope table `E02E10` binds that finally. `C12968` is the named 739-byte small-block allocator; its region/group providers are not reconstructed by this packet.

`BF9DC8` sets up its actual SEH4 frame even for null, but null selects the epilog before reading heap mode or acquiring a lock. A nonnull pointer in mode 3 locks 4, then arms state 0 and calls the actual descriptor search. That search captures count `0109ED64` and table `0109ED68`, forms a wrapped count-times-20 end, and walks descriptors in 20-byte strides. A descriptor matches when unsigned `pointer - descriptor[+0Ch] < 100000h`. It does not establish the remaining small-block validity/free contract. A match calls the named 788-byte `C11D68(descriptor,pointer)`; this packet leaves its bitmap, region, group and virtual-memory behavior unresolved.

The free wrapper disarms to -2 before its normal call to `BF9E1E` (unlock 4), with `E02DF0` binding the exceptional finally. A found descriptor then completes free. If no descriptor matched, it reloads the original stack argument before ordinary heap release; mode other than 3 instead uses the pointer captured in ESI. Both paths push flags 0 and the current `0109E1BC`, then call the actual HeapFree IAT `CE20FC`.

HeapFree success does not publish errno. Failure first calls `BFFB8B`, captures that returned slot, then calls GetLastError, maps it through `BFFB50`, and writes the mapped errno to the captured slot. Reused PTD evidence is essential: `C051B7` is a **creating** accessor and restores captured LastError on every normal return. It allocates 214h, publishes through the actual TLS setter before `C050F8`, and frees on setter failure. It supplies no added restoration or rollback on exceptional exits. `BFFB8B` returns PTD+8 or actual fallback word `E159E8`. Thus the intermediate accessor does not normally erase the HeapFree error; replacing it with an arbitrary errno callback would need an explicit additional contract.

`BFFB50` performs first-match lookup in the 45 native `(Win32 error, errno)` pairs at `E15880`. If absent, unsigned errors 13h..24h map to 13, BCh..CAh to 8, and all others to 22. The exact table is retained in binary and decoded report form.

## Lock initialization and its exceptional limits

`C11C21(index)` uses the pointer at `E16478 + 8*index`. If null, it calls `C11B5E(index)`; zero calls `BFBA09(11h)`. If that error provider returns, the physical continuation still reloads the current table pointer and calls EnterCriticalSection (`CE2218`). `C11B31` similarly reloads the table pointer and calls LeaveCriticalSection (`CE2210`), without a private lock or null check.

`C11B5E` first checks the actual CRT heap, computes the indexed table slot, and compares its current pointer at `C11B9C`. If already nonnull, `C11BA0` selects EAX=1 and jumps to the epilog before allocation or lock 10. Only a null slot allocates a 24-byte CRITICAL_SECTION through `C0481A(18h)` before locking 10. Null allocation obtains actual errno, writes 12, and returns zero. It arms state 0 only after lock 10 returns, then rereads the indexed pointer. If another publisher won, it frees the new candidate. Otherwise CALL `C17653` at `C11BD6` receives the candidate and **0FA0h (4000 decimal)**. Success publishes the pointer at `C11BF8`. A normal zero result frees the candidate, calls `BFFB8B` at the omitted `C11BE8`, stores errno 12, and records failure. Both paths disarm to -2 before normal finally `C11C18`; scope `E03678` binds unlock 10. An exception while armed invokes that unlock, not an invented candidate free, errno write or publication rollback.

Reused C17653 evidence distinguishes its exact exception policy: decoding/platform selection/API lookup/encoding/cache publication occur outside its try. Its filter handles only `C0000017`; its handler may set LastError(8) and returns zero. Other exceptions continue search. It preserves the real API's normal result and does not clear the cache on API failure. Its full source and native cache owner remain incomplete. The complete C17643 fallback source alone cannot replace this wrapper.

`C0481A` repeatedly calls actual malloc. After a null result, current `0109DE30 == 0` ends the operation. Otherwise it calls Sleep with the current delay (initially zero), adds 1000 modulo 32 bits, rereads the current time budget, substitutes `FFFFFFFF` if the new delay is too large, and stops on that sentinel. It does not call Sleep(INFINITE) in the sentinel path. This retry layer is separate from malloc's inner and operator-new's outer handler loops.

## Stored listing omissions; no repair

`C11D4F..C11D60` contains the 18-byte descriptor loop. The saved tail function lists only `C11D61..67`, and the public entry jumps into that tail. The omitted load/subtract/unsigned comparison/found branch/stride are indispensable. They are not a recursive C++ function inferred from pseudocode. The omitted region contains no CALL.

Lazy lock initialization omits `C11BE7..C11BF7` (17 bytes) after free at `C11BE2`, and `C11C02` (one POP ECX) after free at `C11BFD`. The first gap contains the direct CALL to `BFFB8B`, its errno store, failure-result store and branch to disarm/unlock. The full physical body is 186 bytes / 62 instructions versus 56 stored instructions. No no-return flag or flow override is guessed. The retained BR disabled-script result leaves those exact properties inaccessible through the supported read-only bridge. A future repair must inspect current instruction/function properties and old values under an explicit write lease before deciding any changes; this packet makes none.

The original `BF05D0` gap remains the retained ten bytes after `BF6989`: stack cleanup, new-buffer publication, saved-register pops, and capacity publication. Native free demonstrably has normal returns, but that does not by itself identify which current Ghidra property caused the missing continuation.

## Consequences for the complete storage source packet

The 298-byte reserve and 125-byte resize contracts remain those of corrected BR. Reserve performs wrapped 12-byte multiplication with signed capacity decisions and no overflow check. Native malloc's unsigned size ceiling does **not** detect earlier multiplication wrap: positive requests such as `15555556h` and `40000000h` can reach it as 8 and 0 bytes. A valid source storage domain therefore needs representable backing extents sufficient for every actual 12-byte access; it must not silently add native overflow rejection.

The source must preserve disjoint string-header writes, raw third-DWORD copying, current count/buffer reads, actual raw string resize/getter/pool-return calls, and release-before-new-buffer/capacity publication. In particular the corrected copy-loop order is increment at `BF0684`, current-count comparison at `BF0687` while state 0 is armed, disarm at `BF068A`, completed-count publication at `BF0692`, then `JL` consuming the earlier comparison flags. Retained `CC7790 -> 401130` is a no-op cleanup provider: there is no prefix destruction, candidate free or rollback to insert. Old-string release and old-buffer free happen after the reserve state is disarmed. Provider exceptions preserve their already-visible changes and prevent subsequent publications. No transactional vector replacement is faithful to this schedule.

Current real source services are `resize_native_string_header_0041dd40` with `NativeStringRawPoolContext`, `native_string_pool_get_or_create_00419cc0`, and `return_native_string_pool_00bd1510`. The latter is noexcept; the getter remains independently required before each nonnull return and can throw. Use the raw context rather than a noexcept release adapter. The established BF7680 source policy admits overlap through nonzero memmove and qualifies omitted zero-byte call boundaries.

`singleton_lifetime_allocate({object,native_bytes,host_bytes})` calls host `std::malloc`, host `_callnewh` on failure, and host `std::bad_alloc` on refusal. `singleton_lifetime_free` calls host `std::free` and is noexcept. These preserve a useful matched **host allocation-domain policy** for source-owned storage, including an explicit allocation retry/throw boundary. They do not supply native heap globals/modes, native new-mode or encoded handler ownership, double errno-access schedule, PTD startup, native small-block allocation/free, original LastError/error mapping, lock-table ownership, or cached native throw metadata. Their host CRT may itself have allocation policies; equal requested byte counts do not prove equal callback counts or failures.

A future full reserve/resize implementation can use that already-established host service only if its contract explicitly adopts the matched host domain and qualifies those native differences, while retaining the complete reader/string schedule and actual providers. It must not be reported as original-heap or native-exception closure. Exact native allocator closure remains blocked by C12968/C11D68 and their region/group providers, original heap/bootstrap state, PTD/TLS/locale ownership, complete C17653/static lock startup, native error termination and cached bad_alloc/atexit/throw owners. The pointer decoder source is a ready dependency with borrowed-state requirements, not that whole ownership chain. This discovery creates no allocation callback, heap substitute, vector substitute, source fragment or runtime claim.

Validation is static PE/live and source-contract inspection only. There were no source edits, builds, tests, probes, game execution, Ghidra renames/repairs/saves, merges or pushes. The constructor's prior valid writable caller-owned 70h/nonfaulting domain is unchanged; FH3, SEH4 frame execution, asynchronous faults and gameplay remain separately unclaimed.
