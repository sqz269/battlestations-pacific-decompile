# Native cube texture pool lifetime

The three complete functions below operate on the actual writable `38h` cube
texture pool and the existing shared allocator list. They allocate and free
through the real host CRT and initialize, leave, and delete the actual Win32
critical section at `pool+0C`. The descriptive names are reconstruction
hypotheses. The public C++ interfaces are new interfaces, not binary ABI
replacements or evidence of game execution.

| Original span, inclusive | Bytes | Original ABI | Reconstructed function |
| --- | ---: | --- | --- |
| `B3F090..B3F162` | 211 | ECX pool, EAX same pool, RET | `initialize_native_cube_texture_pool_00b3f090` |
| `B3E5B0..B3E639` | 138 | ECX initialized pool, RET | `destroy_native_cube_texture_pool_00b3e5b0` |
| `B3D3F0..B3D3FD` | 14 | ECX table header, RET | `free_native_cube_texture_pool_table_00b3d3f0` |

The third function receives `pool+28`, not the pool base. It reads the first
DWORD, frees that pointer if nonnull, and clears nothing. Native `B3D3FC` is
the returning call's `POP ECX`; it is absent from the current Ghidra listing.

## Actual storage and composition

| Offset | Meaning |
| --- | --- |
| `00/04/08` | Native `AllocatorListElement`: profile, previous, next |
| `0C..23` | Actual 24-byte Win32 `CRITICAL_SECTION` |
| `24` | Signed recursion depth |
| `28` | Current actual slab-pointer table |
| `2C` | Current unsigned slab count |
| `30` | Current unsigned table capacity |
| `34` | Earliest available slab index or `FFFFFFFF` |

The caller supplies sufficient aligned raw storage, starts the list element's
lifetime, and binds `bind_native_cube_texture_pool_trim_00d61944(pool, list)`
before initialization. That binding is the real cube `D61944/B3E690` pair in
the same `AllocatorListDomain` borrowing the actual shared head corresponding
to `E188B4`. Allocation can invoke a real CRT new-handler after the node has
been published. Initializing storage must not reset the shared list, create a
private companion pool, or substitute an unrelated allocator's callback.
The caller also owns binding removal and raw-storage release after destruction.

`singleton_lifetime_allocate({pointer_slots,128,128})` supplies the existing
real malloc/new-handler/retry/bad-allocation boundary corresponding to native
`BF55BE -> BF681B`. `singleton_lifetime_free` supplies the existing real free
boundary corresponding to `BF6989 -> BF65AC -> BF9DC8`. These are host CRT
boundaries; this packet does not reconstruct the original CRT's heap internals.

## Initialization and unwind

The constructor installs base profile `D7A0C0`, clears previous, captures the
current head into next, repairs the old head's previous link, and publishes
the node. It arms native unwind state0, installs cube profile `D61944`, and
initializes the section. It then clears depth, table, count and capacity,
writes earliest=`FFFFFFFF`, and arms state2. Capacity32 is published before
the actual 128-byte allocation.

On return from allocation, the constructor retains the replacement pointer,
copies the current count from the current old table, and reloads those fields
at each reached iteration. It frees the current old table and publishes the
captured replacement afterward. The native free continuation `B3F149` is
`ADD ESP,4`; `B3F14C` still publishes the replacement and `B3F154` returns the
actual pool. A real new-handler can change count, table, capacity, earliest,
and list state; the constructor does not reset those reached changes.

The exact FH3 handler `CBEE7E`, FuncInfo `DF76DC`, and map `DF76C4` establish
state2 -> state1 -> state0 -> -1:

1. `CBEE73` calls `B3D3F0` on captured `pool+28`, freeing its current table.
2. `CBEE68` calls `402F70` on `pool+0C`, draining positive recursion and deleting
   the section.
3. `CBEE60` calls `403970` on the actual pool, restoring the base profile and
   unlinking through current neighbor links.

State0 section-initialization failure unlinks only the base node. The source
uses MSVC `__try/__finally` and an explicit reached-state value to preserve
actual unwind, without a catch/rethrow edge or an invented rollback callback.
No section-initialization failure is artificially injected into Win32.

## Destruction

Native `B3E5B6` captures the initial nonempty-count condition before `B3E5B9`
installs `D61944`. The slab loop reloads the current table and count, and frees
every reached slab. The omitted eleven bytes at `B3E5CD` increment the index,
balance the stack, compare the current count, and loop. After the current
nonnull table is freed, `B3E5E5` balances the stack and reaches section cleanup.

Each positive recursion iteration decrements `pool+24` before the real
`LeaveCriticalSection`. The section is then deleted. Base unlinking restores
`D7A0C0`, updates the current previous node or shared head, and repairs the
current next node's previous link. Own links, table/count/capacity/earliest
remain retained. This does not destroy logical slot owners or free raw pool
storage. Destroying an initialized pool does not add a trim operation.

## Evidence and validation

The independent byte capture verifies `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe` before each live read. Twelve live Ghidra spans,
577 bytes total, match the installed PE, including all 363 owned bytes, the
40-byte FH3 code and 60-byte FH3 data. All eight existing native seed checks
match. The audit records native hashes, original names and signatures,
exact spans, repaired free continuations, and source hashes.

The ignored fixture reuses the established native-image and real-import
observation harness, with independently checked cube addresses, FH3 map, slab
size `6C4`, free count at `6C0`, and the actual cube trim implementation. It
links the owned lifetime code from the primary `bsp_core.lib`, with a private
CMake source registration, and compiles under strict MSVC Win32 flags. No
shared build file or permanent test is added by this worker.

The focused comparison covers a successful retry with a new-handler-modified
current table, allocation failure with a populated current table, and failure
with a null table. The actual CRT handler installs three genuine cube slabs
(free counts32/31/30), a 28-byte table with capacity7, and two real recursive
section entries. The bound cube trim removes the empty slab and compacts to
two slabs. Successful construction copies those current entries while
retaining capacity7; destruction frees both slabs and the replacement table.
Failure unwinds the original native FH3 map, freeing only the current table,
draining both section entries, and repairing the shared list. Unowned surviving
slabs are fixture-cleaned after the compared unwind.

Only declared absolute address relocations, four dependency-entry adapters,
and three real Win32 import bindings modify the private reference image. The
owned instruction bodies and relative calls remain intact. The actual original
FH3 map is reached through a registered entry and forwards to the host
`__CxxFrameHandler3`; the fixture observes both search and unwind. Allocation,
free, and section observers execute the actual services on both sides. Cube
trim is a shared genuine dependency here; its independent original-body proof
belongs to the trim packet.

The repository Release build and both existing tests passed. All three
native/source comparisons passed with 62,337 matching DWORDs (37,352 retry,
24,615 populated unwind, 370 empty unwind); all twelve postimages match the
declared edits. Detailed build, trace, postimage and artifact results are in
`reports/native_cube_texture_pool_lifetime_audit.json`. Field snapshots retain
all pool words and allocation bytes, normalizing actual pointer identities and
initialized section debug/semaphore pointer identities. Build and fixture
results do not establish binary replacement compatibility or game validation.
The cube static startup/shutdown pair, allocation routines, and logical cube
owner remain separate packets. Saved Ghidra annotations and shared ledgers are
integrator-owned.
