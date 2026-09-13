# Ship AI embedded searcher storage

Addresses: `009E4401..009E444F` (inline in `009E4330`); consumed allocation
chain `009F3F20`, `009F3BA0`, `009F39C0`, `009F1160`; list clear `004158A0`.

`ShipAiSearchStorage` owns an exact **2268h-byte allocation** and exposes the
three embedded **20h-byte searchers** through the existing cache/list types.
It preserves every allocator-provided bounds and padding byte. This is a
process storage adaptation: it does not construct the full native composite,
navigation block, director, or brain, and does not replace their other runtime
state. No original allocator-history or complete native object-lifetime parity
is claimed. Descriptive names are hypotheses.

| Scope | Coverage | Evidence |
| --- | --- | --- |
| `009E4401..009E444F` searcher stores | Complete stores in an inline fragment | Twelve stores; 79 original bytes include `OR EAX,-1` and `PUSH EDI` |
| `009E4330..009E46C2` navigation constructor | Partial: only the searcher stores above; all earlier/later work excluded | `ESI=ECX` at `009E434A`, `EBX=0` at `009E4351`, native `RET 4` at `009E46C0` |
| `009F3F20..009F3F7F` allocation wrapper | Consumed allocation-size evidence; full wrapper excluded | `PUSH 2268h` at `009F3F37`, allocator CALL at `009F3F3E`, `ADD ESP,4` at `009F3F43` |
| Process owner destruction/move | New ownership adaptation using existing complete list clear | Clear the three current heads in index order, then matching free of backing |

The allocation chain was established in `SHIP_AI_AVOIDANCE_OWNERS.md` and
rechecked against the current listings. The wrapper passes allocated storage
in ECX to `009F3BA0` at `009F3F59`. That constructor derives composite+58h for
`009F39C0` at `009F3BE5`; its `009F1160` call at `009F39E5` retains that receiver.
`009F1160` passes brain+8h to `009E4330` at `009F118D`. Thus nav=composite+60h,
and the searchers are composite+**A84h/AA4h/AC4h**. The earlier base constructor
stores only composite+00h..57h; there is no proven zero fill of these searchers.

| Record-relative bytes | Native stores | Owner behavior |
| --- | --- | --- |
| +00h | enabled=1 at `009E4417/4430/4449` | Same byte store |
| +01h..03h | None | Preserved padding |
| +04h/+08h/+0Ch/+10h | None | Preserved raw bounds; snapshot by byte copy into twelve `uint32_t` words before any FP access |
| +14h | key=-1 at `009E4411/442A/4443` | Same dword store |
| +18h/+1Ch | list head=0, list layer=0 at `009E4401/4407`, `441E/4424`, `4437/443D` | Same two dword stores, before key/enabled |

No placement/default construction or whole-record copy runs over the backing.
In particular, the existing `ShipAiAvoidZoneSearcher` defaults would zero the
four bounds, so those constructors are not invoked. The typed references are
an explicit **MSVC Win32 raw-object overlay**, not a portable C++ object-model
claim. Static assertions check the 18h cache prefix, 8h list, every used member
offset, 20h combined stride, 4-byte alignment and pointer width. Other allocation
bytes remain untouched and are not exposed as a constructed composite.

`cache(i)` and `list(i)` alias the same live record; `cache_views()` returns the
three actual cache pointers for pointer-based callers. Indexing requires a
nonempty owner and index less than three. `initial_bounds_bits()` is a persistent
snapshot of allocation inputs, not the current query bounds. Consumers must
not infer zero bounds from key=-1: the key-mismatch branch of `009D7050` reads
old width/height at `009D70F9..009D710B` before replacing them. The owner supplies
no synthetic zero, NaN, or finite initial box.

The owner copies `AvoidZoneAllocationAccess`, borrowing its context. It calls
`allocate_record_00bf681b(context,2268h)` once. The required existing contract is
aligned, nonnull storage or an exception, with a matching noexcept free service.
`00BF681B` retries malloc via the new-handler and throws on failure; the wrapper's
defensive null branch is not an alternate owner policy. Exceptions propagate
without fallback, recovery allocation or partially published owner. Native CRT
exception identity and SEH are outside this process API.

Every selected node published through a list view must share that same free
contract. The callback context must outlive the owner; borrowed geometry must
outlive the operations that read it. Lists must contain disjoint valid native
selected runs. The existing `004158A0` implementation follows `next` unless
`closes_run` is set or `next` is null, then follows `next_run`; this safely exits
the closing cyclic link. Its actual free call is `004158C3 -> 00BF65AC`, followed
by `ADD ESP,4` at `004158C8`. Destruction clears all three current heads and only
then frees backing. This is not a recovered full composite destructor.

Move construction transfers the allocation and callback table without relocating
records, so existing borrowed aliases keep their addresses and follow the new
owner's lifetime. Move assignment first clears/releases its old allocation,
invalidating aliases into that allocation. Self-move is harmless. A moved-from
owner has null `data()` and null cache pointers and performs no cleanup.

The ignored fixture executes only the original 79-byte initialization fragment:
ESI points at allocation+60h and EBX is zero; an appended `POP EDI; RET` balances
the native `PUSH EDI` at `009E4410`. There are no native calls, globals or address
relocations in that fragment. Disk and live-Ghidra bytes are checked equal.
Seeded allocator bytes include signaling/quiet NaNs, negative zero and subnormal
bounds. The full 2268h output, all preserved bytes and the raw snapshot are
compared; x87 status is checked unchanged across capture/stores. Those seeds are
explicit fixture inputs, not observations of the game's heap.

The probe passed: **8,808 bytes equal, 39 bytes changed, 8,769 bytes preserved**,
including all twelve bounds words. The optimized constructor disassembly also
confirms only the twelve intended backing stores. Snapshot copies use `MOVUPS`
as a bit transfer, with no arithmetic or NaN conversion. MSVC reorders the first
record's enabled store ahead of its other stores; this unpublished-storage
adapter does not claim native instruction/store timing or fault-order fidelity.

The same focused fixture uses finite initial bounds for three live queries over
explicit square polygons. Existing reconstructed query/selection routines create
the cyclic selected runs, and the borrowed lists feed the existing segment-hit
routine. It checks distinct keys, cache reuse, move construction, move assignment
between different allocator contexts, self-move, allocation exception propagation,
and clear-before-free ordering. This part executes reconstructed source, not
original query/cleanup bytes. Installed gameplay and original heap-history effects
are unvalidated. Counts, checks, artifact hashes and exact source dependencies
are recorded in `reports/ship_ai_search_storage.json` and the ignored manifest.
The source fixture passed four first queries, three cache reuses and three
borrowed-list hits; it created and freed 32 selected nodes in eight cyclic runs,
freed all three backing allocations, and ended with zero tracked allocations.
The forced allocation exception propagated after one request and no fallback.
Win32 Release and both existing CTest checks passed; no tracked tests were added.
