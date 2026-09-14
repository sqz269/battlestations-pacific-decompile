# Canonical mutable CRT data owner proposal CY

A bounded canonical owner is feasible without mapping the full original `.data`
section: reserve **E10000[10000h] and1090000[10000h]**, commit only
**E15000, E16000 and109E000** as writable non-executable4KiB pages, and retain
the existing actual read-only D6 band containing D6E1CC. This requires one
coordinated startup/ownership transaction, not two late independent mappers.
It supplies actual fixed-address storage and cookie initialization, not a full
CRT runtime. No allocation experiment, process launch, implementation, build,
test, source execution or Ghidra mutation was performed here.

Base75379b3383780740f61409c4223004a0fc3fd7d1. All836 owned data bytes and the
external8-byte read-only pair were freshly matched between pinned PE and live
saved Ghidra bytes. These are image-initial/saved-analysis values, not current
running-game state. CT8aef5c8752e7f30bc9c8df44e4d50a6a7077f692 and its complete
332-artifact seal were verified twice and remain unchanged.

## Exact section and ownership boundaries

| Item | Half-open virtual range / storage | Initial bytes or ownership |
|---|---|---|
| Original `.rdata` | CE2000..E07B24 | Raw8E2000[126000h], flags40000040h; virtual125B24h |
| Original `.data` | E08000..109FEDC | RawA08000[10000h], flagsC0000040h; virtual297EDCh |
| Initialized `.data` | E08000..E18000 | Copy verified file bytes only |
| Virtual zero-fill | E18000..109FEDC | No file bytes; explicit loader zero-fill |
| Existing RO allowed bands | CE0000..E10000 | 19 supported64KiB bands, selected by mask |
| Current game RO request | CF0000,D10000,D50000,D60000 | Four reservations; mask18Ah |
| Cookie pair | E15590[8], pageE15000/bandE10000 | Little-endian words BB40E64E,44BF19B1 |
| NLG descriptor | E16830[10h], pageE16000/bandE10000 | Words19930520,0,0,0 |
| Shared failure block | 109E568[324h], page109E000/band1090000 | All804 bytes zero-fill |
| Feature/hook | 109EEA4[8], same109E000 page | Both DWORDs zero-fill |
| Actual exception pair | D6E1CC[8], pageD6E000/bandD60000 | Words109E568,109E5C0; current RO page |

The failure block comprises record109E568[50h], debugger109E5B8[4], untouched
padding109E5BC[4], and CONTEXT109E5C0[2CCh], ending109E88C. It and both feature/
hook words share one page; independent per-object reservations are impossible.
Only the cookie page and NLG page require initialized file copies (offsets
A15000 andA16000). The complete109E000 page is within the virtual zero-fill.
Keep the full selected-page image bytes; advertise only the recovered cell
contracts. Other copied pointer/TLS/global bytes do not acquire callable or
initialized runtime ownership merely by becoming readable.

The RO mapper's last possible reservation is **E00000..E10000**, not E10000.
Its section bytes are committed/protected only through E08000; `.rdata` padding
E07B24..E08000 remains zero. Thus `.data` starts on a different4KiB page but
inside that same64KiB reservation. A new full-section VirtualAlloc reservation
starting atE08000 would collide with/round into the existing E00000 reservation.
Its lifetime could not be independent because the RO destructor releases the
whole band. Full `.data` spans42 bands; union with all19 RO bands is60 bands,
exceeding the current32-bit mask. Mapping the whole section also publishes
unestablished heap/TLS/lock/runtime state. It is unnecessary for this packet.

## Existing real owner and bootstrap

`GameNativeReadOnlyData` owns selected bands in its19-pointer array. It reserves
before allocating its large file buffer, checks exact original size/SHA256,
commits the section-covered pages RW, copies original bytes, then protects them
READONLY. `data_at` checks section bounds and owned bands. It resolves no imports
and executes no original code. The direct constructor rejects occupied bands;
the handoff constructor adopts only `GameNativeDataReservation` capability.
Failure after transfer releases its owned bands and marks the handoff failed;
normal destruction releases every owned band with MEM_RELEASE.

`GameNativeDataReservation` is move-only. Its private transfer verifies expected
mask and exact64KiB MEM_RESERVE/PAGE_NOACCESS/MEM_PRIVATE allocation identity,
then clears its transferred pointers. Its destructor releases only still-owned
exact reservations and reports failed if no successful completion occurred.
Its current transfer and allowed mask accept only the19 RO bands.

`GameNativeDataBootstrapChild` creates a suspended copy of the rebuilt game,
using an inherited page-file mapping and a restricted one-handle inheritance
list. Version1 record contains magic, version, parent/child IDs, mask and atomic
state. Parent transitions pending->ready after exact VirtualAllocEx reservations
and before ResumeThread. Child checks inherited handle, magic/version, its PID,
different parent PID, exact mask, ready state and all reservation identities;
CAS ready->claimed consumes once. This is the current controlled-child protocol,
not an assertion of broader authentication. No foreign allocation is adopted.

Mapper completion currently CASes claimed->mapped and closes the child's mapping
view/handle. A timed-out parent's state cannot be overwritten by late success.
The parent polls for mapped/failed/exit with a final state recheck on fast exit;
timeout races are settled by CAS. Failure/timeout terminates only its owned
child; process teardown reclaims transferred pages. Resume failure frees only
verified owned reservations and terminates that child. No retry, foreign free,
stack relocation or Windows mitigation change is used.

`game_main` parent verifies the original image then waits30s for mapper readiness
and subsequently waits for child exit. The child accepts handoff before creating
the mapper, then creates `GameStartupHost`. Host destruction retains dependencies
through the actual singleton drain and precedes local RO mapper destruction.
This is sufficient for the documented current raw table users. It is not proof
that all later CRT/atexit/thread teardown can stop using a canonical cookie,
NLG descriptor or failure pair before `main` returns.

The current link policy applies only to Win32 `bsp_game`: /BASE:0x10000000,
/DYNAMICBASE:NO, /FIXED:NO, with relocation data retained. Read-only inspection
of the frozen CT build found image base10000000, image size212000h, DLL flags
8100h, COFF0102h and72544 relocation bytes. Its full link command and PE are
retained. No new launch was performed; that layout is not proof that E10000 or
1090000 is available under every loader/system policy. A collision must remain
a guarded startup failure, without changing current policy or freeing its owner.

## Minimum coherent integration packet

1. Add `include/bsp/game_native_mutable_crt_data.hpp` and
   `src/game_native_mutable_crt_data.cpp` for a noncopyable process owner with a
   private bootstrap capability constructor. It owns the two new reservations,
   commits only the three exact pages, loads initial bytes from the same verified
   PE, and exposes typed borrowed access only to the recovered actual cells.
   It neither accepts arbitrary pre-existing mappings nor manufactures TLS,
   heap, lock or callback state. Zero-fill is image initialization once, not a
   reset on each user/call or a synthetic feature-policy default.
2. Extend `game_native_data_bootstrap.hpp/.cpp` with a version2 explicit plan:
   the existing19 RO slots plus sparse slots19=E10000 and20=1090000, or separate
   equivalent typed RO/RW subsets. A21-bit mask fits; addresses beyondE00000
   cannot use the old `first_band+i*10000h` formula. Include and validate the
   exact mutable page/role plan as well as the band set on both sides. The
   current game union is six bands, mask18018Ah under this ordered slot plan.
   Preserve restricted inheritance, state/identity checks and owned rollback.
3. Introduce one joint coordinator that consumes the handoff once and transfers
   disjoint capabilities to RO and mutable owners. Refactor the RO adoption path
   so its successful initialization does **not** prematurely publish mapped.
   `GameNativeReadOnlyData::initialize` currently calls finish(true); a public
   sequence of existing constructors would ACK too early. Keep the joint ACK
   handle until all RO/RW pages, protections, hashes and pair relationships are
   established and actual cookie initialization has completed. Any intermediate
   failure rolls back both transferred subsets; never double-release a band.
   Transfer ownership before committing pages. The capability's original
   exact-MEM_RESERVE predicate will cease to hold once a band contains committed
   subregions; the established page owner must then release its own allocation
   base, rather than mistake that expected split for an unowned reservation.
   Check the ACK CAS result before exposing native-callable consumers: the old
   `finish(true)` discards that result. A cancelled parent must prevent local
   publication, not just prevent changing its failed state. Allocate ownership
   metadata before ACK so the final successful retention/publication steps are
   nonthrowing; do not leave an allocation failure after mapped was reported.
4. Call the existing complete `initialize_native_crt_security_cookie_00c1815e`
   with references to actualE15590/E15594 before admitting any frame/check/NLG
   consumer. Preserve its real Win32 entropy calls, uninitialized QPC output
   contract and cookie-before-complement publication. No invented process-cookie
   value or host compiler cookie is substituted. Publish only scoped mapping/
   cookie readiness, not full-CRT readiness; the SSE feature detector/bootstrap,
   TLS/PTD, heap, locks and native exception registrations remain separate.
5. Update `game_main.cpp` and its startup lifetime aggregate to request the joint
   plan and retain both owners **through every possible native-capable consumer**.
   Minimum safe successful lifetime is process lifetime with OS reclamation:
   make the ownership transfer explicit in a non-destructed process owner after
   publication, retaining the RO object/D6 pair as well as RW bands. Keep RAII
   rollback before successful publication. Do not merely let a local pointer
   release RW pages on host shutdown or assume an atexit ordering. A reclaiming
   variant requires separately proving all threads, registered EH frames,
   callbacks and exit consumers drained; no such proof exists here. This is a
   deliberate process-owner policy, not a borrowed view or forgotten allocation.

Retain the present read-only API/protections and ordinary RO-only ownership path
for other clients. The joint owner must be unique per process, initialized under
controlled startup before exposing consumers, and refuse duplicate replacement
or reinitialization of live state. No active cookie may change while frames
encoded with it exist. Source lifetime-control metadata is distinct from the
actual canonical CRT words at their original numeric addresses.

## Native callable provider follow-up

The owner permits a complete original-entry BFE120 variant with actual ECX cookie,
no added stack argument, unchanged EAX, original CMP/F3C3 and a direct relocated
tail to a complete no-argument failure reporter using the fixed canonical cells.
That reporter can preserve its original capture frame and register/flags stores
without CT's context/scratch shim. Reuse actual CL and two-argument hook APIs with
qualified internal argument plumbing after capture, and real Win32 imports.
Preserve every return tail. Original code addresses are not mapped or executed:
the source caller's actual relocation must target these compiled entries.

CT's complete context-bearing checker/reporter remains a separate usable source
interface. Pushing its context in a wrapper does not close C0DC54's unchanged
no-argument edge or original mismatch capture frame. A new complete native-entry
variant is needed; not a stub, numeric jump into the installed game, synthetic
failure callback or C++ security-check replacement. C0DC54 must still preserve
EAX=R at its C0DC6F checker call so C0DC75 can load [R+18h].

Accepted BS evidence already covers complete C16879 `__NLG_Notify`[31]: EAX is
destination, EBP actual frame, stacked code; writes descriptor+8 code, +4 EAX,
+Ch EBP, preserving signature+0. It preserves EBX/ECX/EAX/EBP and flags, retains
the debugger-visible push/pop sequence and RET4. Fixed canonical E16830 makes
this complete instruction source feasible without a substitute descriptor or
extra argument. Fresh data xrefs also identify C16870 `__NLG_Notify1`; any packet
implementing that additional entry must retain its full exact span/common-tail
contract separately. It was not newly reconstructed here. C0DCCD handler transfer
and C0DBC4 local unwind remain actual native-frame consumers, not generic C++
callbacks. The owner does not itself establish their FS/SEH/cleanup domains.

Thus the next useful packet is the joint two-band/three-page owner and bootstrap
integration, followed by the complete native-entry checker/reporter and31-byte
NLG provider with actual source relocations. The existing five API imports,
cookie initializer, CL fill and hook are concrete source dependencies; actual
TLS/PTD/heap/locks and feature initialization are not silently declared complete.
The report retains exact data/page hashes, source snapshots, current placement,
provider availability and the whole twice-hashed local evidence inventory.
The bounded live xref samples are not exhaustive writer/reader inventories;
the selected page bytes outside the four requested data spans have no new CRT
service contract. Existing accepted BS and CT evidence supplies the stated
provider bodies; this packet does not claim new ownership of their code.
