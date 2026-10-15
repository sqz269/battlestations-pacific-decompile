# Scene-record map node factory

Addresses: `0046A340`, `0046A3B2`, `00C61C30`.

The whole factory and its Source cleanup/catch behavior bind the existing
complete actual-storage `00B1AD10` implementation. Its raw node constructor
`00B1ACA0` is equivalent to this factory's `00469F50`. Both factories and both
constructors were compared in full, including relative call targets, cleanup
instructions and EH metadata. No new allocator, callback, stub, alternate node
owner, provider edit, or positive-only reconstruction is added.

| Routine | Inclusive body | Bytes / instructions | Coverage | Original ABI |
| --- | --- | --- | --- | --- |
| `0046A340` | `0046A340..0046A3B1` | 114 / 43 | complete at concrete CRT/raw-string/current C++ transport boundaries | five stacked DWORDs; incoming ECX/EDX unused; EAX captured allocation; `RET14` at `0046A3AF` |
| `0046A3B2` | `0046A3B2..0046A3C6` | 21 / 7 | complete Source catch scope through canonical provider; no independent frame entry | EBP-14h current allocation cleanup cell; free and native rethrow |
| `00C61C30` | `00C61C30..00C61C40` | 17 / 7 | complete Source placement-cleanup scope through canonical provider; no independent frame entry | EBP-14h then EBP-18h current reads; `00401130` is RET-only |

This is 152 bytes/57 instructions of native evidence. The one new public
Source function delegates the complete existing raw factory. The two EH
actions are recorded as Source fragments of that composition, not new native
funclet entry points. Existing Ghidra names are retained as descriptive
hypotheses. The handler and metadata remain read-only evidence.

## Allocator and return domain

Native `BF681B` performs `_malloc(size)`, then `__callnewh(size)` on failure.
A nonzero handler result retries allocation; zero constructs and throws the
native bad-alloc object. Handler exceptions propagate. It has no conforming
normal-null return. The existing `singleton_lifetime_allocate` service uses
real `std::malloc`/`_callnewh` with that retry schedule and throws current
`std::bad_alloc` on zero. The request is `{object, native_bytes=1Ch,
host_bytes=1Ch}`. Allocation remains raw and does not initialize node padding.
The native size push at `0046A361` is followed by `ADD ESP,4` at `0046A36A`.

The Source binding retains the native null branch, including no constructor
argument reads when the captured result is null. The current concrete service,
like conforming native `BF681B`, returns usable storage or throws; no null
fallback, ordinary SDK `operator new` substitution, or invented success-only
service is supplied. Allocation failure occurs before cleanup is armed and
therefore causes no free or node/argument read.

Catch target `BF65AC` is the genuine `_free` thunk to `BF9DC8`; the existing
matching `singleton_lifetime_free` executes `std::free` in the same current CRT
domain as allocation. Null is legal; a nonnull cleanup value must be a valid
block of that current domain. Source allocations are not mixed with original
game heap blocks. Original CRT heap/global new-handler/static bad-alloc
object/vtable/exception transport identity is not replaced by these current
services. That qualification applies to the whole binding, including failure.

## Live inputs, construction, cleanup

After the EBP prologue, incoming slots are `+8=left`, `+C=parent`, `+10=right`,
`+14=pair`, `+18=color`. Incoming ECX and EDX are never consumed. Allocate
`1Ch` first, capture EAX in ESI, write allocation cleanup `EBP-14h`, set state0,
write placement cleanup `EBP-18h`, then set state1 before the null branch.
Return ESI at `0046A3A1`, not a mutable cleanup cell or child return value.

For a nonnull captured allocation, the native current-cell sequence is:

1. Read color, pair, right.
2. Push color into its distinct constructor cell; read parent.
3. Push pair; read left.
4. Push right, parent, left; set ECX to the captured allocation.
5. Enter `00469F50` at `0046A398`; its `RET14` consumes exactly five DWORDs.

The existing frame has five LIVE volatile DWORD bindings for incoming values,
five separate constructor cells, and two cleanup cells. The canonical Source
preserves this read/store interleaving after allocation. In particular, a
current new handler can change incoming cells before those reads. A by-value
Source snapshot would not implement this schedule. The child uses actual
`1Ch` node bytes, raw `+C/+10` string header, borrowed `+14` mapped bits,
late low-byte color at `+18`, nil0 at `+19`, and untouched `+1A/+1B` padding.
Its source pair and color accesses retain their established current-field
schedule and concrete raw-pool boundaries.

Handler `00C61C41` selects FuncInfo `00D8932C`: magic `19930522`, max state3,
EH flags1, unwind map `00D89314`, one try block at `00D89300` covering states0
through1, catch-high2, one catch-all handler record at `00D892F0` (flags40h,
null type, no catch object, target `0046A3B2`). The unwind map has no action
for state0; state1 transitions to0 through `00C61C30`; catch state2 transitions
to-1 with no action. Canonical `00B1AD10` has the same structure and scalar
fields, with only its corresponding addresses rebound.

`00C61C30` reads CURRENT allocation cleanup, then CURRENT placement cleanup,
and passes both to `00401130`. That body is a single RET; `ADD ESP,8` at
`00C61C3D` discards the two DWORDs. Source retains the two volatile reads and
adds no release or partial-string destructor. The catch then reads CURRENT
allocation cleanup, frees it at `0046A3B6`, uses `ADD ESP,4` at `0046A3BB`,
and rethrows through `BF6885(0,0)` at `0046A3C2`. The canonical Source catch
uses the matching current-CRT free and `throw;` for current C++ transport.
No child/partial-key destructor, rollback, or additional free is introduced.

The caller provides a fresh default/zero-initialized diagnostic Acquired and
address-stable frame/backing through success/failure disposition. Metadata and
Acquired are disjoint from all native payload, argument and local cells.
Incoming, constructor and cleanup cells have distinct native frame locations.
Payload/current-cell aliases remain admitted. Cross-provider coincidence of
original private-stack addresses, native saved registers, FH3/SEH frame and
exception classification, and resumption are separate from this new ordinary
C++ interface. Acquired records frontiers and raw addresses: if cleanup-cell
mutation selects another valid block, no claim says the captured allocation
was freed. There is no blanket success/cleanup assertion or new safety guard.

## Equivalence, caller and verification

The full saved live-byte comparisons establish equality outside these fields.
The relative offsets in this table are hexadecimal.

| Native / canonical | Bytes | Permitted address fields |
| --- | --- | --- |
| `0046A340` / `00B1AD10` | 114 | handler immediate at relative+6; allocation displacement+24; constructor displacement+59 |
| `00469F50` / `00B1ACA0` | 112 | resize displacement+3A; byte-copy displacement+50 |
| `0046A3B2` / `00B1AD82` | 21 | free displacement+5; rethrow displacement+11 |
| `00C61C30` / `00CBC6D0` | 17 | placement displacement+9 |

Both allocator transfers target `BF681B`; both children target the equivalent
constructors; constructor transfers target `0041DD40`/`BF7680`; both catches
target `BF65AC`/`BF6885`; both placement actions target `00401130`. The local
equivalence receipt records every actual differing offset and decoded target.
The existing provider files are unchanged.

The only live factory caller is `0046AD70..0046AF5B`, with the transfer at
`0046ADF4`. Its excerpt reads pair then head then parent, pushes color0, pair,
head as right, parent, head as left; native `RET14` discards that list. The
caller captures returned EAX after the transfer. This packet reconstructs the
factory, not that caller's later tree linking/rebalancing/count/output work.

Every live batch uses the configured `bsp` project
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, language
`x86:LE:32:default`, image base `00400000`. Prior names/comments/documentation
are saved before locked append/save/export. The report carries actual draft,
build, call-check and compiler receipts. No new tests are added. Build/static
and original-byte comparison evidence are distinct from packet-native
differential execution, original ABI/CRT/FH3 identity, caller adoption and game
validation; none of the latter is claimed.

The worker `scripts/build.ps1` Release run at Source draft
`6723b0946638e7c20fc17eaa188936327770180d` passed all three existing CTests:
`reconstructed_math`, `native_math_differential`, and `tool_tests` (8.10s).
The primary's combined run also passed 3/3 (8.95s) before publication at
`303a0b29256bfe7df267e1648a949a94cd5934ec`. Exact Source/header bytes and
the two existing provider source files match that primary tree. The math
differential suite does not execute this new factory entry. `CL=/MP4` is a
parallel-only build addition.

The actual worker CMake object was inspected with `dumpbin /DISASM /SYMBOLS
/RELOCATIONS`. Its nine-byte `__cdecl` entry emits only `PUSH EBP; MOV EBP,ESP;
POP EBP`, then `E9` at relative+4 to the complete typed canonical factory.
The REL32 operand is at+5. There are no argument-cell reads, value snapshots,
additional guards, or callbacks in this forwarding entry; the three existing
reference bindings pass unchanged to the verified current-cell provider.
Object SHA-256 is `c5c2cc019d5182b3315b5db41bd2ab3036891309385a4770913a16fdd86bba05`.
This is focused Source compiler evidence, not original native COFF/byte/frame
identity or packet-native execution.
