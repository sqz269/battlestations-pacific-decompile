# Native hardware-layout pool startup: next packets

This read-only discovery establishes the concrete lifetime and virtual-trim
dependencies of native pool `108FE9C`. It proposes three disjoint source packets;
it does not implement or annotate them. Complete installed-PE and live-Ghidra
bytes match for 31 spans / 1,599 bytes, including all 555 candidate body bytes.
The companion report records every byte, hash, query and PE mapping boundary.

| Candidate | Complete span | Native ABI | Packet |
| --- | --- | --- | --- |
| `CD7CA0` | 22 bytes, through `CD7CB5` | no arguments; EAX is `_atexit` result; RET | static2 |
| `CE0D40` | 10 bytes, through `CE0D49` | no arguments; tailcalls destructor with ECX=`108FE9C` | static2 |
| `B604D0` | 211 bytes, through `B605A2` | ECX actual 38h pool, EAX same pool, RET | lifetime3 |
| `B60270` | 138 bytes, through `B602F9` | ECX actual pool, RET | lifetime3 |
| `B60020` | 14 bytes, through `B6002D` | ECX actual table header (`pool+28`), RET | lifetime3 |
| `B60350` | 160 bytes, through `B603EF` | ECX actual pool, RET | trim1 |

`CD7CA0` is missing as a Ghidra function; the enclosing lookup at `CD7B60`
does not identify this body's owner. `CE0D40` is currently called
`CG_static_init_00ce0d40`, although its complete body is a destructor thunk.
The other four candidates are named `FUN_<address>`. Descriptive source names
will remain hypotheses and the proposed C++ APIs will not reproduce the ABI.

## Construction and actual list participation

`B604D0` writes base profile `D7A0C0`, clears previous+04, copies the current
`E188B4` head to next+08, rereads the head for its previous-link update, then
publishes the actual pool at `E188B4`. State0 covers the base node. It installs
derived profile `D62AF0` and calls actual `InitializeCriticalSection(pool+0C)`.
After that returns it writes recursion depth+24=0, table+28=0, count+2C=0,
capacity+30=0 and earliest-free+34=`FFFFFFFF` before arming state2.

The unsigned capacity test grows to 32. Capacity is published before
`BF55BE(80h)`, the real throwing allocation path. Its captured result becomes
the replacement table. The copy loop rereads current count and current old
table, then frees the current old table if nonnull and publishes only the
captured replacement pointer. Native new-handler changes to current count or
capacity are not overwritten. Unused table words remain uninitialized. This
constructor allocates no slab.

The native profile `D62AF0` is exactly one slot, pointing at actual `B60350`;
`D62AF4` begins a different profile. Global trim `4B46B0` walks the actual
`E188B4` list, dispatches each current node's current slot0, then reloads that
node's next link. A real new-handler can reach this pool after list/profile
publication and before allocation returns. A source binding must therefore
bind actual trim before construction can publish the node. An inert binding
or a companion object for another pool does not satisfy this dependency.

Constructor handler `CC12DE`, FuncInfo `DFA058`, map `DFA040` describe the chain
state2 -> state1 -> state0 -> -1: `CC12D3` sends current `pool+28` to `B60020`,
`CC12C8` sends current `pool+0C` to existing recursive-section cleanup `402F70`,
and `CC12C0` sends the captured actual pool to base unlink `403970`.
`B60020` frees only the nonnull first word through `BF6989`; it clears nothing.
State0 failure during section initialization unlinks only the node. Win32
fault behavior is a host boundary, not permission to invent a C++ failure
callback or to treat an uninitialized section as initialized.

## Trim and destruction

`B60350` scans current table/count, freeing a slab only when its WORD at +940
equals 32. It then reloads current table and count, copies the last pointer to
the current index before decrementing count, and, if an entry moved, rewrites
its 32 slot-index tokens at +44 with stride 48h. Decrementing the loop index
rescans the moved slab. Removed trailing table words are left stale. After
removal it captures the current nonempty-count test, writes
earliest-free=`FFFFFFFF`, and, if that test was true, captures current table and
scans current count for the first slab whose WORD free count is nonzero. There is
no critical-section enter or leave in this routine.

`B60270` captures the initial count test before installing `D62AF0`. For each
reached slab it reloads current table/cell, calls actual `BF65AC`, increments
the index and compares against current count. It frees the nonnull current
table through `BF6989`. It then decrements positive signed depth+24 before each
actual `LeaveCriticalSection(pool+0C)`, reloading depth after each call, and
calls `DeleteCriticalSection`. Inline base teardown captures current previous,
installs `D7A0C0`, updates that predecessor or the actual global head, reloads
current next, and updates its previous link. Own links, table, count, capacity
and earliest-free are not cleared. Global pool storage is not freed; owners
within slab slots are not destroyed. There is no destructor EH frame.

## Static registration and service limits

The initializer table word at `CE353C` is `CD7CA0`. It lies within the C++
initializer interval `[CE2734, CE36E4)` iterated by CRT `__cinit` at `BFBC47`
after successful C initialization. `CD7CA0` sets ECX=`108FE9C`, calls `B604D0`,
then registers actual `CE0D40` through `_atexit` `BF6FF5`. The dispatcher ignores
the initializer return; registration failure has no pool rollback. The thunk
has no own EH frame. `CE0D40` sets ECX=`108FE9C` and jumps to `B60270`.

`_atexit` translates `__onexit`'s pointer result to 0 or -1. Wrapper `BF6FB9`
uses SEH4 and CRT lock8 around append `BF6ED1`, with finally `BF6FEF` unlocking.
The append decodes the current CRT begin/end slots, grows the actual buffer
using msize/realloc, and encodes the new callback/end. These are CRT service
boundaries, not a reconstructed private callback collector. Static2 should use
real host `std::atexit` with a real canonical shutdown binding; it must not
claim reconstruction of the CRT initializer dispatcher or CRT global state.

`BF55BE -> BF681B` provides actual malloc/new-handler retry and bad_alloc.
Existing `singleton_lifetime_allocate` represents that established host CRT
boundary. `BF6989 -> BF65AC -> BF9DC8` reaches CRT free, including its native
small-block-heap/CRT-heap paths; it is not GetProcessHeap allocation. The
existing corresponding free service uses actual host CRT free. Kernel32 IAT
cells `CE220C/10/14` are verified against PE imports for Initialize, Leave and
DeleteCriticalSection. File IAT words are name-table pointers, not runtime
resolved addresses.

Existing `AllocatorListDomain` borrows a shared head reference and provides
concrete profile/entry/context bindings, prepend, unlink and global trim.
Binding metadata can allocate and must be established before native list
publication. `402F70` semantics are established but currently exposed only as
private methods on other pool classes; lifetime3 may implement that proven raw
cleanup locally. It must not construct those foreign pool objects. Pool
storage is 38h bytes, but equal layout alone does not make the camera or
texture pool's sizes, profiles, or virtual operations correct here.

## Ownership and integration order

1. `native_hardware_layout_pool_trim1` owns only `B60350` and
   `include/bsp/native_hardware_layout_pool_trim.hpp`, matching `src/` file,
   `docs/NATIVE_HARDWARE_LAYOUT_POOL_TRIM.md`, and matching audit report. It
   exposes raw actual-pool trim and a genuine `D62AF0/B60350` list binding.
2. `native_hardware_layout_pool_lifetime3` owns only `B604D0`, `B60270`,
   `B60020`, and the corresponding four `native_hardware_layout_pool_lifetime`
   files. It borrows actual storage and `AllocatorListDomain`; callers must
   first establish the genuine trim binding. Source work is independent of
   trim1, but a production initialization composition requires both.
3. `native_hardware_layout_pool_static2` owns only `CD7CA0`, `CE0D40`, and the
   corresponding four `native_hardware_layout_pool_static` files. It depends
   on lifetime3 and trim1 plus one canonical actual global storage/list binding
   and real `std::atexit`. No reset or implicit private global is authorized.

The related tree global `108D530` has separate startup `CD7960` and shutdown
`CE0C50`; this discovery does not reconstruct or initialize it. Shared list,
section and CRT entries above are dependency evidence, not packet-owned
functions to recount. Shared CMake/ledger/packet/Ghidra changes remain with the
integrator.

## Saved-analysis repairs and evidence boundary

The bytes prove normal return continuations that current Ghidra flow omits:
`B60584 -> B60589..B6058B`, `B60288 -> B6028D..B60297`,
`B602A0 -> B602A5..B602A7`, `B60027 -> B6002C`, and
`B60371 -> B60376..B603AC`. Clear the incorrect call-site CALL_RETURN overrides,
decode the proven gaps, then refresh the affected exports under the write lock.
Also create the exact 22-byte `CD7CA0` body. Preserve prior comments and correct
CRT library names. No such mutations were made by this discovery.

The PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Each live read verified project `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. Image globals `E188B4`, `108FE9C` and CRT callback
slots include PE virtual zero fill: this proves initial image bytes, not live
game state or permission to reset actual globals. This packet is byte-audited
discovery, with no C++ change, build, differential execution, binary replacement
or game validation claim.
