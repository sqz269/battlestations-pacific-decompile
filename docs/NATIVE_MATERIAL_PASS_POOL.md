# Actual material-pass pool at 0108FBF8

Addresses: 00B401D0, 00B402B0, 00B40980, 00B40A40, 00B41040,
00B41090, 00B41170, 00B41210, 00B41820, 00CD7BC0, 00CE0CD0.

The actual 38h pool at **0108FBF8** supplies 88h material-pass payloads in
8Ch slots. `NativeMaterialPassPool` extends the existing material/parameter
pool implementation with the recovered shape. It uses the same allocator-list
domain, heap domain and actual 24-byte Windows critical section. This is a
distinct pool; its list element, pointer table, slabs and recursion counter are
the fields of the supplied native storage. Descriptive names are hypotheses,
not recovered symbols. The new C++ interfaces are not binary replacements.

## Layout and original ABI

| Field | Actual offset / value |
| --- | --- |
| Allocator element: vtable / previous / next | 00 / 04 / 08 |
| Pool vtable and virtual slot 0 | D61A1C / B41170 |
| Critical section / signed recursion | 0C, 24 bytes / 24 |
| Slab table / unsigned count / capacity / first free | 28 / 2C / 30 / 34 |
| Slots per slab / slot stride / payload | 32 / 8Ch / 88h |
| Each slot's DWORD slab index | +88 |
| Slab size / 32 unsigned-short free indices / free count | 11C4h / 1180h / 11C0h |

The final two slab bytes are padding. Payload bytes remain uninitialized by
the allocator. The first-free sentinel is FFFFFFFF; the table starts at 32
pointers and grows to twice its old capacity plus two.

All spans below are complete, checked against the installed PE. End addresses
are exclusive. The table reports the original ABI, not the host C++ interface.

| Start / end | Reconstructed behavior | Original ABI |
| --- | --- | --- |
| B401D0 / B40213 | Initialize slab metadata only | ECX slab, stack index; EAX slab; RET 4 |
| B402B0 / B402BE | Free current nonnull pointer-table data | ECX 12-byte header at pool+28; RET |
| B41090 / B41163 | Construct actual pool | ECX pool; EAX pool; RET |
| B41210 / B4134C | Allocate raw slot | ECX pool; EAX slot; RET |
| B40A40 / B40AAC | Return raw slot | ECX pool, stack slot; RET 4 |
| B41170 / B41210 | Trim empty slabs | ECX pool; RET |
| B40980 / B40A0A | Destroy actual pool | ECX pool; RET |
| B41040 / B4104C | Return slot to fixed global 0108FBF8 | ECX slot; RET |
| B41820 / B4182A | Tail-call allocation on fixed global | No arguments; EAX slot; RET |
| CD7BC0 / CD7BD6 | Construct fixed global, register shutdown | No arguments; EAX CRT registration result; RET |
| CE0CD0 / CE0CDA | Tail-call destruction on fixed global | No arguments; RET |

## Recovered ordering and failure boundaries

Construction prepends the actual element to E188B4, publishes D7A0C0 then
D61A1C, initializes the critical section, clears recursion/table/count/capacity,
and sets first-free to -1. It publishes capacity 32 **before** allocating its
128-byte table. Replacement copies the current count of entries, frees the
old table and then publishes the new pointer.

The constructor's three-state unwind map at DF78E4, referenced by FuncInfo
DF78FC, calls CBEFE0 (00403970 list unlink), CBEFE8 (00402F70 section cleanup
at +0C), and CBEFF3 (B402B0 header cleanup at +28). CBEFFE is its handler.
These targets and metadata were independently checked from PE-matched spans.
Unwind does not reset published fields. B402B0 preserves pointer/count/capacity
after freeing the nonnull data pointer.

Allocation enters the real critical section and increments actual recursion.
It publishes a new first-free index before allocating and initializing a slab,
grows a full table with early capacity publication, and appends the slab.
It pops the last free index; an exhausted slab triggers a scan of later slabs.
There is no allocation unwind region: failure does not acquire an invented
rollback or automatic unlock. Successful exit decrements recursion and leaves
the section. Native allocation-failure execution remains untested.

Return acquires the lock before reading the live slot+88 slab ID and table
entry. Its signed division by 8Ch uses the EA0EA0EB multiplication/shift
sequence, with truncation toward zero. It appends the unsigned-short slot
index, increments the free count and lowers first-free as needed. Valid live
slots from this pool are required; the original has no extra validation.

Trim does not lock. It frees any slab with free count 32, copies the last
table entry to the removed index even when removing the last entry, decrements
the count, rewrites **all 32** moved slot IDs when an entry remains there, and
retries that index. It then recomputes first-free. The table and capacity stay
allocated. Callers must serialize trimming with pool access.

Shutdown requires already-destroyed payloads. It republishes D61A1C, frees
all slabs and the nonnull table, drains positive recursion with real
LeaveCriticalSection calls, deletes the section, publishes D7A0C0 and unlinks
from the shared list. Count/capacity/pointers and removed-element links remain
stale, as in the original.

The fixed-global C++ wrappers borrow one explicitly bound canonical companion.
The caller must keep the binding unchanged through shutdown and preserve its
storage/list lifetime. CD7BC0 returns the actual `atexit` result, without
rollback on registration failure or a private exit registry. The rebuilt
wrapper uses the real process `std::atexit` callback. B41820's earlier static
destructor label and CE0CD0's earlier static initializer label were incorrect;
their full instructions show allocation and destruction respectively.

## Validation and Ghidra evidence

The ignored focused native fixture executes all eleven complete original
functions with current heap and Windows API imports rebound. It compares eight
paired actual 56-byte pool snapshots plus live slab metadata, normalizing
pointer identities and the 24 bytes of OS critical-section internals. It
allocates 1,026 slots across 33 slabs, checks table growth 32 to 66, moved IDs,
payload preservation during trim, LIFO reuse, empty trim, stale shutdown
metadata, list membership and draining two levels of actual recursion.
A full 4,548-byte slab preimage comparison checks untouched payload and padding.
Direct header cleanup covers both nonnull and null data.

The original registration call is rebound to an observer returning 7; the
fixture verifies that result and invokes the captured original shutdown.
Separately, the rebuilt fixed-global wrappers register actual process-exit
cleanup, which is checked by a later-running verifier. The fixture exits 0.
The MSVC Win32 build and both existing CTests pass; no permanent tests were
added. This is instruction-fixture evidence, not original exception-delivery,
pass-payload construction, binary ABI, shader/draw or gameplay validation.

CD7BC0 was defined as a function. Six false no-return fall-through gaps across
B41090, B40980, B41170, B41210 and B402B0 were repaired under the write lock;
the internal call gaps are gone. Alignment padding was left intact. The eleven
names/comments preserve prior evidence, are read back, saved and force-exported.
`reports/native_material_pass_pool.json` pins the tested source and preserved
local captures, fixture, build products and annotation records by hash.

## Follow-up packets

The actual 88h pass constructor B44B10, destructor B454E0, binding destructor
B42250 and scalar deleting wrapper B46910 can now use this pool together with
the reconstructed 5Ch pass base. Full copy B455C0, secondary-pass construction
B45E00 and effect-loader orchestration B45EE0 remain separate work. Check
current leases before claiming them. No rendering or gameplay claim follows
from this allocator packet.
