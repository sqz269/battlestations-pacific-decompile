# Native hardware-layout pool allocation

`src/native_hardware_layout_pool_allocate.cpp` implements four complete native
storage operations: slab initialization `00b5ff50`, raw-slot allocation
`00b605b0`, the canonical-pool allocation wrapper `00b606f0`, and the
constructor-failure raw-slot return wrapper `00b60260`. The last composes the
existing complete `00b60110`; it does not destruct an owner.

The functions borrow the actual initialized 38h pool normally at `0108fe9c`.
They create no substitute pool, stored dispatch callback, registry, or shadow
free list. The current pool contains a real Win32 critical section at `+0Ch`,
depth DWORD at `+24h`, table pointer/count/capacity at `+28h/+2Ch/+30h`, and
earliest available slab at `+34h`. Both source and interface target MSVC Win32.

| Native entry | Original ABI | Implemented extent |
| --- | --- | --- |
| `00b5ff50` | ECX raw slab, stack slab index; EAX same slab; RET4 | `[00b5ff50,00b5ff90)` |
| `00b605b0` | ECX initialized pool; EAX raw slot; RET | `[00b605b0,00b606ec)` |
| `00b606f0` | Incoming ECX allocation size ignored; select `108fe9c` and tail-call `B605B0` | `[00b606f0,00b606fa)` |
| `00b60260` | ECX raw slot; select `108fe9c`, call `B60110`; RET | `[00b60260,00b6026c)` |

The new wrapper APIs take the borrowed actual pool explicitly. The allocation
wrapper omits the ignored native size register. These are new C++ interfaces,
not binary replacements for the original entry points.

## Storage and ordering

A slab is 944h bytes: 32 slots of 48h bytes, 32 free-index WORDs at `+900h`, a
free-count WORD at `+940h`, and two trailing padding bytes. Each slot consists
of 44h owner bytes plus its slab-index DWORD at `+44h`. `B5FF50` writes count32
first, then each free index `31..0` followed by the corresponding slot index.
It preserves all owner payloads and the final padding bytes.

`B605B0` enters the real lock, then increments the current depth. If earliest
is `FFFFFFFFh`, it publishes the current slab count as the next index before
allocating the new slab. The slab initializer reads that current index after
allocation returns, including changes made by the real CRT new handler.

If the current table count equals its captured capacity, capacity becomes
`2*capacity+2` with DWORD wrap **before** the new table allocation. Byte size
also wraps as a DWORD. The copy walks forward, reloads the current count and
old table, and retains the native null-destination branch. It captures/frees
the old table before publishing the replacement. Ghidra's false return after
`BF6989` is excluded by the full bytes: `B60646` is `add esp,4`, followed by
replacement publication at `B60649`.

Publishing the new slab precedes incrementing the **current** slab count. The
allocator then loads the selected slab, decrements its free-count WORD, reloads
that word, and reads the selected free index. If the captured remaining count
is zero, it publishes earliest `FFFFFFFFh` and searches subsequent slabs using
the native captured table cursor and current count checks. It decrements the
current depth before leaving the lock. Raw DWORD/WORD memory operations and
wrapped address arithmetic preserve overlapping storage effects; there is no
count clamp or replacement vector.

## Allocation and failure boundary

The implementation uses the existing `singleton_lifetime_allocate/free`
service, with native and host sizes equal in Win32. `BF55BE` is a jump to
`BF681B`; the latter repeatedly tries malloc, invokes the CRT new handler after
failure, retries for a nonzero result, and otherwise throws bad_alloc.
The existing service preserves this allocation protocol through the host CRT;
the original game's CRT globals, new-handler identity, exception object, and
exception ABI remain explicit boundaries.

All four native bodies have no EH map. In particular, `B605B0` contains no
exception unlock, rollback, or release of an unpublished new slab. A table
allocation exception therefore retains the entered lock, incremented depth,
new earliest index, published capacity, old table, and unpublished slab. The
source deliberately contains no automatic lock guard or catch that would
change these effects.

The input pool and its real lock must already be initialized. Full allocator
list/global startup `CD7CA0 -> B604D0` and pool shutdown are separate work.
An initialized pool is borrowed, not fabricated by zeroing runtime fields.

## Validation

Eight complete original code spans, including the four owned functions,
existing `B60110`, allocator `BF681B`, and allocator/free thunks, matched live
Ghidra bytes to the installed binary in project `bsp`, program
`/battlestationspacific.exe`. Full assembly includes both allocator exits and
the omitted post-free stack adjustment/alignment bytes.

One ignored local Win32 fixture executes all four complete original functions
and the original `B60110`, then compares the new interfaces against them. Its
only component service bridges are the existing production CRT allocation/free
service and actual Win32 imports. No implementation callback or substitute lock
was introduced. The fixture covers:

- Full slab-byte equality, payload/padding preservation, and original return ABI.
- Table growth/free/publication, both subsequent-slab scan outcomes, and the two wrappers.
- A slab free-count WORD overlapping the pool depth DWORD, exposing current read/write order.
- A real `malloc(FFFFFFF8h)` failure with the actual CRT new handler. The handler
  sees capacity `3FFFFFFEh`, old count/earliest `1FFFFFFEh`, and depth1. Another
  thread cannot enter the real critical section during the handler or after
  bad_alloc reaches the caller; both implementations agree.

The forced failures intentionally leave one unpublished slab per run until
fixture process exit, matching the native failure path. They are not evidence
of successful cleanup.

The new source passes separate Release Win32 `/O2 /W4 /WX /fp:strict`
compilation. Primary integration registered it in `bsp_core` and reran the
unchanged fixture with all four providers from the actual primary library;
the slab/growth/alias/lock/failure results and eight loaded postimages passed.
The exact linked library is archived. `scripts/build.ps1` and both existing
CTests passed. All four names/evidence comments are saved in Ghidra, and the
free continuation at `B60646` is restored with prior comments preserved.
No tracked tests were added. Audit data, explicit CRT boundaries, saved
annotations, refreshed exports and source pins are in
`reports/native_hardware_layout_pool_allocate_audit.json`.
This proves the stated storage behavior within the borrowed input domain;
it does not establish original CRT ABI compatibility or game execution.
