# Graphics pool startup, trimming and shutdown

Addresses: 00B48480, 00B486A0, 00B4AB80, 00B4ADA0, 00B4AFC0,
00B47DF0, 00B47F70, 00B4A190, 00B4A310, 00B4A490,
00B47ED0, 00B48050, 00B4A270, 00B4A3F0, 00B4A570,
00CD7BE0, 00CD7C00, 00CD7C20, 00CD7C40, 00CD7C60, 00CD7C80,
00CE0CE0, 00CE0CF0, 00CE0D00, 00CE0D10, 00CE0D20, 00CE0D30.

Existing declaration and buffer factories require initialized native pools.
`NativeGraphicsPoolLifetime` supplies their recovered lifecycle over the same
38h storage layout already represented by `NativeMeshPoolStorage`. Each companion
binds its real trim implementation to the shared `AllocatorListDomain`; no second
allocator list, slab table, lock or slot ownership is introduced.

| Pool kind | Constructor | Destructor | Trim virtual zero | Vtable | Slot bytes / index offset / free-count offset |
| --- | --- | --- | --- | --- | --- |
| Vertex declarations | 00B48480 | 00B47DF0 | 00B47ED0 | 00D61D08 | D4 / D0 / 1AC0 |
| Hardware vertex layouts | 00B486A0 | 00B47F70 | 00B48050 | 00D61D0C | 44 / 40 / 8C0 |
| Physical buffers | 00B4AB80 | 00B4A190 | 00B4A270 | 00D61D60 | 30 / 2C / 640 |
| Logical vertex buffers | 00B4ADA0 | 00B4A310 | 00B4A3F0 | 00D61D64 | 78 / 74 / F40 |
| Logical index buffers | 00B4AFC0 | 00B4A490 | 00B4A570 | 00D61D68 | 28 / 24 / 540 |

All sizes and offsets in the table are hexadecimal. The original member ABI is
ECX = pool, RET with no stack arguments; constructors return this in EAX. These
are descriptive hypotheses and new C++ interfaces, not recovered symbols or
drop-in binary ABI replacements.

The constructor publishes the base allocator-list links and concrete vtable,
initializes the Win32 critical section at +0C, zeroes recursion +24 and the
table/count/capacity fields, and sets first-free +34 to FFFFFFFF. It stores capacity
32 before allocating the 128-byte pointer table. After allocation it reads the
live count and table again, copies existing pointers, frees the old table and
then publishes the replacement. Native FH3 metadata gives descending table,
critical-section and base-list cleanup; the source preserves that order with
MSVC finally cleanup. No automatic retry or duplicate-initialization guard is
inserted into the native lifecycle.

Trim frees a slab only when its 16-bit free count equals 32. It moves the last
table entry into the hole, decrements the count, rewrites all 32 moved trailing
slot indices, and examines the moved slab again. It then selects the first
nonzero free count, or retains FFFFFFFF. It neither enters the lock nor destroys
slot payloads. Shutdown frees each current slab and the table, decrements the
stored recursion before each real LeaveCriticalSection call, deletes the lock,
publishes the base vtable and unlinks the element. Old table/count/capacity fields
and this element's old links are retained, as in the original.

| Actual global | Kind | Static initializer | Shutdown thunk |
| --- | --- | --- | --- |
| 0108FD38 | Vertex declarations | 00CD7BE0 | 00CE0CE0 |
| 0108FD70 | Hardware layouts | 00CD7C00 | 00CE0CF0 |
| 0108FDA8 | Physical indices | 00CD7C20 | 00CE0D00 |
| 0108FDE0 | Physical vertices | 00CD7C40 | 00CE0D10 |
| 0108FE18 | Logical vertices | 00CD7C60 | 00CE0D20 |
| 0108FE50 | Logical indices | 00CD7C80 | 00CE0D30 |

The two physical pools require distinct storage despite sharing a kind. Bind
each actual global and keep its companion/list domain alive through explicit
shutdown. Each 22-byte initializer calls its constructor, registers the 10-byte
shutdown thunk and returns the CRT registration result. A failure result does
not undo construction. The adapters retain this behavior and require an actual
storage binding; host companion destruction does not trigger native shutdown.

The saved Ghidra repair record covers 20 false free-call fall-through gaps in
15 existing functions. Six previously undefined initializer bodies were defined
from verified 22-byte disk ranges. No callee no-return flags were changed.

Validation uses 27 original PE bodies whose bytes match the restarted live
Ghidra program. The ignored Win32 probe executes those bodies with 101 declared
operand relocations, real Win32 critical sections and the existing CRT allocation
boundary. Its registration adapter captures the actual callback and returns the
selected result. It compares all five profiles across five slab arrangements
and all six static lifecycles: 31 comparisons and 60 complete retained slab images
pass. Cases include repeated empty-slab removal, all-full/all-empty tables,
nonzero 16-bit free counts, middle/head/tail list removal, recursion depth two,
and registration failure without rollback. Allocation addresses and opaque OS
lock internals are compared through identity and operation checks.

The Win32 build, both existing CTests and eight seed byte checks pass. Evidence,
direct transfer rows and exact fixture hashes are in
`reports/native_graphics_pool_lifetime_af.json`; retained inputs are under
`local/graphics_pool_af/`. Native FH3 fault delivery and allocation-failure
reentry were not executed. No D3D9 renderer composition or full numbering caller
was run in this batch, and these checks do not establish game or rendering parity.

## Integrated compatibility validation

Commit `45db6dace12fc42f37d24094a2dee9c0d6a58bb4` passes the Win32 build and both existing CTests. Its preserved executable has SHA-256 `b53b15d1ee7d6d53d307dc5132b178e659e645ff5127ccb3247bd17c83e6a042`. The existing 120-frame USN01 check exits successfully with 18,557 finite trajectory rows, 241 unchanged Airfield2 samples, 2,400 avoidance queries, 10,080 generic ticks, 420 world-list nodes and the existing observer/pending-owner teardown checks. The lifecycle probe was rebuilt against this integrated library and its 31 comparisons pass. This mission check establishes combined-build compatibility; it does not exercise the new graphics pool bindings. The report records the immutable manifest of fixture, compiler, linked-object and mission artifacts. No workers were dispatched.

The separately published cache/reader changes were then merged. Combined commit `26efc484e20c97ec1327fe8da5718dfbefe7a05a` also passes the build, both CTests and the same 120-frame compatibility checks. Its executable SHA-256 is `540e400b2d4f4c8138e9ecaca9d8989846dcff6a67bbbc0ef80d97cd93d05bfc`. All three production objects linked into the lifecycle fixture remain byte-identical to the sealed proof. The additional integration manifest is retained separately; the original fixture archive remains unchanged.
