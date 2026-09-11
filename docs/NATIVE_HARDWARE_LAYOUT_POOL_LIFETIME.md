# Native hardware-layout pool lifetime

This packet reconstructs three complete routines over actual borrowed native
pool storage and the existing shared allocator-list domain. It composes the
established CRT allocation/free service and real Win32 critical-section calls.

| Complete native body | Original ABI | New C++ function |
| --- | --- | --- |
| `B604D0..B605A2`, 211 bytes | ECX actual 38h pool; EAX same pool; RET | `initialize_native_hardware_layout_pool_00b604d0` |
| `B60270..B602F9`, 138 bytes | ECX actual initialized pool; RET | `destroy_native_hardware_layout_pool_00b60270` |
| `B60020..B6002D`, 14 bytes | ECX actual table header (`pool+28`); RET | `free_native_hardware_layout_pool_table_00b60020` |

These are new MSVC Win32 C++ interfaces, not original binary entry points.
Names are descriptive hypotheses. The interface borrows aligned 38h storage:
actual 12-byte allocator element at +00, actual 18h-byte critical section at
+0C, recursion depth at +24, table pointer at +28, count at +2C, capacity at
+30, and earliest-free index at +34. Reached pointers and table extents must be
valid. Initialization is once per uninitialized lifetime; destruction requires
the corresponding initialized pool.

## Publication, allocation and current fields

Before initialization, callers must establish the genuine
`D62AF0/B60350` binding through
`bind_native_hardware_layout_pool_trim_00d62af0(pool, list)`. The binding and
this packet borrow the same actual pool and shared list. Binding metadata may
allocate, so it is established before the native constructor publishes the
node. No foreign pool companion, placeholder trim callback, private allocator
head, or implicit global reset is used.

`B604D0` prepends the actual node through the existing proven list operation:
base profile `D7A0C0`, previous=null, next=current head, old head's previous=pool,
then shared head=pool. State0 is armed before derived profile `D62AF0` and
`InitializeCriticalSection(pool+0C)`. The source establishes the actual section
object's lifetime without initializing its bytes before the Win32 call.
After that returns, it writes depth/table/count/capacity=0 and
earliest-free=`FFFFFFFF`, then arms state2.

The unsigned capacity test grows to 32 and publishes that capacity before the
real `80h`-byte allocation. `singleton_lifetime_allocate` follows the established
host CRT malloc, `_callnewh`, retry and `bad_alloc` boundary corresponding to
`BF55BE -> BF681B`. The new-handler can already traverse this actual pool.
The captured successful replacement receives current table entries, with count
and old table reloaded for each reached copy. The current old table is freed,
then only the captured replacement pointer is published. Current count and
capacity changes made by actual new-handler processing remain observable.
Unused replacement words remain untouched. No slab is allocated by this body.

`B60020` captures only the actual header's first DWORD, frees it when nonnull,
and clears nothing. Passing the pool itself would address a different field
and is outside its contract. Actual free composes the existing host CRT service
corresponding to `BF6989 -> BF65AC -> BF9DC8`; native CRT heap state is not
replaced with a process-heap allocator.

## Unwind and teardown order

Native constructor handler `CC12DE`, FuncInfo `DFA058`, and unwind map `DFA040`
run state2 -> state1 -> state0 -> -1. `CC12D3` invokes actual `B60020` on the
captured pool's current table header, `CC12C8` invokes `402F70` on its actual
section, and `CC12C0` invokes actual base unlink `403970` on that same pool.
State0 alone applies while section initialization has not completed. The source
uses a Win32 finally around these states, preserving actual unwind without a
catch/rethrow edge. No artificial section-initialization failure callback is
introduced. Native table and list fields remain stale after cleanup.

The local section helper composes already established `402F70` behavior:
decrement positive signed recursion depth before each real LeaveCriticalSection,
reload depth for the next test, then DeleteCriticalSection. It does not claim
another reconstructed shared function or instantiate another pool class.

`B60270` captures the initial nonempty-count test before installing `D62AF0`.
Each reached iteration reloads the current table and slab pointer, performs
actual free, increments the index and compares against current count. It frees
the nonnull current table, drains/deletes the actual section, then uses existing
base unlink `403970`. Unlink captures current previous before writing `D7A0C0`,
updates that predecessor or the shared head, then reloads current next and
repairs its previous link. Own links, table/count/capacity/earliest fields are
not cleared. Destruction frees neither the raw pool storage nor each slot's
logical owner and leaves the caller-owned host virtual binding in place.

## Verification

The focused ignored fixture rechecks 12 live-Ghidra/installed-PE spans,
577 bytes including all 363 owned bytes, complete constructor FH3 code/map,
complete shared section/unlink bodies and explicit service boundaries. Each
live query verifies `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. The executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Execution rechecks those installed bytes and protects the private code and
maps. Independent postimage validation permits only 16 declared relocation
operands, four dependency jumps, and three actual Win32 import bindings; no
owned body is replaced. Profiles remain original native tokens. The complete
shared section/unlink bodies execute on the original side.

The fixture links the strict compiled production lifetime body and committed
trim implementation `dd67627`. The original allocation/free entries adapt into
the existing actual production CRT service. Five observed host imports still
perform real malloc/free and Initialize/Leave/DeleteCriticalSection; only the
selected first table malloc returns null. Actual CRT `_callnewh` invokes a real
registered new-handler. It can allocate actual slabs plus a capacity-seven
table, publish current fields, acquire the real section twice, and invoke
genuine shared-list trim. Snapshots cover both actual list nodes, section state,
all table and slab words, allocation/free state and retained native fields.
Pointer identities are normalized; OS-private section pointer words are
compared by nullness. Bytes are captured before actual free, never read from
freed memory.

Three complete-body comparisons match 52,094 DWORDs: successful retry with
current-table copy and destruction (30,443 words / 32 events); allocation
failure with a populated current table (21,281 / 25); and failure with the
empty current table (370 / 10). The two failing original calls each execute
one actual FH3 search and unwind. They verify table cleanup, positive recursion
drain, base-profile restoration, unchanged retained fields, and repair of the
existing list neighbor. The populated failing constructor leaves its surviving
slab allocated, as the native unwind frees the table only; fixture cleanup
reclaims it after observation ends.

All eight seeds match. The worker built the lifetime source through an ignored
CMake include; primary integration now registers it in shared `bsp_core`.
The strict primary build and both existing CTests pass. The unchanged fixture
was rerun against a frozen current primary library containing the actual pool,
trim, allocator-list and shared CRT service implementations. It again matches
all 52,094 DWORDs, including actual original FH3 search and unwind for both
failure cases. All 12 fresh live/PE spans and loaded postimages were checked;
the 10 worker artifact pins and current dependency sources match. The table
free leaf is emitted in the library and inlined at the compared host call
sites; its unused separate linked entry is discarded. No permanent test was
added. Ghidra annotations and refreshed exports include the restored normal
continuations after free sites `B60584`, `B60288`, `B602A0`, and `B60027`.
Static registration is a separate dependent packet.
Actual Win32 section-initialization fault injection, original CRT binary ABI,
global startup integration and game execution are not validated here.
