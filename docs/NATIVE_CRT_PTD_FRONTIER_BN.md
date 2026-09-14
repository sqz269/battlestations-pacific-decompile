# PTD initialization and locale reference ownership

The next smallest source-ready primitive is the complete 134-byte
`___addlocaleref` at `00C0190B`. It acquires references through the real Win32
`InterlockedIncrement` import, with no allocator, lock, errno callback or private
locale state. It requires the actual readable locale record, its writable
reference-count targets, and the actual `00E161D0` sentinel address. This is a
qualified source boundary; it does not create or own the PTD/locale domain.

The complete `C050F8` PTD initializer is not independently closed. It needs
actual static tables, current locale ownership, lock 12, the existing qualified
pointer gate, and the original SEH4 frame/cleanup domain. Splitting its early
stores into a purported complete initializer would hide those dependencies.

Discovery only, based on `45980f8a3a0512d10cf4f874fff2fde1eb59ee5e`.
The accepted Watson worktree at `ed185cb2dfdf1ba045dbf30ebfd47f083eeece80`
and cookie source worktree at `47260b0cd08e9474bfd42a5e0f054fecbdbbd4c9`
remain unchanged. No C++, tests, runtime execution, Ghidra mutation or names
were added. Exact evidence and hashes accompany
`reports/native_crt_ptd_frontier_bn.json`.

## Complete bounded bodies

| Entry | Bytes | Native contract |
| --- | ---: | --- |
| `C0190B` | 134 | One cdecl pointer; EBX/EBP/ESI/EDI saved; plain RET |
| `C050F8` | 182 | PTD at EBP+8 and optional locale at EBP+Ch after SEH4 prolog; plain RET |
| `C051AE` | 9 | No arguments; call indexed unlock for lock 12; plain RET |
| `C11A93` | 73 | No arguments; EAX 0/1; ESI/EDI saved; two return paths |
| `C11B31` | 21 | One cdecl lock index; real LeaveCriticalSection on table entry |
| `C11B5E` | 186 | Lock index at EBP+8 after SEH4 prolog; EAX 0/1, plain RET |
| `C11C18` | 9 | No arguments; call indexed unlock for lock 10; plain RET |
| `C11C21` | 49 | One cdecl lock index; EBP/ESI saved; actual EnterCriticalSection |

These eight newly retained complete bodies total 663 bytes. Three existing
complete PE/live spans are reused with explicit provenance: `C051B7` (119),
`C05070` (50), and `BFFB8B` (19). The external `__mtinit` publication excerpt
at `C05484..C0555F` is not represented as a complete ninth helper.

## PTD publication and exact initialization order

`BFFB8B` obtains `C051B7`'s PTD and returns PTD+8, or the canonical fallback
word `E159E8` if null. It does not establish a new errno object. `C051B7` saves
LastError, calls the actual getter supplied through `C05070`, and, if necessary,
uses `C0485A(1,214h)` before publishing through the decoded actual setter.
Only successful setter publication is followed by `C050F8(PTD,0)`. Initialization
therefore occurs after the actual getter can already find that same PTD.

`C050F8` performs this exact sequence:

1. Call `GetModuleHandleA("KERNEL32.DLL")`; store `E16840` at PTD+5Ch and
   DWORD 1 at PTD+14h.
2. If the module is nonnull and the actual `C04EFB` gate returns nonzero,
   resolve `EncodePointer` then `DecodePointer`, publishing their actual
   GetProcAddress results at PTD+1F8h then +1FCh. A null result is still stored.
   Skipped branches leave those fields unchanged; this body does not clear them.
3. Store DWORD 1 at +70h, byte 43h at +C8h, and byte 43h at +14Bh.
4. Publish the actual static mbcinfo address `E15CA8` at +68h, then atomically
   increment the DWORD at `E15CA8` through the real Win32 import.
5. Acquire lock 12 through `C11C21`; only after return set the SEH try level to 0.
6. Store the second argument at +6Ch, including an intermediate null store.
   If null, reload the current `E162B0` word and publish that pointer at +6Ch.
7. Call `C0190B` with the current +6Ch pointer. Mark the try level inactive,
   run `C051AE` to release lock 12, and use the SEH4 epilog.

The scope table `E032B0` contains one finally entry targeting `C051AE`.
Already published PTD fields and acquired references are not rolled back by
this finally; it releases the lock. Native exceptions/faults and native SEH4
continuations remain a separate frame domain, not an invented C++ catch policy.

After initialization, `C051B7` obtains the thread ID, ORs PTD+4 with FFFFFFFF,
then stores the thread ID at PTD+0. Every returning path restores saved
LastError. Setter failure frees the allocation and executes the retained
`C0521F POP ECX; C05220 XOR ESI,ESI` before restoring LastError and returning
null. These three bytes remain missing from the live listing; they are present
in the accepted complete PE/live span. Exceptional exits have no new restoration
or allocation rollback policy supplied here.

## Reference-acquisition primitive and alias requirements

`C0190B` first increments the DWORD at the supplied locale address. It then
loads and, when nonnull, increments count pointers at +B0h, +B8h, +B4h and +C0h,
in that order. It does not sort the fields by offset.

For six iterations, let q = 50h + 10h*i. If the pointer at q-8 differs from the
actual address `E161D0`, a nonnull count pointer at q is incremented. Then, if
the pointer at q-4 is nonnull, a nonnull count pointer at q+4 is incremented.
Finally, it loads the pointer at +D4h and unconditionally increments its DWORD
at +B4h. Neither the root pointer nor the final time-locale pointer has a null
guard. All calls use the cached actual `InterlockedIncrement` import in EDI.

Pointer aliases are significant. Repeated count targets receive repeated
increments; later pointer fields are loaded after preceding increments. A
snapshot of all fields, pointer deduplication, string-content comparison against
"C", or private substitute count storage would change the native behavior.
The body adds no lock, release, overflow repair or error translation.

The PE/live database initial state has `E162B0 -> E161D8`. That locale record
starts with count 1; the four optional count pointers and category count pointers
are zero. Categories 1 through 5 point to the exact `E161D0` sentinel; category
0's pointer is zero. The time-locale pointer at +D4h is `E159F0`, whose count at
`E15AA4` is zero. The static mbcinfo count at `E15CA8` is zero. These are initial
image/database values, not observations of a running game's current counts.

## Actual lock ownership and remaining dependencies

The table at `E16478` has 36 entries of two DWORDs: critical-section pointer,
then type. `C11A93` examines type exactly equal to 1 and assigns successive
18h-byte objects from the actual `109E1C0` static pool. It publishes each
pointer before calling `C17653(pointer,4000)`. Failure clears only the current
pointer and returns 0; it does not undo earlier initialized entries.

There are 14 static entries: 0,1,3,4,6,7,8,10,12,13,14,16,17,18. Successful
initialization binds lock 10's pointer word `E164C8` to `109E268`, and lock 12's
word `E164D8` to `109E280`. The retained complete pool is 150h bytes, initially
zero. The `__mtinit` caller at `C054E8` invokes this initializer before it
allocates/publishes the initial PTD and calls `C050F8`. These addresses require
successful actual startup; table flags alone do not establish initialized locks.

`C11C21` lazily calls `C11B5E` for a null pointer, calls `BFBA09(11h)` on failure,
and then enters the actual selected critical section. It adds no range check or
new termination if that error provider returns. `C11B31` loads the same indexed
pointer and passes it to real LeaveCriticalSection, without a null guard.

`C11B5E` requires the original heap word `109E1BC`. Its missing-heap path calls
`C05B68`, `C059A8(1Eh)` and `BFBA53(FFh)`. Otherwise it allocates 18h bytes via
`C0481A`, acquires lock 10, rechecks the table, and either frees a race-losing
allocation or calls actual `C17653`. It publishes only on successful initialization.
Failed allocation/initialization uses actual `BFFB8B` and stores errno 12.
Scope table `E03678` targets `C11C18`, which releases lock 10.

The live listing omits `C11BE7..C11BF7`: POP ECX, CALL BFFB8B, store errno 12,
set the saved result to zero, and jump to the common finally path. It also omits
the race-loser POP ECX at `C11C02`. Full live/PE bytes and raw disassembly retain
both. The missing CALL at `C11BE8` is not counted as a listing-verifier success.

## Concrete source availability and recommendation

Current source supplies the qualified full `C04EFB` module gate and `C04FDE`
pointer decoder. Their contexts still borrow actual OS words and owning errno,
invalid-parameter and TLS state. `LegacyCrtMathRuntime` exposes a caller-bound
errno accessor; that adapter does not implement the original `BFFB8B` PTD owner.
No reconstruction record or exact source body exists at this base for the eight
new helpers, `C051B7`, `C05070`, `C0481A`, `C0485A`, or `C17653`.

Implementing `C0190B` alone is the smallest complete next packet. `C11B31` is
also a small source-ready leaf with a borrowed actual lock table; its 9-byte
finally callers can compose it but do not establish native SEH ownership.
The full PTD path still needs real `C17653` lock initialization, native
calloc/free and failure ownership, canonical TLS/FLS words/index publication,
the `C05246` registered cleanup owner, and actual shared locale/exception-action
storage. Do not substitute host allocation, a new errno variable, or generic
callbacks to declare that path complete.

All 135 baseline artifacts were checked before reuse. Twenty-two newly retained
code/data spans match the installed PE and current target-verified Ghidra bytes.
`verify_report_calls.py` validates 33 direct rows with zero failures; one omitted
listing call is separately byte-qualified, and 21 indirect sites are separately
qualified through actual imports or retained getter/setter provenance. The
whole packet-local directory has a two-pass SHA256/SHA512 inventory. No source,
build, ABI, game, or runtime parity advancement is claimed by this discovery.
